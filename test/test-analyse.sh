#!/bin/bash

source config.sh

# test single frame
OUT=analyse_basketball_player_1frame
echo $OUT
$CMD analyse --outputCsv ${TMP}/${OUT}.csv --outputVar ${TMP}/${OUT}_var.txt \
	--inputModel ${DATA}/basketball_player_00000001.obj \
	--inputMap ${DATA}/basketball_player_00000001.png END \
	> ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
diff -a ${TMP}/${OUT}.csv ${REFS}/${OUT}.csv
diff -a ${TMP}/${OUT}_var.txt ${REFS}/${OUT}_var.txt

# test single frame cpv
OUT=analyse_basketball_player_1frame_cpv
echo $OUT
$CMD analyse --outputCsv ${TMP}/${OUT}.csv --outputVar ${TMP}/${OUT}_var.txt \
	--inputModel ${DATA}/cpv_basketball_player_00000001.ply END \
	> ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
diff -a ${TMP}/${OUT}.csv ${REFS}/${OUT}.csv
diff -a ${TMP}/${OUT}_var.txt ${REFS}/${OUT}_var.txt

# test invalid model filename
OUT=analyse_invalid_model_filename
echo $OUT
$CMD analyse  \
	--inputModel /path/to/file/invalidfilenamenoextension END \
	> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "Error, invalid model filename extension (not in obj, ply)" 1

# test invalid texture map filename
OUT=analyse_invalid_map_filename
echo $OUT
$CMD analyse  \
	--inputModel ${DATA}/basketball_player_00000001.obj \
	--inputMap /path/to/file/invalidfilenamenoextension END \
	> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "Error: missing map filename extension" 1

# test sequence mode
OUT=analyse_basketball_player_3frames
echo $OUT
$CMD sequence --firstFrame 1 --lastFrame 3 END \
	analyse --outputCsv ${TMP}/${OUT}.csv --outputVar ${TMP}/${OUT}_var.txt \
	--inputModel ${DATA}/basketball_player_0000000%1d.obj \
	--inputMap ${DATA}/basketball_player_0000000%1d.png END \
	> ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
diff -a ${TMP}/${OUT}.csv ${REFS}/${OUT}.csv
diff -a ${TMP}/${OUT}_var.txt ${REFS}/${OUT}_var.txt
