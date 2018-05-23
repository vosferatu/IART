from __future__ import absolute_import, division, print_function

import os
import matplotlib.pyplot as plt

import tensorflow as tf
import tensorflow.contrib.eager as tfe

"""
import numpy as np # linear algebra
import pandas as pd # data processing, CSV file I/O (e.g. pd.read_csv)
"""
tf.enable_eager_execution()

#TensorFlow version 1.8.0 with eager execution enabled

train_dataset_fp = os.path.abspath("numonly.csv")


_CSV_COLUMNS = [
    'MFCCs_ 1', 'MFCCs_ 2', 'MFCCs_ 3', 'MFCCs_ 4', 'MFCCs_ 5',
    'MFCCs_ 6', 'MFCCs_ 7', 'MFCCs_ 8', 'MFCCs_ 9', 'MFCCs_ 10',
    'MFCCs_ 11', 'MFCCs_ 12', 'MFCCs_ 13', 'MFCCs_ 14', 'MFCCs_ 15'
    'MFCCs_ 16', 'MFCCs_ 17', 'MFCCs_ 18', 'MFCCs_ 19', 'MFCCs_ 20'
    'MFCCs_ 21', 'MFCCs_ 22',
    'Result',
    'RecordID'
]

_CSV_COLUMN_DEFAULTS = [[0.], [0.], [0.], [0.], [0.],
                        [0.], [0.], [0.], [0.], [0.],
                        [0.], [0.], [0.], [0.], [0.],
                        [0.], [0.], [0.], [0.], [0.],
                        [0.], [0.],
                        [0.],
                        [0]]



def parse_csv(line):
  print('Parsing CSV')
  columns = tf.decode_csv(line, record_defaults=_CSV_COLUMN_DEFAULTS)
  features = dict(zip(_CSV_COLUMNS, columns))
  label = features.pop('Result')
  print('label:', label)
  return features, label


train_dataset = tf.data.TextLineDataset(train_dataset_fp)
train_dataset = train_dataset.skip(1)             # skip the first header row
train_dataset = train_dataset.map(parse_csv)      # parse each row
train_dataset = train_dataset.shuffle(buffer_size=1000)  # randomize
train_dataset = train_dataset.batch(32)

# View a single example entry from a batch
features, label = iter(train_dataset).next()
print("example features:", features[0])
print("example label:", label[0])
