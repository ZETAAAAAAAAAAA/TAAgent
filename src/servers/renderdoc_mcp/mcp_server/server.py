"""
RenderDoc MCP Server
FastMCP 2.0 server providing access to RenderDoc capture data.
"""

from typing import Any, Literal

from fastmcp import FastMCP

from .bridge.client import RenderDocBridge, RenderDocBridgeError
from .config import settings

# Initialize FastMCP server
mcp = FastMCP(
    name="RenderDoc MCP Server",
)

# RenderDoc bridge client
bridge = RenderDocBridge(host=settings.renderdoc_host, port=settings.renderdoc_port)


@mcp.tool
def get_capture_status() -> dict:
    """
    Check if a capture is currently loaded in RenderDoc.
    Returns the capture status and API type if loaded.
    """
    return bridge.call("get_capture_status")


@mcp.tool
def get_draw_calls(
    include_children: bool = True,
    marker_filter: str | None = None,
    exclude_markers: list[str] | None = None,
    event_id_min: int | None = None,
    event_id_max: int | None = None,
    only_actions: bool = False,
    flags_filter: list[str] | None = None,
) -> dict:
    """
    Get the list of all draw calls and actions in the current capture.

    Args:
        include_children: Include child actions in the hierarchy (default: True)
        marker_filter: Only include actions under markers containing this string (partial match)
        exclude_markers: Exclude actions under markers containing these strings (list of partial matches)
        event_id_min: Only include actions with event_id >= this value
        event_id_max: Only include actions with event_id <= this value
        only_actions: If True, exclude marker actions (PushMarker/PopMarker/SetMarker)
        flags_filter: Only include actions with these flags (list of flag names, e.g. ["Drawcall", "Dispatch"])

    Returns a hierarchical tree of actions including markers, draw calls,
    dispatches, and other GPU events.
    """
    params: dict[str, object] = {"include_children": include_children}
    if marker_filter is not None:
        params["marker_filter"] = marker_filter
    if exclude_markers is not None:
        params["exclude_markers"] = exclude_markers
    if event_id_min is not None:
        params["event_id_min"] = event_id_min
    if event_id_max is not None:
        params["event_id_max"] = event_id_max
    if only_actions:
        params["only_actions"] = only_actions
    if flags_filter is not None:
        params["flags_filter"] = flags_filter
    return bridge.call("get_draw_calls", params)


@mcp.tool
def get_frame_summary() -> dict:
    """
    Get a summary of the current capture frame.

    Returns statistics about the frame including:
    - API type (D3D11, D3D12, Vulkan, etc.)
    - Total action count
    - Statistics: draw calls, dispatches, clears, copies, presents, markers
    - Top-level markers with event IDs and child counts
    - Resource counts: textures, buffers
    """
    return bridge.call("get_frame_summary")


@mcp.tool
def find_draws_by_shader(
    shader_name: str,
    stage: Literal["vertex", "hull", "domain", "geometry", "pixel", "compute"] | None = None,
) -> dict:
    """
    Find all draw calls using a shader with the given name (partial match).

    Args:
        shader_name: Partial name to search for in shader names or entry points
        stage: Optional shader stage to search (if not specified, searches all stages)

    Returns a list of matching draw calls with event IDs and match reasons.
    """
    params: dict[str, object] = {"shader_name": shader_name}
    if stage is not None:
        params["stage"] = stage
    return bridge.call("find_draws_by_shader", params)


@mcp.tool
def find_draws_by_texture(texture_name: str) -> dict:
    """
    Find all draw calls using a texture with the given name (partial match).

    Args:
        texture_name: Partial name to search for in texture resource names

    Returns a list of matching draw calls with event IDs and match reasons.
    Searches SRVs, UAVs, and render targets.
    """
    return bridge.call("find_draws_by_texture", {"texture_name": texture_name})


@mcp.tool
def find_draws_by_resource(resource_id: str) -> dict:
    """
    Find all draw calls using a specific resource ID (exact match).

    Args:
        resource_id: Resource ID to search for (e.g. "ResourceId::12345" or "12345")

    Returns a list of matching draw calls with event IDs and match reasons.
    Searches shaders, SRVs, UAVs, render targets, and depth targets.
    """
    return bridge.call("find_draws_by_resource", {"resource_id": resource_id})


