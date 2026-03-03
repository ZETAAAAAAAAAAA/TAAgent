"""Texture handler for Unreal Render MCP"""

import logging
from typing import Dict, Any, List, Optional

logger = logging.getLogger("UnrealRenderMCP.Texture")


class TextureHandler:
    """Handler for texture-related operations"""
    
    def __init__(self, connection):
        self.connection = connection
    
    def import_texture(
        self,
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
        Import a texture file into Unreal Engine.
        
        Args:
            source_path: Absolute path to the source texture file
            name: Name for the imported texture asset
            destination_path: Destination path in Unreal
            delete_source: If True, delete the source file after import
            srgb: Whether texture uses sRGB gamma
            compression_settings: Compression format (Default, TC_NormalMap, TC_Grayscale, etc.)
            filter: Filter mode (Default, Nearest, Bilinear, Trilinear)
            address_x: Address mode for X (Wrap, Clamp, Mirror)
            address_y: Address mode for Y (Wrap, Clamp, Mirror)
        """
        params = {
            "source_path": source_path,
            "name": name,
            "destination_path": destination_path,
            "delete_source": delete_source
        }
        if srgb is not None:
            params["srgb"] = srgb
        if compression_settings:
            params["compression_settings"] = compression_settings
        if filter:
            params["filter"] = filter
        if address_x:
            params["address_x"] = address_x
        if address_y:
            params["address_y"] = address_y
        
        return self.connection.send_command("import_texture", params)
    
    def set_texture_properties(
        self,
        texture_path: str,
        compression_settings: str = None,
        srgb: bool = None,
        filter: str = None,
        address_x: str = None,
        address_y: str = None,
        mip_gen_settings: str = None,
        max_texture_size: int = None
    ) -> Dict[str, Any]:
        """Set texture properties."""
        params = {"texture_path": texture_path}
        if compression_settings:
            params["compression_settings"] = compression_settings
        if srgb is not None:
            params["srgb"] = srgb
        if filter:
            params["filter"] = filter
        if address_x:
            params["address_x"] = address_x
        if address_y:
            params["address_y"] = address_y
        if mip_gen_settings:
            params["mip_gen_settings"] = mip_gen_settings
        if max_texture_size:
            params["max_texture_size"] = max_texture_size
        
        return self.connection.send_command("set_texture_properties", params)
