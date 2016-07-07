#!/bin/bash
let arg_num="$#"+0
if [ $arg_num -lt 2 ]; then
	echo ""
	echo "Error: we didn't receive a minimum of 2 arguments as input!!!"
	echo "USAGE: BioFormats2TissueStackConverter input.format output.raw [optional: temporary dir] [optional: avoid 3D reconstr. (false/true)]"
	echo ""
	exit -1
fi

#for cvl
source /opt/tissuestack/conf/tissuestack_modules.sh 2> /dev/null
source $TISSUE_STACK_ENV 2> /dev/null

java -Xmx1024m -cp $TISSUE_STACK_JARS/bioformats-5.1.1.jar:$TISSUE_STACK_JARS/TissueStackBioFormatsConverter.jar au.edu.cai.TissueStackBioFormatsConverter $1 $2 $3 $4