@mcp.tool
def get_draw_call_details(event_id: int) -> dict:
    """
    Get detailed information about a specific draw call.

    Args:
        event_id: The event ID of the draw call to inspect

    Includes vertex/index counts, resource outputs, and other metadata.
    """
    return bridge.call("get_draw_call_details", {"event_id": event_id})


@mcp.tool
def get_action_timings(
    event_ids: list[int] | None = None,
    marker_filter: str | None = None,
    exclude_markers: list[str] | None = None,
) -> dict:
    """
    Get GPU timing information for actions (draw calls, dispatches, etc.).

    Args:
        event_ids: Optional list of specific event IDs to get timings for.
                   If not specified, returns timings for all actions.
        marker_filter: Only include actions under markers containing this string (partial match).
        exclude_markers: Exclude actions under markers containing these strings.

    Returns timing data including:
    - available: Whether GPU timing counters are supported
    - unit: Time unit (typically "seconds")
    - timings: List of {event_id, name, duration_seconds, duration_ms}
    - total_duration_ms: Sum of all durations
    - count: Number of timing entries

    Note: GPU timing counters may not be available on all hardware/drivers.
    """
    params: dict[str, object] = {}
    if event_ids is not None:
        params["event_ids"] = event_ids
    if marker_filter is not None:
        params["marker_filter"] = marker_filter
    if exclude_markers is not None:
        params["exclude_markers"] = exclude_markers
    return bridge.call("get_action_timings", params)


@mcp.tool
def get_shader_info(
    event_id: int,
    stage: Literal["vertex", "hull", "domain", "geometry", "pixel", "compute"],
) -> dict:
    """
    Get shader information for a specific stage at a given event.

    Args:
        event_id: The event ID to inspect the shader at
        stage: The shader stage (vertex, hull, domain, geometry, pixel, compute)

    Returns shader disassembly, constant buffer values, and resource bindings.
    """
    return bridge.call("get_shader_info", {"event_id": event_id, "stage": stage})


@mcp.tool
def get_buffer_contents(
    resource_id: str,
    offset: int = 0,
    length: int = 0,
) -> dict:
    """
    Read the contents of a buffer resource.

    Args:
        resource_id: The resource ID of the buffer to read
        offset: Byte offset to start reading from (default: 0)
        length: Number of bytes to read, 0 for entire buffer (default: 0)

    Returns buffer data as base64-encoded bytes along with metadata.
    """
    return bridge.call(
        "get_buffer_contents",
        {"resource_id": resource_id, "offset": offset, "length": length},
    )


@mcp.tool
def get_texture_info(resource_id: str) -> dict:
    """
    Get metadata about a texture resource.

    Args:
        resource_id: The resource ID of the texture

    Includes dimensions, format, mip levels, and other properties.
    """
    return bridge.call("get_texture_info", {"resource_id": resource_id})


@mcp.tool
def get_texture_data(
    resource_id: str,
    mip: int = 0,
    slice: int = 0,
    sample: int = 0,
    depth_slice: int | None = None,
) -> dict:
    """
    Read the pixel data of a texture resource.

    Args:
        resource_id: The resource ID of the texture to read
        mip: Mip level to retrieve (default: 0)
        slice: Array slice or cube face index (default: 0)
               For cube maps: 0=X+, 1=X-, 2=Y+, 3=Y-, 4=Z+, 5=Z-
        sample: MSAA sample index (default: 0)
        depth_slice: For 3D textures only, extract a specific depth slice (default: None = full volume)
                     When specified, returns only the 2D slice at that depth index

    Returns texture pixel data as base64-encoded bytes along with metadata
    including dimensions at the requested mip level and format information.
    """
    params = {"resource_id": resource_id, "mip": mip, "slice": slice, "sample": sample}
    if depth_slice is not None:
        params["depth_slice"] = depth_slice
    return bridge.call("get_texture_data", params)


