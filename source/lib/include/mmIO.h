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

#ifndef _MM_IO_H_
#define _MM_IO_H_

#include <map>
#include <string>

#include "mmModel.h"
#include "mmImage.h"
#include "mmContext.h"

namespace mm {

class IO {
  IO() {}

 public:
  // stores the context to sol
  static void setContext( Context* context );

  // converts a string containing "%nd" with n an integer into a string
  // containing the frame number with n digits
  // e.g. filename00%3d.png filename00156.png if frame is 156
  static std::string resolveName( const uint32_t frame, const std::string& input );

  // name can be filename or "ID:xxxx"
  // return invalid shared pointer in case of error (to check with isValid(model)).
  static ModelPtr loadModel( std::string templateName );

  static bool saveModel( std::string templateName, ModelPtr model );

  // load image files and images from videos
  // name can be filename or "ID:xxxx"
  // return invalid shared pointer in case of error (to check with isValid(image)).
  static ImagePtr loadImage( std::string templateName );

  // load list of images from files and images from videos
  // empty string urls are skipped silently and null is stored, but it is not considered an error (see materials without map)
  // names in imageUrlList can be filename or "ID:xxxx"
  // fills the images vector with images, some may be invalid shared pointer in case of error 
  // returns false if at least one image load failed
  static bool loadImages( const std::vector< std::string >& imageUrlList, std::vector<mm::ImagePtr>& images );

  /*
  static bool saveImage(std::string name, Image* image);*/

  // free all the models and images, and reset cache.
  static void purge( void );

 private:
  // access to context for frame name resolution
  static Context* _context;

  // model store
  static std::map<std::string, ModelPtr> _models;
  // image store
  static std::map<std::string, ImagePtr> _images;

 public:
  // Automatic choice on extension
  static bool _loadModel( std::string filename, Model& output );
  static bool _saveModel( std::string filename, const Model& input );

  // OBJ
  static bool _loadObj( std::string filename, Model& output );
  static bool _saveObj( std::string filename, const Model& input );

  // PLY
  static bool _loadPly( std::string filename, Model& output );
  static bool _savePly( std::string filename, const Model& input );

  // Images
  static bool _loadImage( std::string filename, Image& output );
  static bool _saveImage( std::string filename, const Image& input, bool flipVertically = false );
  static bool _loadImageFromVideo( std::string filename, Image& output );

  static void getTextureMapPathFromMTL( 
      const std::string& path, 
      const std::string& mtl,
      std::vector<std::string>& materialName,
      std::vector<std::string>& textMapFilename );
};

}  // namespace mm

#endif