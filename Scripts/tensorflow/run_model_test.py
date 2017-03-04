from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

# pylint: disable=missing-docstring
import argparse
import os.path
import sys
import time

from six.moves import xrange  # pylint: disable=redefined-builtin
import tensorflow as tf
import numpy as np

import model

ckpt_dir = 'C:/Users/rajat/Desktop/Soulvision NN Scripts/tmp/tensorflow/model/logs'
#saver = tf.train.Saver()

filename_queue = tf.train.string_input_producer(["test_datafile.csv"])

reader = tf.TextLineReader()
key, value = reader.read(filename_queue)

# Default values, in case of empty columns. Also specifies the type of the
# decoded result.
record_defaults = [[0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0]]
col1, col2, col3, col4, col5, col6, col7, col8, col9, col10, col11, col12, col13, col14, col15, col16, col17, col18 = tf.decode_csv(
    value, record_defaults=record_defaults)
features = tf.stack([col1, col2, col3, col4, col5, col6, col7, col8, col9, col10, col11, col12, col13, col14, col15, col16, col17, col18])

with tf.Session() as sess:
    # Start populating the filename queue.
    coord = tf.train.Coordinator()
    threads = tf.train.start_queue_runners(coord=coord)
    
    for i in range(2):
        # Retrieve a single instance:
        example = sess.run([features])
        example = np.asarray(example)
        example.reshape((1,model.INPUT_SIZE))
        print(example)
        print(example.shape)
        #features_placeholder = tf.placeholder(tf.float32, shape=(1,
        #                                                 model.INPUT_SIZE))
        #logits = model.inference(features_placeholder)
        #logits_val = sess.run(logits, feed_dict = {features_placeholder: example})
        #print(logits_val)
        
    coord.request_stop()
    coord.join(threads)
   
    
graph=tf.Graph()

with graph.as_default():
    with tf.Session() as sess:
        tf.global_variables_initializer().run()
        #ckpt = tf.train.get_checkpoint_state(ckpt_dir)
        #checkpoint_file = os.path.join(ckpt_dir, 'model.ckpt')
        checkpoint_file = tf.train.latest_checkpoint(ckpt_dir)
        new_saver = tf.train.import_meta_graph("{}.meta".format(checkpoint_file)) 
        new_saver.restore(sess, checkpoint_file)
        #print(sess.run(tf.global_variables()))
        print('####################################')
        op = sess.graph.get_operations()
        #for m in op:
            #print(m.values())
        print('####################################')
        
        features_placeholder = tf.placeholder(tf.float32, shape=(1,
                                                         model.INPUT_SIZE))
        #input = graph.get_operation_by_name("features").outputs[0]
        prediction=tf.nn.relu(graph.get_operation_by_name("softmax_linear/logits").outputs[0])
        #prediction=graph.get_operation_by_name("softmax_linear/logits").outputs[0]
        #prediction=sess.run('softmax_linear/logits', feed_dict = {'Placeholder:0': example})
        print(prediction)
        newdata=example
        print (sess.run(prediction,feed_dict={'Placeholder:0':newdata}))
    
