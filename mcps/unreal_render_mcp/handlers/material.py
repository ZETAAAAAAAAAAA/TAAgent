"""Material handler for Unreal Render MCP"""

import logging
from typing import Dict, Any, List, Optional

logger = logging.getLogger("UnrealRenderMCP.Material")


class MaterialHandler:
    """Handler for material-related operations"""
    
    def __init__(self, connection):
        self.connection = connection
    
    def create_material(self, name: str) -> Dict[str, Any]:
        """Create a new empty material asset."""
        return self.connection.send_command("create_material", {"name": name})
    
    def create_material_instance(
        self, 
        name: str, 
        parent_material: str,
        destination_path: str = "/Game/Materials/Instances/"
    ) -> Dict[str, Any]:
        """Create a new material instance asset."""
        params = {
            "name": name,
            "parent_material": parent_material,
            "destination_path": destination_path
        }
        return self.connection.send_command("create_material_instance", params)
    
    def create_material_function(
        self,
        name: str,
        path: str = None,
        description: str = None
    ) -> Dict[str, Any]:
        """Create a new material function asset."""
        params = {"name": name}
        if path:
            params["path"] = path
        if description:
            params["description"] = description
        return self.connection.send_command("create_material_function", params)
    
    def set_material_instance_parameter(
        self,
        material_instance: str,
        parameter_name: str,
        parameter_type: str,
        value: Any
    ) -> Dict[str, Any]:
        """Set a parameter in a material instance."""
        params = {
            "material_instance": material_instance,
            "parameter_name": parameter_name,
            "parameter_type": parameter_type,
            "value": value
        }
        return self.connection.send_command("set_material_instance_parameter", params)
    
    def set_material_properties(
        self,
        material_name: str,
        shading_model: str = None,
        blend_mode: str = None,
        two_sided: bool = None,
        material_domain: str = None
    ) -> Dict[str, Any]:
        """Set material properties."""
        params = {"material_name": material_name}
        if shading_model:
            params["shading_model"] = shading_model
        if blend_mode:
            params["blend_mode"] = blend_mode
        if two_sided is not None:
            params["two_sided"] = two_sided
        if material_domain:
            params["material_domain"] = material_domain
        return self.connection.send_command("set_material_properties", params)
    
    def get_material_expressions(self, material_name: str) -> Dict[str, Any]:
        """Get list of all expressions in a material."""
        return self.connection.send_command("get_material_expressions", {"material_name": material_name})
    
    def get_material_functions(
        self,
        path: str = "/Engine/Functions/",
        name_filter: str = None
    ) -> Dict[str, Any]:
        """Get list of Material Functions in a specified path."""
        params = {"path": path}
        if name_filter:
            params["name_filter"] = name_filter
        return self.connection.send_command("get_material_functions", params)
    
    def get_material_function_content(self, function_path: str) -> Dict[str, Any]:
        """Get detailed content of a Material Function."""
        return self.connection.send_command("get_material_function_content", {"function_path": function_path})
    
    def get_available_materials(self, search_path: str = "/Game/") -> Dict[str, Any]:
        """Get a list of available materials in the project."""
        return self.connection.send_command("get_available_materials", {"search_path": search_path})
    
    def add_material_expression(
        self,
        material_name: str,
        expression_type: str,
        pos_x: int = 0,
        pos_y: int = 0,
        **kwargs
    ) -> Dict[str, Any]:
        """Add a material expression node to a material."""
        params = {
            "material_name": material_name,
            "expression_type": expression_type,
            "pos_x": pos_x,
            "pos_y": pos_y
        }
        params.update(kwargs)
        return self.connection.send_command("add_material_expression", params)
    
    def connect_material_nodes(
        self,
        material_name: str,
        source_node: str,
        source_output: str = "Output",
        target_node: str = None,
        target_input: str = None
    ) -> Dict[str, Any]:
        """Connect material expression nodes."""
        params = {
            "material_name": material_name,
            "source_node": source_node,
            "source_output": source_output,
            "target_node": target_node,
            "target_input": target_input
        }
        return self.connection.send_command("connect_material_nodes", params)
