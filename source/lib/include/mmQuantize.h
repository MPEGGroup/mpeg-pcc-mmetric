/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2021, InterDigital
 * Copyright (c) 2021-2025, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the copyright holder(s) nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MM_QUANTIZE_H_
#define _MM_QUANTIZE_H_

// internal headers
#include "mmModel.h"
#include <limits>

namespace mm {

class Quantize {
 public:
  Quantize(){};

  // quantizes the model using parameters and range. If a range is not set (i.e. min == max) then a range is
  // computed internally. If quantization parameter is < 7, the related attribute is not quantized.
  static void quantize(const Model&       input,
                       Model&             output,
                       const uint32_t     qp,
                       const uint32_t     qt,
                       const uint32_t     qn,
                       const uint32_t     qc,
                       const std::string& outputVarFilename,
                       bool               useFixedPoint,
                       bool               colorSpaceConversion,
                       glm::vec3&         minPos,
                       glm::vec3&         maxPos,
                       glm::vec2&         minUv,
                       glm::vec2&         maxUv,
                       glm::vec3&         minNrm,
                       glm::vec3&         maxNrm,
                       glm::vec3&         minCol,
                       glm::vec3&         maxCol,
                       const bool         verbose = true);
};

}  // namespace mm

#endif
