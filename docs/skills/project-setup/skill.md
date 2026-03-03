# Project Setup

Setup and maintenance guide for the Rendering MCP project.

## Description

This skill provides instructions for setting up, building, and maintaining the Rendering MCP project, including the RenderDoc extension and Unreal MCP plugin.

## Capabilities

- Initial project setup and configuration
- Build and compile RenderDoc extension
- Install and configure Unreal MCP plugin
- Update dependencies and components
- Troubleshoot common setup issues

## Files

| File | Description |
|------|-------------|
| `update-and-build.md` | Complete update and build guide |
| `renderdoc-mcp-update.md` | RenderDoc MCP specific update instructions |

## Prerequisites

- Python 3.10+
- C++ compiler (MSVC on Windows)
- Unreal Engine 5.x
- RenderDoc installed

## Quick Start

```
1. Clone repository
2. Install Python dependencies: pip install -e .
3. Configure MCP server in Claude Desktop
4. Build Unreal MCP plugin (if needed)
5. Test connection to RenderDoc and UE
```

## Key Configuration Files

| File | Purpose |
|------|---------|
| `mcp_config.example.json` | MCP server configuration template |
| `pyproject.toml` | Python project dependencies |
| `renderdoc_extension/extension.json` | RenderDoc extension config |

## Common Issues

| Issue | Solution |
|-------|----------|
| MCP connection failed | Check config path and server status |
| RenderDoc extension not loading | Verify extension path in RenderDoc settings |
| UE plugin not working | Rebuild plugin for UE version |
| Import errors | Reinstall: `pip install -e .` |

## Related Skills

All other skills depend on proper project setup.
