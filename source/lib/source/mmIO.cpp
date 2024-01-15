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
// Author: loadObj/saveObj methods based on original code from Owlii
// *****************************************************************

// remove warning when using sprintf on MSVC
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include <cmath>
#include <filesystem>
// ply loader
#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"
// images
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "mmIO.h"

using namespace mm;

//
Context* IO::_context = NULL;
// create the stores
std::map<std::string, Model*> IO::_models;
std::map<std::string, Image*> IO::_images;

//
void IO::setContext( Context* context ) { _context = context; }

//
std::string IO::resolveName( const uint32_t frame, const std::string& input ) {
  std::string output;
  if ( input.find( "%" ) != std::string::npos ) {
    char buffer[4092];
    auto n = sprintf( buffer, input.c_str(), frame );
    output = buffer;
    std::cout << output << std::endl;
  } else {
    output = input;
  }
  return output;
}

//
Model* IO::loadModel( std::string templateName ) {
  std::string                             name = resolveName( _context->getFrame(), templateName );
  std::map<std::string, Model*>::iterator it   = IO::_models.find( name );
  if ( it == IO::_models.end() ) {
    if ( name.substr( 0, 3 ) == "ID:" ) {
      std::cout << "Error: model with id " << name << "not defined" << std::endl;
      return NULL;
    } else {  // we try to load the model
      Model* model = new Model();
      if ( !IO::_loadModel( name, *model ) ) {
        delete model;
        return NULL;
      } else {
        IO::_models[name] = model;
        return model;
      }
    }
  }
  return it->second;
};

//
bool IO::saveModel( std::string templateName, Model* model ) {
  std::string                             name = resolveName( _context->getFrame(), templateName );
  std::map<std::string, Model*>::iterator it   = IO::_models.find( name );
  if ( it != IO::_models.end() ) {
    std::cout << "Warning: model with id " << name << " already defined, overwriting" << std::endl;
    delete it->second;
    it->second = model;
  } else {
    IO::_models[name] = model;
  }
  // save to file if not an id
  if ( name.substr( 0, 3 ) != "ID:" ) { return IO::_saveModel( name, *model ); }
  return true;
}

//
Image* IO::loadImage( std::string templateName ) {
  // The IO store is purged for each new frame.
  // So in case of video file without %d template we just use the filename (unchanged by resolveName).
  std::string                             name = resolveName( _context->getFrame(), templateName );
  std::map<std::string, Image*>::iterator it   = IO::_images.find( name );

  // use image/frame from store
  if ( it != IO::_images.end() ) { return it->second; }

  // not found in store but name is an ID => error
  if ( name.substr( 0, 3 ) == "ID:" ) {
    std::cout << "Error: image with id " << name << "not defined" << std::endl;
    return NULL;
  }

  // else try to load the image/frame
  Image* image = new Image();

  std::string ext = templateName.substr( templateName.find_last_of( "." ) );
  std::transform( ext.begin(), ext.end(), ext.begin(), []( unsigned char c ) { return std::tolower( c ); } );

  if ( ext == ".yuv" || ext == ".rgb" ) {
    // try to load as video
    if ( !IO::_loadImageFromVideo( name, *image ) ) {
      delete image;
      return NULL;
    }
  } else {
    // try to load as image
    if ( !IO::_loadImage( name, *image ) ) {
      delete image;
      return NULL;
    }
  }
  // add to the store
  IO::_images[name] = image;
  return image;
};

bool IO::loadImages( const std::vector<std::string>& imageUrlList, std::vector<mm::Image*>& images ) {
  bool res = true;
  for ( auto url : imageUrlList ) {
    if ( url.size() != 0 ) {
      images.push_back( mm::IO::loadImage( url ) );  // thus if fails, the element of the array contains NULL
      res = res && ( images.back() != NULL );
    } else {
      images.push_back( NULL ); // not an error
    }
  }
  return res;
}

/*
bool  IO::saveImage(std::string name, Image* image) {
        std::map<std::string, Image*>::iterator it = IO::_images.find(name);
        if (it == IO::_images.end()) {
                std::cout << "Warning: image with id " << name << "already defined, overwriting" << std::endl;
                delete it->second;
                it->second = image;
        }
        else {
                IO::_images[name] = image;
        }
        // save to file if not an id
        if (name.substr(0, 3) != "ID:") {
                IO::_saveImage(name, *image);
        }
}*/

//
void IO::purge( void ) {
  // free all the texture maps
  std::map<std::string, Image*>::iterator imageIt = IO::_images.begin();
  for ( ; imageIt != _images.end(); ++imageIt ) { delete imageIt->second; }
  _images.clear();

  // free all the models
  std::map<std::string, Model*>::iterator modelIt = IO::_models.begin();
  for ( ; modelIt != _models.end(); ++modelIt ) { delete modelIt->second; }
  _models.clear();
}

///////////////////////////
// Private methods

bool IO::_loadModel( std::string filename, Model& output ) {
  bool success = true;

  // sanity check
  if ( filename.size() < 5 ) {
    std::cout << "Error, invalid mesh file name " << filename << std::endl;
    return false;
  }

  // get extension
  std::string ext = filename.substr( filename.size() - 3, 3 );
  std::for_each( ext.begin(), ext.end(), []( char& c ) { c = ::tolower( c ); } );

  // do the job
  if ( ext == "ply" ) {
    std::cout << "Loading file: " << filename << std::endl;
    auto t1 = clock();
    success = IO::_loadPly( filename, output );  // TODO handle read error
    if ( success ) {
      auto t2 = clock();
      std::cout << "Time on loading: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
    }
  } else if ( ext == "obj" ) {
    std::cout << "Loading file: " << filename << std::endl;
    auto t1 = clock();
    success = IO::_loadObj( filename, output );  // TODO handle read error
    if ( success ) {
      auto t2 = clock();
      std::cout << "Time on loading: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
    }
  } else {
    std::cout << "Error, invalid mesh file extension (not in obj, ply)" << std::endl;
    return false;
  }

  if ( success ) {
    // print stats
    std::cout << "Input model: " << filename << std::endl;
    std::cout << "  Vertices: " << output.vertices.size() / 3 << std::endl;
    std::cout << "  UVs: " << output.uvcoords.size() / 2 << std::endl;
    std::cout << "  Colors: " << output.colors.size() / 3 << std::endl;
    std::cout << "  Normals: " << output.normals.size() / 3 << std::endl;
    std::cout << "  Triangles: " << output.triangles.size() / 3 << std::endl;
    std::cout << "  Trianglesuv: " << output.trianglesuv.size() / 3 << std::endl;
  }

  return success;
}

