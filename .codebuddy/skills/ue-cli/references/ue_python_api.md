# Unreal Engine Python API Reference

## EditorLevelLibrary

### Get All Actors
```python
actors = unreal.EditorLevelLibrary.get_all_level_actors()
for actor in actors:
    print(actor.get_name())
```

### Spawn Actor
```python
location = unreal.Vector(0, 0, 0)
rotation = unreal.Rotator(0, 0, 0)
actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor, location, rotation
)
```

### Destroy Actor
```python
unreal.EditorLevelLibrary.destroy_actor(actor)
```

### Save Level
```python
unreal.EditorLevelLibrary.save_current_level()
```

### Load Level
```python
unreal.EditorLevelLibrary.load_level("/Game/Maps/MyMap")
```

## EditorAssetLibrary

### List Assets
```python
assets = unreal.EditorAssetLibrary.list_assets("/Game", recursive=True)
for asset in assets:
    print(asset.asset_name, asset.object_path)
```

### Load Asset
```python
asset = unreal.EditorAssetLibrary.load_asset("/Game/Materials/MyMat")
```

### Save Asset
```python
unreal.EditorAssetLibrary.save_asset("/Game/Materials/MyMat")
```

## AssetTools

### Create Asset
```python
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
factory = unreal.MaterialFactoryNew()
material = asset_tools.create_asset(
    asset_name="MyMat",
    package_path="/Game/Materials",
    asset_class=unreal.Material,
    factory=factory
)
```

### Import Asset
```python
task = unreal.AssetImportTask()
task.set_editor_property('filename', "C:/model.fbx")
task.set_editor_property('destination_path', "/Game/Models")
task.set_editor_property('replace_existing', True)
task.set_editor_property('automated', True)

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks([task])
```

## Actor Operations

### Transform
```python
# Location
actor.set_actor_location(unreal.Vector(x, y, z), sweep=False, teleport=False)
location = actor.get_actor_location()

# Rotation
actor.set_actor_rotation(unreal.Rotator(pitch, yaw, roll), teleport=False)
rotation = actor.get_actor_rotation()

# Scale
actor.set_actor_scale3d(unreal.Vector(x, y, z))
scale = actor.get_actor_scale3d()
```

### Components
```python
# Get component
mesh_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
light_comp = actor.get_component_by_class(unreal.PointLightComponent)

# Set material
mesh_comp.set_material(0, material)
```

### Properties
```python
# Name
actor.set_actor_label("MyActor")
name = actor.get_name()
label = actor.get_actor_label()

# Class
class_name = actor.get_class().get_name()
```

## Material Operations

### Create Material
```python
factory = unreal.MaterialFactoryNew()
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
material = asset_tools.create_asset(
    asset_name="NewMaterial",
    package_path="/Game/Materials",
    asset_class=unreal.Material,
    factory=factory
)
```

### Edit Material
```python
# Get base color node
base_color = unreal.MaterialEditingLibrary.get_material_property_input_node(
    material, unreal.MaterialProperty.MP_BASE_COLOR
)

# Create constant node
constant = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant4Vector
)
constant.set_editor_property("Constant", unreal.LinearColor(1, 0, 0, 1))

# Connect to base color
unreal.MaterialEditingLibrary.connect_material_property(
    constant, "", unreal.MaterialProperty.MP_BASE_COLOR
)

# Update material
unreal.MaterialEditingLibrary.update_material_instance(material)
```

## Screenshot

### High Resolution Screenshot
```python
unreal.SystemLibrary.execute_console_command(
    None,
    "HighResShot 1 filename=C:/screenshot.png"
)
```

### Capture from Camera
```python
camera = unreal.EditorLevelLibrary.get_actor_reference("CameraActor_1")
unreal.GameplayStatics.get_player_controller(None, 0).set_view_target_with_blend(camera)
```

## Utility Functions

### Console Commands
```python
unreal.SystemLibrary.execute_console_command(None, "stat fps")
unreal.SystemLibrary.execute_console_command(None, "show collision")
```

### Logging
```python
unreal.log("Message")
unreal.log_warning("Warning")
unreal.log_error("Error")
```

### Paths
```python
project_dir = unreal.Paths.project_dir()
content_dir = unreal.Paths.project_content_dir()
```

## Common Classes

### Actor Classes
- `unreal.StaticMeshActor`
- `unreal.PointLight`
- `unreal.DirectionalLight`
- `unreal.SkyLight`
- `unreal.CameraActor`
- `unreal.PlayerStart`
- `unreal.TriggerBox`

### Component Classes
- `unreal.StaticMeshComponent`
- `unreal.PointLightComponent`
- `unreal.DirectionalLightComponent`
- `unreal.CameraComponent`

### Asset Classes
- `unreal.Material`
- `unreal.StaticMesh`
- `unreal.Texture2D`
- `unreal.Blueprint`
