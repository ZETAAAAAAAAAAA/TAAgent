"""
Niagara Tools for Unreal Render MCP

Consolidated Niagara system analysis and modification tools.

Design Philosophy:
- 4 tools following "Minimal Tool Set" principle
- Graph form: get/update Niagara script graphs
- Emitter form: get/update Emitter hierarchy structure
"""

from typing import Dict, Any, List, Optional
import logging

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from common import send_command, save_json_to_file, with_unreal_connection

logger = logging.getLogger("UnrealRenderMCP")


# ============================================================================
# Graph Form Tools - Read/Update Niagara Script Graphs
# ============================================================================

@with_unreal_connection
def get_niagara_graph(
    # Method 1: Locate script within System's Emitter
    asset_path: Optional[str] = None,
    emitter: Optional[str] = None,
    script: str = "spawn",
    
    # Method 2: Direct script path for standalone assets
    script_path: Optional[str] = None,
    
    # Optional filter
    module: str = "",
    save_to: Optional[str] = None
) -> Dict[str, Any]:
    """
    Get Niagara Graph nodes and connections.
    
    Unified interface for reading graphs from:
    1. Scripts within a Niagara System (specify asset_path + emitter + script)
    2. Standalone Niagara Script assets (specify script_path only)
    
    Args:
        asset_path: Path to Niagara System asset (for embedded scripts)
        emitter: Emitter name within the system
        script: Script type - "spawn" or "update" (default: "spawn")
        script_path: Direct path to standalone Niagara Script asset
        module: Optional filter - only return nodes matching this module name
        save_to: Optional path to save JSON
        
    Examples:
        # Get graph from System's Emitter spawn script
        get_niagara_graph(asset_path="/Game/Effects/NS_Fire", emitter="Flame", script="spawn")
        
        # Get graph from standalone Niagara Module
        get_niagara_graph(script_path="/Niagara/Modules/Update/Forces/CurlNoiseForce")
    """
    params = {
        "script": script,
        "module": module
    }
    
    # Determine location method
    if script_path:
        params["script_path"] = script_path
    elif asset_path and emitter:
        params["asset_path"] = asset_path
        params["emitter"] = emitter
    else:
        return {
            "success": False,
            "error": "Must provide either (asset_path + emitter) or script_path"
        }
    
    result = send_command("get_niagara_graph", params)
    
    if save_to and result.get("success"):
        graph_name = script_path.split("/")[-1] if script_path else f"{emitter}_{script}"
        save_info = save_json_to_file(result, save_to, "niagara_graph", graph_name)
        result.update(save_info)
    
    return result


@with_unreal_connection
def update_niagara_graph(
    # Method 1: Locate script within System's Emitter
    asset_path: Optional[str] = None,
    emitter: Optional[str] = None,
    script: str = "spawn",
    
    # Method 2: Direct script path for standalone assets
    script_path: Optional[str] = None,
    
    operations: List[Dict[str, Any]] = None
) -> Dict[str, Any]:
    """
    Update Niagara Graph with operations.
    
    Unified interface for updating graphs in:
    1. Scripts within a Niagara System (specify asset_path + emitter + script)
    2. Standalone Niagara Script assets (specify script_path only)
    
    Operations:
    - add_module: Add a module to the script
    - remove_module: Remove a module from the script
    - set_parameter: Set a parameter value
    - connect: Connect two nodes
    - disconnect: Disconnect nodes
    
    Args:
        asset_path: Path to Niagara System asset (for embedded scripts)
        emitter: Emitter name within the system
        script: Script type - "spawn" or "update"
        script_path: Direct path to standalone Niagara Script asset
        operations: List of operations to perform
        
    Examples:
        # Add module to System's Emitter script
        update_niagara_graph(
            asset_path="/Game/Effects/NS_Fire",
            emitter="Flame",
            script="update",
            operations=[{"action": "add_module", "module_path": "/Niagara/Modules/Update/Forces/CurlNoiseForce"}]
        )
        
        # Add module to standalone script
        update_niagara_graph(
            script_path="/Game/Niagara/MyCustomModule",
            operations=[{"action": "add_module", "module_path": "/Niagara/Modules/Common/Math"}]
        )
    """
    params = {
        "script": script,
        "operations": operations or []
    }
    
    # Determine location method
    if script_path:
        params["script_path"] = script_path
    elif asset_path and emitter:
        params["asset_path"] = asset_path
        params["emitter"] = emitter
    else:
        return {
            "success": False,
            "error": "Must provide either (asset_path + emitter) or script_path"
        }
    
    return send_command("update_niagara_graph", params)


# ============================================================================
# Emitter Form Tools - Read/Update Emitter Hierarchy
# ============================================================================