bool IO::_saveModel( std::string filename, const Model& input ) {
  // sanity check
  if ( filename.size() < 5 ) {
    std::cout << "Error, invalid mesh file name " << filename << std::endl;
    return false;
  }

  // check output file extension
  std::string out_ext = filename.substr( filename.size() - 3, 3 );
  std::for_each( out_ext.begin(), out_ext.end(), []( char& c ) { c = ::tolower( c ); } );

  // write output
  if ( out_ext == "ply" ) {
    std::cout << "Saving file: " << filename << std::endl;
    auto t1  = clock();
    auto err = IO::_savePly( filename, input );
    if ( !err ) {
      auto t2 = clock();
      std::cout << "Time on saving: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
    }
    return err;
  } else if ( out_ext == "obj" ) {
    std::cout << "Saving file: " << filename << std::endl;
    auto t1  = clock();
    auto err = IO::_saveObj( filename, input );
    if ( !err ) {
      auto t2 = clock();
      std::cout << "Time on saving: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
    }
    return err;
  } else {
    std::cout << "Error: invalid mesh file extension (not in obj, ply)" << std::endl;
    return false;
  }

  // success
  return true;
}

#define FAST_OBJ_READ
#ifdef FAST_OBJ_READ
class iBuffer {

public:
    std::vector<char> buffer; // the bitstream
    char* head = 0;           // current read position
    char* end = 0;            // end of buffer
    char* ls = 0;             // start of current line
    int lc = 1;               // linecount

    inline bool load(std::string filename) {
        // if loader is reused need to reset
        buffer.clear();
        lc=1; 

        FILE* fp = fopen(filename.c_str(), "rb"); // open input file in binary mode
        if (!fp) {
            std::cerr << "Error: can't open file " << filename << std::endl;
            return false;
        }

        if (fseek(fp, SEEK_SET, SEEK_END) != 0) {
            std::cerr << "Error: can't seek to end of file to get size " << filename << std::endl;
            fclose(fp);
            return false;
        }
        const size_t fileSize = static_cast<size_t>(ftell(fp));
        if (fileSize == 0) {
            std::cerr << "Error: empty file " << filename << std::endl;
            fclose(fp);
            return false;
        }
        // load the data into memory
        buffer.resize(fileSize + 1); // forcing buffer ending with 0x00 to fix calls to int/float parsing (strtol ..)
        rewind(fp);
        auto size = 0;
        if ((size = fread(buffer.data(), 1, fileSize, fp)) != fileSize) {
            if (feof(fp)) {
                std::cerr << "Error: can't load file into memory for parsing, unexpected end of file " << filename << std::endl;
            }
            else if (ferror(fp)) {
                std::cerr << "Error: can't load file into memory for parsing, error while reading " << filename << std::endl;
            } // else shall never occur ?
            fclose(fp);
            return false;
        }
        fclose(fp);

        ls = head = &buffer[0];
        end = buffer.data() + buffer.size();

        return true;
    }

    // reads a char, current head must be valid
    // returns new head, 
    inline char* getChar(char& c) {
        c = *head;
        return ++head;
    }
    // moves read head nb char back, current head can be = end
    // no sanity check, head-n shall not be less than buffer start.
    // returns new head, 
    inline char* readBack(int n) {
        return (head = head - n);
    }
    // reads a line and append to s, current head must be valid
    // returns new head, 
    inline char* getLine(std::string& s) {
        char c;
        while (getChar(c) < end && c != '\n' && c != '\r') {
            s.push_back(c);
        }
        // put back the last getChar, since it was a special char
        readBack(1);

        // eat eventual additional end of line special characters
        if (head != end) {
            while (getChar(c) < end && (c == '\n' || c == '\r')) { // we skip
                if (c == '\n') {++lc;ls = head;}
            }
            // put back the last getChar, since it was not a special char
            readBack(1);
        }
        return head;
    }
    // reads a word (space separator) and append to s, current head must be valid
    // returns new head,
    inline char* getWord( std::string& s ) {
        char c;
        while ( getChar( c ) != end && c != ' ' && c != '\n' && c != '\r' ) {
            s.push_back( c );
        }  // \n or \r was read but not pushed
        // eat eventual additional end of line special characters
        if ( head != end ) {
            while ( getChar( c ) != end && ( c == '\n' || c == '\r' ) ) {  // we skip
                if ( c == '\n' ) ++lc;
            }
            // put back the last getChar, since it was not a special char
            readBack( 1 );
        }
        return head;
    }
    // reads all successive spaces,\n and \r if any, current head must be valid
    // this can skip several lines in one call
    // returns new head, 
    inline char* skipSpaces() {
        char c;
        while (getChar(c) < end && isspace(c)) {  // we skip
            if (c == '\n') {++lc; ls = head;}
        }
        // last char is not a apsce or special, we readback
        return readBack(1);
    }
    // skip line, eat chars until '\n' included or end of buffer is reached, current head must be valid
    // returns new head, 
    inline char* skipLine() {
        char c;
        while (head < end && getChar(c) < end && c != '\n' && c != '\r') {
            // we skip
        } // \n or \r was read but not pushed
        // put back the last getChar, since it was not special char
        readBack(1);
        // eat eventual additional end of line special characters
        while (head < end && getChar(c) < end && (c == '\n' || c == '\r')) {
            // we skip
            if (c == '\n') {++lc; ls = head; } // count a new line
        }
        // put back the last getChar, since it was not a special char
        readBack(1);
        return head;
    }

    // no error check
    inline double getDouble(void) {
        char* endptr = 0;
        const double val = strtod(head, &endptr);
        const auto success = (endptr != 0) && (endptr != head);
        if (success)
            head = endptr;
        return val;
    }

    // with error check
    inline bool getDouble(double& val) {
        char* endptr = 0;
        val = strtod(head, &endptr);
        const auto success = (endptr != 0) && (endptr != head);
        if (success)
            head = endptr;
        return success;
    }

    // no error check
    inline long int getInteger(const int base = 10) {
        char* endptr = 0;
        auto val = strtol(head, &endptr, base);
        const auto success = (endptr != 0) && (endptr != head);
        if (success)
            head = endptr;
        return val;
    }

    // with error check
    inline bool getInteger(long int& val, const int base = 10) {
        char* endptr = 0;
        val = strtol(head, &endptr, base);
        const auto success = (endptr != 0) && (endptr != head);
        if (success)
            head = endptr;
        return success;
    }
};

