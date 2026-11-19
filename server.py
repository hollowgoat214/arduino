import json, time, uuid
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
feeds = {}
class H(BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def send(self, code, body, ct="application/json"):
        b = body.encode() if isinstance(body, str) else body
        self.send_response(code); self.send_header("Content-Type", ct)
        self.send_header("Content-Length", str(len(b))); self.end_headers(); self.wfile.write(b)
    def do_GET(self):
        p = self.path.split("?")[0]
        if p in ("/", "/index.html"):
            return self.send(200, open("index.html", "rb").read(), "text/html")
        if p.startswith("/api/feeds/") and p.endswith("/data/last"):
            d = feeds.get(p.split("/")[3])
            return self.send(200, json.dumps(d)) if d else self.send(404, "{}")
        if p.startswith("/raw/"):  # plain text for the Arduino
            d = feeds.get(p.split("/")[2])
            return self.send(200, f"{d['id']}|{d['value']}" if d else "none|", "text/plain")
        self.send(404, "{}")
    def do_POST(self):
        p = self.path.split("?")[0]
        n = int(self.headers.get("Content-Length", 0)); raw = self.rfile.read(n).decode()
        try: v = json.loads(raw)["value"]
        except Exception: v = raw.strip()
        f = p.split("/")[3] if p.startswith("/api/") else p.split("/")[2]
        feeds[f] = {"id": uuid.uuid4().hex[:8], "value": str(v), "created_at": time.strftime("%Y-%m-%dT%H:%M:%S%z")}
        print(f, v, flush=True)
        self.send(200, json.dumps(feeds[f]))
ThreadingHTTPServer(("0.0.0.0", 8000), H).serve_forever()
