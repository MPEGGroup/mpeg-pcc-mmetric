#!/bin/bash

# hardware renderer will not provide metric results matching exactly software results
# hence we only test software results stability as reference

source config.sh

STATS=${TMP}/compare_ibsm.csv
# reset csv stats file
> ${STATS}

for renderer in sw_raster gl12_raster
do

	# compare ib
	OUT=compare_ibsm_${renderer}_plane_self
	if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
			--inputMapA ${DATA}/plane.png --inputMapB ${DATA}/plane.png --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 99.99" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 99.99" 1
		fi
	fi

	# no map, no color
	OUT=compare_ibsm_${renderer}_sphere_qp8
	if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} --inputModelA ${DATA}/sphere.obj --inputModelB ${DATA}/sphere_qp8.obj --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 99.99" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 99.99" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 58.8384282" 1
		fi
	fi

	####
	# extended tests

	OUT=compare_ibsm_${renderer}_basket_self
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ${DATA}/basketball_player_00000001.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 99.99" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 99.99" 1
		fi
	fi

	# internal reordering of model A shall lead to identical meshes
	OUT=compare_ibsm_${renderer}_basket_reordered
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD \
			reindex --sort oriented --inputModel ${DATA}/basketball_player_00000001.obj --outputModel ID:reordered END \
			compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ID:reordered --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 99.99" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 99.99" 1
		fi
	fi
	
	# internal reordering disabled shall lead to numerical differences
	OUT=compare_ibsm_${renderer}_basket_reordering_disabled
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD \
			reindex --sort oriented --inputModel ${DATA}/basketball_player_00000001.obj --outputModel ID:reordered END \
			compare --mode ibsm --ibsmDisableReordering --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ID:reordered --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 86.8614937" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 89.9344996" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 99.99" 1
		fi
	fi
	
	OUT=compare_ibsm_${renderer}_basket_qp8_self
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 99.99" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 99.99" 1
		fi
	fi

	OUT=compare_ibsm_${renderer}_basket_qp8
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 28.914318" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 30.5168603" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 42.1189127" 1
		fi
	fi

	OUT=compare_ibsm_${renderer}_basket_qp8_hole
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ${DATA}/basketball_player_00000001_qp8_hole.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 28.8013856" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 30.4007669" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 42.1192512" 1
		fi
	fi

	OUT=compare_ibsm_${renderer}_basket_qp16_nomap
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA "" \
			--inputModelB ${TMPDATA}/basketball_player_00000001_qp16.obj --inputMapB "" \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 99.99" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 99.99" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 65.7603989" 1
		fi
	fi

	OUT=compare_ibsm_${renderer}_basket_qp16_seq
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		# to dump the color buffers:
		#   uncomment next line to prepare output fodler
		#   mkdir -p ${TMP}/cmpIbsm
		#   And add the following line to the compare command
		#   --ibsmOutputPrefix ${TMP}/cmpIbsm/$OUT \
		echo $OUT
		$CMD sequence --firstFrame 1 --lastFrame 3 END \
			compare --mode ibsm --ibsmRenderer ${renderer}  \
			--inputModelA ${DATA}/basketball_player_0000000%1d.obj \
			--inputMapA  ${DATA}/basketball_player_0000000%1d.png \
			--inputModelB ${TMPDATA}/basketball_player_0000000%1d_qp16.obj \
			--inputMapB  ${DATA}/basketball_player_0000000%1d.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 3
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "BoxRatio Min=99.9997664"  1
			fileHasString ${TMP}/${OUT}.txt "BoxRatio Max=100"         1 
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR Mean=57.0410227" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR Mean=66.1271437" 1
			fileHasString ${TMP}/${OUT}.txt "Y   PSNR Mean=57.5864303" 1
			fileHasString ${TMP}/${OUT}.txt "U   PSNR Mean=68.0485078" 1
			fileHasString ${TMP}/${OUT}.txt "V   PSNR Mean=68.9786194" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR Mean=58.7193421" 1
		fi
	fi
done

# EOF