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

import numpy
from gnuradio import gr

class FFTavgr_ff(gr.basic_block):
    """
    docstring for block FFTavgr_ff
    """
    def __init__(self, no_avgs, vec_length):
        self.no_avgs = no_avgs     # in constructor the input arg is still a temp variable, hence can be assigned to functions; in_sig
        self.vec_length = vec_length
                                   # here it assigns it as a class attribute which can then be used later in code
                                   # https://lists.gnu.org/archive/html/discuss-gnuradio/2015-10/msg00374.html
        gr.basic_block.__init__(self,
            name="FFTavgr_ff",
            in_sig=[(numpy.float32,self.vec_length)],
            out_sig=[(numpy.float32,self.vec_length)])
    """
    def forecast(self, noutput_items, ninput_items_required):
        #setup size of input_items[i] for work call
        for i in range(len(ninput_items_required)):
            ninput_items_required[i] = noutput_items #self.no_avgs # that would tell the scheduler that you'll need at least noutput_items input items to produce noutput_items output items
            print(ninput_items_required[i])
    """
    def general_work(self, input_items, output_items):
        #output_items[0][:] = input_items[0]
        #consume(0, len(input_items[0]))

        in0 = input_items[0]
        out0 = output_items[0] # Describe relation between in0 and out0
        #output_items[0][:] = self._averagingAlgo(in0)
        #return len(output_items[0])
        #for idx in range(len(in0[0])):
        #    out0[idx] = numpy.full(self.vec_length, idx)

        #print "output items is {}".format(output_items.shape)
        print "number of averages: {}".format(self.no_avgs)

        for k in range( int(len(in0)/self.no_avgs) ):
            out = numpy.zeros((1,self.vec_length))

            for i in range(self.no_avgs):
                #print "this is idx:",i
                out = in0[i+int(self.no_avgs*k),:] + out # the columns of in0 (64,64) are taken as a row (1,64)

            out0[k,:] = out/self.no_avgs # computing mean

        print "The out0 shape is {}".format(out0.shape)
        print "The out shape is {}".format(out.shape)
        print "The in0 shape is {}".format(in0.shape)
        """
        print("The output of out0")
        print(out0)
        print("The input of in0")
        print(in0)
        """

        # https://lists.gnu.org/archive/html/discuss-gnuradio/2015-10/msg00374.html
        """
        def _averagingAlgo(self, in0):
           out = numpy.zeros((1,len(input_items[0])))
           for i in range(self.no_avgs):
               out = input_items[0] + out

           output_items = out/self.no_avgs # computing mean
        """
        self.consume_each(len(input_items[0]))
        return len(output_items[0])
