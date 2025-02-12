

<!--- 
   ###################################################### 
   # File automatically generated by ./doc/build-doc.sh # 
   # sript and mm, do not edit manually.                # 
   ###################################################### 
 --> 



# Compilation

The build of the software as well the download and build of its dependencies 
is performed with following command:

```
./build.sh 
```

The following options are available:

- `-h|--help  `: Display this information.
- `-o|--ouptut`: Output build directory.
- `-n|--ninja `: Use Ninja
- `--debug    `: Build in debug mode.
- `--release  `: Build in release mode.
- `--doc      `: Build documentation (ouput readme.md)
- `--test     `: Execute all tests
- `--format   `: Format source code
- `--nojobs   `: Disables multi-processor build on unix
- `--noomp    `: Disables openmp build
- `--nocmd    `: Disables build of mm command

Software can also be built manually:

```
cmake -B ./build/Release -H.
cmake --build ./build_test/ --config Release --parallel 20
./build/Relase/bin/mm

```

# Usage examples

Note: 
- the system can read/write point clouds as obj or ply format.
- the system can read/write meshes as obj or ply format.
- once a file loaded the system detects a mesh by checking if any topology is available.

## Simple commands

Sample input.obj into output.ply using subdivision mode (sdiv).

```
mm.exe \
  sample \
    --mode           sdiv \
    --areaThreshold  1.0 \
    --mapThreshold \
    --inputModel     input.obj \
    --outputModel    output.ply 
```

Compare textured meshB.obj mesh against meshA reference mesh using pcc_error. Will use default sampling method.

```
mm.exe \
  compare \
    --mode        pcc \
    --inputModelA inputA.obj \
    --inputMapA   mapA.png \
    --inputModelB inputB.obj \
    --inputMapB   mapB.png
```

If the models are obj files with associated material files pointing to proper texture files the inputMap parameters 
can be omitted. The obj file can use multiple textures defined in the mtl file. The hardware renderer and any 
hardware accelerated command using the HW renderer (such as IBSM compare) do not support multi-texturing.

```
mm.exe \
  compare \
    --mode        pcc \
    --inputModelA inputA.obj \
    --inputModelB inputB.obj \
```

Overriding the multi-textures can be performed through the command line. For instance if the model A uses originally two textures 
defined in the mtl file, one can replace those as follows:

```
mm.exe \
  compare \
    --mode        pcc \
    --inputModelA inputA.obj \
    --inputMapA   "anotherFirstMapA.png anotherSecondMapA.png" \
    --inputModelB inputB.obj \
```

Deactivating the use of textures can be dones by providing an empty string as parameter. The following example will systematically 
decativate texture mapping for modelA whereas modelB will use the mtl file if any or do not use any texture map otherwise.

```
mm.exe \
  compare \
    --mode        pcc \
    --inputModelA inputA.obj \
    --inputMapA   "" \
    --inputModelB inputB.obj \
```

## Commands combination

Following example uses specific grid sampling method, then compare using pcc_error and pcqm metrics in a single call.

```
mm.exe \
  sample \
    --mode        grid \
    --inputModel  inputA.obj \
    --inputMap    mapA.png \
    --outputModel ID:pcA \
  END \
  sample \
    --mode        grid \
    --inputModel  inputB.obj \
    --inputMap    mapB.png \
    --outputModel ID:pcB \
  END \
  compare \
    --mode        pcc  \
    --inputModelA ID:pcA \
    --inputModelB ID:pcB \
  END \
  compare \
    --mode        pcqm \
    --inputModelA ID:pcA \
    --inputModelB ID:pcB
```

Same as previous but dumping intermediate sampling results into files. Compare still use the versions in memory.

```
mm.exe \
  sample \
    --mode        grid \
    --inputModel  inputA.obj \
    --inputMap    mapA.png \
    --outputModel pcA.ply \
  END \
  sample \
    --mode        grid \
    --inputModel  inputB.obj \
    --inputMap    mapB.png \
    --outputModel pcB.ply \
  END \
  compare \
    --mode        pcc  \
    --inputModelA pcA.ply \
    --inputModelB pcB.ply \
  END \
  compare \
    --mode        pcqm \
    --inputModelA pcA.ply \
    --inputModelB pcB.ply
```

