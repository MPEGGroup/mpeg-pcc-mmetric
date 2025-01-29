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

// argument parsing
#include <cxxopts.hpp>

// internal headers
#include "mmContext.h"
#include "mmCmdSequence.h"

const char* CmdSequence::name  = "sequence";
const char* CmdSequence::brief = "Sequence global parameters";

//
Command* CmdSequence::create() { return new CmdSequence(); }

//
bool CmdSequence::initialize( Context* ctx, std::string app, int argc, char* argv[] ) {
  // command line parameters
  try {
    cxxopts::Options options( app + " " + name, brief );
    // clang-format off
		options.add_options()
			("firstFrame", "Sets the first frame of the sequence, included.",
				cxxopts::value<int>()->default_value("0"))
			("lastFrame", "Sets the last frame of the sequence, included. Must be >= to firstFrame.",
				cxxopts::value<int>()->default_value("0"))
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
    int firstFrame = 0;
    int lastFrame  = 0;
    if ( result.count( "firstFrame" ) ) firstFrame = result["firstFrame"].as<int>();
    if ( result.count( "lastFrame" ) ) lastFrame = result["lastFrame"].as<int>();
    if ( lastFrame < firstFrame || lastFrame < 0 || firstFrame < 0 ) {
      std::cerr << "Error: must have 0 >= lastFrame >= firstFrame. Got firstFrame=" << firstFrame
                << " and lastFrame = " << lastFrame << std::endl;
      return false;
    }
    ctx->setFrameRange( firstFrame, lastFrame );
  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "Error: parsing options, " << e.what() << std::endl;
    return false;
  }

  return true;
}
