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

#ifndef _MM_CMD_RENDER_H_
#define _MM_CMD_RENDER_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmRendererHw.h"
#include "mmRendererSw.h"

class CmdRender : Command {
 private:
  // the command options
  std::string inputModelFilename;
  std::vector<std::string> inputTextureFilenames;
  std::string outputImageFilename = "output.png";
  std::string outputDepthFilename = "";
  std::string renderer            = "sw_raster";
  bool        hideProgress        = false;
  // the type of processing
  unsigned int width  = 1920;
  unsigned int height = 1080;
  // filtering bilinear or nearest
  bool bilinear = true;
  // view vectors
  std::string viewDirStr;
  std::string viewUpStr;
  glm::vec3   viewDir = {0.0F, 0.0F, 1.0F};
  glm::vec3   viewUp  = {0.0F, 1.0F, 0.0F};
  // bbox min and max
  std::string bboxMinStr;
  std::string bboxMaxStr;
  glm::vec3   bboxMin;
  glm::vec3   bboxMax;
  bool        bboxValid = false;
  // clear color
  std::string clearColorStr;
  glm::vec4   clearColor = {0, 0, 0, 0};
  // culling
  bool enableCulling = false;
  bool cwCulling     = true;
  // lighting
  bool        enableLighting    = false;
  bool        autoLightPosition = false;
  std::string lightAutoDirStr;
  glm::vec3   lightAutoDir{1.0F, 1.0F, 1.0F};
  // the Software renderer
  mm::RendererSw _swRenderer;
  // the Hardware renderer
  mm::RendererHw _hwRenderer;

 public:
  CmdRender(){};

  // Description of the command
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