bool IO::_loadObj(std::string filename, Model& output) {

    // find path to file for material loading
    std::string path = std::filesystem::path( filename ).parent_path().string();
    if (path == "")
        path = ".";

    // open file and load in memory
    iBuffer bs; // the bitstream
    if (!bs.load(filename))
        return false;
    //
    char c = 0; // current character
    std::string token; // buffer to build some tokens of more than one char
    int nIdxCount = 0;  // number of normal indices read
    int uvIdxCount = 0; // number of uv indices read
    int nbTessPol = 0;  // number of polygons/quads tesselated
    int nbTessAdd = 0;  // number of triangles added by tesselation
    int matIdx = 0; // the material index, 0 by default (only used if multiple textures are present, indicated by a list of materials inside the material library file
    // consume the first character
    if ((bs.skipSpaces() == bs.end) || (bs.getChar(c) == bs.end))
        return false;
    // consume lines
    while (bs.head < bs.end) {
        // at this point either line necessarly have a first 
        // character and it is not a space or special
        if (c == 'v') {
            char c2 = 0;
            // consume the second character
            if (bs.getChar(c2) == bs.end)
                return false;

            if (c2 == ' ') {
                // parse the position
                for (int i = 0; i < 3; i++) {
                    double value = 0;
                    if (!bs.getDouble(value)) {
                        std::cerr << "Error: line " << bs.lc << " expected floating point value" << std::endl;
                    }
                    // may push zero to be "robust"
                    output.vertices.push_back(value);
                }
                // parse the color if any (re map 0.0-1.0 to 0-255 internal color format)
                double value = 0;
                if (bs.getDouble(value)) {
                    output.colors.push_back(std::roundf(value * 255));
                    for (int i = 0; i < 2; i++) {
                        value = 0;
                        if (!bs.getDouble(value)) {
                            std::cerr << "Error: line " << bs.lc << " expected floating point value" << std::endl;
                        }
                        // may push zero to be "robust"
                        output.colors.push_back( std::roundf( value * 255 ) );
                    }
                }
            }
            else if (c2 == 'n') {
                // parse the normal
                for (int i = 0; i < 3; i++) {
                    double value = 0;
                    if (!bs.getDouble(value)) {
                        std::cerr << "Error: line " << bs.lc << " expected floating point value" << std::endl;
                    }
                    // may push zero to be "robust"
                    output.normals.push_back(value);
                }
            }
            else if (c2 == 't') {
                // parse the texture coordinate
                for (int i = 0; i < 2; i++) {
                    double value = 0;
                    if (!bs.getDouble(value)) {
                        std::cerr << "Error: line " << bs.lc << " expected floating point value" << std::endl;
                    }
                    // may push zero to be "robust"
                    output.uvcoords.push_back(value);
                }
            }
        }
        else if (c == 'f') {
            // max vertices for a polygon (the rest will be skiped)
            const auto maxVertices = 8;
            std::array<int32_t, 3> indices[maxVertices];
            int numValidIndices = 0;
            for (int i = 0; i < maxVertices; i++) { // up to 8 vertices
                bool valid = true;
                // parses one vertex pos/[tex][/norm]
                // ugly if cascade sorry :-(
                // coudl be moved in a separate func for better 
                // legibility and les if/else (through early returns)
                long int value = 0;
                if (bs.getInteger(value)) {
                    indices[i][0] = value; // pos index
                    indices[i][1] = indices[i][2] = 1;
                    // some more ?
                    char f2;
                    if (bs.head < bs.end) {
                        bs.getChar(f2);
                        if (f2 == '/') {
                            if (bs.head < bs.end) {
                                if (bs.getInteger(value)) {
                                    ++uvIdxCount;
                                    indices[i][1] = value; // UV index (optional)
                                }
                                if (bs.head < bs.end) {
                                    char f3;
                                    bs.getChar(f3);
                                    if (f3 == '/') {
                                        if (bs.head < bs.end) {
                                            if (bs.getInteger(value)) {
                                                ++nIdxCount;
                                                indices[i][2] = value; // normal index (mandatory)
                                            }
                                            else {
                                                valid = false;
                                            }
                                        }
                                        else {
                                            valid = false;
                                        }
                                    }
                                    else {
                                        bs.readBack(1);
                                    }
                                }
                            }
                            else {
                                valid = false;
                            }
                        }
                        else {
                            bs.readBack(1);
                        }
                    } // else no more
                }
                else {
                    valid = false;
                }

                //
                if (valid) {
                    ++numValidIndices;
                }
                else if (i < 3) {
                    std::cerr << "Error: line " << bs.lc << " invalid vertex indices, skipping face" << std::endl;
                    std::cerr << "Error: " << std::string( bs.ls, std::min(bs.head, bs.end) ) << std::endl;
                }
                else { // we stop it is not valid but this is allowed
                    break;
                }
            }
            if (numValidIndices >= 3) { // skip if error
                // Process the first triangle.
                for (int i = 0; i < 3; ++i) {
                    output.triangles.push_back(indices[i][0] - 1);
                    output.trianglesuv.push_back(indices[i][1] - 1);
                    // no normal index table for the time being
                }
                output.triangleMatIdx.push_back(matIdx);
                // triangulate eventual quad or polygon from first vertex
                // tri 0 -> 0 1 2 (the main one already done)
                // tri 1 -> 0 2 3 si=2
                // tri 2 -> 0 3 4 si=3 and so on
                // Iterate over start index
                for (int si = 2; si < numValidIndices - 1; si++) {
                    output.triangles.push_back(indices[0][0] - 1);
                    output.trianglesuv.push_back(indices[0][1] - 1);
                    // push the two other indices
                    for (int ci = 0; ci < 2; ci++) {
                        output.triangles.push_back(indices[si + ci][0] - 1);
                        output.trianglesuv.push_back(indices[si + ci][1] - 1);
                    }
                    output.triangleMatIdx.push_back(matIdx);
                }
                nbTessPol += (int)(numValidIndices > 3);
                nbTessAdd += numValidIndices - 3;
            }
        }
        else if (c == 'm') { 
            token="m";
            if ( bs.getWord( token ) != bs.end && token == "mtllib" ) {
                std::string materialLibFilename;
                if ( bs.skipSpaces() != bs.end ) {
                    bs.getLine( materialLibFilename );  
                }
                // if there is already a material in the header we push 
                // a line breck before adding an additional material
                if ( output.header.size() != 0 ) output.header += '\n';
                output.header += "mtllib ";
                output.header += materialLibFilename; 
                getTextureMapPathFromMTL( path, materialLibFilename, output.materialNames, output.textureMapUrls );
                bs.readBack(1); // push back the end of line symbol so generic skipLine will work
            } 
        }
        
        else if (c == 'u') {
            token = "u";
            if ( bs.getWord( token ) != bs.end && token == "usemtl" ) {
                // the materialIndex may be updated at this point
                std::string materialName;
                if ( bs.skipSpaces() != bs.end ) { bs.getLine( materialName ); }
                for ( int i = 0; i < output.materialNames.size(); i++ ) {
                    if ( materialName == output.materialNames[i] ) {
                        matIdx = i;
                        break;
                    }
                }
                bs.readBack( 1 );  // push back the end of line symbol so generic skipLine will work
            }
        }

        // purge eventual end of line
        // This silent skip unsupported tags or errors
        if ( bs.skipLine() < bs.end ) {
            // purge useless chars and then consume the first character of next non empty line
            if ( bs.skipSpaces() < bs.end ) { 
                bs.getChar( c ); 
            }
        }
    }

    if (uvIdxCount == 0) {
        // did not find any uv indices in the file
        // if partial indices we keep the table (set to 0 for faces with no idx in the file)
        output.trianglesuv.clear();
    }

    if (nIdxCount == 0) {
        //output.trianglesnrm.clear();
    }
    else {
        std::cout << "Warning: obj read, normals with separate index table are not yet supported. Skipping normals." << std::endl;
        output.normals.clear();
    }
    if (nbTessPol != 0) {
        std::cout << "Tesselated " << nbTessPol << " polygons/quads introducing " << nbTessAdd << " triangles" << std::endl;
    }

    return true;
}
#else
bool IO::_loadObj(std::string filename, Model& output) {

    std::ifstream fin;
    // use a big 4MB buffer to accelerate reads
    char* buf = new char[4 * 1024 * 1024 + 1];
    fin.rdbuf()->pubsetbuf(buf, 4 * 1024 * 1024 + 1);
    fin.open(filename.c_str(), std::ios::in);
    if (!fin) {
        std::cerr << "Error: can't open file " << filename << std::endl;
        delete[] buf;
        return false;
    }
    int         temp_index;
    float       temp_pos, temp_uv, temp_normal, temp_col;
    std::string temp_flag, temp_str;
    std::string line;
    std::getline(fin, line);
    bool hasNormalIndices = false; // if file contains a normal index table
    while (fin) {
        std::istringstream in(line);
        temp_flag = "";
        in >> temp_flag;  // temp_flag: the first word in the line
        if (temp_flag.compare(std::string("mtllib")) == 0) {
            output.header = std::string(line.c_str());
        }
        else if (temp_flag.compare(std::string("v")) == 0) {
            // parse the position
            for (int i = 0; i < 3; i++) {
                in >> temp_pos;
                output.vertices.push_back(temp_pos);
            }
            // parse the color if any (re map 0.0-1.0 to 0-255 internal color format)
            while (in >> temp_col) { output.colors.push_back(std::roundf(temp_col * 255)); }
        }
        else if (temp_flag.compare(std::string("vn")) == 0) {
            for (int i = 0; i < 3; i++) {
                in >> temp_normal;
                output.normals.push_back(temp_normal);
            }
        }
        else if (temp_flag.compare(std::string("vt")) == 0) {
            for (int i = 0; i < 2; i++) {
                in >> temp_uv;
                output.uvcoords.push_back(temp_uv);
            }
        }
        else if (temp_flag.compare(std::string("f")) == 0) {
            // TODO parsing of normals indexes and reindex
            for (int i = 0; i < 3; i++) {
                in >> temp_str;
                const auto found = temp_str.find_first_of("/");
                if (found != std::string::npos) {
                    const auto foundLast = temp_str.find_last_of("/");
                    temp_index = atoi(temp_str.substr(0, found).c_str()) - 1;
                    // found exists so foundLast necessarly exists beeing at least equal to found
                    if (foundLast != found) {
                        hasNormalIndices = true;
                        if (foundLast - found > 1) { // vidx/uvidx/nrmidx
                            int uv_index = atoi(temp_str.substr(found + 1, foundLast - found).c_str()) - 1;
                            output.trianglesuv.push_back(uv_index);
                        }
                        // else vidx//nrmidx or 
                    }
                    else { // "vidx/uvidx"
                        int uv_index = atoi(temp_str.substr(found + 1, temp_str.size() - found).c_str()) - 1;
                        output.trianglesuv.push_back(uv_index);
                    }
                }
                else // vidx alone, no '/' sumbol
                    temp_index = atoi(temp_str.c_str()) - 1;
                output.triangles.push_back(temp_index);
            }
        }
        std::getline(fin, line);
    }
    fin.close();
    delete[] buf;

    if (hasNormalIndices || (output.normals.size() != 0 && output.normals.size() != output.vertices.size())) {
        std::cout << "Warning: obj read, normals with separate index table are not yet supported. Skipping normals."
            << std::endl;
        output.normals.clear();
    }

    return true;
}
#endif

