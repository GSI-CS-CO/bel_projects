#!/bin/bash
if [ $(grep 'pos=' $1) ] 
then
  neato -n -Tpdf -o ${1//.dot/.pdf1} $1
else
  dot -Tpdf -o ${1//.dot/.pdf1} $1
#  neato -Tpdf -o ${1//.dot/.pdf1} $1
fi
gs -o ${1//.dot/.pdf} -sDEVICE=pdfwrite -dColorConversionStrategy=/sRGB -dProcessColorModel=/DeviceRGB ${1//.dot/.pdf1}
rm ${1//.dot/.pdf1}
