"""
Unreal Render MCP Server

专门用于Unreal Engine渲染操作的MCP服务器
包含材质、纹理、网格导入相关的21个工具

工具列表:
- 材质图构建 (1个): build_material_graph
- 材质分析 (3个): compile_material, get_material_graph, get_material_function_content
- 通用资产操作 (7个): create_asset, delete_asset, set_asset_properties, get_asset_properties,
  get_assets, batch_create_assets, batch_set_assets_properties
- 通用Actor操作 (8个): spawn_actor, delete_actor, get_actors, set_actor_properties, 
  get_actor_properties, batch_spawn_actors, batch_delete_actors, batch_set_actors_properties
- 纹理导入 (2个): import_texture, set_texture_properties
- 网格导入 (2个): import_fbx, create_static_mesh_from_data
- 视口截图 (1个): get_viewport_screenshot
"""

import logging
import socket
import json
import struct
import time
import threading
from contextlib import asynccontextmanager
from typing import AsyncIterator, Dict, Any, Optional, List
from mcp.server.fastmcp import FastMCP

# Configure logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s',
    handlers=[
        logging.FileHandler('unreal_render_mcp.log'),
    ]
)
logger = logging.getLogger("UnrealRenderMCP")

# Configuration
UNREAL_HOST = "127.0.0.1"
UNREAL_PORT = 55557