#define FAST_OBJ_WRITE
#ifdef FAST_OBJ_WRITE

// Buffer used for encoding float/int numbers.
char num_buffer_[20];

bool Encode(const void* data, size_t data_size, std::vector<char>& bitstream) {
    const uint8_t* src_data = reinterpret_cast<const uint8_t*>(data);
    bitstream.insert(bitstream.end(), src_data, src_data + data_size);
    return true;
}

void EncodeFloat(float val, std::vector<char>& bitstream) {
    snprintf(num_buffer_, sizeof(num_buffer_), "%.9g", val); //%.9g keeps backward compat with previous printer
    Encode(num_buffer_, strlen(num_buffer_), bitstream);
}

void EncodeFloatList(float* vals, int num_vals, std::vector<char>& bitstream) {
    for (int i = 0; i < num_vals; ++i) {
        if (i > 0) {
            Encode(" ", 1, bitstream);
        }
        EncodeFloat(vals[i], bitstream);
    }
}

void EncodeInt(int32_t val, std::vector<char>& bitstream) {
    snprintf(num_buffer_, sizeof(num_buffer_), "%d", val);
    Encode(num_buffer_, strlen(num_buffer_), bitstream);
}
bool IO::_saveObj(std::string filename, const Model& input) {

    FILE* fp = fopen(filename.c_str(), "w+");
    if (!fp) {
        std::cerr << "Error: can't open file " << filename << std::endl;
        return false;
    }

    std::vector<char> bs; // the bitstream

    printf("_saveObj %-40s: V = %zu Vc = %zu N = %zu UV = %zu F = %zu Fuv = %zu \n",
        filename.c_str(),
        input.vertices.size() / 3,
        input.colors.size() / 3,
        input.normals.size() / 3,
        input.uvcoords.size() / 2,
        input.triangles.size() / 3,
        input.trianglesuv.size() / 3);
    fflush(stdout);

    Encode(input.header.data(), input.header.size(), bs);
    Encode("\n", 1, bs);
    int refTextId = input.triangleMatIdx.size() == 0 ? -1 : input.triangleMatIdx[0];
    for (int i = 0; i < input.vertices.size() / 3; i++) {
        Encode("v", 1, bs);
        for (auto c = 0; c < 3; ++c) {
            Encode(" ", 1, bs);
            EncodeFloat(input.vertices[i * 3 + c], bs);
        }
        if (input.colors.size() == input.vertices.size()) {
            for (auto c = 0; c < 3; ++c) {
                Encode(" ", 1, bs);
                EncodeFloat(input.colors[i * 3 + c] / 255, bs);
            }
        }
        Encode("\n", 1, bs);
    }
    for (int i = 0; i < input.normals.size() / 3; i++) {
        Encode("vn", 2, bs);
        for (auto c = 0; c < 3; ++c) {
            Encode(" ", 1, bs);
            EncodeFloat(input.normals[i * 3 + c], bs);
        }
        Encode("\n", 1, bs);
    }
    for (int i = 0; i < input.uvcoords.size() / 2; i++) {
        Encode("vt", 2, bs);
        for (auto c = 0; c < 2; ++c) {
            Encode(" ", 1, bs);
            EncodeFloat(input.uvcoords[i * 2 + c], bs);
        }
        Encode("\n", 1, bs);
    }
    if (input.hasUvCoords()) {
        if (input.materialNames.size() > 0) {
            std::string materialName = "usemtl " + input.materialNames[refTextId] + "\n";
            Encode(materialName.data(), materialName.size(), bs);
        }
        else
            Encode("usemtl material0000\n", 20, bs);
    }
    for (int i = 0; i < input.triangles.size() / 3; i++) {
        if (input.hasUvCoords() && refTextId != -1){
            if(input.triangleMatIdx[i] != refTextId) {
                refTextId = input.triangleMatIdx[i];
                std::string materialName = "usemtl " + input.materialNames[refTextId] + "\n";
                Encode(materialName.data(), materialName.size(), bs);
            }
        }
        if (input.trianglesuv.size() == input.triangles.size()) {
            Encode("f ", 2, bs);
            for (auto c = 0; c < 3; ++c) {
                if ( c > 0 ) Encode( " ", 1, bs );
                EncodeInt(input.triangles[i * 3 + c] + 1, bs);
                Encode("/", 1, bs);
                EncodeInt(input.trianglesuv[i * 3 + c] + 1, bs);
            }
            Encode("\n", 1, bs);
        }
        else {
            if (input.getUvCount() == input.getPositionCount()) {
                Encode("f ", 2, bs);
                for (auto c = 0; c < 3; ++c) {
                    if ( c > 0 ) Encode( " ", 1, bs );
                    EncodeInt(input.triangles[i * 3 + c] + 1, bs);
                    Encode("/", 1, bs);
                    EncodeInt(input.triangles[i * 3 + c] + 1, bs);
                }
                Encode("\n", 1, bs);
            }
            else {
                Encode("f ", 2, bs);
                for (auto c = 0; c < 3; ++c) {
                    if ( c > 0 ) Encode( " ", 1, bs );
                    EncodeInt(input.triangles[i * 3 + c] + 1, bs);
                }
                Encode("\n", 1, bs);
            }
        }
    }

    auto resSize = fwrite(bs.data(), sizeof(char), bs.size(), fp);
    fclose(fp);

    return resSize > 0;
}

