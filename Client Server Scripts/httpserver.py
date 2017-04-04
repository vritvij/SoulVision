#!/usr/bin/env python
 
from http.server import BaseHTTPRequestHandler, HTTPServer
import json as simplejson
 
# HTTPRequestHandler class
class testHTTPServer_RequestHandler(BaseHTTPRequestHandler):
 
  # GET
  def do_GET(self):
        # Send response status code
        self.send_response(200)
 
        # Send headers
        self.send_header('Content-type','text/html')
        self.end_headers()
 
        # Send message back to client
        message = "Hello world!"
        # Write content as utf-8 data
        self.wfile.write(bytes(message, "utf8"))
        return
		
  # POST
  def do_POST(self):
        # Send response status code
        self.send_response(200)
 
        # Send headers
        self.send_header('Content-type','text/html')
        self.end_headers()
        
        # Read JSON Object from request headers
        self.data_string = self.rfile.read(int(self.headers['Content-Length']))
        data = simplejson.loads(self.data_string.decode())
        print ("{}".format(data))
 
        # Send message back to client
        message = "Hello world in POST!"
        data = {"user2_proximity": 3, "Wifi_1": -80, "Wifi_2": -40, "Wifi_3": -40, \
                "thermostat": 18, "light": 0, "hour_of_day": 0, "user3_proximity": 3, \
                "user1_proximity": 1, "day_of_week": 1, "security": 0, "minute_of_hour": 9, \
                "Act_1": 1, "Act_2": 0, "Act_3": 0}

        json_data = simplejson.dumps(data)
        # Write content as utf-8 data
        self.wfile.write(bytes(json_data, "utf8"))
        return
 
def run():
  print('starting server...')
 
  # Server settings
  # Choose port 8080, for port 80, which is normally used for a http server, you need root access
  server_address = ('127.0.0.1', 8081)
  httpd = HTTPServer(server_address, testHTTPServer_RequestHandler)
  print('running server...')
  httpd.serve_forever()
 
 
run()