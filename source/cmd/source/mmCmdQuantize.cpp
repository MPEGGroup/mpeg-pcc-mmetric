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

#include <iostream>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include <math.h>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
// argument parsing
#include <cxxopts.hpp>

// internal headers
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmDequantize.h"
#include "mmQuantize.h"
#include "mmGeometry.h"
#include "mmColor.h"
#include "mmCmdQuantize.h"

const char* CmdQuantize::name  = "quantize";
const char* CmdQuantize::brief = "Quantize model (mesh or point cloud)";

//
Command* CmdQuantize::create() { return new CmdQuantize(); }

//
bool CmdQuantize::initialize( Context* ctx, std::string app, int argc, char* argv[] ) {
  // command line parameters
  try {
    cxxopts::Options options( app + " " + name, brief );
    // clang-format off
		options.add_options()
			("i,inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("o,outputModel", "path to output model (obj or ply file)",
				cxxopts::value<std::string>())
			("h,help", "Print usage")
			("dequantize", "set to process dequantification at the ouput")
			("qp", "Geometry quantization bitdepth. A value < 7 means no quantization.",
				cxxopts::value<uint32_t>()->default_value("12"))
			("qt", "UV coordinates quantization bitdepth.  A value < 7 means no quantization.",
				cxxopts::value<uint32_t>()->default_value("12"))
			("qn", "Normals quantization bitdepth. A value < 7 no quantization.",
				cxxopts::value<uint32_t>()->default_value("12"))
			("qc", "Colors quantization bitdepth. A value < 7 no quantization.",
				cxxopts::value<uint32_t>()->default_value("8"))
			("minPos", "min corner of vertex position bbox, a string of three floats. Computed if not set.",
				cxxopts::value<std::string>())
			("maxPos", "max corner of vertex position bbox, a string of three floats. Computed if not set.",
				cxxopts::value<std::string>())
			("minUv", "min corner of vertex texture coordinates bbox, a string of three floats. Computed if not set.",
				cxxopts::value<std::string>())
			("maxUv", "max corner of vertex texture coordinates bbox, a string of three floats. Computed if not set.",
				cxxopts::value<std::string>())
			("minNrm", "min corner of vertex normal bbox, a string of three floats. Computed if not set.",
				cxxopts::value<std::string>())
			("maxNrm", "max corner of vertex normal bbox, a string of three floats. Computed if not set.",
				cxxopts::value<std::string>())
			("minCol", "min corner of vertex color bbox, a string of three floats. Computed if not set.",
				cxxopts::value<std::string>())
			("maxCol", "max corner of vertex color bbox, a string of three floats. Computed if not set.",
				cxxopts::value<std::string>())
			("outputVar", "path to the output variables file.",
				cxxopts::value<std::string>())
			("useFixedPoint", "internally convert the minPos and maxPos to fixed point 16.",
				cxxopts::value<bool>())
			("colorSpaceConversion", "Convert color space from RGB to YUV.",
				cxxopts::value<bool>())
			;
    // clang-format on

    auto result = options.parse( argc, argv );

    // Analyse the options
    if ( result.count( "help" ) || result.arguments().size() == 0 ) {
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if ( result.count( "inputModel" ) ) _inputModelFilename = result["inputModel"].as<std::string>();
    else {
      std::cerr << "Error: missing inputModel parameter" << std::endl;
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if ( result.count( "outputModel" ) ) _outputModelFilename = result["outputModel"].as<std::string>();
    else {
      std::cerr << "Error: missing outputModel parameter" << std::endl;
      std::cout << options.help() << std::endl;
      return false;
    }

    if ( result.count( "outputVar" ) ) _outputVarFilename = result["outputVar"].as<std::string>();

    if ( result.count( "dequantize" ) ) _dequantize = result["dequantize"].as<bool>();

    if ( result.count( "qp" ) ) _qp = result["qp"].as<uint32_t>();

    if ( result.count( "qt" ) ) _qt = result["qt"].as<uint32_t>();

    if ( result.count( "qn" ) ) _qn = result["qn"].as<uint32_t>();

    if ( result.count( "qc" ) ) _qc = result["qc"].as<uint32_t>();

    if ( result.count( "minPos" ) ) {
      _minPosStr = result["minPos"].as<std::string>();
      if ( !parseVec3( _minPosStr, _minPos ) ) {
        std::cout << "Error: parsing --minPos=\"" << _minPosStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }
    if ( result.count( "maxPos" ) ) {
      _maxPosStr = result["maxPos"].as<std::string>();
      if ( !parseVec3( _maxPosStr, _maxPos ) ) {
        std::cout << "Error: parsing --maxPos=\"" << _maxPosStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }
    if ( result.count( "minUv" ) ) {
      _minUvStr = result["minUv"].as<std::string>();
      if ( !parseVec2( _minUvStr, _minUv ) ) {
        std::cout << "Error: parsing --minUv=\"" << _minUvStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }
    if ( result.count( "maxUv" ) ) {
      _maxUvStr = result["maxUv"].as<std::string>();
      if ( !parseVec2( _maxUvStr, _maxUv ) ) {
        std::cout << "Error: parsing --maxUv=\"" << _maxUvStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }
    if ( result.count( "minNrm" ) ) {
      _minNrmStr = result["minNrm"].as<std::string>();
      if ( !parseVec3( _minNrmStr, _minNrm ) ) {
        std::cout << "Error: parsing --minNrm=\"" << _minNrmStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }
    if ( result.count( "maxNrm" ) ) {
      _maxNrmStr = result["maxNrm"].as<std::string>();
      if ( !parseVec3( _maxNrmStr, _maxNrm ) ) {
        std::cout << "Error: parsing --maxNrm=\"" << _maxNrmStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }
    if ( result.count( "minCol" ) ) {
      _minColStr = result["minCol"].as<std::string>();
      if ( !parseVec3( _minColStr, _minCol ) ) {
        std::cout << "Error: parsing --minCol=\"" << _minColStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }
    if ( result.count( "maxCol" ) ) {
      _maxColStr = result["maxCol"].as<std::string>();
      if ( !parseVec3( _maxColStr, _maxCol ) ) {
        std::cout << "Error: parsing --maxCol=\"" << _maxColStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }
    if ( result.count( "useFixedPoint" ) ) { _useFixedPoint = result["useFixedPoint"].as<bool>(); }
    if ( result.count( "colorSpaceConversion" ) ) _colorSpaceConversion = result["colorSpaceConversion"].as<bool>();
  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return false;
  }

  return true;
}

bool CmdQuantize::process( uint32_t frame ) {
  // the input
    mm::ModelPtr inputModel = mm::IO::loadModel(_inputModelFilename);
    if (!inputModel) return false;
  if ( inputModel->vertices.size() == 0 ) {
    std::cout << "Error: invalid input model from " << _inputModelFilename << std::endl;
    return false;
  }

  // the output
  mm::ModelPtr outputModel( new mm::Model() );

  // Perform the processings
  clock_t t1 = clock();

  std::cout << "Quantizing" << std::endl;
  std::cout << "  qp = " << _qp << std::endl;
  std::cout << "  qt = " << _qt << std::endl;
  std::cout << "  qn = " << _qn << std::endl;
  std::cout << "  qc = " << _qc << std::endl;

  // use temp values to prevent global param erase by call to quantize at each new frame
  glm::vec3 minPos = _minPos;
  glm::vec3 maxPos = _maxPos;
  glm::vec2 minUv  = _minUv;
  glm::vec2 maxUv  = _maxUv;
  glm::vec3 minNrm = _minNrm;
  glm::vec3 maxNrm = _maxNrm;
  glm::vec3 minCol = _minCol;
  glm::vec3 maxCol = _maxCol;

  if ( _dequantize ) {
    mm::Model* quantizedModel = new mm::Model();

    mm::Quantize::quantize( *inputModel,
                            *quantizedModel,
                            _qp,
                            _qt,
                            _qn,
                            _qc,
                            _outputVarFilename,
                            _useFixedPoint,
                            _colorSpaceConversion,
                            minPos,
                            maxPos,
                            minUv,
                            maxUv,
                            minNrm,
                            maxNrm,
                            minCol,
                            maxCol );

    // uses min/max potentially updated by previous line call to quantize
    mm::Dequantize::dequantize( *quantizedModel,
                                *outputModel,
                                _qp,
                                _qt,
                                _qn,
                                _qc,
                                minPos,
                                maxPos,
                                minUv,
                                maxUv,
                                minNrm,
                                maxNrm,
                                minCol,
                                maxCol,
                                _useFixedPoint,
                                _colorSpaceConversion );

    delete quantizedModel;
  } else {
    // uses min/max potentially updated by previous frame call to quantize
    mm::Quantize::quantize( *inputModel,
                            *outputModel,
                            _qp,
                            _qt,
                            _qn,
                            _qc,
                            _outputVarFilename,
                            _useFixedPoint,
                            _colorSpaceConversion,
                            minPos,
                            maxPos,
                            minUv,
                            maxUv,
                            minNrm,
                            maxNrm,
                            minCol,
                            maxCol );
  }

  clock_t t2 = clock();
  std::cout << "Time on processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  // save the result
  return mm::IO::saveModel( _outputModelFilename, outputModel );

}