#else
bool IO::_saveObj(std::string filename, const Model& input) {

    std::ofstream fout;
    // use a big 4MB buffer to accelerate writes
    char* buf = new char[4 * 1024 * 1024 + 1];
    fout.rdbuf()->pubsetbuf(buf, 4 * 1024 * 1024 + 1);
    fout.open(filename.c_str(), std::ios::out);
    if (!fout) {
        std::cerr << "Error: can't open file " << filename << std::endl;
        delete[] buf;
        return false;
    }
    // this is mandatory to print floats with full precision
    fout.precision(std::numeric_limits<float>::max_digits10);

    printf("_saveObj %-40s: V = %zu Vc = %zu N = %zu UV = %zu F = %zu Fuv = %zu \n",
        filename.c_str(),
        input.vertices.size() / 3,
        input.colors.size() / 3,
        input.normals.size() / 3,
        input.uvcoords.size() / 2,
        input.triangles.size() / 3,
        input.trianglesuv.size() / 3);
    fflush(stdout);

    fout << input.header << std::endl;
    for (int i = 0; i < input.vertices.size() / 3; i++) {
        fout << "v " << input.vertices[i * 3 + 0] << " " << input.vertices[i * 3 + 1] << " " << input.vertices[i * 3 + 2];
        if (input.colors.size() == input.vertices.size()) {
            fout << " " << input.colors[i * 3 + 0] / 255 << " " << input.colors[i * 3 + 1] / 255 << " "
                << input.colors[i * 3 + 2] / 255 << std::endl;
        }
        else {
            fout << std::endl;
        }
    }
    for (int i = 0; i < input.normals.size() / 3; i++) {
        fout << "vn " << input.normals[i * 3 + 0] << " " << input.normals[i * 3 + 1] << " " << input.normals[i * 3 + 2]
            << std::endl;
    }
    for (int i = 0; i < input.uvcoords.size() / 2; i++) {
        fout << "vt " << input.uvcoords[i * 2 + 0] << " " << input.uvcoords[i * 2 + 1] << std::endl;
    }
    if (input.hasUvCoords()) {
        fout << "usemtl material0000" << std::endl;
    }
    for (int i = 0; i < input.triangles.size() / 3; i++) {
        if (input.trianglesuv.size() == input.triangles.size()) {
            fout << "f " << input.triangles[i * 3 + 0] + 1 << "/" << input.trianglesuv[i * 3 + 0] + 1 << " "
                << input.triangles[i * 3 + 1] + 1 << "/" << input.trianglesuv[i * 3 + 1] + 1 << " "
                << input.triangles[i * 3 + 2] + 1 << "/" << input.trianglesuv[i * 3 + 2] + 1 << std::endl;
        }
        else {
            if (input.getUvCount() == input.getPositionCount()) {
                fout << "f " << input.triangles[i * 3 + 0] + 1 << "/" << input.triangles[i * 3 + 0] + 1 << " "
                    << input.triangles[i * 3 + 1] + 1 << "/" << input.triangles[i * 3 + 1] + 1 << " "
                    << input.triangles[i * 3 + 2] + 1 << "/" << input.triangles[i * 3 + 2] + 1 << std::endl;
            }
            else {
                fout << "f " << input.triangles[i * 3 + 0] + 1 << " " << input.triangles[i * 3 + 1] + 1 << " "
                    << input.triangles[i * 3 + 2] + 1 << std::endl;
            }
        }
    }
    fout.close();
    delete[] buf;
    return true;
}
#endif


