import time
from http.server import HTTPServer, BaseHTTPRequestHandler
from os import curdir, sep


class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        try:
            f = open(curdir + sep + self.path, "rb")
        except IOError:
            f = open(curdir + sep + "index.html", "rb")
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(f.read())
        f.close()

    def do_POST(self):
        content_length = int(self.headers["Content-Length"])
        content = self.rfile.read(content_length).decode("utf-8")
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(("{\"success\":\"%s\"}" % self.path).encode("utf-8"))


if __name__ == "__main__":
    httpd = HTTPServer(("", 80), SimpleHTTPRequestHandler)
    print(time.asctime(), "Server started")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    print(time.asctime(), "Server closed")
