"""
Mesh Tools for Unreal Render MCP

Mesh import and creation tools.
"""

from typing import Dict, Any, List, Optional
import logging

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from common import send_command, with_unreal_connection

logger = logging.getLogger("UnrealRenderMCP")


@with_unreal_connection
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
        destination_path: Destination path in Unreal
        asset_name: Name for the imported asset
        spawn_in_level: Whether to spawn the mesh in the level
        location: Spawn location [x, y, z]
    """
    params = {
        "fbx_path": fbx_path,
        "destination_path": destination_path,
        "spawn_in_level": spawn_in_level,
    }
    if asset_name is not None:
        params["asset_name"] = asset_name
    if location is not None:
        params["location"] = location

    return send_command("import_fbx", params)
