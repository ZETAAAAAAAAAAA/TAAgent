"""
Material Tools for Unreal Render MCP

Material graph building and analysis tools.
"""

from typing import Dict, Any, List, Optional
import logging

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from common import send_command, save_json_to_file, with_unreal_connection

logger = logging.getLogger("UnrealRenderMCP")


@with_unreal_connection
def build_material_graph(
    material_name: str,
    nodes: List[Dict[str, Any]],
    connections: List[Dict[str, Any]] = None,
    properties: Dict[str, Any] = None,
    compile: bool = True
) -> Dict[str, Any]:
    """
    Build an entire material graph in a single call (batch operation).
    
    Args:
        material_name: Name or path of the material (must already exist)
        nodes: List of node definitions
        connections: List of connection definitions
        properties: Optional material properties to set
        compile: Whether to compile the material after building
    """
    params = {
        "material_name": material_name,
        "nodes": nodes,
        "compile": compile
    }
    if connections:
        params["connections"] = connections
    if properties:
        params["properties"] = properties
    
    return send_command("build_material_graph", params)


@with_unreal_connection
def get_material_graph(asset_path: str, save_to: Optional[str] = None) -> Dict[str, Any]:
    """
    Get complete material or material function graph including nodes and connections.
    
    Supports both Material and MaterialFunction assets. Automatically detects the asset type.
    
    Args:
        asset_path: Path to the material or material function (e.g., "/Game/Materials/M_MyMaterial")
        save_to: Optional path to save JSON
    
    Returns:
        {
            "asset_type": "Material" | "MaterialFunction",
            "name": "...",
            "path": "...",
            "nodes": [...],
            "connections": [...],
            "node_count": N,
            "connection_count": M,
            # For Material only:
            "property_connections": {"BaseColor": {...}, "Normal": {...}, ...},
            # For MaterialFunction only:
            "inputs": [...],
            "outputs": [...],
            "description": "..."
        }
    """
    result = send_command("get_material_graph", {"asset_path": asset_path})
    
    if save_to and result.get("success"):
        asset_name = result.get("name", asset_path.split("/")[-1])
        save_info = save_json_to_file(result, save_to, "material_graph", asset_name)
        result.update(save_info)
    
    return result
