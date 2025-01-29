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
#include "mmCmdReindex.h"

const char* CmdReindex::name  = "reindex";
const char* CmdReindex::brief = "Reindex mesh and optionaly sort vertices and face indices";

//
Command* CmdReindex::create() { return new CmdReindex(); }

//
bool CmdReindex::initialize( Context* ctx, std::string app, int argc, char* argv[] ) {
  // command line parameters
  try {
    cxxopts::Options options( app + " " + name, brief );
    // clang-format off
		options.add_options()
			("i,inputModel",  "path to input model (obj or ply file)", cxxopts::value<std::string>() )
			("o,outputModel", "path to output model (obj or ply file)", cxxopts::value<std::string>())
			("h,help", "Print usage")
			("sort", "Sort method in none, vertices, oriented, unoriented.", cxxopts::value<std::string>()->default_value("none"))
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
    if ( result.count( "sort" ) ) _sort = result["sort"].as<std::string>();
  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return false;
  }

  return true;
}

bool CmdReindex::process( uint32_t frame ) {
  // the input
  mm::ModelPtr inputModel = mm::IO::loadModel(_inputModelFilename);
  if (!inputModel) return false;
  if ( inputModel->vertices.size() == 0 ) {
    std::cout << "Error: invalid input model from " << _inputModelFilename << std::endl;
    return false;
  }

  // the output
  mm::ModelPtr outputModel = mm::ModelPtr( new mm::Model() );
  outputModel->header    = inputModel->header;               // preserve material
  outputModel->materialNames = inputModel->materialNames;    // preserve material
  outputModel->textureMapUrls = inputModel->textureMapUrls;  // preserve material
  outputModel->comments  = inputModel->comments;

  // Perform the processings
  clock_t t1 = clock();

  std::cout << "Reindex" << std::endl;
  std::cout << "  sort = " << _sort << std::endl;
  if ( _sort == "none" ) {
    reindex( *inputModel, *outputModel );
  } else if ( _sort == "vertex" || _sort == "oriented" || _sort == "unoriented" ) {
    reorder( *inputModel, _sort, *outputModel );
  } else {
    std::cout << "Error: invalid sorting method " << _sort << std::endl;
  }

  clock_t t2 = clock();
  std::cout << "Time on processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  // save the result
  return mm::IO::saveModel( _outputModelFilename, outputModel );

}
