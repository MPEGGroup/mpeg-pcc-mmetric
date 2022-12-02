// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
// Copyright 2021 - InterDigital
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
// Author: jean-eudes.marvie@interdigital.com
// *****************************************************************

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
#include "mmDequantize.h"
#include "mmQuantize.h"
#include "mmGeometry.h"
#include "mmColor.h"

using namespace mm;

// input can be mesh or point cloud
// the min/max values are not set to const so that internally computed min/max
// will have the expected impact onto the --dequant option of the quantize method (see. Quantize::process).
void Quantize::quantize( const Model&       input,
                         Model&             output,
                         const uint32_t     qp,
                         const uint32_t     qt,
                         const uint32_t     qn,
                         const uint32_t     qc,
                         const std::string& outputVarFilename,
                         const bool         useFixedPoint,
                         const bool         colorSpaceConversion,
                         glm::vec3&         minPos,
                         glm::vec3&         maxPos,
                         glm::vec2&         minUv,
                         glm::vec2&         maxUv,
                         glm::vec3&         minNrm,
                         glm::vec3&         maxNrm,
                         glm::vec3&         minCol,
                         glm::vec3&         maxCol,
                         const bool         verbose ) {
  // copy the input
  output = input;

  // prepare logging and output var
  std::vector<std::ostream*> out;
  if ( verbose ) out.push_back( &std::cout );
  std::ofstream fout;
  if ( outputVarFilename != "" ) {
    fout.open( outputVarFilename.c_str(), std::ios::out );
    // this is mandatory to print floats with full precision
    fout.precision( std::numeric_limits<float>::max_digits10 );
    if ( fout ) {
      // this is mandatory to print floats with full precision
      fout.precision( std::numeric_limits<float>::max_digits10 );
      out.push_back( &fout );
    } else {
      std::cout << "Error: could not create output file " << outputVarFilename << std::endl;
    }
  }

  // quantize position
  if ( !input.vertices.empty() && qp >= 7 ) {
    if ( minPos == maxPos ) {
      if ( verbose ) std::cout << "Computing positions range" << std::endl;
      Geometry::computeBBox( input.vertices, minPos, maxPos );
    } else {
      if ( verbose ) std::cout << "Using parameter positions range" << std::endl;
    }
    const int32_t fixedPoint16 = ( 1u << 16 );
    if ( useFixedPoint ) {
      // converting the values to a fixed point representation
      // minPos(FP16) will be used in AAPS -> shift
      for ( int i = 0; i < 3; i++ ) {
        if ( minPos[i] > 0 ) minPos[i] = ( std::floor( minPos[i] * fixedPoint16 ) ) / fixedPoint16;
        else minPos[i] = ( -1 ) * ( std::ceil( std::abs( minPos[i] ) * fixedPoint16 ) ) / fixedPoint16;
        if ( maxPos[i] > 0 ) maxPos[i] = ( std::ceil( maxPos[i] * fixedPoint16 ) ) / fixedPoint16;
        else maxPos[i] = ( -1 ) * ( std::floor( std::abs( maxPos[i] ) * fixedPoint16 ) ) / fixedPoint16;
      }
    }
    const glm::vec3 diag                      = maxPos - minPos;
    const float     range                     = std::max( std::max( diag.x, diag.y ), diag.z );
    const int32_t   maxPositionQuantizedValue = ( 1u << static_cast<uint32_t>( qp ) ) - 1;
    double          scale                     = (double)range / maxPositionQuantizedValue;
    if ( useFixedPoint ) scale = ( std::ceil( scale * fixedPoint16 ) ) / fixedPoint16;

    for ( size_t i = 0; i < out.size(); ++i ) {
      *out[i] << "  minPos=\"" << minPos.x << " " << minPos.y << " " << minPos.z << "\"" << std::endl;
      *out[i] << "  maxPos=\"" << maxPos.x << " " << maxPos.y << " " << maxPos.z << "\"" << std::endl;
      *out[i] << "  rangePos=" << range << std::endl;
      *out[i] << "  maxPositionQuantizedValue=" << maxPositionQuantizedValue << std::endl;
      *out[i] << "  scale=" << scale << std::endl;
    }

    for ( size_t i = 0; i < input.vertices.size() / 3; i++ ) {
      for ( glm::vec3::length_type c = 0; c < 3; ++c ) {
        uint32_t pos =
          static_cast<uint32_t>( std::floor( ( ( double( input.vertices[i * 3 + c] - minPos[c] ) ) / scale ) + 0.5f ) );
        output.vertices[i * 3 + c] = static_cast<float>( pos );
      }
    }
  }

  // quantize UV coordinates
  if ( !input.uvcoords.empty() && qt >= 7 ) {
    if ( minUv == maxUv ) {
      if ( verbose ) std::cout << "Computing uv coordinates range" << std::endl;
      Geometry::computeBBox( input.uvcoords, minUv, maxUv );
    } else {
      if ( verbose ) std::cout << "Using parameter uv coordinates range" << std::endl;
    }
    const glm::vec2 diag                    = maxUv - minUv;
    const float     range                   = std::max( diag.x, diag.y );
    const int32_t   maxUVcordQuantizedValue = ( 1u << static_cast<uint32_t>( qt ) ) - 1;

    for ( size_t i = 0; i < out.size(); ++i ) {
      *out[i] << "  minUv=\"" << minUv.x << " " << minUv.y << "\"" << std::endl;
      *out[i] << "  maxUv=\"" << maxUv.x << " " << maxUv.y << "\"" << std::endl;
      *out[i] << "  rangeUv=" << range << std::endl;
    }

    for ( size_t i = 0; i < input.uvcoords.size() / 2; i++ ) {
      for ( glm::vec2::length_type c = 0; c < 2; ++c ) {
        uint32_t uv = static_cast<uint32_t>(
          std::floor( ( ( input.uvcoords[i * 2 + c] - minUv[c] ) / range ) * maxUVcordQuantizedValue + 0.5f ) );
        output.uvcoords[i * 2 + c] = static_cast<float>( uv );
      }
    }
  }

  // quantize normals
  if ( !input.normals.empty() && qn >= 7 ) {
    if ( minNrm == maxNrm ) {
      if ( verbose ) std::cout << "Computing normals range" << std::endl;
      Geometry::computeBBox( input.normals, minNrm, maxNrm );
    } else {
      if ( verbose ) std::cout << "Using parameter normals range" << std::endl;
    }
    const glm::vec3 diag                    = maxNrm - minNrm;
    const float     range                   = std::max( std::max( diag.x, diag.y ), diag.z );
    const int32_t   maxNormalQuantizedValue = ( 1u << static_cast<uint32_t>( qn ) ) - 1;

    for ( size_t i = 0; i < out.size(); ++i ) {
      *out[i] << "  minNrm=\"" << minNrm.x << " " << minNrm.y << " " << minNrm.z << "\"" << std::endl;
      *out[i] << "  maxNrm=\"" << maxNrm.x << " " << maxNrm.y << " " << maxNrm.z << "\"" << std::endl;
      *out[i] << "  rangeNrm=" << range << std::endl;
    }

    for ( size_t i = 0; i < input.normals.size() / 3; i++ ) {
      for ( glm::vec3::length_type c = 0; c < 3; ++c ) {
        uint32_t nrm = static_cast<uint32_t>(
          std::floor( ( ( input.normals[i * 3 + c] - minNrm[c] ) / range ) * maxNormalQuantizedValue + 0.5f ) );
        output.normals[i * 3 + c] = static_cast<float>( nrm );
      }
    }
  }

  // quantize colors
  if ( !input.colors.empty() && ( ( qc >= 7 ) || ( colorSpaceConversion ) ) ) {
    if ( minCol == maxCol ) {
      if ( verbose ) std::cout << "Computing colors range" << std::endl;
      Geometry::computeBBox( input.colors, minCol, maxCol );
    } else {
      if ( verbose ) std::cout << "Using parameter colors range" << std::endl;
    }
    const glm::vec3 diag                   = maxCol - minCol;
    const float     range                  = std::max( std::max( diag.x, diag.y ), diag.z );
    const int32_t   maxColorQuantizedValue = ( 1u << static_cast<uint32_t>( qc ) ) - 1;

    for ( size_t i = 0; i < out.size(); ++i ) {
      *out[i] << "  minCol=\"" << minCol.x << " " << minCol.y << " " << minCol.z << "\"" << std::endl;
      *out[i] << "  maxCol=\"" << maxCol.x << " " << maxCol.y << " " << maxCol.z << "\"" << std::endl;
      *out[i] << "  rangeCol=" << range << std::endl;
    }

    for ( size_t i = 0; i < input.colors.size() / 3; i++ ) {
      if ( colorSpaceConversion ) {
        glm::vec3 inYUV_256, inYUV;
        rgbToYuvBt709_256( glm::vec3( input.colors[i * 3], input.colors[i * 3 + 1], input.colors[i * 3 + 2] ),
                           inYUV_256 );
        color256ToUnit( inYUV_256, inYUV );
        for ( glm::vec3::length_type c = 0; c < 3; ++c ) {
          uint32_t col             = static_cast<uint32_t>( std::floor( inYUV[c] * maxColorQuantizedValue + 0.5f ) );
          output.colors[i * 3 + c] = static_cast<float>( col );
        }
      } else {
        for ( glm::vec3::length_type c = 0; c < 3; ++c ) {
          uint32_t col = static_cast<uint32_t>(
            std::floor( ( ( input.colors[i * 3 + c] - minCol[c] ) / range ) * maxColorQuantizedValue + 0.5f ) );
          output.colors[i * 3 + c] = static_cast<float>( col );
        }
      }
    }
  }
}