template<typename T, typename D>
void templateConvert( std::shared_ptr<tinyply::PlyData> src,
                      const uint8_t                     numSrc,
                      std::vector<T>&                   dst,
                      const uint8_t                     numDst ) {
  const size_t   numBytes = src->buffer.size_bytes();
  std::vector<D> data;
  data.resize( src->count * numSrc );
  std::memcpy( data.data(), src->buffer.get(), numBytes );
  if ( numSrc == numDst ) {
    dst.assign( data.begin(), data.end() );
  } else {
    dst.resize( src->count * numDst );
    for ( size_t i = 0; i < src->count; i++ )
      for ( size_t c = 0; c < numDst; c++ ) dst[i * numDst + c] = (T)data[i * numSrc + c];
  }
}

template<typename T>
void set( std::shared_ptr<tinyply::PlyData> src,
          const uint8_t                     numSrc,
          std::vector<T>&                   dst,
          const uint8_t                     numDst,
          std::string                       name ) {
  if ( src ) {
    switch ( src->t ) {
    case tinyply::Type::INT8: templateConvert<T, int8_t>( src, numSrc, dst, numDst ); break;
    case tinyply::Type::UINT8: templateConvert<T, uint8_t>( src, numSrc, dst, numDst ); break;
    case tinyply::Type::INT16: templateConvert<T, int16_t>( src, numSrc, dst, numDst ); break;
    case tinyply::Type::UINT16: templateConvert<T, uint16_t>( src, numSrc, dst, numDst ); break;
    case tinyply::Type::INT32: templateConvert<T, int32_t>( src, numSrc, dst, numDst ); break;
    case tinyply::Type::UINT32: templateConvert<T, uint32_t>( src, numSrc, dst, numDst ); break;
    case tinyply::Type::FLOAT32: templateConvert<T, float>( src, numSrc, dst, numDst ); break;
    case tinyply::Type::FLOAT64: templateConvert<T, double>( src, numSrc, dst, numDst ); break;
    default:
      printf( "ERROR: PLY type not supported: %s \n", name.c_str() );
      fflush( stdout );
      exit( -1 );
      break;
    }
  }
}

bool IO::_loadPly( std::string filename, Model& output ) {
  std::unique_ptr<std::istream> file_stream;
  file_stream.reset( new std::ifstream( filename.c_str(), std::ios::binary ) );
  tinyply::PlyFile file;
  file.parse_header( *file_stream );
  std::shared_ptr<tinyply::PlyData> _vertices, _normals, _colors, _colorsRGBA, _texcoords, _faces, _tripstrip, _uvfaces,
    _nrmfaces;

  // The header information can be used to programmatically extract properties on elements
  // known to exist in the header prior to reading the data. For brevity of this sample, properties
  // like vertex position are hard-coded:
  try {
    _vertices = file.request_properties_from_element( "vertex", { "x", "y", "z" } );
  } catch ( const std::exception& e ) { std::cerr << "skipping: " << e.what() << std::endl; }
  try {
    _normals = file.request_properties_from_element( "vertex", { "nx", "ny", "nz" } );
  } catch ( const std::exception& e ) { std::cerr << "skipping: " << e.what() << std::endl; }

  try {
    _colors = file.request_properties_from_element( "vertex", { "red", "green", "blue" } );
  } catch ( const std::exception& ) {}
  try {
    _colors = file.request_properties_from_element( "vertex", { "r", "g", "b" } );
  } catch ( const std::exception& ) {}
  try {
    _colorsRGBA = file.request_properties_from_element( "vertex", { "red", "green", "blue", "alpha" } );
  } catch ( const std::exception& ) {}

  try {
    _colorsRGBA = file.request_properties_from_element( "vertex", { "r", "g", "b", "a" } );
  } catch ( const std::exception& ) {}

  try {
    _texcoords = file.request_properties_from_element( "vertex", { "texture_u", "texture_v" } );
  } catch ( const std::exception& ) {}

  // Providing a list size hint (the last argument) is a 2x performance improvement. If you have
  // arbitrary ply files, it is best to leave this 0.
  try {
    _faces = file.request_properties_from_element( "face", { "vertex_indices" }, 3 );
  } catch ( const std::exception& e ) { std::cerr << "skipping: " << e.what() << std::endl; }

  try {
    _uvfaces = file.request_properties_from_element( "face", { "texcoord" }, 6 );
  } catch ( const std::exception& e ) { std::cerr << "skipping: " << e.what() << std::endl; }

  // // Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
  // // are specifically in the file, which is unlikely);
  // try {
  //   _tripstrip = file.request_properties_from_element( "tristrips", {"vertex_indices"}, 0 );
  // } catch ( const std::exception& e ) { std::cerr << "skipping " << e.what() << std::endl; }

  file.read( *file_stream );

  // now feed the data to the frame structure
  set( _vertices, 3, output.vertices, 3, "vertices" );
  set( _texcoords, 2, output.uvcoords, 2, "uvcoords" );
  set( _normals, 3, output.normals, 3, "normals" );
  set( _colors, 3, output.colors, 3, "colors" );
  set( _colorsRGBA, 4, output.colors, 3, "colorsRGBA" );
  set( _faces, 3, output.triangles, 3, "triangles" );
  if ( _uvfaces ) {
    const auto triCount = _uvfaces->count;
    output.trianglesuv.resize( triCount * 3 );    
    set( _uvfaces, 6, output.uvcoords, 6, "uvfaces" );    
    for ( size_t i = 0; i < triCount * 3; i++ ) output.trianglesuv[i] = i;
  }
  return true;
}