class UnrealConnection:
    """
    Robust connection to Unreal Engine with automatic retry and reconnection.
    """
    
    MAX_RETRIES = 3
    BASE_RETRY_DELAY = 0.5
    MAX_RETRY_DELAY = 5.0
    CONNECT_TIMEOUT = 10
    DEFAULT_RECV_TIMEOUT = 30
    LARGE_OP_RECV_TIMEOUT = 300
    BUFFER_SIZE = 8192
    
    LARGE_OPERATION_COMMANDS = {
        "get_available_materials",
    }
    
    def __init__(self):
        self.socket = None
        self.connected = False
        self._lock = threading.RLock()
        self._last_error = None
    
    def _create_socket(self) -> socket.socket:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(self.CONNECT_TIMEOUT)
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 131072)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 131072)
        
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('hh', 1, 0))
        except OSError:
            pass
        
        return sock
    
    def connect(self) -> bool:
        for attempt in range(self.MAX_RETRIES + 1):
            with self._lock:
                self._close_socket_unsafe()
                
                try:
                    logger.info(f"Connecting to Unreal at {UNREAL_HOST}:{UNREAL_PORT} (attempt {attempt + 1})...")
                    
                    self.socket = self._create_socket()
                    self.socket.connect((UNREAL_HOST, UNREAL_PORT))
                    self.connected = True
                    self._last_error = None
                    
                    logger.info("Successfully connected to Unreal Engine")
                    return True
                    
                except socket.timeout as e:
                    self._last_error = f"Connection timeout: {e}"
                    logger.warning(f"Connection timeout (attempt {attempt + 1})")
                except ConnectionRefusedError as e:
                    self._last_error = f"Connection refused: {e}"
                    logger.warning(f"Connection refused - is Unreal Engine running?")
                except OSError as e:
                    self._last_error = f"OS error: {e}"
                    logger.warning(f"OS error during connection: {e}")
                except Exception as e:
                    self._last_error = f"Unexpected error: {e}"
                    logger.error(f"Unexpected connection error: {e}")
                
                self._close_socket_unsafe()
                self.connected = False
            
            if attempt < self.MAX_RETRIES:
                delay = min(self.BASE_RETRY_DELAY * (2 ** attempt), self.MAX_RETRY_DELAY)
                logger.info(f"Retrying connection in {delay:.1f}s...")
                time.sleep(delay)
        
        logger.error(f"Failed to connect after {self.MAX_RETRIES + 1} attempts")
        return False
    
    def _close_socket_unsafe(self):
        if self.socket:
            try:
                self.socket.shutdown(socket.SHUT_RDWR)
            except:
                pass
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
        self.connected = False
    
    def disconnect(self):
        with self._lock:
            self._close_socket_unsafe()
            logger.debug("Disconnected from Unreal Engine")

    def _get_timeout_for_command(self, command_type: str) -> int:
        if any(large_cmd in command_type for large_cmd in self.LARGE_OPERATION_COMMANDS):
            return self.LARGE_OP_RECV_TIMEOUT
        return self.DEFAULT_RECV_TIMEOUT

    def _receive_response(self, command_type: str) -> bytes:
        timeout = self._get_timeout_for_command(command_type)
        self.socket.settimeout(timeout)
        
        chunks = []
        total_bytes = 0
        start_time = time.time()
        
        try:
            while True:
                elapsed = time.time() - start_time
                if elapsed > timeout:
                    raise socket.timeout(f"Overall timeout after {elapsed:.1f}s")
                
                try:
                    chunk = self.socket.recv(self.BUFFER_SIZE)
                except socket.timeout:
                    if chunks:
                        data = b''.join(chunks)
                        try:
                            json.loads(data.decode('utf-8'))
                            return data
                        except json.JSONDecodeError:
                            pass
                    raise
                
                if not chunk:
                    if not chunks:
                        raise ConnectionError("Connection closed before receiving any data")
                    break
                
                chunks.append(chunk)
                total_bytes += len(chunk)
                
                data = b''.join(chunks)
                try:
                    json.loads(data.decode('utf-8'))
                    return data
                except json.JSONDecodeError:
                    continue
                except UnicodeDecodeError:
                    continue
                    
        except socket.timeout:
            if chunks:
                data = b''.join(chunks)
                try:
                    json.loads(data.decode('utf-8'))
                    return data
                except:
                    pass
            raise TimeoutError(f"Timeout waiting for response")
        
        raise ConnectionError("Connection closed without response")

    def send_command(self, command: str, params: Dict[str, Any] = None) -> Optional[Dict[str, Any]]:
        last_error = None
        
        for attempt in range(self.MAX_RETRIES + 1):
            try:
                return self._send_command_once(command, params, attempt)
            except (ConnectionError, TimeoutError, socket.error, OSError) as e:
                last_error = str(e)
                logger.warning(f"Command failed (attempt {attempt + 1}): {e}")
                
                self.disconnect()
                
                if attempt < self.MAX_RETRIES:
                    delay = min(self.BASE_RETRY_DELAY * (2 ** attempt), self.MAX_RETRY_DELAY)
                    time.sleep(delay)
            except Exception as e:
                logger.error(f"Unexpected error sending command: {e}")
                self.disconnect()
                return {"status": "error", "error": str(e)}
        
        return {"status": "error", "error": f"Command failed after {self.MAX_RETRIES + 1} attempts: {last_error}"}

    def _send_command_once(self, command: str, params: Dict[str, Any], attempt: int) -> Dict[str, Any]:
        with self._lock:
            if not self.connect():
                raise ConnectionError(f"Failed to connect to Unreal Engine: {self._last_error}")
            
            try:
                command_obj = {
                    "type": command,
                    "params": params or {}
                }
                command_json = json.dumps(command_obj)
                
                logger.info(f"Sending command (attempt {attempt + 1}): {command}")
                
                self.socket.settimeout(10)
                self.socket.sendall(command_json.encode('utf-8'))
                
                response_data = self._receive_response(command)
                
                try:
                    response = json.loads(response_data.decode('utf-8'))
                except json.JSONDecodeError as e:
                    raise ValueError(f"Invalid JSON response: {e}")
                
                logger.info(f"Command {command} completed successfully")
                
                if response.get("status") == "error":
                    error_msg = response.get("error") or response.get("message", "Unknown error")
                    logger.warning(f"Unreal returned error: {error_msg}")
                elif response.get("success") is False:
                    error_msg = response.get("error") or response.get("message", "Unknown error")
                    response = {"status": "error", "error": error_msg}
                
                return response
                
            finally:
                self._close_socket_unsafe()


# Global connection instance
_unreal_connection: Optional[UnrealConnection] = None
_connection_lock = threading.Lock()


def get_unreal_connection() -> UnrealConnection:
    global _unreal_connection
    
    with _connection_lock:
        if _unreal_connection is None:
            logger.info("Creating new UnrealConnection instance")
            _unreal_connection = UnrealConnection()
        return _unreal_connection


def reset_unreal_connection():
    global _unreal_connection
    
    with _connection_lock:
        if _unreal_connection:
            _unreal_connection.disconnect()
            _unreal_connection = None
        logger.info("Unreal connection reset")