Several commands can be cascaded using this mechanism, for instance doing quantization then sampling then compare. 
Note however that memory won't be released between sub command calls so cascading many commands may be very consuming in terms of memory.

## Sequence processing

Following sample demonstrates how to execute commands on a numerated sequence of objects ranging from 00150 to 00165 included. 
The "%04d" part of the file names will be replaced by the frame number ranging from firstFrame to lastFrame, coded on 4 digits.

```
mm.exe \
  sequence \
    --firstFrame  150 \
    --lastFrame   165 \
  END \
  sample \
    --mode        grid \
    --inputModel  inputA_%04d.obj \
    --inputMap    mapA_%04d.png \
    --outputModel ID:pcA \
  END \
  sample \
    --mode        grid \
    --inputModel  inputB_%04d.obj \
    --inputMap    mapB_%04d.png \
    --outputModel ID:pcB \
  END \
  compare \
    --mode        pcc  \
    --inputModelA ID:pcA \
    --inputModelB ID:pcB \
  END \
  compare \
    --mode        pcqm \
    --inputModelA ID:pcA \
    --inputModelB ID:pcB
```

The replacement mechanism can also be used on final or intermediate output file names as shown in the two following examples.

```
mm.exe \
  sequence \
    --firstFrame  150 \
    --lastFrame   165 \
  END \
  sample \
    --mode        grid \
    --inputModel  input_%04d.obj \
    --inputMap    map_%04d.png \
    --outputModel output_%04d_pcloud.obj
```

```
mm.exe \
  sequence \
    --firstFrame  150 \
    --lastFrame   165 \
  END \
  quantize \
    --qp          12 \
    --inputModel  input_%04d.obj \
    --outputModel quantized_%3d.obj \
  END \
  sample \
    --mode        grid \
    --inputModel  quantized_%3d.obj \
    --inputMap    map_%04d.png \
    --outputModel pcloud_%04d.obj
```

The following statement will perform an analysis of the frames of a sequence and ouput a summary into file globals.txt. This
text file can then be directly sourced by bash to access the variables and reinject into quantization command for instance. 
In the following example, the extremums (Position bounding box, normal bounding box and uv bounding box) computed for the entire 
sequence by analyse will be used as the quantization range for each frame by the quantize sequence.

```
mm.exe \
  sequence \
    --firstFrame  150 \
    --lastFrame   165 \
  END \
  analyse \
    --inputModel  input_%04d.obj \
    --outputVar   globals.txt

# load result in memory
source globals.txt

mm.exe \
  sequence \
    --firstFrame  150 \
    --lastFrame   165 \
  END \
  quantize \
    --inputModel  input_%04d.obj  \
    --outputModel output_%04d.obj \
    --qp          12 \
    --qt          12 \
    --qn          10 \
    --minPos      "${globalMinPos}" \
    --maxPos      "${globalMaxPos}" \
    --minUv       "${globalMinUv}"   \
    --maxUv       "${globalMaxUv}" \
    --minNrm      "${globalMinNrm}" \
    --maxNrm      "${globalMaxNrm}"
  
```


# Command references 
```

3D model processing commands v1.1.7
Usage:
  mm command [OPTION...]

Command help:
  mm command --help

Command:
  analyse	Analyse model and/or texture map
  compare	Compare model A vs model B
  degrade	Degrade a mesh (todo points)
  dequantize	Dequantize model (mesh or point cloud) 
  normals	Computes the mesh normals.
  quantize	Quantize model (mesh or point cloud)
  reindex	Reindex mesh and optionaly sort vertices and face indices
  render	Render a mesh (todo points)
  sample	Convert mesh to point cloud
  sequence	Sequence global parameters

```


## Analyse 
```

Analyse model and/or texture map
Usage:
  mm analyse [OPTION...]

      --inputModel arg  path to input model (obj or ply file)
      --inputMap arg    path to input texture map (png, jpeg), can be
                        multiple paths surrounded by double quotes and separated by
                        spaces.
      --outputCsv arg   optional path to output results file
      --outputVar arg   optional path to output variables file
  -h, --help            Print usage

```


