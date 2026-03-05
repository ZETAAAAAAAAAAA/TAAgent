"""
Unreal Render MCP Server

专门用于Unreal Engine渲染操作的MCP服务器
包含材质、纹理、网格导入相关的15个工具

工具列表:
- 材质管理 (10个): create_material, create_material_instance, create_material_function,
  set_material_instance_parameter, set_material_properties, compile_material,
  get_material_expressions, get_material_functions, get_material_function_content,
  get_available_materials
- 材质节点 (2个): add_material_expression, connect_material_nodes
- 纹理导入 (2个): import_texture, set_texture_properties
- 网格导入 (1个): import_fbx
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
# Material Creation Tools (10 tools)
# ============================================================================

@mcp.tool()
def create_material(name: str) -> Dict[str, Any]:
    """
    Create a new empty material asset.
    
    Args:
        name: Name for the new material
    
    Returns:
        Dictionary with material name, path, and creation status
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("create_material", {"name": name})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"create_material error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def create_material_instance(
    name: str,
    parent_material: str,
    destination_path: str = "/Game/Materials/Instances/"
) -> Dict[str, Any]:
    """
    Create a new material instance asset.
    
    Args:
        name: Name for the new material instance
        parent_material: Path to the parent material (e.g., "/Game/Materials/M_Base")
        destination_path: Path where to create the material instance (default: /Game/Materials/Instances/)
    
    Returns:
        Dictionary with material instance name, path, and creation status
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "name": name,
            "parent_material": parent_material,
            "destination_path": destination_path
        }
        response = unreal.send_command("create_material_instance", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"create_material_instance error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def create_material_function(
    name: str,
    path: str = None,
    description: str = None
) -> Dict[str, Any]:
    """
    Create a new material function asset.
    
    Args:
        name: Name for the new material function
        path: Optional path (default: /Game/MaterialFunctions/)
        description: Optional description for the function
    
    Returns:
        Dictionary with function name, path, and creation status
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name}
        if path:
            params["path"] = path
        if description:
            params["description"] = description
        response = unreal.send_command("create_material_function", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"create_material_function error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_material_instance_parameter(
    material_instance: str,
    parameter_name: str,
    parameter_type: str,
    value: Any
) -> Dict[str, Any]:
    """
    Set a parameter in a material instance.
    
    Args:
        material_instance: Path to the material instance (e.g., "/Game/Materials/MI_Base.MI_Base")
        parameter_name: Name of the parameter
        parameter_type: Type of parameter - "scalar", "vector", or "texture"
        value: Parameter value (float for scalar, dict for vector, string path for texture)
    
    Returns:
        Dictionary with success status
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "material_instance": material_instance,
            "parameter_name": parameter_name,
            "parameter_type": parameter_type,
            "value": value
        }
        response = unreal.send_command("set_material_instance_parameter", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_material_instance_parameter error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_material_properties(
    material_name: str,
    shading_model: str = None,
    blend_mode: str = None,
    two_sided: bool = None,
    material_domain: str = None
) -> Dict[str, Any]:
    """
    Set material properties.
    
    Args:
        material_name: Name or path of the material
        shading_model: Unlit, DefaultLit, Subsurface, TwoSidedFoliage
        blend_mode: Opaque, Masked, Translucent, Additive
        two_sided: Whether material is two-sided
        material_domain: Surface, DeferredDecal, LightFunction, Volume, PostProcess, UserInterface
    
    Returns:
        Dictionary with success status
    """
    unreal = get_unreal_connection()
    try:
        params = {"material_name": material_name}
        if shading_model:
            params["shading_model"] = shading_model
        if blend_mode:
            params["blend_mode"] = blend_mode
        if two_sided is not None:
            params["two_sided"] = two_sided
        if material_domain:
            params["material_domain"] = material_domain
        response = unreal.send_command("set_material_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_material_properties error: {e}")
        return {"success": False, "message": str(e)}


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
def get_material_expressions(material_name: str) -> Dict[str, Any]:
    """
    Get list of all expressions in a material.
    
    Args:
        material_name: Name or path of the material
    
    Returns:
        Dictionary with list of material expressions
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_material_expressions", {"material_name": material_name})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_material_expressions error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_material_functions(
    path: str = "/Engine/Functions/",
    name_filter: str = None
) -> Dict[str, Any]:
    """
    Get list of Material Functions in a specified path.
    
    Args:
        path: Search path (default: /Engine/Functions/)
        name_filter: Optional filter string for function names
    
    Returns:
        Dictionary with list of material functions and their paths
    """
    unreal = get_unreal_connection()
    try:
        params = {"path": path}
        if name_filter:
            params["name_filter"] = name_filter
        response = unreal.send_command("get_material_functions", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_material_functions error: {e}")
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


@mcp.tool()
def get_material_properties(material_name: str) -> Dict[str, Any]:
    """
    Get material properties including BlendMode, ShadingModel, TwoSided, MaterialDomain, etc.
    
    Args:
        material_name: Name or path of the material
    
    Returns:
        Dictionary with material properties:
        - blend_mode: Opaque, Masked, Translucent, Additive, Modulate, AlphaComposite
        - shading_models: List of enabled shading models
        - two_sided: Whether material is two-sided
        - material_domain: Surface, DeferredDecal, LightFunction, Volume, PostProcess, UserInterface
        - is_masked: Whether material uses masking
        - opacity_mask_clip_value: Clip value for masked materials
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_material_properties", {"material_name": material_name})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_material_properties error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_material_connections(material_name: str) -> Dict[str, Any]:
    """
    Get node connection relationships in a material.
    
    Args:
        material_name: Name or path of the material
    
    Returns:
        Dictionary with:
        - node_connections: List of nodes and their input connections
          - node_id: Unique identifier for the node
          - inputs: Dict mapping input names to connected nodes
            - connected_node: node_id of the source node
            - output_index: Which output pin of the source node
            - output_name: Name of the output pin
        - property_connections: Dict mapping material properties to connected nodes
          - BaseColor, Metallic, Specular, Roughness, Normal, etc.
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_material_connections", {"material_name": material_name})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_material_connections error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_available_materials(search_path: str = "/Game/") -> Dict[str, Any]:
    """
    Get a list of available materials in the project that can be applied to objects.
    
    Args:
        search_path: Path to search for materials (default: /Game/)
    
    Returns:
        Dictionary with list of available materials
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_available_materials", {"search_path": search_path})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_available_materials error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Material Node Tools (2 tools)
# ============================================================================

@mcp.tool()
def add_material_expression(
    material_name: str,
    expression_type: str,
    pos_x: int = 0,
    pos_y: int = 0,
    value: List[float] = None,
    texture: str = None,
    mask: List[bool] = None,
    code: str = None,
    description: str = None,
    output_type: str = None,
    inputs: List[str] = None,
    parameter_name: str = None,
    group: str = None,
    sampler_type: str = None
) -> Dict[str, Any]:
    """
    Add a material expression node to a material, or update an existing node.
    
    Args:
        material_name: Name or path of the material
        expression_type: Type of expression (Constant, Constant3Vector, Multiply, Add, TextureSample, Custom, etc.)
        pos_x: X position in material graph
        pos_y: Y position in material graph
        value: Value for constant/parameter expressions: [value] or [r, g, b, a]
        texture: Texture path for TextureSample/TextureParameter expressions
        mask: Channel mask for ComponentMask [R, G, B, A]
        code: HLSL code for Custom expression
        description: Description/name for Custom expression node
        output_type: Output type for Custom expression: "Float1", "Float2", "Float3", "Float4"
        inputs: Input pin names for Custom expression (e.g., ["Input1", "Input2"])
        parameter_name: Name for parameter nodes (exposed in material instance)
        group: Group name for organizing parameters in material instance editor
        sampler_type: Sampler type for TextureSampleParameter2D: "Color", "Normal", "Grayscale", "Alpha", "LinearColor"
    
    Returns:
        Dictionary with success status, node_id, and position
    
    Supported expression types:
        Basic Math: Constant, Constant2Vector, Constant3Vector, Constant4Vector,
                   Multiply, Add, Subtract, Divide, Lerp, Clamp, Power, Abs,
                   Floor, Ceil, Frac, Sine, Cosine, DotProduct, CrossProduct,
                   OneMinus, Normalize, Saturate, SquareRoot, Min, Max, Round, Sign
        
        Texture: TextureSample, TextureCoordinate, TextureObject, Panner, Rotator
        
        Parameters: ScalarParameter, VectorParameter, TextureObjectParameter,
                   TextureSampleParameter2D, StaticBoolParameter
        
        Utility: Time, ComponentMask, AppendVector, Reroute,
                WorldPosition, ObjectPosition, VertexNormal, VertexTangent,
                Fresnel, DepthFade, CameraPosition, CameraVector,
                PixelDepth, SceneDepth, ReflectionVector, LightVector,
                Distance, Desaturation, If, StaticSwitch, Custom
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "material_name": material_name,
            "expression_type": expression_type,
            "pos_x": pos_x,
            "pos_y": pos_y
        }
        if value is not None:
            params["value"] = value
        if texture is not None:
            params["texture"] = texture
        if mask is not None:
            params["mask"] = mask
        if code is not None:
            params["code"] = code
        if description is not None:
            params["description"] = description
        if output_type is not None:
            params["output_type"] = output_type
        if inputs is not None:
            params["inputs"] = inputs
        if parameter_name is not None:
            params["parameter_name"] = parameter_name
        if group is not None:
            params["group"] = group
        if sampler_type is not None:
            params["sampler_type"] = sampler_type
        
        response = unreal.send_command("add_material_expression", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"add_material_expression error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def connect_material_nodes(
    material_name: str,
    source_node: str,
    source_output: str = "Output",
    target_node: str = None,
    target_input: str = None
) -> Dict[str, Any]:
    """
    Connect material expression nodes or connect to material property.
    
    Args:
        material_name: Name or path of the material
        source_node: Source expression node ID
        source_output: Output name on source node (usually "Output")
        target_node: Target expression node ID (use "Material" for material property)
        target_input: Input name on target node or material property name
    
    Material properties: BaseColor, Metallic, Specular, Roughness, Normal,
    EmissiveColor, Opacity, OpacityMask, WorldPositionOffset, AmbientOcclusion
    
    Returns:
        Dictionary with success status and connection details
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "material_name": material_name,
            "source_node": source_node,
            "source_output": source_output,
            "target_node": target_node,
            "target_input": target_input
        }
        response = unreal.send_command("connect_material_nodes", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"connect_material_nodes error: {e}")
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
# Light Tools (4 tools)
# ============================================================================

@mcp.tool()
def create_light(
    light_type: str = "point",
    name: str = None,
    location: List[float] = None,
    rotation: List[float] = None,
    intensity: float = None,
    color: List[float] = None,
    mobility: str = "movable",
    cast_shadows: bool = True,
    attenuation_radius: float = None,
    inner_cone_angle: float = None,
    outer_cone_angle: float = None,
    source_radius: float = None
) -> Dict[str, Any]:
    """
    Create a light in the scene.
    
    Args:
        light_type: Type of light - "point", "directional", "spot", "rect" (default: "point")
        name: Name for the light (auto-generated if not provided)
        location: World position [x, y, z] (default: [0, 0, 200])
        rotation: World rotation [pitch, yaw, roll] in degrees (default: [0, 0, 0])
        intensity: Light intensity/brightness
        color: Light color [r, g, b] or [r, g, b, a] (0.0-1.0)
        mobility: "movable", "stationary", or "static" (default: "movable")
        cast_shadows: Whether light casts shadows (default: True)
        attenuation_radius: Attenuation radius for point/spot lights
        inner_cone_angle: Inner cone angle for spot lights (degrees)
        outer_cone_angle: Outer cone angle for spot lights (degrees)
        source_radius: Source radius for point/spot lights (soft shadows)
    
    Returns:
        Dictionary with light name, type, and actor path
    """
    unreal = get_unreal_connection()
    try:
        params = {"light_type": light_type}
        if name:
            params["name"] = name
        if location:
            params["location"] = location
        if rotation:
            params["rotation"] = rotation
        if intensity is not None:
            params["intensity"] = intensity
        if color:
            params["color"] = color
        params["mobility"] = mobility
        params["cast_shadows"] = cast_shadows
        if attenuation_radius is not None:
            params["attenuation_radius"] = attenuation_radius
        if inner_cone_angle is not None:
            params["inner_cone_angle"] = inner_cone_angle
        if outer_cone_angle is not None:
            params["outer_cone_angle"] = outer_cone_angle
        if source_radius is not None:
            params["source_radius"] = source_radius
        
        response = unreal.send_command("create_light", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"create_light error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_light_properties(
    name: str,
    intensity: float = None,
    color: List[float] = None,
    temperature: float = None,
    use_temperature: bool = None,
    cast_shadows: bool = None,
    affects_world: bool = None,
    location: List[float] = None,
    rotation: List[float] = None,
    attenuation_radius: float = None,
    inner_cone_angle: float = None,
    outer_cone_angle: float = None,
    source_radius: float = None,
    source_length: float = None,
    shadow_distance: float = None
) -> Dict[str, Any]:
    """
    Set properties of an existing light.
    
    Args:
        name: Name or path of the light
        intensity: Light intensity/brightness
        color: Light color [r, g, b] or [r, g, b, a] (0.0-1.0)
        temperature: Color temperature in Kelvin (1700-12000)
        use_temperature: Whether to use temperature instead of color
        cast_shadows: Whether light casts shadows
        affects_world: Whether light affects the world
        location: World position [x, y, z]
        rotation: World rotation [pitch, yaw, roll] in degrees
        attenuation_radius: Attenuation radius for point/spot lights
        inner_cone_angle: Inner cone angle for spot lights (degrees)
        outer_cone_angle: Outer cone angle for spot lights (degrees)
        source_radius: Source radius for soft shadows
        source_length: Source length for point lights
        shadow_distance: Shadow distance for directional lights
    
    Returns:
        Dictionary with success status and modification info
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name}
        if intensity is not None:
            params["intensity"] = intensity
        if color:
            params["color"] = color
        if temperature is not None:
            params["temperature"] = temperature
        if use_temperature is not None:
            params["use_temperature"] = use_temperature
        if cast_shadows is not None:
            params["cast_shadows"] = cast_shadows
        if affects_world is not None:
            params["affects_world"] = affects_world
        if location:
            params["location"] = location
        if rotation:
            params["rotation"] = rotation
        if attenuation_radius is not None:
            params["attenuation_radius"] = attenuation_radius
        if inner_cone_angle is not None:
            params["inner_cone_angle"] = inner_cone_angle
        if outer_cone_angle is not None:
            params["outer_cone_angle"] = outer_cone_angle
        if source_radius is not None:
            params["source_radius"] = source_radius
        if source_length is not None:
            params["source_length"] = source_length
        if shadow_distance is not None:
            params["shadow_distance"] = shadow_distance
        
        response = unreal.send_command("set_light_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_light_properties error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_lights(light_type: str = None) -> Dict[str, Any]:
    """
    Get list of all lights in the current level.
    
    Args:
        light_type: Optional filter by type - "point", "directional", "spot", "rect"
    
    Returns:
        Dictionary with list of lights, each containing:
        - name: Light actor name
        - label: Actor label
        - light_type: Type of light
        - path: Full actor path
        - location: World position [x, y, z]
        - rotation: World rotation [pitch, yaw, roll]
        - intensity: Light intensity
        - color: Light color [r, g, b, a]
        - cast_shadows: Whether casts shadows
        - affects_world: Whether affects world
        - mobility: Component mobility type
    """
    unreal = get_unreal_connection()
    try:
        params = {}
        if light_type:
            params["light_type"] = light_type
        response = unreal.send_command("get_lights", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_lights error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def delete_light(name: str) -> Dict[str, Any]:
    """
    Delete a light from the scene.
    
    Args:
        name: Name or path of the light to delete
    
    Returns:
        Dictionary with success status and deleted light name
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("delete_light", {"name": name})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"delete_light error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Post Process Tools (2 tools)
# ============================================================================

@mcp.tool()
def create_post_process_volume(
    name: str = "PostProcessVolume_Lookdev",
    location: Dict[str, float] = None,
    scale: Dict[str, float] = None
) -> Dict[str, Any]:
    """
    Create a Post Process Volume for lookdev environment setup.
    
    Creates a volume with default Lookdev settings:
    - Manual exposure mode (EV100=0)
    - Bloom disabled
    - Vignette disabled
    - Ambient Occlusion disabled
    
    Args:
        name: Name for the volume (default: "PostProcessVolume_Lookdev")
        location: World position {"x": 0, "y": 0, "z": 0} (default: origin)
        scale: Volume scale {"x": 1000, "y": 1000, "z": 1000} (default: large unbound)
    
    Returns:
        Dictionary with success status, volume name, location, and scale
    
    Example:
        create_post_process_volume(name="Lookdev_PP", location={"x": 0, "y": 0, "z": 0})
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name}
        if location:
            params["location"] = location
        if scale:
            params["scale"] = scale
        response = unreal.send_command("create_post_process_volume", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"create_post_process_volume error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_post_process_settings(
    name: str = None,
    exposure_mode: str = None,
    exposure_bias: float = None,
    exposure_value: float = None,
    bloom_enabled: bool = None,
    vignette_enabled: bool = None,
    ao_enabled: bool = None,
    unbound: bool = None,
    enabled: bool = None
) -> Dict[str, Any]:
    """
    Set Post Process Volume settings for lookdev environment.
    
    Args:
        name: Name of the volume (if None, uses first available)
        exposure_mode: "manual" or "auto" (default: manual for lookdev)
        exposure_bias: Exposure bias value (default: 0 for EV100=0)
        exposure_value: Camera exposure value (default: 0 for EV100=0)
        bloom_enabled: Enable/disable bloom (default: False for lookdev)
        vignette_enabled: Enable/disable vignette (default: False for lookdev)
        ao_enabled: Enable/disable ambient occlusion (default: False for lookdev)
        unbound: Whether volume affects entire scene (default: True)
        enabled: Whether volume is active (default: True)
    
    Returns:
        Dictionary with success status and applied settings
    
    Example:
        # Set manual exposure for lookdev
        set_post_process_settings(
            name="Lookdev_PP",
            exposure_mode="manual",
            exposure_value=0,
            bloom_enabled=False,
            vignette_enabled=False
        )
    """
    unreal = get_unreal_connection()
    try:
        params = {}
        if name:
            params["name"] = name
        if exposure_mode:
            params["exposure_mode"] = exposure_mode
        if exposure_bias is not None:
            params["exposure_bias"] = exposure_bias
        if exposure_value is not None:
            params["exposure_value"] = exposure_value
        if bloom_enabled is not None:
            params["bloom_enabled"] = bloom_enabled
        if vignette_enabled is not None:
            params["vignette_enabled"] = vignette_enabled
        if ao_enabled is not None:
            params["ao_enabled"] = ao_enabled
        if unbound is not None:
            params["unbound"] = unbound
        if enabled is not None:
            params["enabled"] = enabled
        response = unreal.send_command("set_post_process_settings", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_post_process_settings error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Actor Spawning Tools (2 tools)
# ============================================================================

@mcp.tool()
def spawn_basic_actor(
    actor_type: str,
    name: str = None,
    location: Dict[str, float] = None,
    rotation: Dict[str, float] = None,
    scale: Dict[str, float] = None,
    mesh_path: str = None
) -> Dict[str, Any]:
    """
    Spawn a basic actor in the scene for lookdev purposes.
    
    Args:
        actor_type: Type of actor - "Sphere", "Cube", "Plane", "Cylinder", or "StaticMeshActor"
        name: Name for the actor (auto-generated if not provided)
        location: World position {"x": 0, "y": 0, "z": 0} (default: origin)
        rotation: Rotation in degrees {"pitch": 0, "yaw": 0, "roll": 0} (default: zero)
        scale: Scale factors {"x": 1, "y": 1, "z": 1} (default: unit scale)
        mesh_path: Static mesh asset path (for StaticMeshActor type)
    
    Returns:
        Dictionary with success status, actor name, type, and transform
    
    Example:
        # Spawn a material ball (sphere)
        spawn_basic_actor(
            actor_type="Sphere",
            name="MaterialBall",
            location={"x": 0, "y": 0, "z": 100},
            scale={"x": 1, "y": 1, "z": 1}
        )
        
        # Spawn a gray card (plane)
        spawn_basic_actor(
            actor_type="Plane",
            name="GrayCard",
            location={"x": 200, "y": 0, "z": 0},
            rotation={"pitch": 0, "yaw": 0, "roll": 0},
            scale={"x": 2, "y": 2, "z": 1}
        )
    """
    unreal = get_unreal_connection()
    try:
        params = {"actor_type": actor_type}
        if name:
            params["name"] = name
        if location:
            params["location"] = location
        if rotation:
            params["rotation"] = rotation
        if scale:
            params["scale"] = scale
        if mesh_path:
            params["mesh_path"] = mesh_path
        response = unreal.send_command("spawn_basic_actor", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"spawn_basic_actor error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_actor_material(
    actor_name: str,
    material_path: str
) -> Dict[str, Any]:
    """
    Apply a material to an actor's mesh component.
    
    Args:
        actor_name: Name of the target actor
        material_path: Path to the material asset (e.g., "/Game/Materials/M_Material")
    
    Returns:
        Dictionary with success status, actor name, and material path
    
    Example:
        set_actor_material(
            actor_name="MaterialBall",
            material_path="/Game/Materials/M_Lookdev_Gray"
        )
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "actor_name": actor_name,
            "material_path": material_path
        }
        response = unreal.send_command("set_actor_material", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_actor_material error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Generic Actor Management Tools (Refactored - 5 Core Tools)
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
    
    This is the new generic actor spawning tool that replaces spawn_basic_actor and create_light.
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


# ============================================================================
# Main Entry Point
# ============================================================================

if __name__ == "__main__":
    mcp.run()
