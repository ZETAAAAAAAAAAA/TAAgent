"""
Blueprint Tools for Unreal Render MCP

Editor Blueprint analysis and update tools.

Note: For listing EditorUtilityWidgetBlueprint assets, use get_assets with:
    get_assets(asset_class="EditorUtilityWidgetBlueprint")
"""

from typing import Dict, Any, List, Optional
import logging

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from common import send_command, with_unreal_connection

logger = logging.getLogger("UnrealRenderMCP")


@with_unreal_connection
def get_blueprint_info(
    blueprint_path: str,
    include_variables: bool = True,
    include_functions: bool = True,
    include_widget_tree: bool = True
) -> Dict[str, Any]:
    """
    Get detailed information about a Blueprint.
    
    Works with both Game Blueprints and Editor Utility Widget Blueprints.
    
    Args:
        blueprint_path: Full path to the blueprint asset
            - Game Blueprint: "/Script/Engine.Blueprint'/Game/MyBP.MyBP'"
            - Editor Widget: "/Script/Blutility.EditorUtilityWidgetBlueprint'/Game/MyWidget.MyWidget'"
        include_variables: Include variable list
        include_functions: Include function list
        include_widget_tree: Include widget tree info (for Editor Utility Widgets)
    
    Returns:
        Blueprint info including variables, functions, graphs, and metadata
    """
    params = {
        "blueprint_path": blueprint_path,
        "include_variables": include_variables,
        "include_functions": include_functions,
        "include_widget_tree": include_widget_tree
    }
    return send_command("get_editor_widget_blueprint_info", params)


@with_unreal_connection
def update_blueprint(
    blueprint_path: str,
    properties: dict = None,
    compile_after: bool = False
) -> Dict[str, Any]:
    """
    Update blueprint properties.
    
    Args:
        blueprint_path: Full path to the blueprint asset
        properties: Blueprint properties to set (handled via UE reflection)
        compile_after: Whether to compile after updating
    
    Returns:
        Success status and any error messages
    """
    params = {"blueprint_path": blueprint_path, "compile_after": compile_after}
    if properties:
        params["properties"] = properties
    return send_command("update_editor_widget_blueprint", params)
