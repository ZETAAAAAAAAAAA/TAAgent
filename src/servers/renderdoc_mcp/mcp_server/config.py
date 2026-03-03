"""Configuration for RenderDoc MCP Server"""

import os


class Settings:
    """Server settings"""

    def __init__(self):
        self.renderdoc_host = os.environ.get("RENDERDOC_MCP_HOST", "127.0.0.1")
        self.renderdoc_port = int(os.environ.get("RENDERDOC_MCP_PORT", "19876"))


settings = Settings()
