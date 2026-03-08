"""
Unreal Render MCP Tools

Modular tool definitions for Unreal Engine operations.
"""

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from material import (
    build_material_graph,
    get_material_graph,
)

from texture import (
    import_texture,
)

from mesh import (
    import_fbx,
)

from actor import (
    spawn_actor,
    delete_actor,
    get_actors,
    set_actor_properties,
    get_actor_properties,
    batch_spawn_actors,
    batch_delete_actors,
    batch_set_actors_properties,
)

from asset import (
    create_asset,
    delete_asset,
    set_asset_properties,
    get_asset_properties,
    get_assets,
    batch_create_assets,
    batch_set_assets_properties,
)

from niagara import (
    get_niagara_graph,
    update_niagara_graph,
    get_niagara_emitter,
    update_niagara_emitter,
    get_niagara_compiled_code,
    get_niagara_particle_attributes,
)

from viewport import (
    get_viewport_screenshot,
)

__all__ = [
    # Material
    "build_material_graph",
    "get_material_graph",
    # Texture
    "import_texture",
    # Mesh
    "import_fbx",
    # Actor
    "spawn_actor",
    "delete_actor",
    "get_actors",
    "set_actor_properties",
    "get_actor_properties",
    "batch_spawn_actors",
    "batch_delete_actors",
    "batch_set_actors_properties",
    # Asset
    "create_asset",
    "delete_asset",
    "set_asset_properties",
    "get_asset_properties",
    "get_assets",
    "batch_create_assets",
    "batch_set_assets_properties",
    # Niagara
    "get_niagara_graph",
    "update_niagara_graph",
    "get_niagara_emitter",
    "update_niagara_emitter",
    "get_niagara_compiled_code",
    "get_niagara_particle_attributes",
    # Viewport
    "get_viewport_screenshot",
]
