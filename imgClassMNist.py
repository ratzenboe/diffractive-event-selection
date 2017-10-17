from keras.models import Sequential
from keras.layers import Dense, Flatten, Conv2D, MaxPooling2D, Dropout
from keras.optimizers import RMSprop
from keras.datasets import mnist
from keras.utils import np_utils
from keras import initializers
from keras import backend as K
import keras


batch_size = 128
nb_classes = 10
nb_epoch = 12

img_rows, img_cols = 28, 28         # input image dimensions
pool_size = (2, 2)                  # size of pooling area for max pooling
prob_drop_conv = 0.2                # drop probability for dropout @ conv layer
prob_drop_hidden = 0.5              # drop probability for dropout @ fc layer

def init_weights(shape, dtype=None):
        return K.random_normal(shape, stddev=0.01, dtype=dtype)

(X_train, y_train), (X_test, y_test) = mnist.load_data()
#print 'y_train original shape: {}'.format(y_train.shape)
#print 'y_train type: {}'.format(type(y_train))
#print 'y_train d-type: {}'.format(y_train.dtype)
#print 'y_train examples: {}, {}, {}'.format(y_train[0], y_train[1], y_train[3])

X_train = X_train.reshape(X_train.shape[0], img_rows, img_cols, 1)
X_test = X_test.reshape(X_test.shape[0], img_rows, img_cols, 1)
input_shape = (img_rows, img_cols, 1)

X_train = X_train.astype('float32')
X_test = X_test.astype('float32')
X_train /= 255
X_test /= 255

# convert class vectors to binary class matrices
y_train = keras.utils.to_categorical(y_train, nb_classes)
y_test = keras.utils.to_categorical(y_test, nb_classes)

model = Sequential()

model.add(Conv2D( 32, kernel_size=(3,3),
                    activation='relu',
                    input_shape=input_shape ))
model.add(Conv2D(64, (3, 3), activation='relu'))
model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Dropout(0.25))
model.add(Flatten())
model.add(Dense(128, activation='relu'))
model.add(Dropout(0.5))
model.add(Dense(nb_classes, activation='softmax'))

model.compile(loss=keras.losses.categorical_crossentropy,
              optimizer=keras.optimizers.Adadelta(),
              metrics=['accuracy'])

model.fit(X_train, y_train,
            batch_size=batch_size,
            shuffle=True,
            epochs=nb_epoch,
            verbose=1,
            validation_data=(X_test, y_test))

score = model.evaluate(X_test, y_test, verbose=0)
print 'test loss: {}'.format(score[0])
print 'test accuracy: {}'.format(score[1])
