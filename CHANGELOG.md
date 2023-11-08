
# Change log

Primary author Jean-Eudes Marvie (InterDigital).

Additional contributors mentioned per version or item hereafter.

## Version 1_1_4

- Add: eqTFAN lossless metric 
  - Contributor: Wenjie Zou (Xidian University)
- Fix: step size processing in meshToPcGrid (m65548)
- Fix: update mpeg dmetric path to new location in cmake file

## Version 1_1_3

- Fix: clip PSNR values to 100 (m62632)
  - the maximal PSNR value is set to 99.99
  - contributor: (Sharp)
- Fix: mmetric normal handling of duplicate vertices (m62542)
  - correction to the calculation of the normal of a point that has duplicate values
  - Contributor: Danillo Graziosi (Sony)
- Add: reordering of face indices in reindex (m62652)
  - face ordering may impact pcc metrics computations over sampled meshes
  - the proposed face ordering fix when reindexing could be explicitly triggered using additional options/flags
  - Contributor: Olivier Mocquard (InterDigital)

## Version 1_1_2

Contributor: Julien Ricard (Interdigital)

Add: Update dmetric and add corresponding parameters:

  - m61585: Update normals used for D2 metrics.
  - m61975: Add similar point threshold parameter to use 10e-20 value.

## Version 1.1.1

Contributor: Julien Ricard (Interdigital)

Add: Fix issues in remove duplicate

## Version 1.1.0

Contributor: Julien Ricard (Interdigital)

Add: separate core engine into library

  - new library architecture
  - update documentation
  - update clang-format command to allow cmake include
  - update dmetric tag (release-v0.14) and static compute function to clip values to 999.99
  - add format command and clang-format style
  - remove ./dependencies/pcc and clone dmetric mpeg repository
  - fix remove duplicate sort difference with gcc or clang and remove duplicate average normal computation
  - add verbose parameter to metric functions to disable logs during vmesh metric calculations
  - update getPSNR function to convert inf value by 999.99 like in CFP documents
  - update load ply to manage vmesh ply files
  - update loadPly form IO to manage vmesh ply files
  - add missing std:: in absolute function inside mmSample.cpp
  - add hausdorff distances in getFinalPccResults() function
  - update ltrim and rtrim functions for c++17
  - update glfw cmake and dependencies directory
  - add glfw fetch repository process

Fix issues reported in gitlab:
  - #1 Build requirements missing?
  - #2 Readme issues.
  - #3 Minor bugfixes in code.
  - #4 RGB / YUV MSE reporting.
  - #6 Typos in README.md.
  - #7 The values of c2p_psnr and haus_c2c_psnr are output to a csv file in the wrong order.
  - #8 Wrong ibsm mode YUV-psnr calculation.

## Version 1.0.1 - version to use for the V-Mesh CfP with corrigendum

- Fix: compiler instability on math calls,
    - add std:: prefix systematically on function invocation
    - checked for all ceil, floor, abs, pow, min and max
	- Contributors: Danillo Graziosi (Sony) and Jean-Eudes Marvie (InterDigital)
- Fix: test attibute size equality in compare eq

## Version 1.0.0 - (replaced by version 1.0.1 for the Cfp)

No modification, only the version number upgrade. 

## Version 0.1.13 - (replaced by version 1.0.1 for the Cfp)

- Add: compare IBSM clamp PSNR max to 999.99, 
	- prevent issues on Mean computations in case of infinite value for some frames
- Fix: remove unmatched pixels penalty (silhouette diffs and holes not emphasised any more)
	- modification performed on WG7 request following meeting 136
    - adding the penalty was leading to inconsistent results between subjective and objective results for some specific cases
	- TODO: shall be refined later to better correlate with perceptual test results
    - report as an extra statistic the ratio between unmatchedPixels and maskSizeSum (the smaller the better)
	- Contributor: Olivier Mocquard (InterDigital) 

## Version 0.1.12

- Add: boxRatio confidence in IBSM results
	- the nearest to 100% the more accurate.
	- also update the test suite that was not updated in previous commits
- Fix: color index issue in compare ibsm
	- Contributor: Danillo Graziosi (Sony)
- Add: allow users to rotate the IBSM camera rig
	- Contributor: Chao Huang et al. (Tencent)
- Fix: renderer use (0, 0, 1) as viewUp when the camera is located at north/south pole 
	- Contributor: Chao Huang et al. (Tencent) 
- Add: ibsm yuv and color utilities
- Fix: signal dynamic calculus in IBSM

## Version 0.1.11

- Add: ibsm automatic reindex and tests update.
- Add: Color Space Conversion for Qunatize/Dequantize functions
	- Contributor: Danillo Graziosi (Sony)
- Add: degrade --nbFaces option
- Add: sw renderer/ibsm metric optimization, gains 0.4sec on 4K renders.
- Fix: use max depth sw render value (fix frederick metric infinite issue).
- Fix: ibsm MSE and PSNR processing use mask Size instead of image size
- Fix: attribute names masking in compare. renamed class attributes with _ prefix.

## Version 0.1.10

- Add: some ibsm test output validation
- Add: image based sampling metric (compare --mode ibsm) and tests
- Add: software and hardware renderer
- Add: outputCsv option to compare pcc and pcqm
- Add: pcc hausdorff option test
- Add: meshlab projects in test folder
- Add: extention of grid sampling
  - add options to use sequence bounding box,
  - add useNormal option to filter sampling directions
  - Contributors: Danillo Graziosi (Sony), J.E. Marvie (InterDigital)
