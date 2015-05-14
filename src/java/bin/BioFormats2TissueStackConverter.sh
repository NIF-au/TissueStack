#!/bin/bash
let arg_num="$#"+0
if [ $arg_num -lt 2 ]; then
	echo ""
	echo "Error: we didn't receive a minimum of 2 arguments as input!!!"
	echo "USAGE: BioFormats2TissueStackConverter input.format output.raw [temp/dir]"
	echo ""
	exit -1
fi
java -Xmx1024m -cp bioformats-5.1.1.jar:TissueStackBioFormatsConverter.jar au.edu.cai.TissueStackBioFormatsConverter $1 $2 $3
