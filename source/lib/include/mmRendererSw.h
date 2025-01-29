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

#ifndef _MM_RENDERER_SW_H_
#define _MM_RENDERER_SW_H_

#include <string>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// internal headers
#include "mmImage.h"
#include "mmModel.h"

namespace mm {

    struct RendererSw {
        // render a mesh to memory
        bool render(
            ModelPtr model,
            const std::vector<ImagePtr>& mapSet,
            std::vector<uint8_t>& fbuffer,
            std::vector<float>& zbuffer,
            const unsigned int    width,
            const unsigned int    height,
            const glm::vec3& viewDir,
            const glm::vec3& viewUp,
            const glm::vec3& bboxMin,
            const glm::vec3& bboxMax,
            bool                  useBBox,
            const bool            verbose = true);

        // render a mesh to PNG image files
        bool render(
            ModelPtr model,
            const std::vector<ImagePtr>& mapSet,
            const std::string& outputImage,
            const std::string& outputDepth,
            const unsigned int width,
            const unsigned int height,
            const glm::vec3& viewDir,
            const glm::vec3& viewUp,
            const glm::vec3& bboxMin,
            const glm::vec3& bboxMax,
            bool               useBBox,
            const bool         verbose = true);

        // Buffers cleanup
        void        clear(std::vector<uint8_t>& fbuffer, std::vector<float>& zbuffer);
        inline void setClearColor(glm::vec4 color) { _clearColor = color; }
        // depth shall be negative
        inline void setClearDepth(float depth) { _clearDepth = depth; }

        // Culling
        inline void enableCulling() { _isCullingEnabled = true; }
        inline void disableCulling() { _isCullingEnabled = false; }
        // if true sets to CW culling, CCW otherwise
        inline void setCwCulling(bool value) { _cwCulling = value; }

        // Lighting
        inline void enableLighting() { _isLigthingEnabled = true; }
        inline void disableLighting() { _isLigthingEnabled = false; }
        inline void enableAutoLightPosition() { _isAutoLightPositionEnabled = true; }
        inline void disableAutoLightPosition() { _isAutoLightPositionEnabled = false; }
        inline void setLightAutoDir(glm::vec3 direction) { _lightAutoDir = direction; }
        inline void setLightPosition(glm::vec4 position) { _lightPosition = position; }
        inline void setLightcolor(glm::vec3 color) { _lightColor = color; }

        // Materials
        inline void setMaterialAmbient(glm::vec3 Ka) { _materialAmbient = Ka; }
        inline void setMaterialDiffuse(glm::vec3 Kd) { _materialDiffuse = Kd; }

        // Post process
        inline void enableAutoLevel() { _isAutoLevelEnabled = true; }
        inline void disableAutoLevel() { _isAutoLevelEnabled = false; }

    public:
        // after each render this values are updated and can be read
        float depthRange;  // represents the maximum depth for the render bbox in the depthBuffer

    private:
        glm::vec4 _clearColor{ 0, 0, 0, 0 };
        float     _clearDepth = -std::numeric_limits<float>::max();
        bool      _isCullingEnabled = false;  // enable back face culling
        bool      _cwCulling = true;   // defaults faces orientation to clock wise culling
        bool      _isLigthingEnabled = false;
        bool      _isAutoLightPositionEnabled = false;
        glm::vec3 _lightAutoDir{ 1.0F, 1.0F, 1.0F };     // vector to compute automatic position, top right by default
        glm::vec4 _lightPosition{ 0, 0, 0, 1 };          // user defined position
        glm::vec3 _lightColor{ 1.0F, 1.0F, 1.0F };       // in 0-1 for each component
        glm::vec3 _materialAmbient{ 0.4F, 0.4F, 0.4F };  // Kd for each component
        glm::vec3 _materialDiffuse{ 0.6F, 0.6F, 0.6F };  // Ka for each component

        bool _isAutoLevelEnabled = false;
    };

}  // namespace mm

#endif
