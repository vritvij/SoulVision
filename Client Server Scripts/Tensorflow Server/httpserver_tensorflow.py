#!/usr/bin/env python
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
 
from http.server import BaseHTTPRequestHandler, HTTPServer
import json as simplejson

import tensorflow as tf
import json
import imp
import operator
import argparse
import os.path
import sys
import time
import numpy
import datetime

import model
 
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
        time1 = datetime.datetime.now()
        run_graph()
        print('Time taken to run graph', datetime.datetime.now() - time1)
        return
 
def run():
  print('starting server...')
 
  # Server settings
  # Choose port 8080, for port 80, which is normally used for a http server, you need root access
  server_address = ('127.0.0.1', 8081)
  httpd = HTTPServer(server_address, testHTTPServer_RequestHandler)
  print('running server...')
  httpd.serve_forever()

# restore trained model in graph
graph = tf.Graph()
sess = tf.Session()
  
def load_graph():
    # get checkpoint file directory
    curr_dir = os.path.dirname(os.path.realpath(__file__))
    rel_dir = "tmp/logs"
    ckpt_dir = os.path.join(curr_dir, rel_dir)

    #with graph.as_default():
    #with sess.as_default():
    init_op = tf.global_variables_initializer()
    sess.run(init_op)
    
    # load checkpoint file
    checkpoint_file = tf.train.latest_checkpoint(ckpt_dir)
    new_saver = tf.train.import_meta_graph("{}.meta".format(checkpoint_file)) 
    new_saver.restore(sess, checkpoint_file)
            
def run_graph():
    print('in run_graph()')
    
    with graph.as_default():
        #with sess.as_default():
        #tf.global_variables_initializer().run()
        
        """
        print('####################################')
        op = sess.graph.get_operations()
        for m in op:
            print(m.values())
        print('####################################')
        """

        # get prediction op by name
        prediction = tf.nn.relu(sess.graph.get_operation_by_name("softmax_linear/logits").outputs[0])

        #create dummy features to test
        features = [[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]]
        features = numpy.asarray(features)
        #features.reshape((1,model.INPUT_SIZE))
        print(features.shape)

        # create feed dictionary
        feed_dict = {'Placeholder:0':features}

        # feed the input to the network to get prediction op output
        # values
        output = sess.run(prediction, feed_dict)
        # convert output to numpy array to manipulate
        output = numpy.array(output).reshape(1,11)
        output /= numpy.max(output)

        print(output)
    
load_graph()
run()