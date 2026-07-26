#!/usr/bin/env python3
"""
Serve the Vista SDK Gmod Explorer WebAssembly app.

WebAssembly requires specific MIME types and cross-origin isolation headers
(COOP/COEP) for SharedArrayBuffer support. This server sets them correctly.

Usage:
    python serve.py [build_dir] [port]

    build_dir  Path to the CMake build output directory containing gmod-explorer.*
               Default: ../../../build/bin (relative to this script)
    port       Port to listen on. Default: 8080

Then open http://localhost:8080/gmod-explorer.html in your browser.
"""

import http.server
import os
import sys


MIME_TYPES = {
    ".wasm": "application/wasm",
    ".js":   "application/javascript",
    ".html": "text/html",
    ".data": "application/octet-stream",
}


class Handler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Required for SharedArrayBuffer (Emscripten threading, Atomics.wait)
        self.send_header("Cross-Origin-Opener-Policy",   "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        super().end_headers()

    def guess_type(self, path):
        _, ext = os.path.splitext(path)
        return MIME_TYPES.get(ext.lower(), super().guess_type(path))

    def log_message(self, fmt, *args):
        print(fmt % args)


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir  = sys.argv[1] if len(sys.argv) > 1 else os.path.join(script_dir, "..", "..", "..", "build", "bin")
    port       = int(sys.argv[2]) if len(sys.argv) > 2 else 8080

    build_dir = os.path.abspath(build_dir)
    if not os.path.isdir(build_dir):
        print(f"error: directory not found: {build_dir}", file=sys.stderr)
        sys.exit(1)

    os.chdir(build_dir)
    print(f"Serving {build_dir} on http://localhost:{port}/gmod-explorer.html")
    print("Press Ctrl+C to stop.")

    with http.server.HTTPServer(("", port), Handler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nStopped.")


if __name__ == "__main__":
    main()
