#!/bin/bash
if [ "" = "$2" ]
then
  echo "dot without positions, arguments: $1 '$2'"
  dot -Tpdf -o ${1//.dot/.pdf1} $1
else
  echo "$2 without positions, arguments: $1 $2"
  $2 -Tpdf -o ${1//.dot/.pdf1} $1
fi
gs -o ${1//.dot/.pdf} -sDEVICE=pdfwrite -dColorConversionStrategy=/sRGB -dProcessColorModel=/DeviceRGB ${1//.dot/.pdf1}
rm ${1//.dot/.pdf1}
