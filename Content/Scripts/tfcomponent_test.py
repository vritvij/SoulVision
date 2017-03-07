from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import unreal_engine as ue
import tensorflow as tf
import json
import imp
import operator
import argparse
import os.path
import sys
import time
import numpy

import model

class TensorFlowComponent:

    # constructor adding a component
    def __init__(self):
        ue.log('Init')
        #self.train('')

    # this is called on game start
    def begin_play(self):
        ue.log('Beginplay')
   
    #tensor input
    def tensorinput(self, args):
        ue.log('TF inputs passed: ' + args)
        
        # load json object to python object
        inputstruct = json.loads(args)
        # stack various input features together
        ue.log('JSON: ' + str(inputstruct))

        f1 = inputstruct['deltaLevel'] #float
        f2 = inputstruct['attackerType'] #float
        f3 = inputstruct['attackerHealth'] #float
        f4 = inputstruct['attackerStatus'] #float
        f5 = inputstruct['defenderType'] #float
        f6 = inputstruct['defenderHealth'] #float
        f7 = inputstruct['defenderStatus'] #float
        f8 = inputstruct['distance'] #float
        f9 = inputstruct['moveSet'] #array of float
        # array of input features
        features = [f1,f2,f3,f4,f5,f6,f7,f8]
        features.extend(f9)
        ue.log('input features len: ' + str(len(features)))
        # reshape features from (x,) to (1,x)
        features = numpy.array(features).reshape(1,len(features))
        
        # get checkpoint file directory
        curr_dir = os.path.dirname(os.path.realpath(__file__))
        rel_dir = "tmp/logs"
        ckpt_dir = os.path.join(curr_dir, rel_dir)
        
        # restore trained model in graph
        graph = tf.Graph()

        with graph.as_default():
            with tf.Session() as sess:
                tf.global_variables_initializer().run()
                
                # load checkpoint file
                checkpoint_file = tf.train.latest_checkpoint(ckpt_dir)
                new_saver = tf.train.import_meta_graph("{}.meta".format(checkpoint_file)) 
                new_saver.restore(sess, checkpoint_file)
                
                # get prediction op by name
                prediction = tf.nn.relu(graph.get_operation_by_name("softmax_linear/logits").outputs[0])
                
                # create feed dictionary
                feed_dict = {'Placeholder:0':features}
                # log to debug
                ue.log(feed_dict)
                
                # feed the input to the network to get prediction op output
                # values
                output = sess.run(prediction, feed_dict)
                # convert output to numpy array to manipulate
                output = numpy.array(output).reshape(1 + len(inputstruct['moveSet']),)
                output /= numpy.max(output)
                # log to debug
                ue.log(output)
                
                # assign output values in json components
                inputstruct['fleeProbability'] = float(output[0])
                inputstruct['moveProbability'] = output[1:].tolist()

		#pass prediction back
        ue.log(str(inputstruct))
        self.uobject.OnResultsFunction(json.dumps(inputstruct))

    """
	def train(self, args):
		ue.log('init data for ' + args)

		imp.reload(mnistSimple)
		self.trained = mnistSimple.train()

		ue.log('trained x: ' + str(self.trained['x']))
		ue.log('trained y: ' + str(self.trained['y']))
		ue.log('trained W: ' + str(self.trained['W']))
		ue.log('trained b: ' + str(self.trained['b']))
    """