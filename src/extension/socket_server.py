"""
File-based IPC Server for RenderDoc MCP Bridge
Uses file polling since RenderDoc's Python doesn't have socket/QtNetwork modules.
"""

import json
import os
import traceback
import tempfile

from PySide2.QtCore import QObject, QTimer


# IPC directory
IPC_DIR = os.path.join(tempfile.gettempdir(), "renderdoc_mcp")
REQUEST_FILE = os.path.join(IPC_DIR, "request.json")
RESPONSE_FILE = os.path.join(IPC_DIR, "response.json")
LOCK_FILE = os.path.join(IPC_DIR, "lock")


class MCPBridgeServer(QObject):
    """File-based IPC server for MCP bridge communication"""

    def __init__(self, host, port, handler, parent=None):
        super(MCPBridgeServer, self).__init__(parent)
        self.handler = handler
        self._timer = None
        self._running = False

        # Create IPC directory
        if not os.path.exists(IPC_DIR):
            os.makedirs(IPC_DIR)

    def start(self):
        """Start the server with polling"""
        self._running = True

        # Clean up old files
        self._cleanup_files()

        # Start polling timer (check every 100ms)
        self._timer = QTimer(self)
        self._timer.timeout.connect(self._poll_request)
        self._timer.start(100)

        print("[MCP Bridge] File-based IPC server started")
        print("[MCP Bridge] IPC directory: %s" % IPC_DIR)
        return True

    def stop(self):
        """Stop the server"""
        self._running = False
        if self._timer:
            self._timer.stop()
            self._timer = None
        self._cleanup_files()
        print("[MCP Bridge] Server stopped")

    def is_running(self):
        """Check if server is running"""
        return self._running

    def _cleanup_files(self):
        """Remove IPC files"""
        for f in [REQUEST_FILE, RESPONSE_FILE, LOCK_FILE]:
            try:
                if os.path.exists(f):
                    os.remove(f)
            except Exception:
                pass

    def _poll_request(self):
        """Check for incoming request"""
        if not self._running:
            return

        # Check if request file exists
        if not os.path.exists(REQUEST_FILE):
            return

        # Check if lock file exists (client is still writing)
        if os.path.exists(LOCK_FILE):
            return

        try:
            # Read request
            with open(REQUEST_FILE, "r", encoding="utf-8") as f:
                request = json.load(f)

            # Remove request file
            os.remove(REQUEST_FILE)

            # Process request
            try:
                response = self.handler.handle(request)
            except Exception as e:
                traceback.print_exc()
                response = {
                    "id": request.get("id"),
                    "error": {"code": -32603, "message": str(e)}
                }

            # Write response
            with open(RESPONSE_FILE, "w", encoding="utf-8") as f:
                json.dump(response, f)

        except Exception as e:
            print("[MCP Bridge] Error processing request: %s" % str(e))
            traceback.print_exc()
