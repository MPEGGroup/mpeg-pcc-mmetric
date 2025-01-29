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

#include "mmCommand.h"

// create the command store
std::map<std::string, std::pair<Command::Creator, std::string>> Command::_cmdCreators;

//
bool Command::addCreator( const std::string& cmdName, const std::string& cmdBrief, Command::Creator cmdCreator ) {
  if ( Command::_cmdCreators.find( cmdName ) != Command::_cmdCreators.end() ) {
    std::cout << "Error: cannot register new commmand " << cmdName << " already exists" << std::endl;
    return false;
  }
  _cmdCreators[cmdName] = make_pair( cmdCreator, cmdBrief );
  return true;
}

//
Command* Command::create( const std::string& app, const std::string& cmd ) {
  std::map<std::string, std::pair<Command::Creator, std::string>>::iterator it = Command::_cmdCreators.find( cmd );
  if ( it == Command::_cmdCreators.end() ) {
    std::cout << "Error: unknown command " << cmd << std::endl;
    return NULL;
  }
  //
  return it->second.first();
}

//
void Command::logCommands( void ) {
  std::map<std::string, std::pair<Command::Creator, std::string>>::iterator it;
  for ( it = Command::_cmdCreators.begin(); it != _cmdCreators.end(); ++it ) {
    std::cout << "  " << it->first << "\t" << it->second.second << std::endl;
  }
}