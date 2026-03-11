#!/usr/bin/env python3
"""Initialize UE CLI environment.

This script sets up the UE CLI environment:
1. Installs the CLI package
2. Checks UE installation
3. Generates listener scripts if needed

Usage:
    python init_ue_cli.py
"""

import os
import sys
import subprocess
from pathlib import Path


def find_ue_editor():
    """Find Unreal Editor installation."""
    paths = [
        r"E:\UE\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
        r"E:\UE\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe",
        r"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
        r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe",
    ]
    
    for path in paths:
        if os.path.exists(path):
            return path
    
    return None


def install_cli():
    """Install UE CLI package."""
    agent_harness = Path(__file__).parent.parent.parent.parent / "unreal" / "agent-harness"
    
    if not agent_harness.exists():
        print(f"Error: Cannot find agent-harness at {agent_harness}")
        return False
    
    print(f"Installing UE CLI from {agent_harness}...")
    result = subprocess.run(
        [sys.executable, "-m", "pip", "install", "-e", str(agent_harness)],
        capture_output=True,
        text=True
    )
    
    if result.returncode == 0:
        print("✓ UE CLI installed successfully")
        return True
    else:
        print(f"Error installing CLI: {result.stderr}")
        return False


def check_ue():
    """Check UE installation."""
    ue_path = find_ue_editor()
    
    if ue_path:
        print(f"✓ Found Unreal Editor: {ue_path}")
        return True
    else:
        print("⚠ Unreal Editor not found in common locations")
        print("  Set UE_EDITOR_PATH environment variable if needed")
        return False


def print_usage():
    """Print usage instructions."""
    print("\n" + "="*60)
    print("UE CLI Setup Complete!")
    print("="*60)
    print("\nNext steps:")
    print("1. Open Unreal Editor")
    print("2. Open Python Console (Window -> Developer Tools -> Python Console)")
    print("3. Run:")
    print('   exec(open(r"D:/CodeBuddy/rendering-mcp/unreal/agent-harness/ue_cli_listener_auto.py").read())')
    print("4. Run:")
    print("   start_ue_cli()")
    print("\n5. In terminal, test CLI:")
    print("   ue-cli info ping")
    print("   ue-cli actor list")
    print("\n" + "="*60)


def main():
    """Main entry point."""
    print("Initializing UE CLI...")
    print("="*60)
    
    # Install CLI
    if not install_cli():
        return 1
    
    # Check UE
    check_ue()
    
    # Print usage
    print_usage()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
