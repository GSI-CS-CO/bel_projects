#!/bin/bash
neato -n -Tpdf -o ${1//.dot/.pdf1} $1
gs -o ${1//.dot/.pdf} -sDEVICE=pdfwrite -dColorConversionStrategy=/sRGB -dProcessColorModel=/DeviceRGB ${1//.dot/.pdf1}
rm ${1//.dot/.pdf1}
