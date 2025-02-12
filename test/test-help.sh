#!/bin/bash

source config.sh

$CMD            > ${TMP}/helpMain.txt 	 2>&1
cmpOsLog helpMain

$CMD sample     > ${TMP}/helpSample.txt	 2>&1
cmpOsLog helpSample

$CMD render     > ${TMP}/helpRender.txt	 2>&1
cmpOsLog helpRender

$CMD degrade     > ${TMP}/helpDegrade.txt 2>&1
cmpOsLog helpDegrade

$CMD compare   > ${TMP}/helpCompare.txt 2>&1
cmpOsLog helpCompare

$CMD reindex   > ${TMP}/helpReindex.txt 2>&1
cmpOsLog helpReindex

$CMD quantize   > ${TMP}/helpQuantize.txt 2>&1
cmpOsLog helpQuantize

$CMD dequantize   > ${TMP}/helpDeQuantize.txt 2>&1
cmpOsLog helpDeQuantize

$CMD sequence   > ${TMP}/helpSequence.txt 2>&1
cmpOsLog helpSequence

$CMD badcmd     > ${TMP}/helpBadCmd.txt 2>&1
cmpOsLog helpBadCmd
