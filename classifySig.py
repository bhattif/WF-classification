#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 <+YOU OR YOUR COMPANY+>.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 
import os
import shutil
import numpy as np
from gnuradio import gr
import cv2
import pmt
import time

from ClassifierCaffe import * # FIND THIS FOR MY CODE

# --- Define the caffe model
# --- Symbols input as np.asarray - this converts input to an array (this is same as cv::MAT)
# --- Pass the symbols through caffe
# --- Create the output probability
# --- Pass on the probability via message

class classifySig(gr.basic_block):
    # class classifySig(gr.basic_block, ClassifierCaffe): # this is for inheritence, not the best way here
    # Create a member variable of an object from a different class (composibility), its better than inheritence
    # Composition is when the class uses another object to provide some or all of its functionality.

    def __init__(self, nrows, vec_length): # constructor
        #model parameters can be passed using a yml file, here we define the parameters directly
        self.nrows = nrows
        self.ncols = vec_length # consistent with the FFTavgr_ff.py block
        self.imgcv = np.zeros((self.nrows,self.ncols,1),np.uint8)
        self.last_result = []

        gr.basic_block.__init__(self,
            name="classifySig",
            in_sig=None,
            out_sig=None)

        # Python GNR message passing: https://gnuradio.org/doc/doxygen/page_python_blocks.html#pyblocks_msgs

        self.message_port_register_in(pmt.intern("spectrogram_in")) # define the input message handler
        self.set_msg_handler(pmt.intern("spectrogram_in"), self.predict)
        self.message_port_register_out(pmt.intern("predictions_out")) # define the output message handler

        self.prototxt_file = '/home/farrukh/Dropbox/WiSHFUL project/Machine Learning/Caffe/CaffeWork Backup/Prototxt-Model-files/deploy.prototxt'
        self.model_file = '/home/farrukh/Dropbox/WiSHFUL project/Machine Learning/Caffe/CaffeWork Backup/Prototxt-Model-files/signal_classifier_3classes_iter_100000.caffemodel',
        #self.prototxt_file = prototxt_file
        #self.model_file = model_file

        self.classifier = ClassifierCaffe_X(self.model_file, self.prototxt_file, image_dims=None,
        gpu=False, mean_file=None, input_scale=None, channel_swap=None) # Object of another class using composition. By being in constructor, this object is called repeatedly
        # Ref example for composition: https://www.eduonix.com/blog/software-development/learn-object-oriented-programming-in-python-composition/


    def predict(self, msg):
        # convert message to numpy array
        u8img = pmt.pmt_to_python.uvector_to_numpy(msg).reshape(self.nrows,self.ncols)
        self.imgcv[:,0:self.vec_length,0] = u8img

        Msg = np.asarray([caffe.io.resize_image(u8img, self.image_dims) for u8img in msg])
        predictions = self.classifier.prediction(Msg)
        print "This is predictions: ",predictions

        self.message_port_pub(pmt.intern("predictions_out"),predictions)




"""
# default functions of python block

    def forecast(self, noutput_items, ninput_items_required):
        #setup size of input_items[i] for work call
        for i in range(len(ninput_items_required)):
            ninput_items_required[i] = noutput_items

    def general_work(self, input_items, output_items):
        output_items[0][:] = input_items[0]
        consume(0, len(input_items[0]))
        #self.consume_each(len(input_items[0]))
        return len(output_items[0])

"""

