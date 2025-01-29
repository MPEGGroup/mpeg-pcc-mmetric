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

#ifndef _MM_CONTEXT_H_
#define _MM_CONTEXT_H_

#include <string>
#include <map>
#include <iostream>

class Context {
 public:
  Context() : _frame( 0 ), _firstFrame( 0 ), _lastFrame( 0 ) {}

  bool setFrame( uint32_t frame ) {
    if ( frame < _firstFrame || frame > _lastFrame ) { return false; }
    _frame = frame;
    return true;
  }
  uint32_t getFrame( void ) { return _frame; }

  // first and last frame are included
  bool setFrameRange( uint32_t first, uint32_t last ) {
    if ( first > last ) return false;
    _firstFrame = first;
    _lastFrame  = last;
    return true;
  }

  uint32_t getFirstFrame( void ) { return _firstFrame; }
  uint32_t getLastFrame( void ) { return _lastFrame; }
  uint32_t getFrameCount( void ) { return _lastFrame - _firstFrame + 1; }

 private:
  uint32_t _frame;  // current frame
  uint32_t _firstFrame;
  uint32_t _lastFrame;
};

#endif
