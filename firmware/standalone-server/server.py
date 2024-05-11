# Python 3 server example
from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import sys
import json

hostName = "172.20.10.2"
serverPort = 8080



class MyServer(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/test':
            print("test print")
        if self.path == "/update_config":
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", "10")
            self.send_header("Random-Header", "This is a header");
            self.end_headers()
            self.wfile.write(bytes("{\"test\":8}", "utf-8"))

        if self.path == "/":
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(bytes("<title>Hello World</title><p>Hello World</p>", "utf-8"))
    
    def do_PUT(self):
        if self.path == '/upload_image': # check current header
            print("upload-image")
            if self.headers.get('Content-Type') == 'image/jpeg': # receiving a jpeg
                try:
                    content_len = int(self.headers.get('Content-Length', 0))
                except TypeError: # content-length has not been set
                    pass # TODO: sent bad response
                    print("Problem with Content-Length")
                try: 
                    savename = self.headers.get('Photo-Name', "defaultname")
                except TypeError:
                    pass
                    print("Problem with Photo-Name")

                imgfile = self.rfile.read(content_len)

                # Save image to file
                save_name = 'received/' + savename  # filename to store the data
                with open(save_name, 'wb') as f:
                    f.write(imgfile)
                self.send_response(200)
                self.send_header('Content-type', 'text/plain')
                self.end_headers()
                self.wfile.write("great")
        if self.path == '/metadata': # check current header
            print("metadata")
            if self.headers.get('Content-Type') == 'application/json': # receiving a jpeg
                try:
                    content_len = int(self.headers.get('Content-Length', 0))
                except TypeError: # content-length has not been set
                    pass # TODO: sent bad response
                    print("Problem with Content-Length")

                receivedfile = self.rfile.read(content_len)

                # Save image to file
                save_name = 'received/' + savename  # filename to store the data
                with open(save_name, 'wb') as f:
                    f.write(receivedfile)

                self.send_response(200)
                self.send_header('Content-type', 'text/plain')
                self.end_headers()
                self.wfile.write("great")
         
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