## Compare 
```

Compare model A vs model B
Usage:
  mm compare [OPTION...]

      --inputModelA arg   path to reference input model (obj or ply file)
      --inputModelB arg   path to distorted input model (obj or ply file)
      --inputMapA arg     path to reference input texture map (png, jpg, rgb,
                          yuv), can be multiple paths surrounded by double
                          quotes and separated by spaces.
      --inputMapB arg     path to distorted input texture map (png, jpg, rgb,
                          yuv), can be multiple paths surrounded by double
                          quotes and separated by spaces.
      --outputModelA arg  path to output model A (obj or ply file)
      --outputModelB arg  path to output model B (obj or ply file)
      --outputCsv arg     filename of the file where per frame statistics
                          will append. (default: )
      --mode arg          the comparison mode in
                          [equ,eqTFAN,pcc,pcqm,topo,ibsm] (default: equ)
  -h, --help              Print usage

 eqTFAN mode options:
      --eqTFAN_epsilon arg  Used for point cloud comparison only. Distance
                            threshold in world units for "equality" comparison.
                            If 0.0 use strict equality (no distace
                            computation). (default: 0.0)
      --eqTFAN_earlyReturn  Return as soon as a difference is found (faster).
                            Otherwise provide more complete report (slower).
                            (default: true)
      --eqTFAN_unoriented   If set, comparison will not consider faces
                            orientation for comparisons.

 equ mode options:
      --epsilon arg  Used for point cloud comparison only. Distance threshold
                     in world units for "equality" comparison. If 0.0 use
                     strict equality (no distace computation). (default: 0.0)
      --earlyReturn  Return as soon as a difference is found (faster).
                     Otherwise provide more complete report (slower). (default:
                     true)
      --unoriented   If set, comparison will not consider faces orientation
                     for comparisons.

 ibsm mode options:
      --ibsmResolution arg      Resolution of the image buffer. (default:
                                2048)
      --ibsmCameraCount arg     Number of virtual cameras to be used per
                                frame. (default: 16)
      --ibsmCameraRotation arg  Three parameters of rotating the virtual
                                camera positions: the polar angle, the azimuthal
                                angle and the rotation magnitude (default: 0.0
                                0.0 0.0)
      --ibsmRenderer arg        Use software or openGL 1.2 renderer. Value in
                                [sw_raster, gl12_raster]. (default:
                                sw_raster)
      --ibsmDisableCulling      Set option to disable the backface culling.
      --ibsmDisableReordering   Set option to disable automatic oriented
                                reordering of input meshes, can be usefull if
                                already done previously to save very small
                                execution time.
      --ibsmOutputPrefix arg    Set option with a proper prefix/path system
                                to dump the color shots as png images (Warning,
                                it is extremly time consuming to write the
                                buffers, use only for debug).

 pcc mode options:
      --singlePass              Force running a single pass, where the loop
                                is over the original point cloud
      --hausdorff               Send the Haursdorff metric as well
      --color                   Check color distortion as well (default:
                                true)
      --resolution arg          Amplitude of the geometric signal. Will be
                                automatically set to diagonal of the models
                                bounding box if value = 0 (default: 0.0)
      --neighborsProc arg       0(undefined), 1(average), 2(weighted average)
                                3(min), 4(max) neighbors with same geometric
                                distance (default: 1)
      --dropDuplicates arg      0(detect), 1(drop), 2(average) subsequent
                                points with same coordinates (default: 2)
      --bAverageNormals         false(use provided normals), true(average
                                normal based on neighbors with same geometric
                                distance) (default: true)
      --normalCalcModificationEnable
                                0: Calculate normal of cloudB from cloudA, 1:
                                Use normal of cloudB(default). (default:
                                true)

 pcqm mode options:
      --radiusCurvature arg     Set a radius for the construction of the
                                neighborhood. As the bounding box is already
                                computed with this program, use proposed value.
                                (default: 0.001)
      --thresholdKnnSearch arg  Set the number of points used for the quadric
                                surface construction (default: 20)
      --radiusFactor arg        Set a radius factor for the statistic
                                computation. (default: 2.0)

 topo mode options:
      --faceMapFile arg    path to the topology text file matching modelB
                           topology (face) to modelA topology (face).
      --vertexMapFile arg  path to the topology text file matching modelB
                           topology (vertex) to modelA topology (vertex).

```


