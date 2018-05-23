from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import urllib

import numpy as np
import tensorflow as tf

from tensorflow.python.keras.models import Sequential
from tensorflow.python.keras.layers import Dense

np.random.seed(22)

path = "./numonly.csv"
dataset = np.loadtxt(path,delimiter=",")

X = dataset[:,0:22]
Y = dataset[:,22]

# create model
model = Sequential()
model.add(Dense(32, input_dim=22, activation='relu'))
model.add(Dense(16, activation='relu'))
model.add(Dense(1, activation='sigmoid'))

# Compile model
model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])

# Fit the model
model.fit(X, Y, epochs=2500, batch_size=150, verbose = 2)

# evaluate the model
scores = model.evaluate(X, Y)
print("\n%s: %.2f%%" % (model.metrics_names[1], scores[1]*100))


predictions = model.predict(X)

rounded = [round(x[0]) for x in predictions]

print(rounded)

"""
if __name__ = "__main__":
	main()
def new_samples():
	return np.array([[]], dtype=np.float32)
"""