bool IO::_savePly( std::string filename, const Model& input ) {
  std::ofstream fout;
  // use a big 4MB buffer to accelerate writes
  char* buf = new char[4 * 1024 * 1024 + 1];
  fout.rdbuf()->pubsetbuf( buf, 4 * 1024 * 1024 + 1 );
  fout.open( filename.c_str(), std::ios::out );
  if ( !fout ) {
    std::cerr << "Error: can't open file " << filename << std::endl;
    delete[] buf;
    return false;
  }
  // this is mandatory to print floats with full precision
  fout.precision( std::numeric_limits<float>::max_digits10 );

  fout << "ply" << std::endl;
  fout << "format ascii 1.0" << std::endl;
  fout << "comment Generated by InterDigital model processor" << std::endl;
  fout << "element vertex " << input.vertices.size() / 3 << std::endl;
  fout << "property float x" << std::endl;
  fout << "property float y" << std::endl;
  fout << "property float z" << std::endl;
  // normals
  if ( input.normals.size() == input.vertices.size() ) {
    fout << "property float nx" << std::endl;
    fout << "property float ny" << std::endl;
    fout << "property float nz" << std::endl;
  }
  // colors
  if ( input.colors.size() == input.vertices.size() ) {
    fout << "property uchar red" << std::endl;
    fout << "property uchar green" << std::endl;
    fout << "property uchar blue" << std::endl;
  }

  if ( !input.triangles.empty() ) {
    fout << "element face " << input.triangles.size() / 3 << std::endl;
    fout << "property list uchar int vertex_indices" << std::endl;
  }
  fout << "end_header" << std::endl;

  // comments
  for ( int i = 0; i < input.comments.size(); i++ ) { fout << input.comments[i] << std::endl; }

  // vertices and colors
  for ( int i = 0; i < input.vertices.size() / 3; i++ ) {
    fout << input.vertices[i * 3 + 0] << " " << input.vertices[i * 3 + 1] << " " << input.vertices[i * 3 + 2] << " ";
    if ( input.normals.size() == input.vertices.size() ) {
      fout << input.normals[i * 3 + 0] << " " << input.normals[i * 3 + 1] << " " << input.normals[i * 3 + 2] << " ";
    }
    if ( input.colors.size() == input.vertices.size() ) {
      // do not cast as char otherwise characters are printed instead if int8 values
      fout << (unsigned short)( std::roundf( input.colors[i * 3 + 0] ) ) << " "
           << (unsigned short)( std::roundf( input.colors[i * 3 + 1] ) ) << " "
           << (unsigned short)( std::roundf( input.colors[i * 3 + 2] ) );
    }
    fout << std::endl;
  }
  // topology
  for ( int i = 0; i < input.triangles.size() / 3; i++ ) {
    fout << "3 " << input.triangles[i * 3 + 0] << " " << input.triangles[i * 3 + 1] << " " << input.triangles[i * 3 + 2]
         << std::endl;
  }

  fout.close();
  delete[] buf;
  return true;
}

bool IO::_loadImage( std::string filename, Image& output ) {
  // Reading map if needed
  if ( filename != "" ) {
    std::cout << "Input map: " << filename << std::endl;
    output.data = stbi_load( filename.c_str(), &output.width, &output.height, &output.nbc, 0 );
    if ( output.data == NULL ) {
      std::cout << "Error: opening file " << filename << std::endl;
      return false;
    }
  } else {
    std::cout << "Error: invalid empty filename" << std::endl;
    return false;
  }

  // success
  return true;
}

bool IO::_saveImage( std::string filename, const Image& input, bool flipVertically ) {
  // Writing map if needed
  if ( filename != "" ) {
    std::cout << "Output map: " << filename << std::endl;
    if ( input.data == NULL ) {
      std::cout << "Error: map is empty " << filename << std::endl;
      return false;
    }
    std::string extension = filename.substr( filename.find_last_of( "." ) );
    if ( extension.compare( ".png" ) == 0 ) {
      if ( flipVertically ) {
        stbi_write_png( filename.c_str(),                                                 // filename
                        input.width,                                                      // width
                        input.height,                                                     // height
                        input.nbc,                                                        // comp number
                        input.data - ( input.width * input.nbc * ( input.height - 1 ) ),  // data
                        -input.width * input.nbc );                                       // stride in bytes
      } else {
        stbi_write_png( filename.c_str(),           // filename
                        input.width,                // width
                        input.height,               // height
                        input.nbc,                  // comp number
                        input.data,                 // data
                        input.width * input.nbc );  // stride in bytes
      }
    } else {
      if ( extension.compare( ".jpg" ) == 0 ) {
        stbi_write_jpg( filename.c_str(),  // filename
                        input.width,       // width
                        input.height,      // height
                        input.nbc,         // comp number
                        input.data,        // data
                        100 );             // quality
      } else {
        std::cout << "Error: format can't be detected: " << filename << std::endl;
        return false;
      }
    }
  } else {
    std::cout << "Error: invalid empty filename" << std::endl;
    return false;
  }

  // success
  return true;
}

