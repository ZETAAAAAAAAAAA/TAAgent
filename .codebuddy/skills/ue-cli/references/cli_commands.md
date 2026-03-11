# UE CLI Commands Reference

## Global Options

```bash
ue-cli [OPTIONS] COMMAND [ARGS]...

Options:
  --json          Output as JSON
  --project TEXT  Path to .ue-cli.json project file
  --help          Show help message
```

## Command Groups

### info
Information and status commands.

#### info status
Check UE connection status.

```bash
ue-cli info status
```

Output:
```json
{
  "ue_running": true,
  "ue_version": "UE_5.7",
  "temp_dir": "C:\\Users\\...\\Temp\\ue_cli"
}
```

#### info ping
Ping UE to verify connection.

```bash
ue-cli info ping
```

### level
Level management commands.

#### level info
Show current level information.

```bash
ue-cli level info
```

#### level open
Open a level by path.

```bash
ue-cli level open /Game/Maps/MyMap
```

#### level save
Save the current level.

```bash
ue-cli level save
```

### actor
Actor management commands.

#### actor list
List all actors in the current level.

```bash
ue-cli actor list
```

Output:
```json
[
  {
    "name": "PointLight_1",
    "class": "PointLight",
    "location": {"x": 0, "y": 0, "z": 0}
  }
]
```

#### actor spawn
Spawn a new actor in the level.

```bash
ue-cli actor spawn [OPTIONS]

Options:
  -t, --type TEXT       Actor class type  [default: StaticMeshActor]
  -n, --name TEXT       Actor name/label
  -l, --location TEXT   Location as x,y,z  [default: 0,0,0]
  -r, --rotation TEXT   Rotation as pitch,yaw,roll  [default: 0,0,0]
```

Examples:
```bash
# Spawn point light
ue-cli actor spawn -t PointLight -n MyLight -l 100,200,300

# Spawn camera
ue-cli actor spawn -t CameraActor -n MainCamera -l 0,0,200 -r -30,0,0

# Spawn static mesh
ue-cli actor spawn -t StaticMeshActor -n MyMesh -l 50,50,0
```

Available types:
- `StaticMeshActor`
- `PointLight`
- `DirectionalLight`
- `CameraActor`
- `SkyLight`
- `PlayerStart`

#### actor delete
Delete an actor by name.

```bash
ue-cli actor delete <name>
```

Example:
```bash
ue-cli actor delete MyLight
```

#### actor move
Move/rotate/scale an actor.

```bash
ue-cli actor move <name> [OPTIONS]

Options:
  -l, --location TEXT   New location as x,y,z
  -r, --rotation TEXT   New rotation as pitch,yaw,roll
  -s, --scale TEXT      New scale as x,y,z
```

Examples:
```bash
# Move actor
ue-cli actor move MyLight -l 200,300,400

# Rotate actor
ue-cli actor move MyLight -r 0,45,0

# Scale actor
ue-cli actor move MyMesh -s 2,2,2

# Combined
ue-cli actor move MyActor -l 100,200,300 -r 0,90,0 -s 1.5,1.5,1.5
```

### material
Material management commands.

#### material create
Create a new material.

```bash
ue-cli material create [OPTIONS]

Options:
  -n, --name TEXT      Material name  [required]
  -p, --path TEXT      Package path  [default: /Game/Materials]
  -c, --color TEXT     Base color as r,g,b,a  [default: 1,1,1,1]
```

Examples:
```bash
# Create red material
ue-cli material create -n RedMat --color 1,0,0,1

# Create blue material in custom path
ue-cli material create -n BlueMat -p /Game/MyMaterials --color 0,0,1,1
```

#### material apply
Apply a material to an actor.

```bash
ue-cli material apply <actor_name> <material_path>
```

Example:
```bash
ue-cli material apply MyMesh /Game/Materials/RedMat.RedMat
```

### asset
Asset management commands.

#### asset list
List assets in a path.

```bash
ue-cli asset list [OPTIONS]

Options:
  -p, --path TEXT   Asset path  [default: /Game]
```

Examples:
```bash
# List all assets
ue-cli asset list

# List materials
ue-cli asset list -p /Game/Materials
```

#### asset import
Import an asset from file system.

```bash
ue-cli asset import <source_path> <destination_path>
```

Example:
```bash
ue-cli asset import C:/model.fbx /Game/Models
```

### screenshot
Screenshot and rendering commands.

#### screenshot capture
Take a screenshot of the viewport.

```bash
ue-cli screenshot capture [OPTIONS]

Options:
  -o, --output TEXT       Output file path
  -r, --resolution TEXT   Resolution as width,height  [default: 1920,1080]
```

Examples:
```bash
# Default screenshot
ue-cli screenshot capture

# Custom path and resolution
ue-cli screenshot capture -o C:/screenshot.png -r 3840,2160
```

### python
Python script execution commands.

#### python exec
Execute Python code in UE.

```bash
ue-cli python exec <code>
```

Example:
```bash
ue-cli python exec "unreal.log('Hello from CLI')"
```

#### python file
Execute a Python file in UE.

```bash
ue-cli python file <file_path>
```

Example:
```bash
ue-cli python file C:/script.py
```

### session
Session management commands.

#### session status
Show session status.

```bash
ue-cli session status
```

#### session undo
Undo last operation.

```bash
ue-cli session undo
```

#### session redo
Redo last undone operation.

```bash
ue-cli session redo
```

### repl
Start interactive REPL mode.

```bash
ue-cli repl
```

In REPL mode:
- Type commands directly without `ue-cli` prefix
- Type `help` for available commands
- Type `exit` or `quit` to exit

## JSON Output

Add `--json` flag to any command for machine-readable output:

```bash
ue-cli --json actor list
```

Output:
```json
{
  "actors": [
    {
      "name": "PointLight_1",
      "class": "PointLight",
      "location": {"x": 0, "y": 0, "z": 0}
    }
  ]
}
```

## Error Handling

Commands return consistent error format:

```json
{
  "success": false,
  "error": "Actor not found: MyActor",
  "command_id": "cmd_123456",
  "command_type": "delete_actor"
}
```

## Environment Variables

- `UE_EDITOR_PATH` - Path to UnrealEditor.exe (optional)
- `TEMP` - Temp directory for command files

## Exit Codes

- `0` - Success
- `1` - Error (with --json: check `success` field)
