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

#ifndef _MM_CMD_ANALYSE_H_
#define _MM_CMD_ANALYSE_H_

#include <limits>

// internal headers
#include "mmCommand.h"
#include "mmModel.h"

class CmdAnalyse : Command {
 private:
  // the context for frame access
  Context* _context;
  // the command options
  std::string _inputModelFilename;
  std::vector<std::string> _inputTextureFilenames;
  std::string _outputCsvFilename;
  std::string _outputVarFilename;
  // count statistics results array of <frame, nbface, nbvert, nbcol, nbnorm, nbuv, nbtex>
  std::vector<std::tuple<uint32_t, double, double, double, double, double, double> > _counts;
  // renge results
  glm::vec3 _minPos = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max()};
  glm::vec3 _maxPos = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::min()};
  glm::vec3 _minCol = {255, 255, 255};
  glm::vec3 _maxCol = {0, 0, 0};
  glm::vec3 _minNrm = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max()};
  glm::vec3 _maxNrm = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::min()};
  glm::vec2 _minUv  = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
  glm::vec2 _maxUv  = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

 public:
  CmdAnalyse(){};

  // Descriptions of the command
  static const char* name;
  static const char* brief;

  // command creator
  static Command* create();

  // the command main program
  virtual bool initialize( Context* ctx, std::string app, int argc, char* argv[] );
  virtual bool process( uint32_t frame );
  virtual bool finalize();
};

#endif
