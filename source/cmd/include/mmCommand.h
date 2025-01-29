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

#ifndef _MM_CMD_COMMAND_H_
#define _MM_CMD_COMMAND_H_

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <vector>

// mathematics
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

//
#include "mmContext.h"

class Command {
 public:  // Command API, to be specialized by command implementation
  // must be overloaded to parse arguments and init the command
  virtual bool initialize( Context*, std::string app, int argc, char* argv[] ) = 0;

  // must be overloaded to execute command for given frame
  virtual bool process( uint32_t frame ) = 0;

  // must be overloaded to collect temporal results after all frames processing
  virtual bool finalize( void ) = 0;

 public:  // Command managment API
  // command creator function type
  typedef Command* ( *Creator )( void );

  // invoke to register a new command
  static bool addCreator( const std::string& cmdName, const std::string& cmdBrief, Creator cmdCreator );

  // print the list of registered commands
  static void logCommands( void );

  // invoke to create a new command
  static Command* create( const std::string& app, const std::string& cmd );

 private:
  // command name -> (command creator, brief description)
  static std::map<std::string, std::pair<Command::Creator, std::string>> _cmdCreators;
};

//
inline bool parseVec2( const std::string& s, glm::vec2& res ) {
  glm::vec2          tmp;
  std::istringstream stream( s );
  stream.exceptions( std::istringstream::failbit | std::istringstream::badbit );
  try {
    stream >> tmp.x;
    stream >> tmp.y;
    res = tmp;
    return true;
  } catch ( std::istringstream::failure ) { return false; }
}

inline bool parseVec3( const std::string& s, glm::vec3& res ) {
  glm::vec3          tmp;
  std::istringstream stream( s );
  stream.exceptions( std::istringstream::failbit | std::istringstream::badbit );
  try {
    stream >> tmp.x;
    stream >> tmp.y;
    stream >> tmp.z;
    res = tmp;
    return true;
  } catch ( std::istringstream::failure ) { return false; }
}

inline bool parseVec4( const std::string& s, glm::vec4& res ) {
  glm::vec4          tmp;
  std::istringstream stream( s );
  stream.exceptions( std::istringstream::failbit | std::istringstream::badbit );
  try {
    stream >> tmp.x;
    stream >> tmp.y;
    stream >> tmp.z;
    stream >> tmp.w;
    res = tmp;
    return true;
  } catch ( std::istringstream::failure ) { return false; }
}

// split a string containing spaces into sub strings
// returns a one element list with "" if str is ""
// this permits to make a distinction with parameter forced to empty string and 
// a parameter not defined. This prevents from doing the test in every command.
inline bool parseStringList( const std::string& str, std::vector<std::string>& urlsList ) {
  std::istringstream       iss( str );
  std::string              token;
  while ( std::getline( iss, token, ' ' ) ) {
    if ( !token.empty() ) { urlsList.push_back( token ); }
  }
  if (urlsList.empty()) {
      urlsList.push_back("");
  }
  return true;
}

#endif
