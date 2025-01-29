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

#ifndef _MM_RENDERER_HW_H_
#define _MM_RENDERER_HW_H_

#include <string>

//
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// internal headers
#include "mmImage.h"
#include "mmModel.h"

namespace mm {

struct RendererHw {
  // open the output render context and associated hidden window
  bool initialize( const unsigned int width, const unsigned int height );
  // release OpenGL resources
  bool shutdown( void );

  // render a mesh to memory
  bool render(
      ModelPtr model,
      const std::vector<mm::ImagePtr>& mapSet,
      std::vector<uint8_t>& fbuffer,
      std::vector<float>& zbuffer,
      const unsigned int    width,
      const unsigned int    height,
      const glm::vec3& viewDir,
      const glm::vec3& viewUp,
      const glm::vec3& bboxMin,
      const glm::vec3& bboxMax,
      const bool useBBox,
      const bool verbose = true);

  // render a mesh to PNG image files
  bool render(
      ModelPtr model,
      const std::vector<mm::ImagePtr>& mapSet,
      const std::string& outputImage,
      const std::string& outputDepth,
      const unsigned int width,
      const unsigned int height,
      const glm::vec3& viewDir,
      const glm::vec3& viewUp,
      const glm::vec3& bboxMin,
      const glm::vec3& bboxMax,
      const bool useBBox,
      const bool verbose = true);

  // clear the buffers
  void clear( std::vector<char>& fbuffer, std::vector<float>& zbuffer );

  //
  inline void setClearColor( glm::vec4 color ) { _clearColor = color; }

  // Culling
  inline void enableCulling() { _isCullingEnabled = true; }
  inline void disableCulling() { _isCullingEnabled = false; }
  // if true sets to CW culling, CCW otherwise
  inline void setCwCulling( bool value ) { _cwCulling = value; }

 private:
  // render parameters
  glm::vec4 _clearColor{0, 0, 0, 0};

  bool _isCullingEnabled = false;  // enable back face culling
  bool _cwCulling        = true;   // defaults faces orientation to clock wise culling

  // GLFW
  GLFWwindow*  _window;
  unsigned int _width  = 640;
  unsigned int _height = 480;

  // GL stuffs
  GLuint _fbo, _color_rb, _depth_rb;
};

}  // namespace mm

#endif