@asynccontextmanager
async def server_lifespan(server: FastMCP) -> AsyncIterator[Dict[str, Any]]:
    logger.info("Unreal Render MCP server starting up")
    logger.info("Connection will be established lazily on first tool call")

    try:
        yield {}
    finally:
        reset_unreal_connection()
        logger.info("Unreal Render MCP server shut down")


# Initialize server
mcp = FastMCP(
    "UnrealRenderMCP",
    lifespan=server_lifespan
)


# ============================================================================
# Material Graph Builder (Recommended)
# ============================================================================

@mcp.tool()
def build_material_graph(
    material_name: str,
    nodes: List[Dict[str, Any]],
    connections: List[Dict[str, Any]] = None,
    properties: Dict[str, Any] = None,
    compile: bool = True
) -> Dict[str, Any]:
    """
    Build an entire material graph in a single call (batch operation).
    
    This is the recommended way to create materials. It replaces multiple
    add_material_expression and connect_material_nodes calls with a single
    atomic operation.
    
    Args:
        material_name: Name or path of the material (must already exist)
        nodes: List of node definitions, each containing:
            - id: Unique node identifier (user-defined)
            - type: Expression type (Constant, Constant3Vector, Multiply, TextureSample, etc.)
            - pos_x, pos_y: Node position in graph (optional)
            - ... type-specific properties (value, texture, parameter_name, etc.)
        connections: List of connection definitions, each containing:
            - source: Source node ID
            - target: Target node ID (use "Material" for material properties)
            - source_output: Output pin name (default: "Output")
            - target_input: Input pin name or material property name
        properties: Optional material properties to set (shading_model, blend_mode, etc.)
        compile: Whether to compile the material after building (default: True)
    
    Returns:
        Dictionary with success status, node_count, connection_count
    
    Example:
        build_material_graph(
            material_name="M_MyMaterial",
            nodes=[
                {"id": "color", "type": "Constant3Vector", "pos_x": -300, "pos_y": 0, "value": [1.0, 0.0, 0.0]},
                {"id": "roughness", "type": "Constant", "pos_x": -300, "pos_y": 200, "value": 0.5}
            ],
            connections=[
                {"source": "color", "target": "Material", "target_input": "BaseColor"},
                {"source": "roughness", "target": "Material", "target_input": "Roughness"}
            ],
            properties={"shading_model": "DefaultLit"}
        )
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "material_name": material_name,
            "nodes": nodes,
            "compile": compile
        }
        if connections:
            params["connections"] = connections
        if properties:
            params["properties"] = properties
        
        response = unreal.send_command("build_material_graph", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"build_material_graph error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Material Analysis Tools (4 tools)
# ============================================================================

@mcp.tool()
def compile_material(material_name: str) -> Dict[str, Any]:
    """
    Compile a material to update its shader.
    
    Args:
        material_name: Name of the material to compile
    
    Returns:
        Dictionary with success status
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("compile_material", {"material_name": material_name})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"compile_material error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_material_graph(material_name: str) -> Dict[str, Any]:
    """
    Get complete material graph including nodes and connections.
    
    Merges get_material_expressions and get_material_connections into one call.
    
    Args:
        material_name: Name or path of the material
    
    Returns:
        Dictionary with:
        - nodes: List of material expression nodes
          - node_id: Unique identifier
          - type: Expression class name (e.g., "MaterialExpressionTextureSample")
          - position: {x, y} coordinates in graph
          - properties: Node-specific properties
        - connections: List of connections between nodes
          - source_node: Source node ID
          - target_node: Target node ID
          - source_output: Output pin name
          - target_input: Input pin name
        - property_connections: Dict mapping material properties to connected nodes
          - BaseColor, Metallic, Specular, Roughness, Normal, etc.
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_material_graph", {"material_name": material_name})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_material_graph error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_material_function_content(function_path: str) -> Dict[str, Any]:
    """
    Get detailed content of a Material Function including inputs, outputs, and expressions.
    
    Args:
        function_path: Full path to the Material Function (e.g., "/Engine/Functions/Engine_MaterialFunctions01/Texturing/BitMask.BitMask")
    
    Returns:
        Dictionary with function details including inputs, outputs, expressions
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_material_function_content", {"function_path": function_path})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_material_function_content error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Texture Tools (2 tools)
# ============================================================================

