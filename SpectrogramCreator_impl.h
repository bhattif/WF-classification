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

#ifndef INCLUDED_FSIL_SPECTROGRAMCREATOR_IMPL_H
#define INCLUDED_FSIL_SPECTROGRAMCREATOR_IMPL_H

#include <Fsil/SpectrogramCreator.h>
#include "/home/farrukh/DSP/gr-Fsil/lib/volk_utils.h"

namespace gr {
  namespace Fsil {

    class SpectrogramCreator_impl : public SpectrogramCreator
    {
     private:
      // Nothing to declare in this block.
        int d_fftsize;
        int d_nrows;
        int d_ncols;
        int d_n_avgs;
        bool d_cancel_DC;

        int d_avg_count;
        int d_idx_offset;
        int d_img_size;
        int d_IQ_per_img;

        volk_utils::volk_array<float> d_mag2;
        volk_utils::volk_array<float> d_mag2_sum;
        volk_utils::volk_array<unsigned char> d_mag2_byte;
        // cv::Mat d_img_mat;
        pmt::pmt_t d_pdu_vector;
        std::vector<float> d_fft_pwr;

     public:
      SpectrogramCreator_impl(int fftsize, int nrows, int ncols, int n_avgs, bool cancel_DC);
      ~SpectrogramCreator_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace Fsil
} // namespace gr

#endif /* INCLUDED_FSIL_SPECTROGRAMCREATOR_IMPL_H */

