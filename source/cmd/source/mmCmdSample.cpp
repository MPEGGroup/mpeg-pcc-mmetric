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

// Note : meshToPcFace method based on original face sampling code from Owlii (m42816)

#include <iostream>
#include <fstream>
#include <set>
#include <time.h>
#include <math.h>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
// argument parsing
#include <cxxopts.hpp>

// internal headers
#include "mmGeometry.h"
#include "mmIO.h"
#include "mmSample.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmCmdSample.h"

// Descriptions of the command
const char* CmdSample::name = "sample";
const char* CmdSample::brief = "Convert mesh to point cloud";

//
Command* CmdSample::create() { return new CmdSample(); }

//
bool CmdSample::initialize(Context* context, std::string app, int argc, char* argv[]) {
    _context = context;

    // command line parameters
    try {
        cxxopts::Options options(app + " " + name, brief);
        // clang-format off
        options.add_options()
            ("i,inputModel", "path to input model (obj or ply file)",
                cxxopts::value<std::string>())
            ("m,inputMap", "path to input texture map (png, jpg, rgb, yuv), can be multiple paths surrounded by double quotes and separated by spaces.",
                cxxopts::value<std::string>())
            ("o,outputModel", "path to output model (obj or ply file)",
                cxxopts::value<std::string>())
            ("mode", "the sampling mode in [face,grid,map,sdiv,ediv,prnd]",
                cxxopts::value<std::string>())
            ("hideProgress", "hide progress display in console for use by robot",
                cxxopts::value<bool>()->default_value("false"))
            ("outputCsv", "filename of the file where per frame statistics will append.",
                cxxopts::value<std::string>()->default_value(""))
            ("h,help", "Print usage")
            ;
        options.add_options("face mode")
            ("float", "if set the processings and outputs will be float32, int32 otherwise",
                cxxopts::value<bool>()->default_value("true"))
            ("thickness", "floating point value, distance to border of the face",
                cxxopts::value<float>()->default_value("0.0"))
            ;
        options.add_options("sdiv mode")
            ("maxDepth", "maximum recursion depth, maxDepth=1 means keep only original vertices",
                cxxopts::value<int>()->default_value("100"))
            ("areaThreshold", "face area limit to stop subdivision",
                cxxopts::value<float>()->default_value("1.0"))
            ("mapThreshold", "if set will refine until face vertices texels are distanced of 1 and areaThreshold reached",
                cxxopts::value<bool>()->default_value("false"))
            ;
        options.add_options("ediv mode")
            ("lengthThreshold", "edge length limit to stop subdivision, used only if > 0, otherwise resolution is used.",
                cxxopts::value<float>()->default_value("0.0"))
            ;
        options.add_options("grid mode")
            ("gridSize", "integer value in [1,maxint], side size of the grid",
                cxxopts::value<int>()->default_value("1024"))
            ("useNormal", "if set will sample only in the direction with the largest dot product with the triangle normal",
                cxxopts::value<bool>()->default_value("false"))
            ("minPos", "min corner of vertex position bbox, a string of three floats.",
                cxxopts::value<std::string>())
            ("maxPos", "max corner of vertex position bbox, a string of three floats.",
                cxxopts::value<std::string>())
            ("useFixedPoint", "interprets minPos and maxPos inputs as fixed point 16.",
                cxxopts::value<bool>())
            ;
        options.add_options("prnd mode")
            ("nbSamples", "integer value specifying the traget number of points in the output point cloud",
                cxxopts::value<size_t>()->default_value("2000000"))
            ;
        options.add_options("face and ediv modes")
            ("resolution", "integer value in [1,maxuint], step/edgeLength = resolution / size(largest bbox side). In ediv mode, the resolution is used only if lengthThreshold=0.",
                cxxopts::value<size_t>()->default_value("1024"))
            ;

        options.add_options("grid, face, sdiv and ediv modes.")
            ("bilinear", "if set, texture filtering will be bilinear, nearest otherwise",
                cxxopts::value<bool>()->default_value("false"))
            ("nbSamplesMin", "if set different from 0, the system will rerun the sampling multiple times to find the best parameter producing a number of samples in [nbAmplesMin, nbSamplesMax]. This process is very time comsuming.",
                cxxopts::value<size_t>()->default_value("0"))
            ("nbSamplesMax", "see --nbSamplesMin documentation. Must be > to --nbSamplesMin.",
                cxxopts::value<size_t>()->default_value("0"))
            ("maxIterations", "Maximum number of iterations in sample count constrained sampling, i.e. when --nbSampleMin > 0.",
                cxxopts::value<size_t>()->default_value("10"))
            ;
        // clang-format on

        auto result = options.parse(argc, argv);

        // Analyse the options
        if (result.count("help") || result.arguments().size() == 0) {
            std::cout << options.help() << std::endl;
            return false;
        }
        //
        if (result.count("inputModel")) inputModelFilename = result["inputModel"].as<std::string>();
        else {
            std::cerr << "Error: missing inputModel parameter" << std::endl;
            std::cout << options.help() << std::endl;
            return false;
        }
        //
        if (result.count("inputMap")) {
            parseStringList(result["inputMap"].as<std::string>(), _inputTextureFilenames);
        }
        //
        if (result.count("outputModel")) outputModelFilename = result["outputModel"].as<std::string>();
        else {
            std::cerr << "Error: missing outputModel parameter" << std::endl;
            std::cout << options.help() << std::endl;
            return false;
        }

        //
        if (result.count("mode")) mode = result["mode"].as<std::string>();

        if (mode != "face" && mode != "grid" && mode != "map" && mode != "sdiv" && mode != "ediv" && mode != "prnd") {
            std::cerr << "Error: invalid mode \"" << mode << std::endl;
            return false;
        }

        //
        if (result.count("outputCsv")) _outputCsvFilename = result["outputCsv"].as<std::string>();

        //
        if (result.count("hideProgress")) hideProgress = result["hideProgress"].as<bool>();
        //
        if (result.count("resolution")) _resolution = result["resolution"].as<size_t>();
        if (result.count("thickness")) thickness = result["thickness"].as<float>();
        if (result.count("maxDepth")) maxDepth = std::max(1, result["maxDepth"].as<int>());
        if (result.count("areaThreshold")) areaThreshold = result["areaThreshold"].as<float>();
        if (result.count("lengthThreshold")) lengthThreshold = result["lengthThreshold"].as<float>();
        if (result.count("mapThreshold")) mapThreshold = result["mapThreshold"].as<bool>();
        if (result.count("gridSize")) _gridSize = result["gridSize"].as<int>();
        if (result.count("useNormal")) _useNormal = result["useNormal"].as<bool>();
        if (result.count("bilinear")) bilinear = result["bilinear"].as<bool>();
        if (result.count("nbSamplesMin")) _nbSamplesMin = result["nbSamplesMin"].as<size_t>();
        if (result.count("nbSamplesMax")) {
            _nbSamplesMax = result["nbSamplesMax"].as<size_t>();
            if (_nbSamplesMax <= _nbSamplesMin) {
                std::cerr << "Error: nbSampleMax must be > to nbSampleMin" << std::endl;
                return false;
            }
        }
        if (result.count("maxIterations")) _maxIterations = result["maxIterations"].as<size_t>();

        // prnd options
        if (result.count("nbSamples")) _nbSamples = result["nbSamples"].as<size_t>();

        // grid options
        if (result.count("minPos")) {
            _minPosStr = result["minPos"].as<std::string>();
            if (!parseVec3(_minPosStr, _minPos)) {
                std::cout << "Error: parsing --minPos=\"" << _minPosStr << "\" expected three floats with space separator"
                    << std::endl;
                return false;
            }
        }
        if (result.count("maxPos")) {
            _maxPosStr = result["maxPos"].as<std::string>();
            if (!parseVec3(_maxPosStr, _maxPos)) {
                std::cout << "Error: parsing --maxPos=\"" << _maxPosStr << "\" expected three floats with space separator"
                    << std::endl;
                return false;
            }
        }
        if (result.count("useFixedPoint")) { _useFixedPoint = result["useFixedPoint"].as<bool>(); }

    }
    catch (const cxxopts::OptionException& e) {
        std::cout << "error parsing options: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool CmdSample::process(uint32_t frame) {
    
    // the input
    mm::ModelPtr inputModel = mm::IO::loadModel(inputModelFilename);
    if (!inputModel) return false;
    if (inputModel->vertices.size() == 0 || inputModel->triangles.size() == 0) {
        std::cout << "Error: invalid input model from " << inputModelFilename << std::endl;
        return false;
    }

    // now handle the textures
    std::vector<std::string> textureMapUrls;
    if (_inputTextureFilenames.size() != 0)
        textureMapUrls = _inputTextureFilenames;
    else
        textureMapUrls = inputModel->textureMapUrls;

    std::vector<mm::ImagePtr> textureMapList;
    mm::IO::loadImages(textureMapUrls, textureMapList);
    bool mustFreeDummyImage=false;
    if (textureMapList.empty()) {
        std::cout << "Skipping map read, will parse/use vertex color if any" << std::endl;
        textureMapList.push_back(mm::ImagePtr( new mm::Image() ));
        textureMapUrls.push_back(""); // no url for this invalid map
    }

    // the output
    mm::ModelPtr outputModel(new mm::Model());

    // create or open in append mode output csv if needed
    std::streamoff csvFileLength = 0;
    std::ofstream  csvFileOut;
    if (_outputCsvFilename != "") {
        // check if csv file is empty, need to open in read mode
        std::ifstream filestr;
        filestr.open(_outputCsvFilename, std::ios::binary);
        if (filestr) {
            filestr.seekg(0, std::ios::end);
            csvFileLength = filestr.tellg();
            filestr.close();
        }
        // let's open in append write mode
        csvFileOut.open(_outputCsvFilename.c_str(), std::ios::out | std::ofstream::app);
        // this is mandatory to print floats with full precision
        csvFileOut.precision(std::numeric_limits<float>::max_digits10);
    }

    // Perform the processings
    clock_t t1 = clock();
    if (mode == "face") {
        std::cout << "Sampling in FACE mode" << std::endl;
        std::cout << "  Resolution = " << _resolution << std::endl;
        std::cout << "  Thickness = " << thickness << std::endl;
        std::cout << "  Bilinear = " << bilinear << std::endl;
        std::cout << "  hideProgress = " << hideProgress << std::endl;
        std::cout << "  nbSamplesMin = " << _nbSamplesMin << std::endl;
        std::cout << "  nbSamplesMax = " << _nbSamplesMax << std::endl;
        std::cout << "  maxIterations = " << _maxIterations << std::endl;
        size_t computedResolution = 0;
        if (_nbSamplesMin != 0) {
            std::cout << "  using contrained mode with nbSamples (costly!)" << std::endl;
            mm::Sample::meshToPcFace(
                *inputModel,
                *outputModel,
                textureMapList,
                _nbSamplesMin,
                _nbSamplesMax,
                _maxIterations,
                thickness,
                bilinear,
                !hideProgress,
                computedResolution);
        }
        else {
            std::cout << "  using contrained mode with resolution " << std::endl;
            mm::Sample::meshToPcFace(
                *inputModel, *outputModel, textureMapList, _resolution, thickness, bilinear, !hideProgress);
        }
        // print the stats
        if (csvFileOut) {
            // print the header if file is empty
            if (csvFileLength == 0) {
                csvFileOut << "model;texture;frame;resolution;thickness;bilinear;nbSamplesMin;"
                    << "nbSamplesMax;maxIterations;computedResolution;nbSamples" << std::endl;
            }
            // print stats
            csvFileOut << inputModelFilename << ";" << textureMapUrls[0] << ";" << frame << ";" << _resolution << ";"
                << thickness << ";" << bilinear << ";" << _nbSamplesMin << ";" << _nbSamplesMax << ";"
                << _maxIterations << ";" << computedResolution << ";" << outputModel->getPositionCount() << std::endl;
            // done
            csvFileOut.close();
        }
    }
    else if (mode == "grid") {
        std::cout << "Sampling in GRID mode" << std::endl;
        std::cout << "  Grid Size = " << _gridSize << std::endl;
        std::cout << "  Use Normal = " << _useNormal << std::endl;
        std::cout << "  Bilinear = " << bilinear << std::endl;
        std::cout << "  hideProgress = " << hideProgress << std::endl;
        std::cout << "  nbSamplesMin = " << _nbSamplesMin << std::endl;
        std::cout << "  nbSamplesMax = " << _nbSamplesMax << std::endl;
        std::cout << "  maxIterations = " << _maxIterations << std::endl;
        size_t computedResolution = 0;
        if (_nbSamplesMin != 0) {
            std::cout << "  using contrained mode with nbSamples (costly!)" << std::endl;
            mm::Sample::meshToPcGrid(*inputModel,
                *outputModel,
                textureMapList,
                _nbSamplesMin,
                _nbSamplesMax,
                _maxIterations,
                bilinear,
                !hideProgress,
                _useNormal,
                _useFixedPoint,
                _minPos,
                _maxPos,
                computedResolution);
        }
        else {
            std::cout << "  using contrained mode with gridSize " << std::endl;
            mm::Sample::meshToPcGrid(*inputModel,
                *outputModel,
                textureMapList,
                _gridSize,
                bilinear,
                !hideProgress,
                _useNormal,
                _useFixedPoint,
                _minPos,
                _maxPos);
        }
        // print the stats
        if (csvFileOut) {
            // print the header if file is empty
            if (csvFileLength == 0) {
                csvFileOut << "model;texture;frame;mode;gridSize;useNormal;bilinear;nbSamplesMin;"
                    << "nbSamplesMax;maxIterations;computedResolution;nbSamples" << std::endl;
            }
            // print stats
            csvFileOut << inputModelFilename << ";" << textureMapUrls[0] << ";" << frame << ";" << mode << ";" << _gridSize
                << ";" << _useNormal << ";" << bilinear << ";" << _nbSamplesMin << ";" << _nbSamplesMax << ";"
                << _maxIterations << ";" << computedResolution << ";" << outputModel->getPositionCount() << std::endl;
            // done
            csvFileOut.close();
        }
    }
    else if (mode == "map") {
        std::cout << "Sampling in MAP mode" << std::endl;
        std::cout << "  hideProgress = " << hideProgress << std::endl;
        mm::Sample::meshToPcMap(*inputModel, *outputModel, textureMapList, !hideProgress);
        // print the stats
        if (csvFileOut) {
            // print the header if file is empty
            if (csvFileLength == 0) { csvFileOut << "model;texture;frame;mode;nbSamples" << std::endl; }
            // print stats
            csvFileOut << inputModelFilename << ";" << textureMapUrls[0] << ";" << frame << ";" << mode << ";"
                << outputModel->getPositionCount() << std::endl;
            // done
            csvFileOut.close();
        }
    }
    else if (mode == "sdiv") {
        std::cout << "Sampling in SDIV mode" << std::endl;
        std::cout << "  Maximum depth = " << maxDepth << std::endl;
        std::cout << "  Area threshold = " << areaThreshold << std::endl;
        std::cout << "  Map threshold = " << mapThreshold << std::endl;
        std::cout << "  Bilinear = " << bilinear << std::endl;
        std::cout << "  hideProgress = " << hideProgress << std::endl;
        std::cout << "  nbSamplesMin = " << _nbSamplesMin << std::endl;
        std::cout << "  nbSamplesMax = " << _nbSamplesMax << std::endl;
        std::cout << "  maxIterations = " << _maxIterations << std::endl;
        float computedThres = 0.0f;
        if (_nbSamplesMin != 0) {
            std::cout << "  using contrained mode with nbSamples (costly!)" << std::endl;
            mm::Sample::meshToPcDiv(*inputModel,
                *outputModel,
                textureMapList,
                _nbSamplesMin,
                _nbSamplesMax,
                _maxIterations,
                bilinear,
                !hideProgress,
                computedThres);
        }
        else {
            mm::Sample::meshToPcDiv(
                *inputModel, *outputModel, textureMapList, maxDepth, areaThreshold, mapThreshold, bilinear, !hideProgress);
        }
        // print the stats
        if (csvFileOut) {
            // print the header if file is empty
            if (csvFileLength == 0) {
                csvFileOut << "model;texture;frame;mode;areaThreshold;bilinear;nbSamplesMin;"
                    << "nbSamplesMax;maxIterations;computedThreshold;nbSamples" << std::endl;
            }
            // print stats
            csvFileOut << inputModelFilename << ";" << textureMapUrls[0] << ";" << frame << ";" << mode << ";"
                << areaThreshold << ";" << bilinear << ";" << _nbSamplesMin << ";" << _nbSamplesMax << ";"
                << _maxIterations << ";" << computedThres << ";" << outputModel->getPositionCount() << std::endl;
            // done
            csvFileOut.close();
        }
    }
    else if (mode == "ediv") {
        std::cout << "Sampling in EDIV mode" << std::endl;
        std::cout << "  Edge length threshold = " << lengthThreshold << std::endl;
        std::cout << "  Resolution = " << _resolution << (lengthThreshold != 0 ? "(skipped)" : "") << std::endl;
        std::cout << "  Bilinear = " << bilinear << std::endl;
        std::cout << "  hideProgress = " << hideProgress << std::endl;
        std::cout << "  nbSamplesMin = " << _nbSamplesMin << std::endl;
        std::cout << "  nbSamplesMax = " << _nbSamplesMax << std::endl;
        std::cout << "  maxIterations = " << _maxIterations << std::endl;
        float computedThres = 0.0f;
        if (_nbSamplesMin != 0) {
            std::cout << "  using contrained mode with nbSamples (costly!)" << std::endl;
            mm::Sample::meshToPcDivEdge(*inputModel,
                *outputModel,
                textureMapList,
                _nbSamplesMin,
                _nbSamplesMax,
                _maxIterations,
                bilinear,
                !hideProgress,
                computedThres);
        }
        else {
            mm::Sample::meshToPcDivEdge(
                *inputModel, *outputModel, textureMapList, lengthThreshold, _resolution, bilinear, !hideProgress, computedThres);
        }
        // print the stats
        if (csvFileOut) {
            // print the header if file is empty
            if (csvFileLength == 0) {
                csvFileOut << "model;texture;frame;mode;resolution;lengthThreshold;bilinear;nbSamplesMin;"
                    << "nbSamplesMax;maxIterations;computedThreshold;nbSamples" << std::endl;
            }
            // print stats
            csvFileOut << inputModelFilename << ";" << textureMapUrls[0] << ";" << frame << ";" << mode << ";"
                << _resolution << ";" << lengthThreshold << ";" << bilinear << ";" << _nbSamplesMin << ";"
                << _nbSamplesMax << ";" << _maxIterations << ";" << computedThres << ";"
                << outputModel->getPositionCount() << std::endl;
            // done
            csvFileOut.close();
        }
    }
    else if (mode == "prnd") {
        std::cout << "Sampling in PRND mode" << std::endl;
        std::cout << "  nbSamples = " << _nbSamples << std::endl;
        std::cout << "  Bilinear = " << bilinear << std::endl;
        std::cout << "  hideProgress = " << hideProgress << std::endl;
        mm::Sample::meshToPcPrnd(*inputModel, *outputModel, textureMapList, _nbSamples, bilinear, !hideProgress);
        // print the stats
        if (csvFileOut) {
            // print the header if file is empty
            if (csvFileLength == 0) {
                csvFileOut << "model;texture;frame;mode;targetPointCount;bilinear;nbSamples" << std::endl;
            }
            // print stats
            csvFileOut << inputModelFilename << ";" << textureMapUrls[0] << ";" << frame << ";" << mode << ";"
                << _nbSamples << ";" << bilinear << ";" << outputModel->getPositionCount() << std::endl;
            // done
            csvFileOut.close();
        }
    }
    clock_t t2 = clock();
    std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

    // save the result
    return mm::IO::saveModel(outputModelFilename, outputModel);
}
