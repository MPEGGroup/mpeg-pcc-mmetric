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

#ifndef _MM_METADATA_H_
#define _MM_METADATA_H_

#include <algorithm>  // for std::min and std::max
#include <cmath>      // for pow and sqrt,
#include <limits>     // for nan
#include <fstream>
#include <vector>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

namespace mm {

class Metadata {
 public:
  Metadata() { _gridRes, _dilateSize, _paddingSize = 0; }

  Metadata( size_t gridRes, size_t dilateSize, size_t paddingSize ) {
    _gridRes     = gridRes;
    _dilateSize  = dilateSize;
    _paddingSize = paddingSize;
  }

  void write( std::string name ) {
    std::string metdataFileName = std::filesystem::path( name ).parent_path().string() + "/" +
                                  std::filesystem::path( name ).stem().string() + ".meta";

    std::ofstream file( metdataFileName, std::ios::out );
    if ( !file.is_open() ) { return; }
    file << _gridRes << std::endl;
    file << _dilateSize << std::endl;
    file << _paddingSize << std::endl;
    file << _patchCoordinates.size() << std::endl;
    for ( auto& element : _patchCoordinates ) { file << element[0] << " " << element[1] << std::endl; }
  };
  void read( std::string name ) {
    std::string metdataFileName = std::filesystem::path( name ).parent_path().string() + "/" +
                                  std::filesystem::path( name ).stem().string() + ".meta";

    int           patchCoordinatesSize = 0;
    std::ifstream file( metdataFileName, std::ios::in );
    if ( !file.is_open() ) { return; }
    file >> _gridRes;
    file >> _dilateSize;
    file >> _paddingSize;
    file >> patchCoordinatesSize;
    _patchCoordinates.resize( patchCoordinatesSize );
    for ( auto& element : _patchCoordinates ) { file >> element[0] >> element[1]; }
  };

  void print() {
    std::cout << "Metadata are:" << std::endl;
    std::cout << "  _gridRes:     " << getGridRes() << std::endl;
    std::cout << "  _dilateSize:  " << getDilateSize() << std::endl;
    std::cout << "  _paddingSize: " << getPaddingSize() << std::endl;
    std::cout << "  _patchCoordinates: ";
    for ( int i = 0; i < getPatchCoordinates().size(); ++i ) {
      std::cout << " [" << i << "]:" << getPatchCoordinates()[i].x << "/" << getPatchCoordinates()[i].y;
    }
    std::cout << " " << std::endl;
  }

  std::vector<glm::ivec2>& getPatchCoordinates() { return _patchCoordinates; };
  size_t&                  getGridRes() { return _gridRes; };
  size_t&                  getDilateSize() { return _dilateSize; };
  size_t&                  getPaddingSize() { return _paddingSize; };

 private:
  size_t                  _gridRes;
  size_t                  _dilateSize;   // dilate is used to prevent texture seams
  size_t                  _paddingSize;  // padding is used to get better compression
  std::vector<glm::ivec2> _patchCoordinates;
};

}  // namespace mm

#endif
