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

#ifndef _MM_SAMPLE_H_
#define _MM_SAMPLE_H_

// internal headers
#include "mmModel.h"
#include "mmImage.h"

namespace mm {

    class Sample {
    public:
        Sample() {};

        // sample the mesh on a face basis
        static void meshToPcFace(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            size_t       resolution,
            float        thickness,
            bool         bilinear,
            bool         logProgress);

        // sample the mesh on a face basis
        // system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
        // costly method, shall be used only for calibration
        static void meshToPcFace(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            size_t       nbSamplesMin,
            size_t       nbSamplesMax,
            size_t       maxIterations,
            float        thickness,
            bool         bilinear,
            bool         logProgress,
            size_t& computedResolution);

        // will sample the mesh on a grid basis of resolution gridRes
        static void meshToPcGrid(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            size_t       gridSize,
            bool         bilinear,
            bool         logProgress,
            bool         useNormal,
            bool         useFixedPoint,
            glm::vec3& minPos,
            glm::vec3& maxPos,
            const bool   verbose = true,
            std::vector<int>* faceIndexPerPoint = nullptr);

        // will sample the mesh on a grid basis of resolution gridRes, result will be generated as float or integer
        // system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
        // costly method, shall be used only for calibration
        static void meshToPcGrid(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            size_t       nbSamplesMin,
            size_t       nbSamplesMax,
            size_t       maxIterations,
            bool         bilinear,
            bool         logProgress,
            bool         useNormal,
            bool         useFixedPoint,
            glm::vec3& minPos,
            glm::vec3& maxPos,
            size_t& computedResolution);

        // revert sampling, guided by texture map
        static void meshToPcMap(
            const Model& input, 
            Model& output, 
            const std::vector<mm::ImagePtr>& textures, 
            bool logProgress);

        // triangle dubdivision based, area stop criterion
        static void meshToPcDiv(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            int          maxDepth,
            float        areaThreshold,
            bool         mapThreshold,
            bool         bilinear,
            bool         logProgress);

        // triangle dubdivision based, area stop criterion
        // system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
        // costly method, shall be used only for calibration
        static void meshToPcDiv(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            size_t       nbSamplesMin,
            size_t       nbSamplesMax,
            size_t       maxIterations,
            bool         bilinear,
            bool         logProgress,
            float& computedThres);

        // triangle dubdivision based, edge stop criterion
        static void meshToPcDivEdge(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            float        lengthThreshold,
            size_t       resolution,
            bool         bilinear,
            bool         logProgress,
            float& computedThres);

        // triangle dubdivision based, edge stop criterion
        // system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
        // costly method, shall be used only for calibration
        static void meshToPcDivEdge(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            size_t       nbSamplesMin,
            size_t       nbSamplesMax,
            size_t       maxIterations,
            bool         bilinear,
            bool         logProgress,
            float& computedThres);

        // pseudo random sampling with point targetPointCount stop criterion
        static void meshToPcPrnd(
            const Model& input,
            Model& output,
            const std::vector<mm::ImagePtr>& textures,
            size_t       targetPointCount,
            bool         bilinear,
            bool         logProgress);
    };

}  // namespace mm

#endif