## Degrade 
```

Degrade a mesh (todo points)
Usage:
  mm degrade [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -o, --outputModel arg  path to output model (obj or ply file)
      --mode arg         the sampling mode in [delface]
      --nthFace arg      in delface mode, remove one face every nthFace.
                         (default: 50)
      --nbFaces arg      in delface mode, if nthFace==0, remove nbFaces.
                         (default: 0)
  -h, --help             Print usage

```


## Dequantize 
```

Dequantize model (mesh or point cloud) 
Usage:
  mm dequantize [OPTION...]

  -i, --inputModel arg        path to input model (obj or ply file)
  -o, --outputModel arg       path to output model (obj or ply file)
  -h, --help                  Print usage
      --qp arg                Geometry quantization bitdepth. No
                              dequantization of geometry if not set or < 7.
      --qt arg                UV coordinates quantization bitdepth. No
                              dequantization of uv coordinates if not set or < 7.
      --qn arg                Normals quantization bitdepth. No
                              dequantization of normals if not set or < 7.
      --qc arg                Colors quantization bitdepth. No dequantization
                              of colors if not set or < 7.
      --minPos arg            min corner of vertex position bbox, a string of
                              three floats. Mandatory if qp set and >= 7
      --maxPos arg            max corner of vertex position bbox, a string of
                              three floats. Mandatory if qp set and >= 7
      --minUv arg             min corner of vertex texture coordinates bbox,
                              a string of three floats. Mandatory if qt set
                              and >= 7
      --maxUv arg             max corner of vertex texture coordinates bbox,
                              a string of three floats. Mandatory if qt set
                              and >= 7
      --minNrm arg            min corner of vertex normal bbox, a string of
                              three floats. Mandatory if qn set and >= 7.
      --maxNrm arg            max corner of vertex normal bbox, a string of
                              three floats. Mandatory if qn set and >= 7
      --minCol arg            min corner of vertex colors bbox, a string of
                              three floats. Mandatory if qc set and >= 7
      --maxCol arg            max corner of vertex colors bbox, a string of
                              three floats. Mandatory if qc set and >= 7
      --useFixedPoint         interprets minPos and maxPos inputs as fixed
                              point 16.
      --colorSpaceConversion  Convert color space from YUV to RGB.

```


## Normals 
```

Computes the mesh normals.
Usage:
  mm normals [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -o, --outputModel arg  path to output model (obj or ply file)
  -h, --help             Print usage
      --normalized       generated normals are normalized (default: true)
      --noSeams          if enabled generation is slower but vertex located
                         on UV seams are properly used as one same vertex,
                         hence removing lighting seams. (default: true)

```


## Quantize 
```

Quantize model (mesh or point cloud)
Usage:
  mm quantize [OPTION...]

  -i, --inputModel arg        path to input model (obj or ply file)
  -o, --outputModel arg       path to output model (obj or ply file)
  -h, --help                  Print usage
      --dequantize            set to process dequantification at the ouput
      --qp arg                Geometry quantization bitdepth. A value < 7
                              means no quantization. (default: 12)
      --qt arg                UV coordinates quantization bitdepth.  A value
                              < 7 means no quantization. (default: 12)
      --qn arg                Normals quantization bitdepth. A value < 7 no
                              quantization. (default: 12)
      --qc arg                Colors quantization bitdepth. A value < 7 no
                              quantization. (default: 8)
      --minPos arg            min corner of vertex position bbox, a string of
                              three floats. Computed if not set.
      --maxPos arg            max corner of vertex position bbox, a string of
                              three floats. Computed if not set.
      --minUv arg             min corner of vertex texture coordinates bbox,
                              a string of three floats. Computed if not set.
      --maxUv arg             max corner of vertex texture coordinates bbox,
                              a string of three floats. Computed if not set.
      --minNrm arg            min corner of vertex normal bbox, a string of
                              three floats. Computed if not set.
      --maxNrm arg            max corner of vertex normal bbox, a string of
                              three floats. Computed if not set.
      --minCol arg            min corner of vertex color bbox, a string of
                              three floats. Computed if not set.
      --maxCol arg            max corner of vertex color bbox, a string of
                              three floats. Computed if not set.
      --outputVar arg         path to the output variables file.
      --useFixedPoint         internally convert the minPos and maxPos to
                              fixed point 16.
      --colorSpaceConversion  Convert color space from RGB to YUV.

```


