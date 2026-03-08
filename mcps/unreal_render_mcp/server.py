"""
Unreal Render MCP Server Entry Point

FastMCP server for Unreal Engine rendering operations.
"""

import sys
import os

# Ensure current directory is in path for imports when running as script
_current_dir = os.path.dirname(os.path.abspath(__file__))
if _current_dir not in sys.path:
    sys.path.insert(0, _current_dir)

import logging
from contextlib import asynccontextmanager
from typing import AsyncIterator, Dict, Any

from fastmcp import FastMCP

# Use absolute imports for MCP runtime compatibility
from connection import reset_unreal_connection
from tools import (
    # Material
    build_material_graph,
    compile_material,
    get_material_graph,
    # Texture
    import_texture,
    set_texture_properties,
    # Mesh
    import_fbx,
    create_static_mesh_from_data,
    # Actor
    spawn_actor,
    delete_actor,
    get_actors,
    set_actor_properties,
    get_actor_properties,
    batch_spawn_actors,
    batch_delete_actors,
    batch_set_actors_properties,
    # Asset
    create_asset,
    delete_asset,
    set_asset_properties,
    get_asset_properties,
    get_assets,
    batch_create_assets,
    batch_set_assets_properties,
    # Niagara
    get_niagara_graph,
    update_niagara_graph,
    get_niagara_emitter,
    update_niagara_emitter,
    get_niagara_compiled_code,
    # Viewport
    get_viewport_screenshot,
)


# Configure logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s',
    handlers=[
        logging.FileHandler('unreal_render_mcp.log'),
    ]
)
logger = logging.getLogger("UnrealRenderMCP")


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
# Register Material Tools
# ============================================================================

mcp.tool()(build_material_graph)
mcp.tool()(compile_material)
mcp.tool()(get_material_graph)


# ============================================================================
# Register Texture Tools
# ============================================================================

mcp.tool()(import_texture)
mcp.tool()(set_texture_properties)


# ============================================================================
# Register Mesh Tools
# ============================================================================

mcp.tool()(import_fbx)
mcp.tool()(create_static_mesh_from_data)


# ============================================================================
# Register Actor Tools
# ============================================================================

mcp.tool()(spawn_actor)
mcp.tool()(delete_actor)
mcp.tool()(get_actors)
mcp.tool()(set_actor_properties)
mcp.tool()(get_actor_properties)
mcp.tool()(batch_spawn_actors)
mcp.tool()(batch_delete_actors)
mcp.tool()(batch_set_actors_properties)


# ============================================================================
# Register Asset Tools
# ============================================================================

mcp.tool()(create_asset)
mcp.tool()(delete_asset)
mcp.tool()(set_asset_properties)
mcp.tool()(get_asset_properties)
mcp.tool()(get_assets)
mcp.tool()(batch_create_assets)
mcp.tool()(batch_set_assets_properties)


# ============================================================================
# Register Niagara Tools
# ============================================================================

mcp.tool()(get_niagara_graph)
mcp.tool()(update_niagara_graph)
mcp.tool()(get_niagara_emitter)
mcp.tool()(update_niagara_emitter)
mcp.tool()(get_niagara_compiled_code)


# ============================================================================
# Register Viewport Tools
# ============================================================================

mcp.tool()(get_viewport_screenshot)


# ============================================================================
# Main Entry Point
# ============================================================================

if __name__ == "__main__":
    mcp.run()
