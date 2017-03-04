# ==========================================
# Disclaimer:
# This code has been forked from Tensorflow MNIST Tutorial
# Link: https://github.com/tensorflow/tensorflow/tree/master/tensorflow/examples/tutorials/mnist
# ==========================================

"""Trains and Evaluates the GAME STARTEGY neural network using a feed dictionary."""
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

import model

# Basic model parameters as external flags.
FLAGS = None


def placeholder_inputs(batch_size):
  """Generate placeholder variables to represent the input tensors.
  These placeholders are used as inputs by the rest of the model building
  code and will be fed from the downloaded data in the .run() loop, below.
  Args:
    batch_size: The batch size will be baked into both placeholders.
  Returns:
    features_placeholder: Features placeholder.
    labels_placeholder: Labels placeholder.
  """
  # Note that the shapes of the placeholders match the shapes of the full
  # features and label tensors, except the first dimension is now batch_size
  # rather than the full size of the train or test data sets.
  features_placeholder = tf.placeholder(tf.float32, shape=(None,
                                                         model.INPUT_SIZE))
  labels_placeholder = tf.placeholder(tf.float32, shape=(None,
                                                        model.OUTPUT_SIZE))
  return features_placeholder, labels_placeholder


def fill_feed_dict(features_batch, label_batch, features_pl, label_pl):
  """Fills the feed_dict for training the given step.
  A feed_dict takes the form of:
  feed_dict = {
      <placeholder>: <tensor of values to be passed for placeholder>,
      ....
  }
  Args:
    features_batch: batch of feature vectors (batch_size, input_size)
    labels_batch: batch of label vectors (batch_size, label_size) label_size is 1 for now
    features_pl: The features placeholder, from placeholder_inputs().
    labels_pl: The labels placeholder, from placeholder_inputs().
  Returns:
    feed_dict: The feed dictionary mapping from placeholders to values.
  """
  # Create the feed_dict for the placeholders filled with the next
  # `batch size` examples.
  ############# TO DO LIST ##################
  
  feed_dict = {
      features_pl: features_batch,
      label_pl: label_batch,
  }
  return feed_dict


def do_eval(sess,
            eval_correct,
            features_placeholder,
            labels_placeholder,
            features_batch, label_batch):
  """Runs one evaluation against the full epoch of data.
  Args:
    sess: The session in which the model has been trained.
    eval_correct: The Tensor that returns the number of correct predictions.
    features_placeholder: The features placeholder.
    labels_placeholder: The labels placeholder.
    features_batch: batch of feature vectors (batch_size, input_size)
    labels_batch: batch of label vectors (batch_size, label_size) label_size is 1 for now
  """
  # And run one epoch of eval.
  true_count = 0  # Counts the number of correct predictions.
  steps_per_epoch = len(features_batch) // FLAGS.batch_size # // represents integer division
  num_examples = steps_per_epoch * FLAGS.batch_size
  for step in xrange(steps_per_epoch):
    feed_dict = fill_feed_dict(features_batch, label_batch,
                               features_placeholder,
                               labels_placeholder)
    true_count += sess.run(eval_correct, feed_dict=feed_dict)
  precision = float(true_count) / num_examples
  print('  Num examples: %d  Num correct: %d  Precision @ 1: %0.04f' %
        (num_examples, true_count, precision))

		
def read_datafile_csv(filename_queue):
    reader = tf.TextLineReader()
    key, value = reader.read(filename_queue)

    # Default values, in case of empty columns. Also specifies the type of the
    # decoded result.
    record_defaults = [[0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], \
                        [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0]]
    col1, col2, col3, col4, col5, col6, col7, col8, col9, col10, col11, col12, col13, col14, col15, col16, col17, col18, \
    col19, col20, col21, col22, col23, col24, col25, col26, col27, col28, col29  = tf.decode_csv(
		value, record_defaults=record_defaults)
    features = tf.stack([col1, col2, col3, col4, col5, col6, col7, col8, col9, col10, col11, col12, col13, col14, col15, col16, col17, col18])
    label = tf.stack([col19, col20, col21, col22, col23, col24, col25, col26, col27, col28, col29])
    return features, label

def input_pipeline(filenames, batch_size, num_epochs=None):
    #files = tf.train.match_filenames_once(filenames)
    
    filename_queue = tf.train.string_input_producer(
        filenames, num_epochs = num_epochs, shuffle = False)
    
    features, label = read_datafile_csv(filename_queue)
    # min_after_dequeue defines how big a buffer we will randomly sample
    #   from -- bigger means better shuffling but slower start up and more
    #   memory used.
    # capacity must be larger than min_after_dequeue and the amount larger
    #   determines the maximum we will prefetch.  Recommendation:
    #   min_after_dequeue + (num_threads + a small safety margin) * batch_size
    min_after_dequeue = 100
    capacity = min_after_dequeue + 2 * batch_size
    features_batch, label_batch = tf.train.shuffle_batch(
      [features, label], 
      batch_size=batch_size, 
      capacity=capacity,
      min_after_dequeue=min_after_dequeue)
    return features_batch, label_batch
    
		