@mcp.tool(output_schema=None)
def save_texture(
    resource_id: str,
    output_path: str,
    file_type: Literal["PNG", "TGA", "DDS", "JPG", "HDR", "BMP", "EXR"] = "PNG",
    mip: int = -1,
    slice_index: int = -1,
) -> dict:
    """
    Save texture data to various image formats using RenderDoc's native API.

    Supports ALL texture formats including BC compressed formats (BC1-BC7).
    DDS format can save all mip levels and array slices.

    Args:
        resource_id: The resource ID of the texture to save
        output_path: Full path where to save the file (e.g., "C:/temp/texture.png")
        file_type: Output format - PNG, TGA, DDS, JPG, HDR, BMP, or EXR (default: PNG)
        mip: Mip level to save (-1 for all mips, only DDS supports all mips)
        slice_index: Array slice to save (-1 for all slices, only DDS supports all slices)

    Returns:
        Dict with success status, file path, dimensions, and format information.
    """
    import sys
    print(f"[save_texture] Called: resource_id={resource_id}, output_path={output_path}, file_type={file_type}", file=sys.stderr, flush=True)
    
    params = {
        "resource_id": resource_id,
        "output_path": output_path,
        "file_type": file_type,
        "mip": mip,
        "slice_index": slice_index,
    }
    
    try:
        result = bridge.call("save_texture", params)
        print(f"[save_texture] Result: {result}", file=sys.stderr, flush=True)
        return result
    except Exception as e:
        print(f"[save_texture] Error: {e}", file=sys.stderr, flush=True)
        return {"success": False, "error": str(e)}


@mcp.tool
def get_pipeline_state(event_id: int) -> dict:
    """
    Get the full graphics pipeline state at a specific event.

    Args:
        event_id: The event ID to get pipeline state at

    Returns detailed pipeline state including:
    - Bound shaders with entry points for each stage
    - Shader resources (SRVs): textures and buffers with dimensions, format, slot, name
    - UAVs (RWTextures/RWBuffers): resource details with dimensions and format
    - Samplers: addressing modes, filter settings, LOD parameters
    - Constant buffers: slot, size, variable count
    - Render targets and depth target
    - Viewports and input assembly state
    """
    return bridge.call("get_pipeline_state", {"event_id": event_id})


@mcp.tool
def list_captures(directory: str) -> dict:
    """
    List all RenderDoc capture files (.rdc) in the specified directory.

    Args:
        directory: The directory path to search for capture files

    Returns a list of capture files with their metadata including:
    - filename: The capture file name
    - path: Full path to the file
    - size_bytes: File size in bytes
    - modified_time: Last modified timestamp (ISO format)
    """
    return bridge.call("list_captures", {"directory": directory})


@mcp.tool
def open_capture(capture_path: str) -> dict:
    """
    Open a RenderDoc capture file (.rdc).

    Args:
        capture_path: Full path to the capture file to open

    Returns success status and information about the opened capture.
    Note: This will close any currently open capture.
    """
    return bridge.call("open_capture", {"capture_path": capture_path})


@mcp.tool
def get_mesh_data(event_id: int) -> dict:
    """
    Get mesh data for a specific draw call.

    Args:
        event_id: The event ID of the draw call

    Returns vertex positions, normals, UVs, tangents, indices and other attributes
    for the mesh rendered in the specified draw call.
    """
    return bridge.call("get_mesh_data", {"event_id": event_id})


