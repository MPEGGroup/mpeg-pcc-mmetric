
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

