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
#include "mmGeometry.h"
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"

#include "mmCmdRender.h"

// Descriptions of the command
const char* CmdRender::name  = "render";
const char* CmdRender::brief = "Render a mesh (todo points)";

//
Command* CmdRender::create() { return new CmdRender(); }

//
bool CmdRender::initialize( Context* ctx, std::string app, int argc, char* argv[] ) {
  // command line parameters
  try {
    cxxopts::Options options( app + " " + name, brief );
    // clang-format off
		options.add_options()
			("i,inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
      ("m,inputMap", "path to input texture map (png, jpg, rgb, yuv), can be multiple paths surrounded by double quotes and separated by spaces.",
        cxxopts::value<std::string>())
			("o,outputImage", "path to output image (png file)",
				cxxopts::value<std::string>()->default_value("output.png"))
			("outputDepth", "path to output depth RGBA png file with 32bit float span on the four components. If empty string will not save depth (default behavior).",
				cxxopts::value<std::string>())
			("renderer", "Use software or openGL 1.2 renderer. Value in [sw_raster, gl12_raster].",
				cxxopts::value<std::string>()->default_value("sw_raster"))
			("hideProgress", "hide progress display in console for use by robot",
				cxxopts::value<bool>()->default_value("false"))
			("h,help", "Print usage")
			("width", "Output image width",
				cxxopts::value<unsigned int>()->default_value("1980"))
			("height", "Output image height",
				cxxopts::value<unsigned int>()->default_value("1024"))
			("bilinear", "Set --bilinear=false to disable bilinear filtering",
				cxxopts::value<bool>()->default_value("true"))
			("viewDir", "the view direction, a string of three floats",
				cxxopts::value<std::string>()->default_value("0.0 0.0 1.0"))
			("viewUp", "the view up vector, a string of three floats",
				cxxopts::value<std::string>()->default_value("0.0 1.0 0.0"))
			("bboxMin", "bbox min corner, a string of three floats. If not both bboxMin and bboxMax set, will compute automatically.",
				cxxopts::value<std::string>())
			("bboxMax", "bbox max corner, a string of three floats. If not both bboxMin and bboxMax set, will compute automatically.",
				cxxopts::value<std::string>())
			("clearColor", "background color, a string of four int8, each component in [0,255].",
				cxxopts::value<std::string>()->default_value("0 0 0 0"))
			("enableLighting", "enable the lighting if model has normals.",
				cxxopts::value<bool>()->default_value("false"))
			("autoLightPosition", "enable light position computation from bounding sphere radius and lightAutoDir parameter.",
				cxxopts::value<bool>()->default_value("false"))
			("lightAutoDir", "if autoLightPosition is set, will compute light position as boundingSphereCenter + lightAutoDir * boundingSphereRadius. Default value stands for top right sector.",
				cxxopts::value<std::string>()->default_value("1 1 1"))
			("enableCulling", "enable back face culling.",
				cxxopts::value<bool>()->default_value("false"))
			("cwCulling", "true sets Clock Wise (cw) orientation for face culling, Counter Clock Wise (ccw) otherwise.",
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
    if ( result.count( "inputModel" ) ) inputModelFilename = result["inputModel"].as<std::string>();
    else {
      std::cerr << "Error: missing inputModel parameter" << std::endl;
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if (result.count("inputMap")) {
        parseStringList(result["inputMap"].as<std::string>(), inputTextureFilenames);
    }
    //
    if ( result.count( "outputImage" ) ) outputImageFilename = result["outputImage"].as<std::string>();
    //
    if ( result.count( "outputDepth" ) ) outputDepthFilename = result["outputDepth"].as<std::string>();
    //
    if ( result.count( "renderer" ) ) renderer = result["renderer"].as<std::string>();

    //
    if ( result.count( "width" ) ) width = result["width"].as<unsigned int>();
    if ( result.count( "height" ) ) height = result["height"].as<unsigned int>();
    if ( result.count( "bilinear" ) ) bilinear = result["bilinear"].as<bool>();

    if ( result.count( "viewDir" ) ) {
      viewDirStr = result["viewDir"].as<std::string>();
      if ( !parseVec3( viewDirStr, viewDir ) ) {
        std::cout << "Error: parsing --viewDir=\"" << viewDirStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }

    if ( result.count( "viewUp" ) ) {
      viewUpStr = result["viewUp"].as<std::string>();
      if ( !parseVec3( viewUpStr, viewUp ) ) {
        std::cout << "Error: parsing --viewUp=\"" << viewUpStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
    }

    if ( result.count( "bboxMin" ) && result.count( "bboxMax" ) ) {
      bboxMinStr = result["bboxMin"].as<std::string>();
      bboxMaxStr = result["bboxMax"].as<std::string>();
      if ( !parseVec3( bboxMinStr, bboxMin ) ) {
        std::cout << "Error: parsing --bboxMin=\"" << bboxMinStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
      if ( !parseVec3( bboxMaxStr, bboxMax ) ) {
        std::cout << "Error: parsing --bboxMax=\"" << bboxMaxStr << "\" expected three floats with space separator"
                  << std::endl;
        return false;
      }
      bboxValid = true;
    }

    if ( result.count( "clearColor" ) ) {
      clearColorStr = result["clearColor"].as<std::string>();
      if ( !parseVec4( clearColorStr, clearColor ) ) {
        std::cout << "Error: parsing --clearColor=\"" << clearColorStr << "\" expected four floats with space separator"
                  << std::endl;
        return false;
      }
    }
    // lighting
    if ( result.count( "enableLighting" ) ) enableLighting = result["enableLighting"].as<bool>();

    if ( result.count( "autoLightPosition" ) ) autoLightPosition = result["autoLightPosition"].as<bool>();

    if ( result.count( "lightAutoDir" ) ) {
      lightAutoDirStr = result["lightAutoDir"].as<std::string>();
      if ( !parseVec3( lightAutoDirStr, lightAutoDir ) ) {
        std::cout << "Error: parsing --lightAutoDir=\"" << clearColorStr
                  << "\" expected four floats with space separator" << std::endl;
        return false;
      }
    }

    // culling
    if ( result.count( "enableCulling" ) ) enableCulling = result["enableCulling"].as<bool>();
    if ( result.count( "cwCulling" ) ) cwCulling = result["cwCulling"].as<bool>();
  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return false;
  }

  // now initialize OpenGL contexts if needed
  // this part is valid for all the frames
  if ( renderer == "gl12_raster" ) {
    if ( !_hwRenderer.initialize( width, height ) ) { return false; }
  }

  return true;
}

bool CmdRender::finalize() {
  if ( renderer == "gl12_raster" ) { return _hwRenderer.shutdown(); }
  return true;
}

bool CmdRender::process( uint32_t frame ) {
  bool success = true;

  // the input
  mm::ModelPtr inputModel = mm::IO::loadModel(inputModelFilename);
  if ( !inputModel ) { return false; }
  if ( inputModel->vertices.size() == 0 || inputModel->triangles.size() == 0 ) {
    std::cout << "Error: invalid input model from " << inputModelFilename << std::endl;
    return false;
  }

  // now handle the textures
  std::vector<std::string> textureMapUrls;
  if (inputTextureFilenames.size() != 0)
      textureMapUrls = inputTextureFilenames;
  else
      textureMapUrls = inputModel->textureMapUrls;

  std::vector<mm::ImagePtr> textureMapList;
  mm::IO::loadImages(textureMapUrls, textureMapList);
  if (textureMapList.empty()) {
      std::cout << "Skipping map read, will parse/use vertex color if any" << std::endl;
      textureMapList.push_back(mm::ImagePtr(new mm::Image()));
      textureMapUrls.push_back("");
  }

  // resolve will be moved in the IO Image write once implemented
  std::string outImage = mm::IO::resolveName( frame, outputImageFilename );
  std::string outDepth = mm::IO::resolveName( frame, outputDepthFilename );

  if ( glm::distance( glm::abs( viewDir ), glm::vec3( 0, 1, 0 ) ) < 1e-6 ) viewUp = glm::vec3( 0, 0, 1 );

  // Perform the processings
  clock_t t1 = clock();
  bool    res;
  if ( renderer == "sw_raster" ) {
    std::cout << "Render sw_raster" << std::endl;
    std::cout << "  Width = " << width << std::endl;
    std::cout << "  Height = " << height << std::endl;
    std::cout << "  Bilinear = " << bilinear << std::endl;
    std::cout << "  hideProgress = " << hideProgress << std::endl;
    _swRenderer.setClearColor( clearColor );
    //
    if ( enableCulling ) _swRenderer.enableCulling();
    else _swRenderer.disableCulling();
    _swRenderer.setCwCulling( cwCulling );
    //
    if ( enableLighting ) {
      _swRenderer.enableLighting();
      if ( autoLightPosition ) {
        _swRenderer.enableAutoLightPosition();
        _swRenderer.setLightAutoDir( lightAutoDir );
      }
    }
    res = _swRenderer.render(
      inputModel, textureMapList, outImage, outDepth, width, height, viewDir, viewUp, bboxMin, bboxMax, bboxValid );
  } else if ( renderer == "gl12_raster" ) {
    std::cout << "Render gl12_raster" << std::endl;
    std::cout << "  Width = " << width << std::endl;
    std::cout << "  Height = " << height << std::endl;
    std::cout << "  Bilinear = " << bilinear << std::endl;
    std::cout << "  hideProgress = " << hideProgress << std::endl;
    _hwRenderer.setClearColor( clearColor );
    res = _hwRenderer.render(
      inputModel, textureMapList, outImage, outDepth, width, height, viewDir, viewUp, bboxMin, bboxMax, bboxValid );
  } else {
    std::cout << "Error: invalid renderer " << renderer << std::endl;
    return false;
  }

  clock_t t2 = clock();
  std::cout << "Time on processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  return success;
}
