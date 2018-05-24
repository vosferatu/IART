from __future__ import absolute_import, division, print_function

import os
import matplotlib.pyplot as plt

import tensorflow as tf
import tensorflow.contrib.eager as tfe

tf.enable_eager_execution()

print("TensorFlow version: {}".format(tf.VERSION))
print("Eager execution: {}".format(tf.executing_eagerly()))

_CSV_COLUMNS = [
    'MFCCs_1', 'MFCCs_2', 'MFCCs_3', 'MFCCs_4', 'MFCCs_5',
    'MFCCs_6', 'MFCCs_7', 'MFCCs_8', 'MFCCs_9', 'MFCCs_10',
    'MFCCs_11', 'MFCCs_12', 'MFCCs_13', 'MFCCs_14', 'MFCCs_15'
    'MFCCs_16', 'MFCCs_17', 'MFCCs_18', 'MFCCs_19', 'MFCCs_20'
    'MFCCs_21', 'MFCCs_22',
    'Result'
]

_CSV_COLUMN_DEFAULTS = [[0.], [0.], [0.], [0.], [0.],
                        [0.], [0.], [0.], [0.], [0.],
                        [0.], [0.], [0.], [0.], [0.],
                        [0.], [0.], [0.], [0.], [0.],
                        [0.], [0.],
                        [0]]

PARAM_NUM = 22
BATCH_SIZE = 201
NUM_EPOCHS = 201
DIV = 50

def parse_csv(line):
    record_defaults=_CSV_COLUMN_DEFAULTS
    parsed_line = tf.decode_csv(line, record_defaults)
    features = tf.reshape(parsed_line[:-1], shape=(PARAM_NUM,))
    label = tf.reshape(parsed_line[-1], shape=())
    return features, label

train_dataset_fp = os.path.abspath("train.csv")
test_dataset_fp = os.path.abspath("test.csv")

train_dataset = tf.data.TextLineDataset(train_dataset_fp)
train_dataset = train_dataset.map(parse_csv)      # parse each row
train_dataset = train_dataset.shuffle(buffer_size=1000)  # randomize
train_dataset = train_dataset.batch(BATCH_SIZE)

test_dataset = tf.data.TextLineDataset(test_dataset_fp)
test_dataset = test_dataset.map(parse_csv)      # parse each row
test_dataset = test_dataset.shuffle(buffer_size=1000)  # randomize
test_dataset = test_dataset.batch(BATCH_SIZE)


print("\nTrain Set:")
features, label = iter(train_dataset).next()
print("example features:", features[0])
print("example label:", label[0])
print("\n")

print("\nTest Set:")
features, label = iter(test_dataset).next()
print("example features:", features[0])
print("example label:", label[0])
print("\n")

# increasing the number of hidden layers and neurons typically creates a more powerful model,
# which requires more data to train effectively
# overfitting -> "memorizing" answers

model = tf.keras.Sequential([
  tf.keras.layers.Dense(36, activation="relu", input_shape=(PARAM_NUM,)),  # input shape required
  tf.keras.layers.Dense(28, activation="relu"),
  tf.keras.layers.Dense(16, activation="relu"),
  tf.keras.layers.Dense(10)
])

# supervised machine learning -> training examples contain labels

def loss(model, x, y):
  y_ = model(x)
  return tf.losses.sparse_softmax_cross_entropy(labels=y, logits=y_)

def grad(model, inputs, targets):
  with tf.GradientTape() as tape:
    loss_value = loss(model, inputs, targets)
  return tape.gradient(loss_value, model.variables)

optimizer = tf.train.GradientDescentOptimizer(learning_rate=0.01)

## Note: Rerunning this cell uses the same model variables

# keep results for plotting
train_loss_results = []
train_accuracy_results = []

