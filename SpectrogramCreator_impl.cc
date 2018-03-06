/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "SpectrogramCreator_impl.h"
#include <gnuradio/blocks/api.h>
#include <gnuradio/blocks/pdu.h>
#include <algorithm>
#include <numeric>

using namespace gr::blocks::pdu;


namespace gr {
  namespace Fsil {

    SpectrogramCreator::sptr
    SpectrogramCreator::make(int fftsize, int nrows, int ncols, int n_avgs, bool cancel_DC)
    {
      return gnuradio::get_initial_sptr
        (new SpectrogramCreator_impl(fftsize, nrows, ncols, n_avgs, cancel_DC));
    }

    /*
     * The private constructor
     */
    SpectrogramCreator_impl::SpectrogramCreator_impl(int fftsize, int nrows, int ncols, int n_avgs, bool cancel_DC)
      : gr::sync_block("SpectrogramCreator",
                      gr::io_signature::make(1, 1, fftsize*sizeof(float)),
                      gr::io_signature::make(0, 0, 0)),
        d_fftsize(fftsize),
        d_nrows(nrows),
        d_ncols(ncols),
        d_n_avgs(n_avgs),
        d_cancel_DC(cancel_DC),
        d_avg_count(0),
        d_idx_offset(0),
        d_pdu_vector(pmt::PMT_NIL) // gr_complex  or gr_vector_float
      {
        const size_t nitems = 64*1024; //Is this complex64 - 1024
        d_mag2.resize(nitems);
        d_mag2_sum.resize(d_fftsize*d_nrows);
        d_mag2_byte.resize(d_fftsize*d_nrows);
        d_img_size = d_fftsize*d_nrows;
        d_IQ_per_img = d_fftsize*d_nrows*d_n_avgs;
        d_fft_pwr.resize(d_nrows);

        std::fill(&d_mag2_sum[0], &d_mag2_sum[d_img_size], 0);
        // register message ports
        message_port_register_out(pmt::mp("spectrogram"));
      }

    /*
     * Our virtual destructor.
     */
    SpectrogramCreator_impl::~SpectrogramCreator_impl()
    {
    }

    int
    SpectrogramCreator_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const float *in = (const float *) input_items[0];
      // unsigned int n_samples = d_fftsize*noutput_items;
      // unsigned int input_data_size = input_signature()->sizeof_stream_item (0);
      // std::cout << "This is the input size:" << input_data_size << std::endl;

      // apply |.|^2
      volk_32fc_magnitude_squared_32f(&d_mag2[0], &in[0], d_fftsize*noutput_items);

      for(int nblock = 0; nblock < noutput_items; ++nblock) {
        // Compute the line averaging
        volk_32f_x2_add_32f(&d_mag2_sum[d_idx_offset], &d_mag2[nblock*d_fftsize], &d_mag2_sum[d_idx_offset], d_fftsize);

        d_avg_count++;
        if(d_avg_count==d_n_avgs) {
          d_avg_count=0;
          // to compute the minimum value across the whole spectrogram
          d_fft_pwr[d_idx_offset/d_fftsize] = std::accumulate(&d_mag2_sum[d_idx_offset],
                                                              &d_mag2_sum[d_idx_offset+d_fftsize],
                                                              0.0f)/d_fftsize;
          d_idx_offset+=d_fftsize;
          if(d_idx_offset==d_img_size) {
            // the image is ready here
            d_idx_offset=0;

            // cancel DC offset
            if(d_cancel_DC) {
              float pwr_min = *std::min_element(d_fft_pwr.begin(),d_fft_pwr.end());
              for(int i = d_fftsize/2; i < d_img_size; i+=d_fftsize)
                d_mag2_sum[i] = pwr_min;
            }

            // convert to dB
            for(int i = 0; i < d_img_size; ++i) {
              d_mag2_sum[i] = 10*log10(d_mag2_sum[i]/d_n_avgs);
            }

            // normalize spectrogram
            float min_val = *std::min_element(&d_mag2_sum[0], &d_mag2_sum[d_img_size]);
            float max_val = *std::max_element(&d_mag2_sum[0], &d_mag2_sum[d_img_size]);
            for(int i = 0; i < d_img_size; ++i)
              d_mag2_byte[i] = (d_mag2_sum[i]-min_val)*255/(max_val-min_val);

            // pass message with the image
            d_pdu_vector = pmt::init_u8vector(d_img_size, &d_mag2_byte[0]);
            message_port_pub(pmt::mp("spectrogram"), d_pdu_vector);

            std::fill(&d_mag2_sum[0], &d_mag2_sum[d_img_size], 0);
          }
        }
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace Fsil */
} /* namespace gr */