def run_training():
  """Train MODEL for a number of steps."""
  # Get the sets of features and labels for training, validation, and
  # test on GAME STRATEGY.

  batch_size = FLAGS.batch_size
  curr_dir = os.path.dirname(os.path.realpath(__file__))
  target_dir = os.path.join(curr_dir, '..', '..')
  rel_path = "Content/Data/TrainingData.csv"
  training_filename = os.path.join(target_dir, rel_path)
  training_filename = [training_filename]
  
  #print("FLAGS.batch_size = ", FLAGS.batch_size)
  
  features_batch, label_batch = input_pipeline(training_filename, batch_size) 


  with tf.Session() as sess1:
    tf.global_variables_initializer().run()
    # Start populating the filename queue.
    coord = tf.train.Coordinator()
    threads = tf.train.start_queue_runners( coord=coord)

    # Retrieve a single batch:
    features_batch, label_batch = sess1.run([features_batch, label_batch])
    features_batch = features_batch.reshape((batch_size,model.INPUT_SIZE))
    label_batch = label_batch.reshape((batch_size,model.OUTPUT_SIZE))
    
    print("##########################################################")
    #print(features_batch)
    #print(label_batch)
    print("Shape of features_batch: ", features_batch.shape)
    print("Shape of label_batch: ", label_batch.shape)
    print("#########################################################")

    coord.request_stop()
    coord.join(threads)  
    
  # Tell TensorFlow that the model will be built into the default Graph.
  with tf.Graph().as_default():
    # Generate placeholders for the features and labels.
    features_placeholder, labels_placeholder = placeholder_inputs(
        FLAGS.batch_size)

    # Build a Graph that computes predictions from the inference model.
    logits = model.inference(features_placeholder)

    # Add to the Graph the Ops for loss calculation.
    loss = model.loss(logits, labels_placeholder)

    # Add to the Graph the Ops that calculate and apply gradients.
    train_op = model.training(loss, FLAGS.learning_rate)

    # Add the Op to compare the logits to the labels during evaluation.
    eval_correct = model.evaluation(logits, labels_placeholder)

    # Build the summary Tensor based on the TF collection of Summaries.
    summary = tf.summary.merge_all()

    # Add the variable initializer Op.
    init = tf.global_variables_initializer()

    # Create a saver for writing training checkpoints.
    saver = tf.train.Saver()

    # Create a session for running Ops on the Graph.
    sess = tf.Session()

    # Instantiate a SummaryWriter to output summaries and the Graph.
    summary_writer = tf.summary.FileWriter(FLAGS.log_dir, sess.graph)
    
    
    # And then after everything is built:

    # Run the Op to initialize the variables.
    sess.run(init)

    # Start the training loop.
    for step in xrange(FLAGS.max_steps):
      start_time = time.time()
      
      # Retrieve a single batch:
      #features_batch, label_batch = sess.run([features_batch, label_batch])
    
      # Fill a feed dictionary with the actual set of features and labels
      # for this particular training step.
	  ##################################################
      feed_dict = fill_feed_dict(features_batch,
                                 label_batch,
                                 features_placeholder,
                                 labels_placeholder)
	  ##################################################						 

      # Run one step of the model.  The return values are the activations
      # from the `train_op` (which is discarded) and the `loss` Op.  To
      # inspect the values of your Ops or variables, you may include them
      # in the list passed to sess.run() and the value tensors will be
      # returned in the tuple from the call.
      _, loss_value = sess.run([train_op, loss],
                               feed_dict=feed_dict)

      duration = time.time() - start_time

      # Write the summaries and print an overview fairly often.
      if step % 5 == 0:
        # Print status to stdout.
        print('Step %d: loss = %.2f (%.3f sec)' % (step, loss_value, duration))
        # Update the events file.
        summary_str = sess.run(summary, feed_dict=feed_dict)
        summary_writer.add_summary(summary_str, step)
        summary_writer.flush()

      # Save a checkpoint and evaluate the model periodically.
      if (step + 1) % 10 == 0 or (step + 1) == FLAGS.max_steps:
        curr_dir = os.path.dirname(os.path.realpath(__file__))
        log_dir = os.path.join(curr_dir, FLAGS.log_dir)
        checkpoint_file = os.path.join(log_dir, 'model.ckpt')
        saver.save(sess, checkpoint_file, global_step=step+1)
        # Evaluate against the training set.
        print('Training Data Eval:')
        do_eval(sess,
                eval_correct,
                features_placeholder,
                labels_placeholder,
                features_batch, label_batch)
 
	
def main(_):
  curr_dir = os.path.dirname(os.path.realpath(__file__))
  log_dir = os.path.join(curr_dir, FLAGS.log_dir)
  if tf.gfile.Exists(log_dir):
    tf.gfile.DeleteRecursively(log_dir)
  tf.gfile.MakeDirs(log_dir)
  run_training()


if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument(
      '--learning_rate',
      type=float,
      default=0.01,
      help='Initial learning rate.'
  )
  parser.add_argument(
      '--max_steps',
      type=int,
      default=model.MAX_STEPS,
      help='Number of steps to run trainer.'
  )
  """
  parser.add_argument(
      '--hidden1',
      type=int,
      default=128,
      help='Number of units in hidden layer 1.'
  )
  parser.add_argument(
      '--hidden2',
      type=int,
      default=32,
      help='Number of units in hidden layer 2.'
  )
  """
  parser.add_argument(
      '--batch_size',
      type=int,
      default=20,
      help='Batch size.  Must divide evenly into the dataset sizes.'
  )
  # This is irrelevant for now
  parser.add_argument(
      '--input_data_dir',
      type=str,
      default='C:/Users/rajat/Desktop/Soulvision NN Scripts/tmp/tensorflow/model/input_data',
      help='Directory to put the input data.'
  )
  parser.add_argument(
      '--log_dir',
      type=str,
      default='tmp/logs',
      help='Directory to put the log data.'
  )
  parser.add_argument(
      '--fake_data',
      default=False,
      help='If true, uses fake data for unit testing.',
      action='store_true'
  )

  FLAGS, unparsed = parser.parse_known_args()
  tf.app.run(main=main, argv=[sys.argv[0]] + unparsed)