## Reindex 
```

Reindex mesh and optionaly sort vertices and face indices
Usage:
  mm reindex [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -o, --outputModel arg  path to output model (obj or ply file)
  -h, --help             Print usage
      --sort arg         Sort method in none, vertices, oriented, unoriented.
                         (default: none)

```


## Render 
```

Render a mesh (todo points)
Usage:
  mm render [OPTION...]

  -i, --inputModel arg     path to input model (obj or ply file)
  -m, --inputMap arg       path to input texture map (png, jpg, rgb, yuv),
                           can be multiple paths surrounded by double quotes and
                           separated by spaces.
  -o, --outputImage arg    path to output image (png file) (default:
                           output.png)
      --outputDepth arg    path to output depth RGBA png file with 32bit
                           float span on the four components. If empty string will
                           not save depth (default behavior).
      --renderer arg       Use software or openGL 1.2 renderer. Value in
                           [sw_raster, gl12_raster]. (default: sw_raster)
      --hideProgress       hide progress display in console for use by robot
  -h, --help               Print usage
      --width arg          Output image width (default: 1980)
      --height arg         Output image height (default: 1024)
      --bilinear           Set --bilinear=false to disable bilinear filtering
                           (default: true)
      --viewDir arg        the view direction, a string of three floats
                           (default: 0.0 0.0 1.0)
      --viewUp arg         the view up vector, a string of three floats
                           (default: 0.0 1.0 0.0)
      --bboxMin arg        bbox min corner, a string of three floats. If not
                           both bboxMin and bboxMax set, will compute
                           automatically.
      --bboxMax arg        bbox max corner, a string of three floats. If not
                           both bboxMin and bboxMax set, will compute
                           automatically.
      --clearColor arg     background color, a string of four int8, each
                           component in [0,255]. (default: 0 0 0 0)
      --enableLighting     enable the lighting if model has normals.
      --autoLightPosition  enable light position computation from bounding
                           sphere radius and lightAutoDir parameter.
      --lightAutoDir arg   if autoLightPosition is set, will compute light
                           position as boundingSphereCenter + lightAutoDir *
                           boundingSphereRadius. Default value stands for top
                           right sector. (default: 1 1 1)
      --enableCulling      enable back face culling.
      --cwCulling          true sets Clock Wise (cw) orientation for face
                           culling, Counter Clock Wise (ccw) otherwise. (default:
                           true)

```


