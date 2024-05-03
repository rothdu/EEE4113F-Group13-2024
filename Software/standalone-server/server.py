# Python 3 server example
from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import sys

hostName = "192.168.1.20"
serverPort = 8080

class MyServer(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/test':
            print("test print")
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(bytes("<html><head><title>https://pythonbasics.org</title></head>", "utf-8"))
        self.wfile.write(bytes("<p>Request: %s</p>" % self.path, "utf-8"))
        self.wfile.write(bytes("<body>", "utf-8"))
        self.wfile.write(bytes("<p>This is an example web server.</p>", "utf-8"))
        self.wfile.write(bytes("</body></html>", "utf-8"))
    
    def do_POST(self):
        if self.path == '/upload_image': # check current header
            if self.headers.get('Content-Type') == 'image/jpeg': # receiving a jpeg
                try:
                    content_len = int(self.headers.get('content-length', 0))
                except TypeError: # content-length has not been set
                    pass # TODO: sent bad response

                imgfile = self.rfile.read(content_len)

                # Save image to file
                save_name = 'images/received_image2.jpg'  # filename to store the data
                with open(save_name, 'wb') as f:
                    f.write(imgfile)
                self.send_response(200)
            else:
                print(self.headers.get('Content-Type'))
         
def main():
    # create webserver
    webServer = HTTPServer((hostName, serverPort), MyServer)

    # debug - print server created
    print("Server started http://%s:%s" % (hostName, serverPort))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt: # cancel on keyboard interrupt
        pass

    webServer.server_close() # close server
    print("Server stopped.")

if __name__ == "__main__": 
    main()