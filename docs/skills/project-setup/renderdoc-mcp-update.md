# RenderDocMCP Update Guide

## Complete Update Workflow

After modifying any code in `renderdoc_extension/` or `mcp_server/`, follow these steps:

### Step 1: Kill RenderDoc Process
```bash
# Windows CMD
taskkill /F /IM qrenderdoc.exe
taskkill /F /IM renderdoccmd.exe

# Windows PowerShell (use cmd /c for batch commands)
taskkill /F /IM qrenderdoc.exe 2>$null
taskkill /F /IM renderdoccmd.exe 2>$null
```

### Step 2: Run Install Script
```bash
# CMD
cd d:/CodeBuddy/rendering-mcp
python scripts/renderdoc/install_extension.py

# PowerShell
Set-Location d:\CodeBuddy\rendering-mcp
python scripts/renderdoc/install_extension.py
```

### Step 3: Start RenderDoc
```bash
# Option A: Start with a capture file (CMD)
start "" "path/to/capture.rdc"

# Option A: Start with a capture file (PowerShell)
Start-Process "path/to/capture.rdc"

# Option B: Start RenderDoc normally (CMD)
start qrenderdoc

# Option B: Start RenderDoc normally (PowerShell)
Start-Process qrenderdoc
```

### Step 4: Verify Extension Loaded
1. Go to `Tools > Manage Extensions`
2. Ensure "RenderDoc MCP Bridge" is enabled
3. Check MCP connection works

---

## Files to Modify When Adding/Removing Tools

When adding or removing a tool, you MUST update **ALL** of these files:

| File | What to Update |
|------|----------------|
| `servers/renderdoc_mcp/mcp_server/server.py` | Add/remove `@mcp.tool()` function |
| `renderdoc_extension/request_handler.py` | Add/remove method in `_methods` dict and handler function |
| `renderdoc_extension/renderdoc_facade.py` | Add/remove facade method |
| `renderdoc_extension/services/*.py` | Add/remove actual implementation |

### Example: Adding a New Tool

1. **`services/resource_service.py`** - Add implementation:
```python
def new_tool(self, param1, param2):
    """New tool implementation"""
    # ... implementation
    return result
```

2. **`renderdoc_facade.py`** - Add facade method:
```python
def new_tool(self, param1, param2):
    """New tool facade"""
    return self._resource.new_tool(param1, param2)
```

3. **`request_handler.py`** - Add handler:
```python
# In __init__._methods dict:
"new_tool": self._handle_new_tool,

# Handler function:
def _handle_new_tool(self, params):
    param1 = params.get("param1")
    param2 = params.get("param2")
    return self.facade.new_tool(param1, param2)
```

4. **`mcp_server/server.py`** - Add MCP tool:
```python
@mcp.tool(output_schema=None)
def new_tool(param1: str, param2: str) -> dict:
    """Tool description"""
    result = bridge.call("new_tool", {"param1": param1, "param2": param2})
    return result
```

### Example: Removing a Tool

Remove the tool from **ALL** files in reverse order:
1. `mcp_server/server.py` - Remove `@mcp.tool()` function
2. `request_handler.py` - Remove from `_methods` dict and handler function
3. `renderdoc_facade.py` - Remove facade method
4. `services/*.py` - Remove implementation

---

## Current Tools (After Updates)

| Tool | Description |
|------|-------------|
| `get_capture_status` | Check if capture is loaded |
| `get_draw_calls` | Get all draw calls/actions |
| `get_frame_summary` | Get frame statistics |
| `find_draws_by_shader` | Find draws using a shader |
| `find_draws_by_texture` | Find draws using a texture |
| `find_draws_by_resource` | Find draws using a resource ID |
| `get_draw_call_details` | Get details of a draw call |
| `get_action_timings` | Get GPU timing info |
| `get_shader_info` | Get shader information |
| `get_buffer_contents` | Read buffer data |
| `get_texture_info` | Get texture metadata |
| `get_texture_data` | Get texture pixel data |
| `save_texture` | Save texture to PNG/TGA/DDS/JPG/HDR/BMP/EXR |
| `get_pipeline_state` | Get full pipeline state |
| `list_captures` | List .rdc files in directory |
| `open_capture` | Open a capture file |
| `get_mesh_data` | Get mesh data for draw call |
| `export_mesh_as_fbx` | Export mesh as FBX file |
| `export_mesh_json` | Export mesh as JSON for UE import |

---

## Troubleshooting

### PowerShell vs CMD Environment
The default shell may be PowerShell, but many commands are CMD batch format.

**Common Issues in PowerShell**:
- `"文件名、目录名或卷标语法不正确"` - Quote handling differs
- Command fails silently - Need `cmd /c` wrapper for batch commands

**Solution**:
```powershell
# Wrap CMD commands with cmd /c
cmd /c "your_batch_command"

# Or use PowerShell equivalents
Start-Process "path/to/file.rdc"  # instead of start ""
Set-Location "path"               # instead of cd
```

### "Method not found" Error
- Extension was not reinstalled after code changes
- Run `python scripts/renderdoc/install_extension.py` and restart RenderDoc

### Extension Not Loading
1. Check `%APPDATA%/qrenderdoc/extensions/renderdoc_mcp_bridge/` exists
2. Enable in `Tools > Manage Extensions`
3. Check RenderDoc's Python console for errors

### Old Code Still Running
- RenderDoc caches Python modules
- Must completely kill and restart RenderDoc process
- `open_capture` does NOT reload the extension