@with_unreal_connection
def get_niagara_emitter(
    asset_path: str,
    detail_level: str = "overview",
    emitters: List[str] = None,
    include: List[str] = None,
    save_to: Optional[str] = None
) -> Dict[str, Any]:
    """
    Get Niagara Emitter structure and properties.
    
    Args:
        asset_path: Path to the Niagara system asset
        detail_level: Level of detail - "overview" or "full"
        emitters: Optional list of emitter names to filter
        include: Optional list of sections to include:
            - "scripts": Spawn/Update script details
            - "renderers": Renderer configurations
            - "parameters": Parameter definitions
            - "stateless_analysis": Analyze Stateless conversion compatibility
        save_to: Optional path to save JSON
        
    Examples:
        # Quick overview of all emitters
        get_niagara_emitter("/Game/Effects/NS_Fire")
        
        # Full detail for specific emitter with stateless analysis
        get_niagara_emitter(
            "/Game/Effects/NS_Fire",
            detail_level="full",
            emitters=["Flame"],
            include=["scripts", "renderers", "parameters", "stateless_analysis"]
        )
    """
    params = {
        "asset_path": asset_path,
        "detail_level": detail_level
    }
    if emitters is not None:
        params["emitters"] = emitters
    if include is not None:
        params["include"] = include
    
    result = send_command("get_niagara_emitter", params)
    
    if save_to and result.get("success"):
        asset_name = result.get("asset_name", asset_path.split("/")[-1])
        save_info = save_json_to_file(result, save_to, "niagara_emitter", asset_name)
        result.update(save_info)
    
    return result


@with_unreal_connection
def update_niagara_emitter(
    asset_path: str,
    operations: List[Dict[str, Any]]
) -> Dict[str, Any]:
    """
    Batch update Niagara Emitter with multiple operations.
    
    Operations format:
    [
        # Emitter operations
        {"target": "emitter", "name": "Smoke", "action": "set_enabled", "value": false},
        {"target": "emitter", "name": "Flame", "action": "rename", "value": "BigFlame"},
        {"target": "emitter", "name": "Smoke", "action": "add", "template": "/Niagara/Templates/SimpleSmoke"},
        {"target": "emitter", "name": "OldEmitter", "action": "remove"},
        
        # Stateless conversion (UE 5.7+)
        {"target": "emitter", "name": "Flame", "action": "convert_to_stateless", "force": false},
        
        # Renderer operations
        {"target": "renderer", "emitter": "Flame", "index": 0, "action": "set_enabled", "value": true},
        
        # Parameter operations
        {"target": "parameter", "emitter": "Flame", "script": "spawn", "name": "SpawnRate", "value": 100.0},
        
        # Simulation stage operations
        {"target": "sim_stage", "emitter": "Flame", "name": "Collision", "action": "set_enabled", "value": true},
        
        # Stateless module operations (UE 5.7+)
        {"target": "stateless_module", "emitter": "Flame", "name": "Lifetime", 
         "action": "set_property", "property": "LifetimeMin", "value": 1.5}
    ]
    
    Args:
        asset_path: Path to the Niagara system asset
        operations: List of operations to perform
        
    Examples:
        # Enable emitter and adjust spawn rate
        update_niagara_emitter("/Game/Effects/NS_Fire", [
            {"target": "emitter", "name": "Flame", "action": "set_enabled", "value": True},
            {"target": "parameter", "emitter": "Flame", "script": "spawn", "name": "SpawnRate", "value": 100.0}
        ])
        
        # Convert emitter to Stateless mode
        update_niagara_emitter("/Game/Effects/NS_Fire", [
            {"target": "emitter", "name": "Flame", "action": "convert_to_stateless", "force": False}
        ])
    """
    return send_command("update_niagara_emitter", {
        "asset_path": asset_path,
        "operations": operations
    })


# ============================================================================
# Debug Tools - Compiled Code Inspection
# ============================================================================

@with_unreal_connection
def get_niagara_compiled_code(
    asset_path: str,
    emitter: Optional[str] = None,
    script: str = "spawn"
) -> Dict[str, Any]:
    """
    Get compiled HLSL code from Niagara scripts.
    
    Returns generated HLSL code for CPU (VM) and GPU (Compute Shader) scripts.
    Useful for debugging and understanding Niagara execution.
    
    Args:
        asset_path: Path to Niagara System asset
        emitter: Emitter name (required for particle scripts)
        script: Script type - "spawn", "update", "gpu_compute", 
                "system_spawn", or "system_update"
    
    Returns:
        {
            "success": true,
            "hlsl_cpu": "...",      // CPU/VM HLSL code
            "hlsl_gpu": "...",      // GPU Compute Shader HLSL
            "hlsl_cpu_length": 1234,
            "hlsl_gpu_length": 5678,
            "compile_errors": [],
            "error_count": 0
        }
    
    Examples:
        # Get spawn script HLSL for an emitter
        get_niagara_compiled_code("/Game/Effects/NS_Fire", emitter="Flame", script="spawn")
        
        # Get update script HLSL
        get_niagara_compiled_code("/Game/Effects/NS_Fire", emitter="Flame", script="update")
        
        # Get GPU compute shader
        get_niagara_compiled_code("/Game/Effects/NS_Fire", emitter="Flame", script="gpu_compute")
        
        # Get system-level script
        get_niagara_compiled_code("/Game/Effects/NS_Fire", script="system_spawn")
    """
    params = {
        "asset_path": asset_path,
        "script": script
    }
    if emitter:
        params["emitter"] = emitter
    
    return send_command("get_niagara_compiled_code", params)
