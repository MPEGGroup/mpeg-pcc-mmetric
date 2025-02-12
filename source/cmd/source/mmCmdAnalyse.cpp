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
#include "mmCmdAnalyse.h"
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmGeometry.h"
#include "mmStatistics.h"

const char* CmdAnalyse::name  = "analyse";
const char* CmdAnalyse::brief = "Analyse model and/or texture map";

//
Command* CmdAnalyse::create() { return new CmdAnalyse(); }

//
bool CmdAnalyse::initialize( Context* context, std::string app, int argc, char* argv[] ) {
  _context = context;

  // command line parameters
  try {
    cxxopts::Options options( app + " " + name, brief );
    // clang-format off
		options.add_options()
			("inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("inputMap", "path to input texture map (png, jpeg), can be multiple paths surrounded by double quotes and separated by spaces.",
				cxxopts::value<std::string>())
			("outputCsv", "optional path to output results file",
				cxxopts::value<std::string>())
			("outputVar", "optional path to output variables file",
				cxxopts::value<std::string>())
			("h,help", "Print usage")
			;
    // clang-format on

    auto result = options.parse( argc, argv );

    // Analyse the options
    if ( result.count( "help" ) || result.arguments().size() == 0 ) {
      std::cout << options.help() << std::endl;
      return false;
    }
    // Optional
    if ( result.count( "inputModel" ) ) _inputModelFilename = result["inputModel"].as<std::string>();
    // Optional
    if ( result.count( "inputMap" ) ) { 
        parseStringList( result["inputMap"].as<std::string>(), _inputTextureFilenames ); 
    }
    // Optional
    if ( result.count( "outputCsv" ) ) _outputCsvFilename = result["outputCsv"].as<std::string>();
    // Optional
    if ( result.count( "outputVar" ) ) _outputVarFilename = result["outputVar"].as<std::string>();
  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return false;
  }

  return true;
}

bool CmdAnalyse::process( uint32_t frame ) {

  // the input
  mm::ModelPtr inputModel = mm::IO::loadModel(_inputModelFilename);
  if (!inputModel) { return false; }

  // now handle the textures
  std::vector<std::string> textureMapUrls;
  if (_inputTextureFilenames.size() != 0)
      textureMapUrls = _inputTextureFilenames;
  else
      textureMapUrls = inputModel->textureMapUrls;

  std::vector<mm::ImagePtr> textureMapList;
  mm::IO::loadImages(textureMapUrls, textureMapList);
  bool mustFreeDummyImage = false;
  if (textureMapList.empty()) {
      std::cout << "Skipping map read, will parse/use vertex color if any" << std::endl;
      textureMapList.push_back(mm::ImagePtr(new mm::Image()));
      textureMapUrls.push_back(""); // no url for this invalid map
  }

  // create or open in append mode output csv if needed
  std::ofstream fout;
  if ( _outputCsvFilename != "" ) {
    if ( frame == _context->getFirstFrame() ) {
      fout.open( _outputCsvFilename.c_str(), std::ios::out );
    } else {
      fout.open( _outputCsvFilename.c_str(), std::ios::out | std::ofstream::app );
    }
    // this is mandatory to print floats with full precision
    fout.precision( std::numeric_limits<float>::max_digits10 );
  }

  // Perform the processings
  clock_t t1 = clock();

  glm::vec3 minPos, maxPos;
  glm::vec3 minNrm, maxNrm;
  glm::vec3 minCol, maxCol;
  glm::vec2 minUv, maxUv;

  // analyse the model if any
  if ( inputModel != NULL ) {
      //checking the number of texture maps
      auto materialIndices = inputModel->triangleMatIdx;
      std::sort(materialIndices.begin(), materialIndices.end());
      auto uniqCnt = std::unique(materialIndices.begin(), materialIndices.end());
      materialIndices.resize(std::distance(materialIndices.begin(), uniqCnt));

    _counts.push_back( std::make_tuple( _context->getFrame(),
                                        (double)inputModel->triangles.size() / 3,
                                        (double)inputModel->vertices.size() / 3,
                                        (double)inputModel->colors.size() / 3,
                                        (double)inputModel->normals.size() / 3,
                                        (double)inputModel->uvcoords.size() / 2,
                                        (double)materialIndices.size()));

    mm::Geometry::computeBBox( inputModel->vertices, minPos, maxPos );
    mm::Geometry::computeBBox( _minPos, _maxPos, minPos, maxPos, _minPos, _maxPos );
    if ( inputModel->normals.size() ) {
      mm::Geometry::computeBBox( inputModel->normals, minNrm, maxNrm );
      mm::Geometry::computeBBox( _minNrm, _maxNrm, minNrm, maxNrm, _minNrm, _maxNrm );
    }
    if ( inputModel->colors.size() ) {
      mm::Geometry::computeBBox( inputModel->colors, minCol, maxCol );
      mm::Geometry::computeBBox( _minCol, _maxCol, minCol, maxCol, _minCol, _maxCol );
    }
    if ( inputModel->uvcoords.size() ) {
      mm::Geometry::computeBBox( inputModel->uvcoords, minUv, maxUv );
      mm::Geometry::computeBBox( _minUv, _maxUv, minUv, maxUv, _minUv, _maxUv );
    }

    // TODO: add more stats
    // find degenerate triangles ?
    // count isolated vertices ?
  }

  // analyse the textures if any
  for ( auto image: textureMapList ) {
    if (image) {
       // do something on the current image
    }
  }

  // print to output csv if needed
  if ( fout ) {
    // print the header if needed
    if ( frame == _context->getFirstFrame() ) {
      fout << "frame";
      if ( inputModel != NULL ) {
        fout << ";triangles;vertices;uvcoords;colors;normals;materials"
             << ";minPosX;minPosY;minPosZ;maxPosX;maxPosY;maxPosZ"
             << ";minU;minV;maxU;maxV"
             << ";minColR;minColG;minColB;maxColR;maxColB;maxColB"
             << ";minNrmY;minNrmY;minNrmZ;maxNrmY;maxNrmY;maxNrmZ";
      }
      if ( textureMapList.size() != 0 ) {
        fout << "";  // nothing yet
      }
      fout << std::endl;
    }
    // print stats
    fout << frame;
    if ( inputModel != NULL ) {
      fout << ";" << inputModel->triangles.size() / 3 << ";" << inputModel->vertices.size() / 3 << ";"
           << inputModel->normals.size() / 3 << ";" << inputModel->colors.size() / 3 << ";"
           << inputModel->uvcoords.size() / 2 << ";" << inputModel->textureMapUrls.size() << ";" << minPos[0] << ";" << minPos[1] << ";" << minPos[2] << ";"
           << maxPos[0] << ";" << maxPos[1] << ";" << maxPos[2];
      if ( inputModel->uvcoords.size() ) {
        fout << ";" << minUv[0] << ";" << minUv[1] << ";" << maxUv[0] << ";" << maxUv[1];
      } else fout << ";;;;";
      if ( inputModel->colors.size() ) {
        fout << ";" << minCol[0] << ";" << minCol[1] << ";" << minCol[2] << ";" << maxCol[0] << ";" << maxCol[1] << ";"
             << maxCol[2];
      } else fout << ";;;;;;";
      if ( inputModel->normals.size() ) {
        fout << ";" << minNrm[0] << ";" << minNrm[1] << ";" << minNrm[2] << ";" << maxNrm[0] << ";" << maxNrm[1] << ";"
             << maxNrm[2];
      } else fout << ";;;;;;";
    }
    fout << std::endl;
    // done
    fout.close();
  }

  // done
  clock_t t2 = clock();
  std::cout << "Time on processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  // success
  return true;
}

