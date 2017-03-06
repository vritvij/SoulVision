# ==========================================
# Disclaimer:
# This code has been forked from Tensorflow MNIST Tutorial
# Link: https://github.com/tensorflow/tensorflow/tree/master/tensorflow/examples/tutorials/mnist
# ==========================================

"""
Implements the inference/loss/training pattern for model building.
1. inference() - Builds the model as far as is required for running the network
forward to make predictions.
2. loss() - Adds to the inference model the layers required to generate loss.
3. training() - Adds to the loss model the Ops required to generate and
apply gradients.

This file is used by the various "run_model_*.py" files and not meant to
be run.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import math

import tensorflow as tf

# Total Number of Attacks for all creatures
NUM_ATTACKS = 10

# Size of input would include:
# Description						(Size)
# Change in level of self and enemy (1)
# Self Normalized Health			(1)
# Enemy Normalized Health			(1)
# Number of attacks self can perform (10)	(Boolean vector casted to int)
# Self creature type 1				(1)
# Enemy creature type 1				(1)
# Distance from Enemy				(1)
# Self status						(1)		(Paralyzed, frozen etc.)
# Enemy Status						(1)

# Total Size of Inputs				(18)
INPUT_SIZE = NUM_ATTACKS + 8

# Size of output would include:
# Probability of fleeing	(1)
# Probability of various attacks (10)

# Total Size of Outputs			(11)
OUTPUT_SIZE = 1 + NUM_ATTACKS

# Maximum number of iteration
MAX_STEPS = 2000

# Hidden units in layer 1
HIDDEN1_UNITS = 128

# Hidden units in layer 2
HIDDEN2_UNITS = 32


def inference(features):
  """Build the model up to where it may be used for inference.
  Args:
    features: Input features placeholder, from inputs().
  Returns:
    softmax_linear: Output tensor with the computed logits.
  """
  # Hidden 1
  with tf.name_scope('hidden1'):
    weights = tf.Variable(
        tf.truncated_normal([INPUT_SIZE, HIDDEN1_UNITS],
                            stddev=1.0 / math.sqrt(float(INPUT_SIZE))),
        name='weights')
    biases = tf.Variable(tf.zeros([HIDDEN1_UNITS]),
                         name='biases')
    hidden1 = tf.nn.relu(tf.matmul(features, weights) + biases)
	
  # Hidden 2
  with tf.name_scope('hidden2'):
    weights = tf.Variable(
        tf.truncated_normal([HIDDEN1_UNITS, HIDDEN2_UNITS],
                            stddev=1.0 / math.sqrt(float(HIDDEN1_UNITS))),
        name='weights')
    biases = tf.Variable(tf.zeros([HIDDEN2_UNITS]),
                         name='biases')
    hidden2 = tf.nn.relu(tf.matmul(hidden1, weights) + biases)
	
  # Linear
  with tf.name_scope('softmax_linear'):
    weights = tf.Variable(
        tf.truncated_normal([HIDDEN2_UNITS, OUTPUT_SIZE],
                            stddev=1.0 / math.sqrt(float(HIDDEN2_UNITS))),
        name='weights')
    biases = tf.Variable(tf.zeros([OUTPUT_SIZE]),
                         name='biases')
    logits = tf.add(biases, tf.matmul(hidden2, weights), name='logits')
    
  return logits


##### I assume loss represents cost #######
def loss(logits, labels):
  """Calculates the loss from the logits and the labels.
  Args:
    logits: Logits tensor, float - [batch_size, OUTPUT_SIZE].
    labels: Labels tensor, int32 - [batch_size, OUTPUT_SIZE].
  Returns:
    loss: Loss tensor of type float.
  """
  # find index of highest probability value
  # and store it in labels. By default k=1
  labels = tf.nn.top_k(labels).indices
  # convert shape from (?,1) to (?,) and type 
  #to int64 to make it compatible to pass in 
  # sparse_softmax_cross_entropy_with_logits()
  labels = tf.reshape(labels, [-1])
  labels = tf.to_int64(labels)
  #print('labels=',labels)
  cross_entropy = tf.nn.sparse_softmax_cross_entropy_with_logits(
      labels=labels, logits=logits, name='xentropy')
  #print('cross_entropy=', cross_entropy)
  return tf.reduce_mean(cross_entropy, name='xentropy_mean')


def training(loss, learning_rate):
  """Sets up the training Ops.
  Creates a summarizer to track the loss over time in TensorBoard.
  Creates an optimizer and applies the gradients to all trainable variables.
  The Op returned by this function is what must be passed to the
  `sess.run()` call to cause the model to train.
  Args:
    loss: Loss tensor, from loss().
    learning_rate: The learning rate to use for gradient descent.
  Returns:
    train_op: The Op for training.
  """
  # Add a scalar summary for the snapshot loss.
  tf.summary.scalar('loss', loss)
  # Create the gradient descent optimizer with the given learning rate.
  optimizer = tf.train.GradientDescentOptimizer(learning_rate)
  # Create a variable to track the global step.
  global_step = tf.Variable(0, name='global_step', trainable=False)
  # Use the optimizer to apply the gradients that minimize the loss
  # (and also increment the global step counter) as a single training step.
  train_op = optimizer.minimize(loss, global_step=global_step)
  return train_op


def evaluation(logits, labels):
  """Evaluate the quality of the logits at predicting the label.
  Args:
    logits: Logits tensor, float - [batch_size, OUTPUT_SIZE].
    labels: Labels tensor, int32 - [batch_size, OUTPUT_SIZE]
  Returns:
    A scalar int32 tensor with the number of examples (out of batch_size)
    that were predicted correctly.
  """
  # find index of highest probability value
  # and store it in labels. By default k=1
  labels = tf.nn.top_k(labels).indices
  # convert shape from (?,1) to (?,) to make
  # it type comaptible with in_top_k()
  labels = tf.reshape(labels, [-1])
  # For a classifier model, we can use the in_top_k Op.
  # It returns a bool tensor with shape [batch_size] that is true for
  # the examples where the label is in the top k (here k=1)
  # of all logits for that example.
  correct = tf.nn.in_top_k(logits, labels, 1)
  # Return the number of true entries.
  return tf.reduce_sum(tf.cast(correct, tf.int32))
