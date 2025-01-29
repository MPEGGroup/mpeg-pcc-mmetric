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

#ifndef _MM_CMD_SAMPLE_H_
#define _MM_CMD_SAMPLE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmImage.h"

class CmdSample : Command {
 private:
  // the context for frame access
  Context* _context;
  // the command options
  std::string inputModelFilename;
  std::vector<std::string> _inputTextureFilenames;
  std::string outputModelFilename;
  std::string _outputCsvFilename;
  bool        hideProgress = false;
  // the type of processing
  std::string mode = "face";
  // Face options
  float thickness = 0.0;
  // Grid options
  int         _gridSize      = 1024;
  bool        _useNormal     = false;
  bool        _useFixedPoint = false;
  std::string _minPosStr;
  std::string _maxPosStr;
  glm::vec3   _minPos = {0.0F, 0.0F, 0.0F};
  glm::vec3   _maxPos = {0.0F, 0.0F, 0.0F};
  // Face, Grid and sdiv options
  bool bilinear = false;
  // Face subdiv options
  int maxDepth = 100;
  float areaThreshold = 1.0F;
  bool  mapThreshold  = false;
  // Edge subdiv options (0.0 mean use resolution)
  float lengthThreshold = 0.0F;
  // Edge and Face options
  size_t _resolution = 1024;
  // sample count constrained sampling
  size_t _nbSamplesMin  = 0;
  size_t _nbSamplesMax  = 0;
  size_t _maxIterations = 10;
  // Prnd options
  size_t _nbSamples = 2000000;

 public:
  CmdSample(){};

  // Description of the command
  static const char* name;
  static const char* brief;
  // command creator
  static Command* create();

  // the command main program
  virtual bool initialize( Context* ctx, std::string app, int argc, char* argv[] );
  virtual bool process( uint32_t frame );
  virtual bool finalize() { return true; }
};

#endif