@mcp.tool(output_schema=None)
def export_mesh_as_fbx(
    event_id: int,
    output_path: str,
    attribute_mapping: dict,
    unit_scale: int = 1,
    coordinate_system: str = "ue",
    decode: dict | None = None,
    buffer_config: dict | None = None,
    flip_winding_order: bool = False,
) -> dict:
    """
    Export mesh data from a draw call as an FBX file.

    IMPORTANT: attribute_mapping is REQUIRED and must be provided by AI after 
    analyzing CSV and DXBC data. NO automatic detection is performed.

    Workflow:
    1. AI calls export_mesh_csv() to get vs_input and vs_output data
    2. AI analyzes CSV to identify attribute semantics (position, normal, UV, etc.)
    3. AI analyzes DXBC shader if needed to understand data transformations
    4. AI determines decode expressions for compressed data
    5. AI calls this function with explicit attribute_mapping and decode

    Args:
        event_id: The event ID of the draw call to export
        output_path: Full path where to save the FBX file (e.g., "C:/temp/mesh.fbx")
        attribute_mapping: REQUIRED dict mapping semantic names to attribute names.
                          MUST include stage prefix - no auto-detection.
                          
                          Format: {"SEMANTIC": "stage:attribute_name"}
                          
                          Stage options:
                          - "vs_input:ATTRIBUTE0" - VS input attributes
                          - "vs_output:TEXCOORD0" - VS output attributes
                          - "buffer:BufferName" - GPU Buffer data (UE5 GPU-driven rendering)
                          
                          Example:
                          {
                              "POSITION": "vs_input:ATTRIBUTE0",
                              "NORMAL": "buffer:Buffer1",
                              "TANGENT": "buffer:Buffer1",
                              "UV": "vs_output:TEXCOORD0"
                          }
                          
                          Supported keys: POSITION, NORMAL, TANGENT, BINORMAL, UV, UV2, COLOR
        unit_scale: Unit scale factor for FBX. Default is 1 (centimeters).
                   Use 1 for UE games (cm), 100 for games using meters.
        coordinate_system: Target coordinate system. Options:
                          "ue" - Unreal Engine (Z-up, left-handed) - default
                          "unity" - Unity (Y-up, left-handed)
                          "blender" - Blender (Z-up, right-handed)
                          "maya" - Maya (Y-up, right-handed)
        decode: Optional dict of decode expressions for attributes.
                AI analyzes shader and generates expressions to decode compressed data.
                
                Supported expressions:
                - "itof(x) / 32768" - Integer to float, then divide (common for compressed normals)
                - "x * 2 - 1" - Map [0,1] to [-1,1] (common for tangents)
                - "normalize(x)" - Normalize vector
                - "normalize(itof(x) / 32768)" - Decode and normalize
                
                Example: {"NORMAL": "itof(x) / 32768", "TANGENT": "x * 2 - 1"}
        buffer_config: Optional dict configuring GPU Buffer data layout.
                      Required when using "buffer:BufferName" in attribute_mapping.
                      
                      Format:
                      {
                          "BufferName": {
                              "resource_id": "ResourceId::12345",  # Optional, auto-detected if not provided
                              "stride": 32,           # Bytes per vertex in buffer
                              "normal_offset": 16,    # Byte offset for normal data
                              "tangent_offset": 0,    # Byte offset for tangent data
                              "uv_offset": 8,         # Byte offset for UV data
                              "color_offset": 24,     # Byte offset for color data
                              "format": "float4"      # Data format: "float2", "float3", "float4"
                          }
                      }
        flip_winding_order: If True, flip the winding order of triangle indices (swap index 1 and 2).
                           Use this when normals are inverted after import into target software.
                           Different games may use different winding orders.

    Returns:
        Dict with success status, file path, mesh statistics (vertex count, triangle count),
        attribute mapping used, decode expressions applied, and file size.

    Note:
        FBX file includes proper axis settings and UnitScaleFactor for the target software.
        
    Example:
        # UE5 GPU-driven rendering with model space position and buffer-based normals
        export_mesh_as_fbx(
            event_id=5249, 
            output_path="mesh.fbx",
            attribute_mapping={
                "POSITION": "vs_input:ATTRIBUTE0",
                "NORMAL": "buffer:Buffer1",
                "TANGENT": "buffer:Buffer1",
                "UV": "vs_output:TEXCOORD0"
            },
            buffer_config={
                "Buffer1": {
                    "stride": 32,
                    "normal_offset": 16,
                    "tangent_offset": 0,
                    "format": "float4"
                }
            },
            coordinate_system="ue",
            unit_scale=1
        )
    """
    params = {
        "event_id": event_id,
        "output_path": output_path,
        "attribute_mapping": attribute_mapping,
        "unit_scale": unit_scale,
        "coordinate_system": coordinate_system,
        "flip_winding_order": flip_winding_order,
    }
    if decode is not None:
        params["decode"] = decode
    if buffer_config is not None:
        params["buffer_config"] = buffer_config
    return bridge.call("export_mesh_as_fbx", params)


@mcp.tool
def export_mesh_csv(
    event_id: int,
    output_path: str,
    stage: Literal["vs_input", "vs_output"] = "vs_output",
) -> dict:
    """
    Export mesh data from a draw call as CSV file.
    
    This exports mesh data in the same format as RenderDoc's Mesh Viewer CSV export.
    The CSV contains vertex data with attributes expanded to component columns.

    Args:
        event_id: The event ID of the draw call to export
        output_path: Full path where to save the CSV file (e.g., "C:/temp/mesh.csv")
        stage: "vs_input" for vertex shader inputs, "vs_output" for vertex shader outputs (default)

    Returns:
        Dict with success status, file path, and mesh statistics.
    """
    params = {"event_id": event_id, "output_path": output_path, "stage": stage}
    return bridge.call("export_mesh_csv", params)


def main():
    """Run the MCP server"""
    mcp.run()


if __name__ == "__main__":
    main()
