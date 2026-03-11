---
name: ue-cli
description: |
  This skill provides comprehensive knowledge for building and using a command-line interface (CLI) 
  for Unreal Engine 5. It enables file-based communication between external Python scripts and 
  a running UE5 editor instance via Python Script Plugin. Use this skill when:
  - Creating UE5 automation tools or pipelines
  - Building CLI tools to control UE5 from the terminal
  - Implementing file-based inter-process communication with UE5
  - Developing agent-native interfaces for Unreal Engine
---

# Unreal Engine CLI Skill

## Overview

This skill enables building a command-line interface for Unreal Engine 5 using **file-based communication** - no network ports or MCP required.

### Architecture

```
┌─────────────┐     command.json      ┌─────────────┐
│  CLI Tool   │ ────────────────────> │  UE Python  │
│  (Python)   │                       │  Listener   │
│             │ <──────────────────── │  (in UE)    │
│             │     result.json       │             │
└─────────────┘                       └─────────────┘
```

### Key Features

- **No external dependencies**: Uses only UE's built-in Python Script Plugin
- **Auto-reload**: Listener automatically reloads when script changes
- **Non-blocking**: Uses UE's Slate tick callback, doesn't freeze the editor
- **Full feature set**: Actor management, materials, screenshots, asset operations

## Quick Start

### 1. Installation

```bash
cd unreal/agent-harness
pip install -e .
```

### 2. Start UE Listener

In Unreal Editor Python Console:

```python
# Auto-reload version (recommended)
exec(open(r"D:/CodeBuddy/rendering-mcp/unreal/agent-harness/ue_cli_listener_auto.py").read())
start_ue_cli()
```

### 3. Use CLI

```bash
# Check connection
ue-cli info ping

# List actors
ue-cli actor list

# Spawn actor
ue-cli actor spawn -t PointLight -n MyLight -l 100,200,300

# Create material
ue-cli material create -n RedMat --color 1,0,0,1

# Take screenshot
ue-cli screenshot capture -o C:/screenshot.png
```

## CLI Commands Reference

### Info Commands
- `ue-cli info status` - Check UE connection status
- `ue-cli info ping` - Ping UE to verify connection

### Level Commands
- `ue-cli level info` - Show current level information
- `ue-cli level open <path>` - Open a level
- `ue-cli level save` - Save current level

### Actor Commands
- `ue-cli actor list` - List all actors
- `ue-cli actor spawn -t <type> -n <name> -l <x,y,z>` - Spawn actor
  - Types: StaticMeshActor, PointLight, DirectionalLight, CameraActor, SkyLight, PlayerStart
- `ue-cli actor delete <name>` - Delete actor
- `ue-cli actor move <name> -l <x,y,z> -r <p,y,r> -s <x,y,z>` - Transform actor

### Material Commands
- `ue-cli material create -n <name> -p <path> --color <r,g,b,a>` - Create material
- `ue-cli material apply <actor> <material_path>` - Apply material to actor

### Asset Commands
- `ue-cli asset list -p <path>` - List assets
- `ue-cli asset import <source> <dest>` - Import asset

### Screenshot Commands
- `ue-cli screenshot capture -o <path> -r <w,h>` - Take viewport screenshot

### Python Commands
- `ue-cli python exec <code>` - Execute Python code in UE
- `ue-cli python file <path>` - Execute Python file

### Session Commands
- `ue-cli session status` - Show session status
- `ue-cli session undo` - Undo last operation
- `ue-cli session redo` - Redo last operation

### Interactive Mode
- `ue-cli repl` - Start interactive REPL

## Implementation Details

### File Communication Protocol

**Command File**: `%TEMP%/ue_cli/command.json`
```json
{
  "id": "cmd_123456",
  "type": "spawn_actor",
  "params": {
    "actor_class": "PointLight",
    "name": "MyLight",
    "location": [100, 200, 300]
  }
}
```

**Result File**: `%TEMP%/ue_cli/result.json`
```json
{
  "command_id": "cmd_123456",
  "success": true,
  "actor": {
    "name": "PointLight_1",
    "label": "MyLight"
  }
}
```

### UE Python API Reference

Key UE Python modules used:

```python
import unreal

# Level operations
unreal.EditorLevelLibrary.get_all_level_actors()
unreal.EditorLevelLibrary.spawn_actor_from_class(class, location, rotation)
unreal.EditorLevelLibrary.destroy_actor(actor)
unreal.EditorLevelLibrary.save_current_level()

# Asset operations
unreal.EditorAssetLibrary.list_assets(path, recursive=True)
unreal.EditorAssetLibrary.load_asset(path)
unreal.AssetToolsHelpers.get_asset_tools()

# Actor operations
actor.set_actor_location(unreal.Vector(x, y, z), sweep, teleport)
actor.set_actor_rotation(unreal.Rotator(pitch, yaw, roll), teleport)
actor.set_actor_scale3d(unreal.Vector(x, y, z))
actor.get_component_by_class(unreal.StaticMeshComponent)

# Screenshot
unreal.SystemLibrary.execute_console_command(world, command)
```

### Auto-Reload Mechanism

The auto-reload version (`ue_cli_listener_auto.py`) uses:

```python
# Check file modification time
if current_modified > last_modified:
    reload_module()

# Register UE tick callback
unreal.register_slate_post_tick_callback(tick_function)
```

## Project Structure

```
unreal/agent-harness/
├── cli_anything/
│   └── unreal/
│       ├── unreal_cli.py          # Main CLI entry
│       ├── core/
│       │   └── session.py         # Session management
│       └── utils/
│           └── ue_backend.py      # File communication backend
├── ue_cli_listener.py             # UE listener (manual)
├── ue_cli_listener_auto.py        # UE listener (auto-reload)
├── setup.py                       # Installation script
└── README.md                      # User documentation
```

## Troubleshooting

### "Timeout waiting for result"
- Ensure listener is running in UE Python Console
- Check that `start_ue_cli()` was called
- Verify Python Script Plugin is enabled

### "UE not running"
- Check UE Editor is open
- Set `UE_EDITOR_PATH` environment variable if needed

### Commands not executing
- Check UE Output Log for Python errors
- Ensure listener script has no syntax errors
- Try manual reload: `reload_ue_cli()`

## Extension Guide

To add new commands:

1. **Add handler in `ue_cli_listener.py`**:
```python
def my_command(params: dict) -> dict:
    try:
        # Implementation
        return {"success": True, "data": result}
    except Exception as e:
        return {"success": False, "error": str(e)}

COMMANDS["my_command"] = my_command
```

2. **Add CLI command in `unreal_cli.py`**:
```python
@cli.group()
def mygroup():
    """My command group."""
    pass

@mygroup.command("action")
@click.option("--param", "-p", help="Parameter")
def mygroup_action(param):
    """Command description."""
    result = ue_backend.execute_command("my_command", {"param": param})
    output(result)
```

3. **Add backend method in `ue_backend.py`**:
```python
def my_command(param: str) -> dict:
    return execute_command("my_command", {"param": param})
```

## References

- See `references/` directory for detailed API documentation
- See `scripts/` directory for utility scripts