## Sample 
```

Convert mesh to point cloud
Usage:
  mm sample [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -m, --inputMap arg     path to input texture map (png, jpg, rgb, yuv), can
                         be multiple paths surrounded by double quotes and
                         separated by spaces.
  -o, --outputModel arg  path to output model (obj or ply file)
      --mode arg         the sampling mode in [face,grid,map,sdiv,ediv,prnd]
      --hideProgress     hide progress display in console for use by robot
      --outputCsv arg    filename of the file where per frame statistics will
                         append. (default: )
  -h, --help             Print usage

 ediv mode options:
      --lengthThreshold arg  edge length limit to stop subdivision, used only
                             if > 0, otherwise resolution is used. (default:
                             0.0)

 face and ediv modes options:
      --resolution arg  integer value in [1,maxuint], step/edgeLength =
                        resolution / size(largest bbox side). In ediv mode, the
                        resolution is used only if lengthThreshold=0. (default:
                        1024)

 face mode options:
      --float          if set the processings and outputs will be float32,
                       int32 otherwise (default: true)
      --thickness arg  floating point value, distance to border of the face
                       (default: 0.0)

 grid mode options:
      --gridSize arg   integer value in [1,maxint], side size of the grid
                       (default: 1024)
      --useNormal      if set will sample only in the direction with the
                       largest dot product with the triangle normal
      --minPos arg     min corner of vertex position bbox, a string of three
                       floats.
      --maxPos arg     max corner of vertex position bbox, a string of three
                       floats.
      --useFixedPoint  interprets minPos and maxPos inputs as fixed point 16.

 grid, face, sdiv and ediv modes. options:
      --bilinear           if set, texture filtering will be bilinear,
                           nearest otherwise
      --nbSamplesMin arg   if set different from 0, the system will rerun the
                           sampling multiple times to find the best parameter
                           producing a number of samples in [nbAmplesMin,
                           nbSamplesMax]. This process is very time comsuming.
                           (default: 0)
      --nbSamplesMax arg   see --nbSamplesMin documentation. Must be > to
                           --nbSamplesMin. (default: 0)
      --maxIterations arg  Maximum number of iterations in sample count
                           constrained sampling, i.e. when --nbSampleMin > 0.
                           (default: 10)

 prnd mode options:
      --nbSamples arg  integer value specifying the traget number of points
                       in the output point cloud (default: 2000000)

 sdiv mode options:
      --maxDepth arg       maximum recursion depth, maxDepth=1 means keep
                           only original vertices (default: 100)
      --areaThreshold arg  face area limit to stop subdivision (default: 1.0)
      --mapThreshold       if set will refine until face vertices texels are
                           distanced of 1 and areaThreshold reached

```


## Sequence 
```

Sequence global parameters
Usage:
  mm sequence [OPTION...]

      --firstFrame arg  Sets the first frame of the sequence, included.
                        (default: 0)
      --lastFrame arg   Sets the last frame of the sequence, included. Must
                        be >= to firstFrame. (default: 0)
  -h, --help            Print usage

```


# COPYRIGHT AND LICENSE

``` 
 The copyright in this software is being made available under the BSD
 License, included below. This software may be subject to other third party
 and contributor rights, including patent rights, and no such rights are
 granted under this license.

 Copyright (c) 2021, InterDigital
 Copyright (c) 2021-2025, ISO/IEC
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the copyright holder(s) nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
```

# 3rdParty softwares:

| name            | license | url                                                           |
|-----------------|---------|---------------------------------------------------------------|
|Eigen3           | MPL2    | http://eigen.tuxfamily.org/index.php?title=Main_Page#License  |
|Nanoflann        | BSD2    | https://github.com/jlblancoc/nanoflann                        |
|Tinyply          | MIT like| https://github.com/ddiakopoulos/tinyply                       |
|PCQM2            | MPL     | https://github.com/MEPP-team/PCQM/blob/master/LICENSE.md      |
|Stb_image        | MIT     | https://github.com/nothings/stb                               |
|Cxxopts          | MIT     | https://github.com/jarro2783/cxxopts                          |
|Glm              | MIT     | https://github.com/g-truc/glm/blob/master/copying.txt         |
|mpec_pcc_dmetric | MIT     | http://mpegx.int-evry.fr/software/MPEG/PCC/mpeg-pcc-dmetric   |
|glad             | MIT     | https://github.com/Dav1dde/glad                               |
|glfw             | ZLIB    | https://github.com/glfw/glfw                                  |
# References

```
@article{article,
	author = {Marvie, Jean-Eudes and Nehme, Yana and Graziosi, Danillo and Lavoué, Guillaume},
	year = {2023},
	month = {06},
	pages = {},
	title = {Crafting the MPEG metrics for objective and perceptual quality assessment of volumetric videos},
	volume = {8},
	journal = {Quality and User Experience},
	doi = {10.1007/s41233-023-00057-4}
}
```

# Feedback

For documented and repeatable bugs, feature requests, etc., please use the [Gitlab](https://git.mpeg.expert/MPEG/3dgh/v-pcc/software/mpeg-pcc-mmetric/-/issues) issues.