@mcp.tool()
def import_texture(
    source_path: str,
    name: str,
    destination_path: str = "/Game/Textures/",
    delete_source: bool = False,
    srgb: bool = None,
    compression_settings: str = None,
    filter: str = None,
    address_x: str = None,
    address_y: str = None
) -> Dict[str, Any]:
    """
    Import a texture file (DDS, PNG, TGA, etc.) into Unreal Engine.
    
    Args:
        source_path: Absolute path to the source texture file
        name: Name for the imported texture asset
        destination_path: Destination path in Unreal (default: /Game/Textures/)
        delete_source: If True, delete the source file after successful import
        srgb: Whether texture uses sRGB gamma (True for color textures, False for data textures)
        compression_settings: Compression format (Default, TC_NormalMap, TC_Grayscale, etc.)
        filter: Filter mode (Default, Nearest, Bilinear, Trilinear)
        address_x: Address mode for X (Wrap, Clamp, Mirror)
        address_y: Address mode for Y (Wrap, Clamp, Mirror)
    
    Returns:
        Dictionary with success status and imported texture path
    
    Common compression_settings values:
        - Default (BC7 for color)
        - TC_NormalMap (BC5 for normal maps)
        - TC_Grayscale (BC4 for single-channel)
        - TC_Displacementmap (for displacement/height maps)
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "source_path": source_path,
            "name": name,
            "destination_path": destination_path,
            "delete_source": delete_source
        }
        if srgb is not None:
            params["srgb"] = srgb
        if compression_settings is not None:
            params["compression_settings"] = compression_settings
        if filter is not None:
            params["filter"] = filter
        if address_x is not None:
            params["address_x"] = address_x
        if address_y is not None:
            params["address_y"] = address_y
        
        response = unreal.send_command("import_texture", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"import_texture error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_texture_properties(
    texture_path: str,
    compression_settings: str = None,
    srgb: bool = None,
    filter: str = None,
    address_x: str = None,
    address_y: str = None,
    mip_gen_settings: str = None,
    max_texture_size: int = None
) -> Dict[str, Any]:
    """
    Set texture properties.
    
    Args:
        texture_path: Path to the texture asset (e.g., "/Game/Textures/T_Normal")
        compression_settings: Compression format
        srgb: Whether texture uses sRGB gamma
        filter: Filter mode (Default, Nearest, Bilinear, Trilinear)
        address_x: Address mode for X (Wrap, Clamp, Mirror)
        address_y: Address mode for Y (Wrap, Clamp, Mirror)
        mip_gen_settings: Mipmap generation settings
        max_texture_size: Maximum texture size
    
    Returns:
        Dictionary with success status
    """
    unreal = get_unreal_connection()
    try:
        params = {"texture_path": texture_path}
        if compression_settings is not None:
            params["compression_settings"] = compression_settings
        if srgb is not None:
            params["srgb"] = srgb
        if filter is not None:
            params["filter"] = filter
        if address_x is not None:
            params["address_x"] = address_x
        if address_y is not None:
            params["address_y"] = address_y
        if mip_gen_settings is not None:
            params["mip_gen_settings"] = mip_gen_settings
        if max_texture_size is not None:
            params["max_texture_size"] = max_texture_size
        
        response = unreal.send_command("set_texture_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_texture_properties error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Mesh Tools (2 tools)
# ============================================================================

@mcp.tool()
def import_fbx(
    fbx_path: str,
    destination_path: str = "/Game/ImportedMeshes/",
    asset_name: str = None,
    spawn_in_level: bool = True,
    location: List[float] = None
) -> Dict[str, Any]:
    """
    Import an FBX file into Unreal Engine and optionally spawn it in the level.

    Args:
        fbx_path: Absolute path to the FBX file
        destination_path: Destination path in Unreal (default: /Game/ImportedMeshes/)
        asset_name: Name for the imported asset (default: filename without extension)
        spawn_in_level: Whether to spawn the mesh in the level (default: True)
        location: Spawn location [x, y, z] (default: [0, 0, 0])

    Returns:
        Dictionary with imported assets info and spawned actor details
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "fbx_path": fbx_path,
            "destination_path": destination_path,
            "spawn_in_level": spawn_in_level,
        }
        if asset_name is not None:
            params["asset_name"] = asset_name
        if location is not None:
            params["location"] = location

        response = unreal.send_command("import_fbx", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"import_fbx error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def create_static_mesh_from_data(
    name: str,
    positions: List[List[float]],
    indices: List[int],
    normals: List[List[float]] = None,
    uvs: List[List[float]] = None,
    tangents: List[List[float]] = None,
    colors: List[List[float]] = None,
    destination_path: str = "/Game/Meshes/",
    spawn_in_level: bool = True,
    location: List[float] = None
) -> Dict[str, Any]:
    """
    Create a static mesh directly from vertex data (bypasses FBX import).
    
    This is the preferred way to create meshes from RenderDoc captures,
    as it avoids FBX format issues and uses UE's native MeshDescription API.
    
    Args:
        name: Name for the new static mesh
        positions: List of vertex positions [[x,y,z], ...]
        indices: List of triangle indices (3 per triangle)
        normals: Optional list of vertex normals [[nx,ny,nz], ...]
        uvs: Optional list of UV coordinates [[u,v], ...]
        tangents: Optional list of tangents [[tx,ty,tz,tw], ...]
        colors: Optional list of vertex colors [[r,g,b,a], ...]
        destination_path: Destination path in Unreal (default: /Game/Meshes/)
        spawn_in_level: Whether to spawn the mesh in the level (default: True)
        location: Spawn location [x, y, z] (default: [0, 0, 0])
    
    Returns:
        Dictionary with mesh path, vertex/triangle counts, and spawned actor info
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "name": name,
            "positions": positions,
            "indices": indices,
            "destination_path": destination_path,
            "spawn_in_level": spawn_in_level
        }
        if normals is not None:
            params["normals"] = normals
        if uvs is not None:
            params["uvs"] = uvs
        if tangents is not None:
            params["tangents"] = tangents
        if colors is not None:
            params["colors"] = colors
        if location is not None:
            params["location"] = location
        
        response = unreal.send_command("create_static_mesh_from_data", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"create_static_mesh_from_data error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Viewport Tools (1 tool)
# ============================================================================

@mcp.tool()
def get_viewport_screenshot(
    output_path: str,
    format: str = "png",
    quality: int = 85,
    include_ui: bool = False
) -> Dict[str, Any]:
    """
    Capture a screenshot of the current viewport and save as image file.
    
    Args:
        output_path: Full path where to save the screenshot (e.g., "C:/temp/screenshot.png")
        format: Image format - "png", "jpg", or "bmp" (default: "png")
        quality: JPEG quality 1-100 (only for jpg, default: 85)
        include_ui: Whether to include editor UI (default: False)
    
    Returns:
        Dictionary with:
        - success: Whether the capture succeeded
        - file_path: Path to the saved image file
        - format: Image format
        - width: Image width in pixels
        - height: Image height in pixels
        - size_bytes: Size of the image data in bytes
        - viewport_type: Type of viewport captured ("Editor", "PIE", or "Game")
    
    Note:
        This captures the active viewport - either the editor viewport or PIE/Game viewport.
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "output_path": output_path,
            "format": format,
            "quality": quality,
            "include_ui": include_ui
        }
        response = unreal.send_command("get_viewport_screenshot", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_viewport_screenshot error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Generic Actor Management Tools (8 tools)
# ============================================================================

@mcp.tool()
def spawn_actor(
    actor_class: str,
    name: str = None,
    location: dict = None,
    rotation: dict = None,
    scale: dict = None,
    properties: dict = None
) -> Dict[str, Any]:
    """
    Spawn any actor by class name. Uses UE reflection for universal actor creation.
    
    This is the new generic actor spawning tool.
    Supports any actor type without code changes through UE's reflection system.
    
    Args:
        actor_class: Actor class name (e.g., "DirectionalLight", "PointLight", "Sphere", "StaticMeshActor", "PostProcessVolume")
                     Common types: DirectionalLight, PointLight, SpotLight, RectLight, 
                     StaticMeshActor, Sphere, Cube, Plane, Cylinder, PostProcessVolume
        name: Optional actor name
        location: Optional {"x": float, "y": float, "z": float}
        rotation: Optional {"pitch": float, "yaw": float, "roll": float}
        scale: Optional {"x": float, "y": float, "z": float}
        properties: Optional dict of properties to set (e.g., {"intensity": 10.0, "cast_shadows": True})
                   Property names match UE UPROPERTY names. Unmatched properties are ignored.
    
    Returns:
        Dictionary with success status, actor name, class, path, and transform
    
    Examples:
        # Create a directional light
        spawn_actor(
            actor_class="DirectionalLight",
            name="KeyLight",
            rotation={"pitch": -45, "yaw": 30, "roll": 0},
            properties={"intensity": 10.0, "cast_shadows": True}
        )
        
        # Create a material ball (sphere)
        spawn_actor(
            actor_class="Sphere",
            name="MaterialBall",
            location={"x": 0, "y": 0, "z": 100}
        )
        
        # Create a gray card (plane)
        spawn_actor(
            actor_class="Plane",
            name="GrayCard",
            location={"x": 200, "y": 0, "z": 0},
            scale={"x": 2, "y": 2, "z": 1}
        )
        
        # Create Post Process Volume for lookdev
        spawn_actor(
            actor_class="PostProcessVolume",
            name="Lookdev_PP",
            scale={"x": 2000, "y": 2000, "z": 2000}
        )
    """
    unreal = get_unreal_connection()
    try:
        params = {"actor_class": actor_class}
        if name:
            params["name"] = name
        if location:
            params["location"] = location
        if rotation:
            params["rotation"] = rotation
        if scale:
            params["scale"] = scale
        if properties:
            params["properties"] = properties
        response = unreal.send_command("spawn_actor", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"spawn_actor error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def delete_actor(name: str) -> Dict[str, Any]:
    """
    Delete an actor by name.
    
    Args:
        name: Name of the actor to delete
    
    Returns:
        Dictionary with success status and deleted actor name
    
    Example:
        delete_actor(name="KeyLight")
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name}
        response = unreal.send_command("delete_actor", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"delete_actor error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_actors(
    actor_class: str = None,
    detailed: bool = False
) -> Dict[str, Any]:
    """
    Get list of actors in the level with optional filtering.
    
    Args:
        actor_class: Optional class filter (e.g., "Light", "StaticMesh", "DirectionalLight")
        detailed: If True, includes all editable properties for each actor
    
    Returns:
        Dictionary with actor count and list of actors
    
    Examples:
        # Get all actors
        get_actors()
        
        # Get only lights
        get_actors(actor_class="Light")
        
        # Get detailed info for all static meshes
        get_actors(actor_class="StaticMesh", detailed=True)
    """
    unreal = get_unreal_connection()
    try:
        params = {"detailed": detailed}
        if actor_class:
            params["actor_class"] = actor_class
        response = unreal.send_command("get_actors", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_actors error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_actor_properties(
    name: str,
    properties: dict
) -> Dict[str, Any]:
    """
    Set properties on an actor using UE reflection.
    
    This is the new generic property setting tool. It uses UE's reflection system
    to match property names at runtime. Properties that match are modified,
    properties that don't match are ignored (no error).
    
    Args:
        name: Actor name
        properties: Dictionary of property names and values
                   Property names should match UE UPROPERTY names (case-insensitive)
                   
                   Common Light properties:
                   - "intensity": float
                   - "light_color": [R, G, B, A] or {"r": R, "g": G, "b": B, "a": A}
                   - "temperature": float (K)
                   - "cast_shadows": bool
                   - "attenuation_radius": float (Point/Spot lights)
                   - "outer_cone_angle": float (Spot lights)
                   
                   Transform shortcuts (directly on actor):
                   - "location": {"x": X, "y": Y, "z": Z} or [X, Y, Z]
                   - "rotation": {"pitch": P, "yaw": Y, "roll": R} or [P, Y, R]
                   - "scale": {"x": X, "y": Y, "z": Z} or [X, Y, Z]
    
    Returns:
        Dictionary with success status, modified count, and lists of modified/failed properties
    
    Examples:
        # Modify light intensity and color
        set_actor_properties(
            name="KeyLight",
            properties={
                "intensity": 15.0,
                "light_color": [1.0, 0.9, 0.8, 1.0],
                "temperature": 6500
            }
        )
        
        # Move actor to new location
        set_actor_properties(
            name="MaterialBall",
            properties={
                "location": {"x": 100, "y": 0, "z": 50}
            }
        )
        
        # Multiple unmatched properties (ignored silently)
        set_actor_properties(
            name="KeyLight",
            properties={
                "intensity": 10.0,     # Matches - will be set
                "foo_bar": 123         # No match - ignored
            }
        )
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "name": name,
            "properties": properties
        }
        response = unreal.send_command("set_actor_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_actor_properties error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_actor_properties(
    name: str,
    properties: list = None
) -> Dict[str, Any]:
    """
    Get properties of an actor using UE reflection.
    
    Args:
        name: Actor name
        properties: Optional list of specific property names to get.
                   If not provided, returns all editable UPROPERTIES.
    
    Returns:
        Dictionary with actor name, class, transform, and properties
    
    Examples:
        # Get all properties
        get_actor_properties(name="KeyLight")
        
        # Get specific properties only
        get_actor_properties(
            name="KeyLight",
            properties=["intensity", "light_color", "cast_shadows"]
        )
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name}
        if properties:
            params["properties"] = properties
        response = unreal.send_command("get_actor_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_actor_properties error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def batch_spawn_actors(items: List[Dict[str, Any]]) -> Dict[str, Any]:
    """
    Batch spawn multiple actors. 批量创建多个 Actor。
    
    Args:
        items: List of actor configurations, each containing:
            - actor_class: Actor class name (required)
            - name: Optional actor name
            - location: Optional {"x": float, "y": float, "z": float}
            - rotation: Optional {"pitch": float, "yaw": float, "roll": float}
            - scale: Optional {"x": float, "y": float, "z": float}
            - properties: Optional dict of properties to set
    
    Returns:
        Dictionary with results list and summary (success_count, fail_count)
    
    Example:
        batch_spawn_actors([
            {"actor_class": "DirectionalLight", "name": "KeyLight", "properties": {"intensity": 10}},
            {"actor_class": "Sphere", "name": "Ball1", "location": {"x": 0, "y": 0, "z": 100}},
            {"actor_class": "Sphere", "name": "Ball2", "location": {"x": 200, "y": 0, "z": 100}}
        ])
    """
    unreal = get_unreal_connection()
    try:
        params = {"items": items}
        response = unreal.send_command("batch_spawn_actors", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"batch_spawn_actors error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def batch_delete_actors(names: List[str]) -> Dict[str, Any]:
    """
    Batch delete multiple actors by name. 批量删除多个 Actor。
    
    Args:
        names: List of actor names to delete
    
    Returns:
        Dictionary with deleted list, failed list, and counts
    
    Example:
        batch_delete_actors(["Light1", "Light2", "Ball1"])
    """
    unreal = get_unreal_connection()
    try:
        params = {"names": names}
        response = unreal.send_command("batch_delete_actors", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"batch_delete_actors error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def batch_set_actors_properties(actors: List[Dict[str, Any]]) -> Dict[str, Any]:
    """
    Batch set properties on multiple actors. 批量修改多个 Actor 的属性。
    
    Args:
        actors: List of actor configurations, each containing:
            - name: Actor name (required)
            - properties: Dict of properties to set (required)
    
    Returns:
        Dictionary with results for each actor and summary
    
    Example:
        batch_set_actors_properties([
            {"name": "KeyLight", "properties": {"intensity": 15.0, "light_color": [1.0, 0.9, 0.8]}},
            {"name": "FillLight", "properties": {"intensity": 8.0}},
            {"name": "MaterialBall", "properties": {"location": {"x": 100, "y": 0, "z": 50}}}
        ])
    """
    unreal = get_unreal_connection()
    try:
        params = {"actors": actors}
        response = unreal.send_command("batch_set_actors_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"batch_set_actors_properties error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Generic Asset Management Tools (6 tools)
# ============================================================================

@mcp.tool()
def create_asset(
    asset_type: str,
    name: str,
    path: str = "/Game/",
    properties: dict = None
) -> Dict[str, Any]:
    """
    Create any asset by type. Uses UE reflection for universal asset creation.
    通过类型名称创建任何类型的资产。
    
    Args:
        asset_type: Asset type name (e.g., "Material", "MaterialInstance", "MaterialFunction", "Texture")
        name: Name for the new asset
        path: Destination path (default: /Game/)
        properties: Type-specific properties (e.g., {"parent_material": "/Game/M/M_Base"} for MaterialInstance)
    
    Returns:
        Dictionary with asset name, path, and creation status
    
    Examples:
        create_asset(asset_type="Material", name="M_Red", path="/Game/Materials/")
        create_asset(asset_type="MaterialInstance", name="MI_Red", properties={"parent_material": "/Game/M/M_Base"})
    """
    unreal = get_unreal_connection()
    try:
        params = {"asset_type": asset_type, "name": name, "path": path}
        if properties:
            params["properties"] = properties
        response = unreal.send_command("create_asset", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"create_asset error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def delete_asset(asset_path: str) -> Dict[str, Any]:
    """
    Delete an asset by path. 删除指定路径的资产。
    
    Args:
        asset_path: Full asset path (e.g., "/Game/Materials/M_Red.M_Red")
    
    Returns:
        Dictionary with success status
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("delete_asset", {"asset_path": asset_path})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"delete_asset error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_asset_properties(asset_path: str, properties: dict) -> Dict[str, Any]:
    """
    Set properties on any asset using UE reflection.
    通过反射设置任何资产的属性。
    
    Args:
        asset_path: Full asset path (e.g., "/Game/Materials/M_Red.M_Red")
        properties: Dictionary of property names and values
    
    Returns:
        Dictionary with success status and modified properties
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("set_asset_properties", {"asset_path": asset_path, "properties": properties})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_asset_properties error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_asset_properties(asset_path: str, properties: list = None) -> Dict[str, Any]:
    """
    Get properties of any asset using UE reflection.
    通过反射获取任何资产的属性。
    
    Args:
        asset_path: Full asset path
        properties: Optional list of specific property names to get
    
    Returns:
        Dictionary with asset properties
    """
    unreal = get_unreal_connection()
    try:
        params = {"asset_path": asset_path}
        if properties:
            params["properties"] = properties
        response = unreal.send_command("get_asset_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_asset_properties error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_assets(
    path: str = "/Game/",
    asset_class: str = None,
    name_filter: str = None
) -> Dict[str, Any]:
    """
    Get list of assets in the project. Replaces get_available_materials and get_material_functions.
    获取项目中的资产列表。替代 get_available_materials 和 get_material_functions。
    
    Args:
        path: Search path (default: /Game/)
        asset_class: Asset class filter (e.g., "Material", "MaterialFunction", "Texture", "StaticMesh")
                     If not specified, searches for all asset types
        name_filter: Optional filter string for asset names (partial match)
    
    Returns:
        Dictionary with:
        - assets: List of asset info
          - name: Asset name
          - path: Full asset path
          - class: Asset class name
        - count: Total number of assets found
    
    Examples:
        # Get all materials (replaces get_available_materials)
        get_assets("/Game/", "Material")
        
        # Get material functions (replaces get_material_functions)
        get_assets("/Engine/Functions/", "MaterialFunction")
        
        # Get all textures with filter
        get_assets("/Game/Textures/", "Texture", name_filter="T_")
        
        # Get all assets in a folder
        get_assets("/Game/Meshes/")
    """
    unreal = get_unreal_connection()
    try:
        params = {"path": path}
        if asset_class:
            params["asset_class"] = asset_class
        if name_filter:
            params["name_filter"] = name_filter
        response = unreal.send_command("get_assets", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_assets error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def batch_create_assets(items: List[Dict[str, Any]]) -> Dict[str, Any]:
    """
    Batch create multiple assets. 批量创建多个资产。
    
    Args:
        items: List of asset configurations
    
    Returns:
        Dictionary with results list and summary
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("batch_create_assets", {"items": items})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"batch_create_assets error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def batch_set_assets_properties(items: List[Dict[str, Any]]) -> Dict[str, Any]:
    """
    Batch set properties on multiple assets. 批量设置多个资产的属性。
    
    Args:
        items: List of asset configurations with asset_path and properties
    
    Returns:
        Dictionary with results for each asset and summary
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("batch_set_assets_properties", {"items": items})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"batch_set_assets_properties error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Main Entry Point
# ============================================================================

if __name__ == "__main__":
    mcp.run()