bool CmdAnalyse::finalize() {
  std::vector<std::ostream*> out;
  out.push_back( &std::cout );
  std::ofstream fout;
  if ( _outputVarFilename != "" ) {
    fout.open( _outputVarFilename.c_str(), std::ios::out );
    if ( fout ) {
      out.push_back( &fout );
    } else {
      std::cout << "Error: could not create output file " << _outputVarFilename << std::endl;
    }
  }

  for ( size_t i = 0; i < out.size(); ++i ) {
    // this is mandatory to print floats with full precision
    out[i]->precision( std::numeric_limits<float>::max_digits10 );

    // display the statistics (compatioble with bash source)
    *out[i] << "firstFrame=" << _context->getFirstFrame() << std::endl;
    *out[i] << "lastFrame=" << _context->getLastFrame() << std::endl;

    mm::Statistics::Results stats;
    mm::Statistics::compute(
      _counts.size(), [&]( size_t i ) -> double { return std::get<1>( _counts[i] ); }, stats , std::numeric_limits<double>::max());
    mm::Statistics::printToLog( stats, "globalTriangleCount", *out[i] );
    mm::Statistics::compute(
      _counts.size(), [&]( size_t i ) -> double { return std::get<2>( _counts[i] ); }, stats, std::numeric_limits<double>::max());
    mm::Statistics::printToLog( stats, "globalVertexCount", *out[i] );
    mm::Statistics::compute(
      _counts.size(), [&]( size_t i ) -> double { return std::get<3>( _counts[i] ); }, stats, std::numeric_limits<double>::max());
    mm::Statistics::printToLog( stats, "globalColorCount", *out[i] );
    mm::Statistics::compute(
      _counts.size(), [&]( size_t i ) -> double { return std::get<4>( _counts[i] ); }, stats, std::numeric_limits<double>::max());
    mm::Statistics::printToLog( stats, "globalNormalCount", *out[i] );
    mm::Statistics::compute(
      _counts.size(), [&]( size_t i ) -> double { return std::get<5>( _counts[i] ); }, stats, std::numeric_limits<double>::max());
    mm::Statistics::printToLog( stats, "globalUvCoordCount", *out[i] );
    mm::Statistics::compute(
        _counts.size(), [&](size_t i) -> double { return std::get<6>(_counts[i]); }, stats, std::numeric_limits<double>::max());
    mm::Statistics::printToLog(stats, "globalTextureCount", *out[i]);

    *out[i] << "globalMinPos=\"" << _minPos[0] << " " << _minPos[1] << " " << _minPos[2] << "\"" << std::endl;
    *out[i] << "globalMaxPos=\"" << _maxPos[0] << " " << _maxPos[1] << " " << _maxPos[2] << "\"" << std::endl;

    *out[i] << "globalMinUv=\"" << _minUv[0] << " " << _minUv[1] << "\"" << std::endl;
    *out[i] << "globalMaxUv=\"" << _maxUv[0] << " " << _maxUv[1] << "\"" << std::endl;

    *out[i] << "globalMinNrm=\"" << _minNrm[0] << " " << _minNrm[1] << " " << _minNrm[2] << "\"" << std::endl;
    *out[i] << "globalMaxNrm=\"" << _maxNrm[0] << " " << _maxNrm[1] << " " << _maxNrm[2] << "\"" << std::endl;

    *out[i] << "globalMinCol=\"" << _minCol[0] << " " << _minCol[1] << " " << _minCol[2] << "\"" << std::endl;
    *out[i] << "globalMaxCol=\"" << _maxCol[0] << " " << _maxCol[1] << " " << _maxCol[2] << "\"" << std::endl;
  }
  // done
  if ( fout ) fout.close();

  return true;
}
