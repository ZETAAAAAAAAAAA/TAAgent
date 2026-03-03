"""Mesh handler for Unreal Render MCP"""

import logging
from typing import Dict, Any, List, Optional

logger = logging.getLogger("UnrealRenderMCP.Mesh")


class MeshHandler:
    """Handler for mesh-related operations"""
    
    def __init__(self, connection):
        self.connection = connection
    
    def import_fbx(
        self,
        fbx_path: str,
        destination_path: str = "/Game/ImportedMeshes/",
        asset_name: str = None,
        spawn_in_level: bool = True,
        location: List[float] = None
    ) -> Dict[str, Any]:
        """
        Import an FBX file into Unreal Engine.
        
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
            "spawn_in_level": spawn_in_level
        }
        if asset_name:
            params["asset_name"] = asset_name
        if location:
            params["location"] = location
        
        return self.connection.send_command("import_fbx", params)
