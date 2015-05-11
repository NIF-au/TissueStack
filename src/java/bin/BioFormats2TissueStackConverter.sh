#!/bin/bash
if [ "$#" -ne 2 ]; then
	echo ""
	echo "Error: we didn't receive 2 arguments as input!!!"
	echo "USAGE: BioFormats2TissueStackConverter input.format output.raw"
	echo ""
	exit -1
fi
java -Xmx2512m -cp bioformats-5.1.1.jar:TissueStackBioFormatsConverter.jar au.edu.cai.TissueStackBioFormatsConverter $1 $2
