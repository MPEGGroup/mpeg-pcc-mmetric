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

//
#include <iostream>
#include <vector>
#include <limits>
#include <time.h>

// internal headers
#include "mmIO.h"
#include "mmContext.h"
#include "mmCommand.h"
#include "mmVersion.h"

// the name of the application binary
// i.e argv[0] minus the eventual path
#ifdef _WIN32
#  define APP_NAME "mm.exe"
#else
#  define APP_NAME "mm"
#endif

// The commands
#include "mmCmdAnalyse.h"
#include "mmCmdCompare.h"
#include "mmCmdDegrade.h"
#include "mmCmdQuantize.h"
#include "mmCmdDequantize.h"
#include "mmCmdReindex.h"
#include "mmCmdSample.h"
#include "mmCmdSequence.h"
#include "mmCmdNormals.h"
#include "mmCmdRender.h"

// analyse command line and run processings
int main( int argc, char* argv[] ) {
  // this is mandatory to print floats with full precision
  std::cout.precision( std::numeric_limits<float>::max_digits10 );

  // register the commands
  Command::addCreator( CmdAnalyse::name, CmdAnalyse::brief, CmdAnalyse::create );
  Command::addCreator( CmdCompare::name, CmdCompare::brief, CmdCompare::create );
  Command::addCreator( CmdDegrade::name, CmdDegrade::brief, CmdDegrade::create );
  Command::addCreator( CmdQuantize::name, CmdQuantize::brief, CmdQuantize::create );
  Command::addCreator( CmdDequantize::name, CmdDequantize::brief, CmdDequantize::create );
  Command::addCreator( CmdReindex::name, CmdReindex::brief, CmdReindex::create );
  Command::addCreator( CmdSample::name, CmdSample::brief, CmdSample::create );
  Command::addCreator( CmdSequence::name, CmdSequence::brief, CmdSequence::create );
  Command::addCreator( CmdNormals::name, CmdNormals::brief, CmdNormals::create );
  Command::addCreator( CmdRender::name, CmdRender::brief, CmdRender::create );

  // execute the commands
  if ( argc > 1 ) {
    // global timer
    clock_t t1 = clock();

    // context shared among commands
    Context context;
    mm::IO::setContext( &context );
    // set of commands to be executer in order
    std::vector<Command*> commands;

    // 1 - initialize the command list
    int startIdx = 1;
    int endIdx;

    do {
      // search for end of command (END or end of argv)
      endIdx          = startIdx;
      std::string arg = argv[endIdx];
      while ( endIdx != argc - 1 && arg != "END" ) {
        endIdx++;
        arg = argv[endIdx];
      }
      int subArgc = endIdx - startIdx + ( ( endIdx == argc - 1 ) ? 1 : 0 );

      // create a new command
      Command* newCmd = NULL;
      if ( ( newCmd = Command::create( APP_NAME, std::string( argv[startIdx] ) ) ) == NULL ) { return 1; }
      commands.push_back( newCmd );

      // initialize the command
      if ( !newCmd->initialize( &context, APP_NAME, subArgc, &argv[startIdx] ) ) { return 1; }

      // start of next command
      startIdx = endIdx + 1;

    } while ( startIdx < argc );

    // 2 - execute each command for each frame
    int procErrors = 0;
    for ( uint32_t frame = context.getFirstFrame(); frame <= context.getLastFrame(); ++frame ) {
      std::cout << "Processing frame " << frame << std::endl;
      context.setFrame( frame );
      for ( size_t cmdIndex = 0; cmdIndex < commands.size(); ++cmdIndex ) {
        if ( !commands[cmdIndex]->process( frame ) ) { procErrors++; }
      }
      // purge the models, clean IO for next frame
      mm::IO::purge();
    }
    if ( procErrors != 0 ) { std::cerr << "There was " << procErrors << " processing errors" << std::endl; }

    // 3 - collect results
    int finErrors = 0;
    for ( size_t cmdIndex = 0; cmdIndex < commands.size(); ++cmdIndex ) {
      if ( !commands[cmdIndex]->finalize() ) { finErrors++; }
    }
    if ( finErrors != 0 ) { std::cerr << "There was " << finErrors << " finalization errors" << std::endl; }

    clock_t t2 = clock();
    std::cout << "Time on overall processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

    // all commands were executed
    return procErrors + finErrors;
  }

  // print help
  std::cout << "3D model processing commands v" << MM_VERSION << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << "  " << APP_NAME << " command [OPTION...]" << std::endl;
  std::cout << std::endl;
  std::cout << "Command help:" << std::endl;
  std::cout << "  " << APP_NAME << " command --help" << std::endl;
  std::cout << std::endl;
  std::cout << "Command:" << std::endl;
  Command::logCommands();
  std::cout << std::endl;

  return 0;
}
