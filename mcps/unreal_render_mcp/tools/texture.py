"""
Texture Tools for Unreal Render MCP

Texture import and property management tools.
"""

from typing import Dict, Any, Optional
import logging

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from common import send_command, with_unreal_connection

logger = logging.getLogger("UnrealRenderMCP")


@with_unreal_connection
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
        destination_path: Destination path in Unreal
        delete_source: If True, delete the source file after successful import
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
    if compression_settings is not None:
        params["compression_settings"] = compression_settings
    if filter is not None:
        params["filter"] = filter
    if address_x is not None:
        params["address_x"] = address_x
    if address_y is not None:
        params["address_y"] = address_y
    
    return send_command("import_texture", params)