- Add: pseudo random sampling (sample --mode prand)
  - Contributors: Jungsun Kim (Apple), Khaled Mammou (Apple).
    - todo: add support for color per vertex
  - unit tests added by jean-eudes marvie (InterDigital)
- Fix: frSize overflow in _loadImageFromVideo
- Fix: nearest and bilinear filtering
- Fix: renaming renderer and compare ibsm parameters
  - rename "compare --mode raster" as "compare --mode ibsm"
    - ibsm: Image Based Sampling Metric
  - rename "render --mode" into "render --renderer"
  - realigned parameter names from on command to the other
  - please read carefully the documentation to update your scripts
  
## Version 0.1.9

- Add: "topo" compare mode for near-lossless metric with vertex and triangle mapping
  - Contributor: Danillo Graziosi (Sony)
- Add: Sample outputCsv and target nbSamples
- Add: add irregular plane sampling test plus meshlab
- Add: update test suite (no more need of extdata)
- Fix: obj printer does not print trianglesuv table if any
- Fix: implement proper Image destructor
- Fix: Update cmakelists.txt for compiling on MacOS/CLang
- Fix: sequence quantization auto box not computed for other frames than first one
- Fix: pcc summary processing wrongly using MSE instead of PSNR
- Fix: statistics computation with 1 sample erroneous

## Version 0.1.8

- add output details to compare --mode equ if --earlyReturn not set
- add texture map comparison in compare --mode equ
- add near lossless topology check in compare --mode pcc and --mode pcqm
- add Video input implementation
- fix remove quantization post cleanup step (was introducting defects: manifold, holes)
- fix color quantize/dequantize using qn instead of qc
- fix Update compare documentation, inputA is the reference
- fix loadOBJ function, issue with empty lines

Video input was implemented by Danillo Graziosi (Sony).
Several stabilizations were also proposed by Danillo.

## Version 0.1.7

- add sample ediv mode, subdivision by edge length 
	- Contributors: J.E. Marvie (InterDigital), Jungsun Kim (Apple), Khaled Mammou (Apple).
- add large memory buffers for faster model read/writes
- add normal command to generate mesh per-vertex normals, 
	- fast mode similar to open3D and MeshLab approach
	- and slower noseams mode for better quality
- add statistics computation of Minkowsky "mean" for ms=3
- fix issues with loading obj models having separate normal index table (skipped)
- fix issues on error handling

## Version 0.1.6

Contributors: Danillo Graziosi (Sony), Ali Tabatabai (Sony)

- add fixed point option to position quantize/dequantize
- add post cleanup to quantize command to remove degenerate triangles (T-vertex removal not yet implemented) 
- add sum in statistics (be aware of potential overflow for many sample and large sample values)
- fix bounding box update in quantize method so --dequantize option works correctly when bbox are processed internally.
- fix set cout floating point precision identical to log file precision
- fix add #include <algorithm>, to mmGeometry.h to remove build issue on some platforms

## Version 0.1.5

- add quantize/dequantize for qp, qt, qn, qc
- add analyse command to get stats on frame and sequences
- fix command registration system

## Version 0.1.4

- add remove duplicates for all sample modes (for exact vertex equality only)
- add degrade command for metric experiments, remove one face every n faces
- fix compare pcc, set bAverageNormals to true by default
- fix compare pcqm, invert ref and deg as in original source code 
- fix compare pcqm, bbox computation in model convert
- fix reindex, issues with model normal API not complete
- fix 3rdparty pcc, change distance threshold from 1e-8 to 1e-20, leads to self compare = Inf

## Version 0.1.3

Update license to Apache 2.0

- Change compare pcqm and pcc default sampling to sdiv
- add reindex command, tests and doc
- add eq comparison method for meshes with topology reordering
- add plane reorder and shift test
- add documentation
- add model builder and generic compare
- add statistics std dev and variance computation for pcqm and pcc
- add support for RGBA input textures
- add filename templates, model cache cleanup
- add sequence command
- add multi frame system
- pcqm use similar maxEnergy as for pcc.
- fix sample sdiv remove duplicates
- fix normal not generated in subdiv sdiv mode
- fix subdiv default areaThreshold type

## Version 0.1.2

- add and use model store for all commands
- add multiple command system
- add doc folder and readme base file
- add root test script
- add change log
- add sample sdiv map threshold option
- fix pcqm bug: if no input color use white
- fix in compare pcqm color not properly set

## Version 0.1.1

- add cmake
- and build.sh script
- add clean.sh script

Compare pcc
- add default options,
- add duplicate points handling
- fix pcc point cloud init,
- fix printers to preserve highest precision
- fix several issues in compare pcc
- update references

Compare pcqm
- compare pcqm set default RadiusCurvature to 0.01 to reduce process time

Sampling
- add subdivision sampling (sdiv)
- sdiv use vertex texels adjacancy criterion
- add mapCoord function

Other fixes
- texture sampling fix texture shift
- ply loader fix loading uvcoordinates
- ply/obj writers use full float dynamic

## Version 0.1.0

- add sampling map, face, grid,
- add compare eq, pcc, pcqm
- add quantize (Contributor: Yannick Olivier - InterDigital)
