﻿/* The copyright in this software is being made available under the BSD
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

// Note: meshToPcFace method based on original face sampling code from Owlii (m42816)

#include <iostream>
#include <fstream>
#include <sstream>
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
#include "mmGeometry.h"
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmCmdDegrade.h"

// Descriptions of the command
const char* CmdDegrade::name  = "degrade";
const char* CmdDegrade::brief = "Degrade a mesh (todo points)";

//
Command* CmdDegrade::create() { return new CmdDegrade(); }

//
bool CmdDegrade::initialize( Context* ctx, std::string app, int argc, char* argv[] ) {
  // command line parameters
  try {
    cxxopts::Options options( app + " " + name, brief );
    // clang-format off
		options.add_options()
			("i,inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("o,outputModel", "path to output model (obj or ply file)",
				cxxopts::value<std::string>())
			("mode", "the sampling mode in [delface]",
				cxxopts::value<std::string>())
			("nthFace", "in delface mode, remove one face every nthFace.",
				cxxopts::value<size_t>()->default_value("50"))
			("nbFaces", "in delface mode, if nthFace==0, remove nbFaces.",
				cxxopts::value<size_t>()->default_value("0"))
			("h,help", "Print usage")
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
    //
    if ( result.count( "mode" ) ) _mode = result["mode"].as<std::string>();

    if ( result.count( "nthFace" ) ) _nthFace = result["nthFace"].as<size_t>();

    if ( result.count( "nbFaces" ) ) _nbFaces = result["nbFaces"].as<size_t>();

  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return false;
  }

  return true;
}

bool CmdDegrade::process( uint32_t frame ) {
  // the input
    mm::ModelPtr inputModel = mm::IO::loadModel(_inputModelFilename);
    if (!inputModel) return false;
  if ( inputModel->vertices.size() == 0 || inputModel->triangles.size() == 0 ) {
    std::cout << "Error: invalid input model from " << _inputModelFilename << std::endl;
    return false;
  }

  // the output
  mm::ModelPtr outputModel = mm::ModelPtr( new mm::Model() );

  // Perform the processings
  clock_t t1 = clock();
  if ( _mode == "delface" ) {
    size_t skipped = 0;
    if ( _nthFace != 0 ) {
      std::cout << "Removing nth face = " << _nthFace << std::endl;
      skipped = delNthFace( *inputModel, _nthFace, *outputModel );
    } else {
      std::cout << "Removing nb faces = " << _nbFaces << std::endl;
      skipped = delNbFaces( *inputModel, _nbFaces, *outputModel );
    }

    std::cout << "Nb face deleted = " << skipped << std::endl;
    std::cout << "Remaining nb faces = " << outputModel->getTriangleCount() << std::endl;
  } else {
    std::cout << "Error: invalid mode " << _mode << std::endl;
    return false;
  }

  clock_t t2 = clock();
  std::cout << "Time on processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  // save the result
  return mm::IO::saveModel( _outputModelFilename, outputModel );
}

size_t CmdDegrade::delNthFace( const mm::Model& input, size_t nthFace, mm::Model& output ) {
  const bool   hasNormals = input.hasNormals();
  const bool   hasUvCoord = input.hasUvCoords();
  const bool   hasColors  = input.hasColors();
  const size_t nbTri      = input.getTriangleCount();

  mm::ModelBuilder builder( output );
  size_t           skipped = 0;

  // remove one triangle every nth triangle
  for ( size_t triIdx = 0; triIdx < nbTri; ++triIdx ) {
    if ( triIdx % nthFace != 0 ) {
      mm::Vertex v1, v2, v3;
      mm::fetchTriangle( input, triIdx, hasUvCoord, hasColors, hasNormals, v1, v2, v3 );
      builder.pushTriangle( v1, v2, v3 );
    } else {
      skipped++;
    }
  }

  return skipped;
}

size_t CmdDegrade::delNbFaces( const mm::Model& input, size_t nbFaces, mm::Model& output ) {
  const bool   hasNormals = input.hasNormals();
  const bool   hasUvCoord = input.hasUvCoords();
  const bool   hasColors  = input.hasColors();
  const size_t nbTri      = input.getTriangleCount();

  mm::ModelBuilder builder( output );
  size_t           skipped = 0;

  // case nbFaces == 0, we keep everything
  size_t nthFace = 1;
  size_t subSize = nbTri;
  size_t rstSize = 0;

  // else we find the subrange that can be divided by nbFaces with no rest
  if ( nbFaces != 0 ) {
    nthFace = nbTri / std::min( nbFaces, nbTri );
    subSize = nthFace * nbFaces;
    rstSize = nbTri - subSize;
  }

  std::cout << "nthFace=" << nthFace << std::endl;
  std::cout << "subSize=" << subSize << std::endl;
  std::cout << "rstSize=" << rstSize << std::endl;

  // first remove the multiple part
  for ( size_t triIdx = 0; triIdx < subSize; ++triIdx ) {
    if ( nbFaces == 0 || ( triIdx + 1 ) % nthFace != 0 ) {
      mm::Vertex v1, v2, v3;
      mm::fetchTriangle( input, triIdx, hasUvCoord, hasColors, hasNormals, v1, v2, v3 );
      builder.pushTriangle( v1, v2, v3 );
    } else {
      skipped++;
    }
  }

  // then push all the rest if needed
  if ( rstSize != 0 ) {
    for ( size_t triIdx = subSize; triIdx < subSize + rstSize; ++triIdx ) {
      mm::Vertex v1, v2, v3;
      mm::fetchTriangle( input, triIdx, hasUvCoord, hasColors, hasNormals, v1, v2, v3 );
      builder.pushTriangle( v1, v2, v3 );
    }
  }

  return skipped;
}