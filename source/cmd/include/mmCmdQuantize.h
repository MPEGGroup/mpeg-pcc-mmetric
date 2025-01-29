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

#ifndef _MM_CMD_QUANTIZE_H_
#define _MM_CMD_QUANTIZE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include <limits>

class CmdQuantize : Command {
 public:
  CmdQuantize(){};

  // Description of the command
  static const char* name;
  static const char* brief;
  // command creator
  static Command* create();

  // the command main program
  virtual bool initialize( Context* ctx, std::string app, int argc, char* argv[] );
  virtual bool process( uint32_t frame );
  virtual bool finalize() { return true; };

 private:
  // Command parameters
  std::string _inputModelFilename;
  std::string _outputModelFilename;
  std::string _outputVarFilename;
  // Quantization parameters
  uint32_t _qp         = 12;  // geometry
  uint32_t _qt         = 12;  // UV coordinates
  uint32_t _qn         = 12;  // normals
  uint32_t _qc         = 8;   // colors
  bool     _dequantize = false;
  //
  bool _computeBboxPos = false;
  bool _computeBboxUv  = false;
  bool _computeBboxNrm = false;
  bool _computeBboxCol = false;
  bool _useFixedPoint  = false;
  // min max vectors
  std::string _minPosStr;
  std::string _maxPosStr;
  glm::vec3   _minPos = {0.0F, 0.0F, 0.0F};
  glm::vec3   _maxPos = {0.0F, 0.0F, 0.0F};
  std::string _minUvStr;
  std::string _maxUvStr;
  glm::vec2   _minUv = {0.0F, 0.0F};
  glm::vec2   _maxUv = {0.0F, 0.0F};
  std::string _minNrmStr;
  std::string _maxNrmStr;
  glm::vec3   _minNrm = {0.0F, 0.0F, 0.0F};
  glm::vec3   _maxNrm = {0.0F, 0.0F, 0.0F};
  std::string _minColStr;
  std::string _maxColStr;
  glm::vec3   _minCol = {0.0F, 0.0F, 0.0F};
  glm::vec3   _maxCol = {0.0F, 0.0F, 0.0F};
  // color space conversion
  bool _colorSpaceConversion = false;
};

#endif