for epoch in range(NUM_EPOCHS):
  epoch_loss_avg = tfe.metrics.Mean()
  epoch_accuracy = tfe.metrics.Accuracy()

  # Training loop - using batches of 32
  for x, y in train_dataset:
    # Optimize the model
    grads = grad(model, x, y)
    optimizer.apply_gradients(zip(grads, model.variables),
                              global_step=tf.train.get_or_create_global_step())

    # Track progress
    epoch_loss_avg(loss(model, x, y))  # add current batch loss
    # compare predicted label to actual label
    epoch_accuracy(tf.argmax(model(x), axis=1, output_type=tf.int32), y)

  # end epoch
  train_loss_results.append(epoch_loss_avg.result())
  train_accuracy_results.append(epoch_accuracy.result())

  if epoch % DIV == 0:
    print("Epoch {:03d}: Loss: {:.3f}, Accuracy: {:.3%}".format(epoch,
                                                                epoch_loss_avg.result(),
                                                                epoch_accuracy.result()))

fig, axes = plt.subplots(2, sharex=True, figsize=(12, 8))
fig.suptitle('Training Metrics')

axes[0].set_ylabel("Loss", fontsize=14)
axes[0].plot(train_loss_results)

axes[1].set_ylabel("Accuracy", fontsize=14)
axes[1].set_xlabel("Epoch", fontsize=14)
axes[1].plot(train_accuracy_results)

plt.show()



test_accuracy = tfe.metrics.Accuracy()


for (x, y) in test_dataset:
  prediction = tf.argmax(model(x), axis=1, output_type=tf.int32)
  test_accuracy(prediction, y)

print("\nTest set accuracy: {:.3%}\n".format(test_accuracy.result()))


class_ids = ["Leptodactylidae  Adenomera  AdenomeraAndre", "Dendrobatidae  Ameerega  Ameeregatrivittata", "Leptodactylidae  Adenomera  AdenomeraHylaedactylus",
 "Hylidae  Dendropsophus  HylaMinuta", "Hylidae  Hypsiboas  HypsiboasCinerascens",
 "Hylidae  Hypsiboas  HypsiboasCordobae", "Leptodactylidae  Leptodactylus  LeptodactylusFuscus",
 "Hylidae  Osteocephalus  OsteocephalusOophagus",
 "Bufonidae  Rhinella  Rhinellagranulosa", "Hylidae  Scinax  ScinaxRuber" ]

predict_dataset = tf.convert_to_tensor([
    [1,0.231645969747594,-0.062799754907128,0.360740385380099,0.383435189111214,0.231630718907751,-0.019113262129877,-0.284785174443936,-0.181143561308725,0.347393759015112,0.425337010017569,-0.128288308298224,-0.426677841113393,-0.004304813572324,0.395496151437028,0.174120298215129,-0.179893707035204,-0.26443554269956,-0.16487712943279,0.064437190202538,0.269065589000625,0.15235846487581], #0
    [1,0.473295884445788,0.61273802793741,0.529802058634611,0.168956383235076,0.173199630054501,0.093227109184488,-0.120572512293531,0.017793431241074,0.068643634225861,-0.063319835195107,0.036507033262845,0.166037288139156,-0.011383670683574,-0.017984851749318,0.170910387130893,-0.055662562814392,-0.066986565236128,0.139570963580329,-0.055940505264538,-0.127576155827949,0.125261145444813], #5
    [1,	0.14238890569877,	0.212683669002399,	0.10043033823719,	0.233289881247379,	0.06138568136563,	-0.093929434096863,	0.0163475679853,	0.23892330402117,	0.027944669388219,	-0.122503281478,	0.22924081664821,	-0.207023840543246,	-0.14532078181285,	-0.19034935356476,	0.1895484955,	0.30762680309459,	-0.78028037782357,	-0.1215159012689,	0.060556521054442,	0.1597790969508,	0.13509359744253] #x
])

predictions = model(predict_dataset)

for i, logits in enumerate(predictions):
  class_idx = tf.argmax(logits).numpy()
  name = class_ids[class_idx]
  print("Example {} prediction: {}".format(i, name))
  print("Class: {}".format(class_idx))
  print("\n")
