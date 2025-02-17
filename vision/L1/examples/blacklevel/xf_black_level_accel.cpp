/*
 * Copyright 2022 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __XF_BLACK_LEVEL_ACCEL_CPP__
#define __XF_BLACK_LEVEL_ACCEL_CPP__

#include "common/xf_utility.hpp"

#include "xf_black_level_accel_config.h"

static constexpr int __XF_DEPTH =
    (XF_MAX_ROWS * XF_MAX_COLS * (XF_PIXELWIDTH(XF_SRC_T, XF_NPPC)) / 8) / (IMAGE_PTR_WIDTH / 8);

// =======================================================================
// H/W Acclerated functions that are visible to host
// =======================================================================

void blackLevelCorrection_accel(ap_uint<IMAGE_PTR_WIDTH>* in_img_ptr,
                                ap_uint<IMAGE_PTR_WIDTH>* out_img_ptr,
                                int black_level,
                                float mul_value,
                                int height,
                                int width) {
// -----------------------------------------------
// HLS directives for ports
// -----------------------------------------------
// clang-format off
    #pragma HLS INTERFACE m_axi     port=in_img_ptr  offset=slave bundle=gmem_in_ptr depth=__XF_DEPTH
    #pragma HLS INTERFACE m_axi     port=out_img_ptr offset=slave bundle=gmem_out_ptr depth=__XF_DEPTH

    #pragma HLS INTERFACE s_axilite port=black_level
    #pragma HLS INTERFACE s_axilite port=mul_value
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=return
    // clang-format on

    XF_CTUNAME(XF_SRC_T, XF_NPPC) bl = XF_CTUNAME(XF_SRC_T, XF_NPPC)(black_level);
    // -----------------------------------------------
    // Internal xf::Mat objects
    // -----------------------------------------------
    xf::cv::Mat<XF_SRC_T, XF_MAX_ROWS, XF_MAX_COLS, XF_NPPC, XF_CV_DEPTH_IN> InImg(height, width);
    xf::cv::Mat<XF_SRC_T, XF_MAX_ROWS, XF_MAX_COLS, XF_NPPC, XF_CV_DEPTH_OUT> OutImg(height, width);

// -----------------------------------------------
// Actual Body
// -----------------------------------------------
// clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    // Convert input image pointer to xf::Mat
    xf::cv::Array2xfMat<IMAGE_PTR_WIDTH, XF_SRC_T, XF_MAX_ROWS, XF_MAX_COLS, XF_NPPC, XF_CV_DEPTH_IN>(in_img_ptr,
                                                                                                      InImg);

    // Actual accelerator
    xf::cv::blackLevelCorrection<XF_SRC_T, XF_MAX_ROWS, XF_MAX_COLS, XF_NPPC, IMAGE_MUL_WIDTH, IMAGE_MUL_FL_BITS,
                                 XF_USE_DSP, XF_CV_DEPTH_IN, XF_CV_DEPTH_OUT>(InImg, OutImg, bl, mul_value);

    // Convert output xf::Mat to image pointer
    xf::cv::xfMat2Array<IMAGE_PTR_WIDTH, XF_SRC_T, XF_MAX_ROWS, XF_MAX_COLS, XF_NPPC, XF_CV_DEPTH_OUT>(OutImg,
                                                                                                       out_img_ptr);

    return;
}

#endif // __XF_BLACK_LEVEL_ACCEL_CPP__
