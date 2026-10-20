# fake_server.py
import http.server
import zstandard as zstd

class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        d = zstd.ZstdCompressionDict(open("dict_32k.zst","rb").read())
        cctx = zstd.ZstdCompressor(level=3, dict_data=d)
        html = b"<html><body><h1>Hello from curl + zstdict!</h1><p>This is real HTTP compression.</p></body></html>"
        packed = cctx.compress(html)
        
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.send_header("Content-Encoding", "zstdict")
        self.send_header("Content-Length", str(len(packed)))
        self.send_header("Content-Dict", "sha256-89ff6702e9d396f01bdf8865451521d2ba23efce9c5968c784b85744d6e55faf")
        self.end_headers()
        self.wfile.write(packed)

http.server.HTTPServer(('127.0.0.1', 8080), Handler).serve_forever()
