"""
RenderDoc Extension Installer
Copies the extension to RenderDoc's extension directory.
"""

import os
import shutil
import sys
from pathlib import Path


def get_extension_dir():
    """Get RenderDoc extension directory"""
    if sys.platform == "win32":
        appdata = os.environ.get("APPDATA")
        if appdata:
            return Path(appdata) / "qrenderdoc" / "extensions"
    else:
        home = Path.home()
        return home / ".local" / "share" / "qrenderdoc" / "extensions"

    raise RuntimeError("Cannot determine RenderDoc extension directory")


def install():
    """Install the extension"""
    # Source directory - project root is parent of scripts/
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent  # Go up from scripts/renderdoc to rendering-mcp
    extension_src = project_root / "renderdoc_extension"

    if not extension_src.exists():
        print("Error: Extension source not found at %s" % extension_src)
        sys.exit(1)

    # Destination directory
    ext_dir = get_extension_dir()
    ext_dir.mkdir(parents=True, exist_ok=True)

    dest = ext_dir / "renderdoc_mcp_bridge"

    # Remove existing installation
    if dest.exists():
        print("Removing existing installation at %s" % dest)
        shutil.rmtree(dest)

    # Copy extension (excluding __pycache__)
    shutil.copytree(
        extension_src,
        dest,
        ignore=shutil.ignore_patterns("__pycache__", "*.pyc", "*.pyo"),
    )
    print("Extension installed to %s" % dest)
    print("  (__pycache__ directories excluded)")
    print("")
    print("Please restart RenderDoc and enable the extension in:")
    print("  Tools > Manage Extensions > RenderDoc MCP Bridge")


def uninstall():
    """Uninstall the extension"""
    ext_dir = get_extension_dir()
    dest = ext_dir / "renderdoc_mcp_bridge"

    if dest.exists():
        shutil.rmtree(dest)
        print("Extension uninstalled from %s" % dest)
    else:
        print("Extension not found at %s" % dest)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "uninstall":
        uninstall()
    else:
        install()
