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

#ifndef _MM_CMD_COMPARE_H_
#define _MM_CMD_COMPARE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmCompare.h"

class CmdCompare : Command {
 private:
  // the context for frame access
  Context* _context;
  // the command options
  std::string _inputModelAFilename, _inputModelBFilename;
  std::vector<std::string> _inputTextureAFilenames, _inputTextureBFilenames;
  std::string _outputModelAFilename, _outputModelBFilename;
  std::string _outputCsvFilename;
  // the type of processing
  std::string _mode = "equ";
  // Equ options
  float _equEpsilon     = 0;
  bool  _equEarlyReturn = true;
  bool  _equUnoriented  = false;
  // EqTFAN options
  float _eqTFANEpsilon = 0;
  bool  _eqTFANEarlyReturn = true;
  bool  _eqTFANUnoriented = false;
  // Topo options
  std::string _topoFaceMapFilename;
  std::string _topoVertexMapFilename;
  // Pcc options
  pcc_quality::commandPar _pccParams;
  // PCQM options
  double _pcqmRadiusCurvature    = 0.001;
  int    _pcqmThresholdKnnSearch = 20;
  double _pcqmRadiusFactor       = 2.0;

  // Raster options
  unsigned int _ibsmResolution        = 2048;
  unsigned int _ibsmCameraCount       = 16;
  std::string  _ibsmCameraRotation    = "0.0 0.0 0.0";
  glm::vec3    _ibsmCamRotParams      = {0.0F, 0.0F, 0.0F};
  std::string  _ibsmRenderer          = "sw_raster";
  std::string  _ibsmOutputPrefix      = "";
  bool         _ibsmDisableReordering = false;
  bool         _ibsmDisableCulling    = false;

  // Compare
  mm::Compare _compare;

 public:
  CmdCompare() {
    _pccParams.singlePass      = false;
    _pccParams.hausdorff       = false;
    _pccParams.bColor          = true;
    _pccParams.bLidar          = false;  // allways false, no option
    _pccParams.resolution      = 0.0;    // auto
    _pccParams.neighborsProc   = 1;
    _pccParams.dropDuplicates  = 2;
    _pccParams.bAverageNormals = true;

    // Modification of D2 metric is enabled for mmetric
    _pccParams.normalCalcModificationEnable = true;
  };

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
