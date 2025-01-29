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
#include "mmGeometry.h"

#include "mmCmdNormals.h"

const char* CmdNormals::name  = "normals";
const char* CmdNormals::brief = "Computes the mesh normals.";

//
Command* CmdNormals::create() { return new CmdNormals(); }

//
bool CmdNormals::initialize( Context* ctx, std::string app, int argc, char* argv[] ) {
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
			("normalized", "generated normals are normalized",
				cxxopts::value<bool>()->default_value("true"))
			("noSeams", "if enabled generation is slower but vertex located on UV seams are properly used as one same vertex, hence removing lighting seams.",
				cxxopts::value<bool>()->default_value("true"))
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
    if ( result.count( "normalized" ) ) _normalized = result["normalized"].as<bool>();
    if ( result.count( "noSeams" ) ) _noSeams = result["noSeams"].as<bool>();
  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return false;
  }

  return true;
}

bool CmdNormals::process( uint32_t frame ) {
  // the input
    mm::ModelPtr inputModel = mm::IO::loadModel(_inputModelFilename);
    if ( !inputModel ) return false;
  if ( inputModel->vertices.size() == 0 ) {
    std::cout << "Error: invalid input model (missing vertices) from " << _inputModelFilename << std::endl;
    return false;
  }
  if ( inputModel->triangles.size() == 0 ) {
    std::cout << "Error: invalid input model (not a mesh) from " << _inputModelFilename << std::endl;
    return false;
  }

  // the output
  mm::ModelPtr outputModel( new mm::Model() );
  *outputModel = *inputModel;

  // Perform the processings
  clock_t t1 = clock();

  std::cout << "Normals generation" << std::endl;
  std::cout << "  NoSeams = " << _noSeams << std::endl;
  std::cout << "  Normalized = " << _normalized << std::endl;

  outputModel->computeVertexNormals( _normalized, _noSeams );

  clock_t t2 = clock();
  std::cout << "Time on processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  // save the result
  return mm::IO::saveModel( _outputModelFilename, outputModel );

}