bool IO::_loadImageFromVideo( std::string filename, Image& output ) {
  // Reading map if needed
  if ( filename != "" ) {
    // parsing filename to extract metadata
    // int frIdx, int width, int height, bool isYUV, bool is444,
    size_t      pos            = filename.find_last_of( "." );
    std::string extension      = filename.substr( pos );
    bool        isYUV          = ( extension.compare( ".yuv" ) == 0 );
    size_t      pos2           = filename.find_last_of( "_" );
    std::string codingType     = filename.substr( pos2 + 1, pos - pos2 - 1 );
    bool        is444          = ( codingType.compare( "yuv420p" ) != 0 );
    size_t      pos3           = filename.substr( 0, pos2 - 1 ).find_last_of( "_" );
    std::string frameDimension = filename.substr( 0, pos2 ).substr( pos3 + 1 );
    int         width          = std::stoi( frameDimension.substr( 0, frameDimension.find( 'x' ) ) );
    int         height         = std::stoi( frameDimension.substr( frameDimension.find( 'x' ) + 1 ) );
    int         frameIndex     = _context->getFrame() - _context->getFirstFrame();
    //
    std::cout << "Reading video frame " << frameIndex << " from file: " << filename << std::endl;
    int    chromaStride = is444 ? width : width / 2;
    int    chromaHeight = is444 ? height : height / 2;
    size_t frSize       = ( height * width + 2 * chromaStride * chromaHeight );
    // open the video file and search for the frame index
    std::ifstream in;
    in.open( filename, std::ifstream::in | std::ios::binary );
    if ( !in.is_open() ) { return false; }
    in.seekg( frameIndex * frSize, std::ios::beg );
    // reading frame (only 8-bit data is allowed at this moment)
    char* frame = new char[frSize];
    in.read( frame, frSize );
    in.close();
    // now convert the frame
    if ( !is444 ) {
      // chroma upsampling using nearest neighbor
      char* frameUpscaled = new char[3 * width * height];
      // copy the luma channel
      memcpy( frameUpscaled, frame, width * height );
      // copy the down-sampled chroma channel
      for ( int y = 0; y < chromaHeight; y++ )
        for ( int x = 0; x < chromaStride; x++ ) {
          frameUpscaled[width * height + ( ( 2 * x ) + width * ( 2 * y ) )] =
            frame[width * height + ( x + chromaStride * y )];
          frameUpscaled[width * height + ( ( 2 * x + 1 ) + width * ( 2 * y ) )] =
            frame[width * height + ( x + chromaStride * y )];
          frameUpscaled[width * height + ( ( 2 * x ) + width * ( 2 * y + 1 ) )] =
            frame[width * height + ( x + chromaStride * y )];
          frameUpscaled[width * height + ( ( 2 * x + 1 ) + width * ( 2 * y + 1 ) )] =
            frame[width * height + ( x + chromaStride * y )];
        }

      // copy the down-sampled chroma channel
      for ( int y = 0; y < chromaHeight; y++ )
        for ( int x = 0; x < chromaStride; x++ ) {
          frameUpscaled[2 * width * height + ( ( 2 * x ) + width * ( 2 * y ) )] =
            frame[width * height + chromaStride * chromaHeight + ( x + chromaStride * y )];
          frameUpscaled[2 * width * height + ( ( 2 * x + 1 ) + width * ( 2 * y ) )] =
            frame[width * height + chromaStride * chromaHeight + ( x + chromaStride * y )];
          frameUpscaled[2 * width * height + ( ( 2 * x ) + width * ( 2 * y + 1 ) )] =
            frame[width * height + chromaStride * chromaHeight + ( x + chromaStride * y )];
          frameUpscaled[2 * width * height + ( ( 2 * x + 1 ) + width * ( 2 * y + 1 ) )] =
            frame[width * height + chromaStride * chromaHeight + ( x + chromaStride * y )];
        }
      delete[] frame;
      frame = frameUpscaled;
    }
    output.data   = new unsigned char[3 * width * height];
    output.height = height;
    output.width  = width;
    output.nbc    = 3;
    if ( isYUV ) {
      // convert to RGB
      for ( int y = 0; y < height; y++ ) {
        for ( int x = 0; x < width; x++ ) {
          double Y  = (unsigned char)frame[x + width * y];
          Y         = std::min<double>( std::max<double>( Y / 255.0, 0.0 ), 1.0 );
          double Cb = (unsigned char)frame[width * height + x + width * y];
          Cb        = std::min<double>( std::max<double>( ( Cb - 128 ) / 255.0, -0.5 ), 0.5 );
          double Cr = (unsigned char)frame[2 * width * height + x + width * y];
          Cr        = std::min<double>( std::max<double>( ( Cr - 128 ) / 255.0, -0.5 ), 0.5 );
          double R  = Y + 1.57480 * Cr;
          output.data[( x + width * y ) * 3 + 0] =
            (unsigned char)std::round( 255 * std::min<double>( std::max<double>( R, 0.0 ), 1.0 ) );
          double G = Y - 0.18733 * Cb - 0.46813 * Cr;
          output.data[( x + width * y ) * 3 + 1] =
            (unsigned char)std::round( 255 * std::min<double>( std::max<double>( G, 0.0 ), 1.0 ) );
          double B = Y + 1.85563 * Cb;
          output.data[( x + width * y ) * 3 + 2] =
            (unsigned char)std::round( 255 * std::min<double>( std::max<double>( B, 0.0 ), 1.0 ) );
        }
      }
    } else {
      // is GBR, so re-order the color planes
      for ( int y = 0; y < height; y++ ) {
        for ( int x = 0; x < width; x++ ) {
          output.data[( x + width * y ) * 3 + 0] = frame[( x + width * y ) + 2 * width * height];
          output.data[( x + width * y ) * 3 + 2] = frame[( x + width * y ) + width * height];
          output.data[( x + width * y ) * 3 + 1] = frame[( x + width * y )];
        }
      }
    }
    delete[] frame;
  } else {
    std::cout << "Error: invalid empty filename" << std::endl;
    return false;
  }

  // success
  return true;
}

// TODO we should better test if mtl already contains a path or not. and append only if needed.
// currently, if mtl or map_Kd contains absolute path (which is rare) will not work.
void IO::getTextureMapPathFromMTL( 
    const std::string&        path,
    const std::string&        mtl,
    std::vector<std::string>& materialName,
    std::vector<std::string>& textMapFilename ) 
{
    std::string ret = "";
    if (mtl != "") {
        std::ifstream fin;
        fin.open( (path + "/" + mtl).c_str(), std::ios::in );
        if (!fin) {
            std::cerr << "Error: can't open file " << mtl << std::endl;
        }
        else { 
            std::string line;
            while (std::getline(fin, line)) {
                /* JEM: no idea what this is for, since we do some rfind, no need to skip anything (right ?) 
                line.erase(
                    std::unique(line.begin(), line.end(), [=](char l, char r) { return (l == r) && (l == ' '); }),
                    line.end());
                if (line.find_first_not_of(' ') != std::string::npos) {
                    line = line.substr(line.find_first_not_of(' '));
                }*/
                // attention, we can have some materials without texture mapsc
                if ( line.rfind( "newmtl", 0 ) == 0 ) { 
                    materialName.push_back( line.substr( line.find( ' ' ) + 1 ) );
                    textMapFilename.push_back( "" );
                }
                if ( line.rfind( "map_Kd", 0 ) == 0 ) {
                    // sets or overwrites the map name for current material
                    // skip if no material exists
                    if ( textMapFilename.size() != 0 ) {
                        textMapFilename.back() = path + "/" + line.substr( line.find( ' ' ) + 1 );
                    } else {
                        // malformed material file
                    }
                }
            }
        }
        fin.close();
    }
}
