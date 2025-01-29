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

#ifndef _MM_COMPARE_H_
#define _MM_COMPARE_H_

#include "mmModel.h"
#include "mmContext.h"
#include "mmRendererHw.h"
#include "mmRendererSw.h"
// MPEG PCC metric
#include "dmetric/source/pcc_distortion.hpp"

namespace mm {

    class Compare {
    public:
        struct IbsmResults {
            double rgbMSE[4] = { 0, 0, 0, 0 };  // [R mse, G mse, B mse, components mean]
            double rgbPSNR[4] = { 0, 0, 0, 0 };  // idem but with psnr
            double yuvMSE[4] = { 0, 0, 0, 0 };  // [Y mse, U mse, V mse, 6/1/1 mean ]
            double yuvPSNR[4] = { 0, 0, 0, 0 };  // idem but with psnr
            double depthMSE = 0;
            double depthPSNR = 0;
            double boxRatio;  // ( dis box size / ref box size ) * 100.0
            double unmatchedPixelPercentage = 0;	// ( unmatchedPixelsSum /  maskSizeSum ) * 100.0
        };

        Compare();
        ~Compare();

    private:
        // Pcc results array of <frame, result>
        std::vector<std::pair<uint32_t, pcc_quality::qMetric> > _pccResults;
        // PCQM results array of <frame, pcqm, pcqm-psnr>
        std::vector<std::tuple<uint32_t, double, double> > _pcqmResults;
        // Raster results array of <frame, result>
        std::vector<std::pair<uint32_t, IbsmResults> > _ibsmResults;

        //per Point
        std::vector<std::pair<uint32_t, std::vector<pcc_quality::qMetric>>> _pccResultsPerPoint[2];

        // Renderers for the ibsm metric
        mm::RendererSw _swRenderer;             // the Software renderer
        mm::RendererHw _hwRenderer;             // the Hardware renderer
        bool           _hwRendererInitialized;  // the Hardware renderer is initilized

    public:
        auto& getPccResults() { return _pccResults.back(); };
        auto& getPcqmResults() { return _pcqmResults.back(); };
        auto& getIbsmResults() { return _ibsmResults.back(); };

        size_t size() {
            return (std::max)(_pccResults.size(),
                (std::max)(_pcqmResults.size(), _pcqmResults.size()));
        }
        std::vector<double> getFinalPccResults();
        std::vector<double> getFinalPcqmResults();
        std::vector<double> getFinalIbsmResults();
        std::vector<double> getPccResults(const size_t index);
        std::vector<double> getPcqmResults(const size_t index);
        std::vector<double> getIbsmResults(const size_t index);

        std::vector <std::vector<double>> getPccResultsPerPoint(const int abIndex);
        std::vector <std::vector<double>> getPccResultsPerPointMse(const int abIndex);
        std::vector<double> getPccResultsPerPoint(const int abIndex, const size_t index, const size_t pointIndex);
        std::vector<double> getPccResultsPerPointMse(const int abIndex, const size_t index, const size_t pointIndex);

        // compare two meshes for equality (using mem comp if epsilon = 0)
        // if epsilon = 0, return 0 on success and 1 on difference
        // if epsilon > 0, return 0 on success and nb diff on difference if sizes are equal, 1 otherwise
        int equ(
            const mm::Model& modelA,
            const mm::Model& modelB,
            const std::vector<mm::ImagePtr>& mapSetA,
            const std::vector<mm::ImagePtr>& mapSetB,
            float epsilon,
            bool earlyReturn,
            bool unoriented,
            mm::Model& outputA,
            mm::Model& outputB);

        int eqTFAN(
            mm::Model& modelA,
            mm::Model& modelB,
            const std::vector<mm::ImagePtr>& mapSetA,
            const std::vector<mm::ImagePtr>& mapSetB,
            float epsilon,
            bool earlyReturn,
            bool unoriented,
            mm::Model& outputA,
            mm::Model& outputB);

        // compare two meshes topology for equivalence up to face index shift
        // check topology will use a bijective face map, associating output triangles to input triangles:
        // - faceMap file shall contain the association dest face index -> orig face index for each face, one face per line
        // - %d in filename is to be resolved before invocation
        // check topology will use a bijective vartex map, associating output vertices to input vertices:
        // - vertexMap file shall contain the association dest vertex index -> orig vertex index for each vertex, one vertex
        // per line
        // - %d in filename is to be resolved before invocation
        // the function validates the following points:
        // - Test if number of triangles of output matches input number of triangles
        // - Test if the proposed association tables for face and vertex are bijective
        // - Test if each output triangle respects the orientation of its associated input triangle
        int topo(
            const mm::Model& modelA,
            const mm::Model& modelB,
            const std::string& faceMapFilenane = "",
            const std::string& vertexMapFilenane = "");

        // compare two meshes using MPEG pcc_distortion metric
        int pcc(
            const mm::Model& modelA,
            const mm::Model& modelB,
            const std::vector<mm::ImagePtr>& mapSetA,
            const std::vector<mm::ImagePtr>& mapSetB,
            pcc_quality::commandPar& params,
            mm::Model& outputA,
            mm::Model& outputB,
            const bool verbose = true,
            const bool removeDupA = true,
            const bool removeDupB = true,
            const bool calcMetPerPoint = false);

        // collect statics over sequence and compute results
        void pccFinalize(void);

        // compare two meshes using PCQM metric
        int pcqm(
            const mm::ModelPtr modelA,
            const mm::ModelPtr modelB,
            const std::vector<mm::ImagePtr>& mapSetA,
            const std::vector<mm::ImagePtr>& mapSetB,
            const double     radiusCurvature,
            const int        thresholdKnnSearch,
            const double     radiusFactor,
            mm::ModelPtr outputA,
            mm::ModelPtr outputB,
            const bool       verbose = true);

        // collect statics over sequence and compute results
        void pcqmFinalize(void);

        // compare two meshes using rasterization
        int ibsm(
            const mm::ModelPtr modelA,
            const mm::ModelPtr modelB,
            const std::vector<mm::ImagePtr>& mapSetA,
            const std::vector<mm::ImagePtr>& mapSetB,
            const bool         disableReordering,
            const uint32_t     resolution,
            const uint32_t     cameraCount,
            const glm::vec3& camRotParams,
            const std::string& renderer,
            const std::string& outputPrefix,
            const bool         disableCulling,
            mm::ModelPtr outputA,
            mm::ModelPtr outputB,
            const bool         verbose = true);

        // collect statics over sequence and compute results
        void ibsmFinalize(void);
    };

}  // namespace mm

#endif
