#!/bin/bash
if (($(grep -c 'pos=' "$1") > 0))
then
  echo "neato with positions, arguments: $1 $2"
  neato -n -Tpdf -o ${1//.dot/.pdf1} $1
else
  if [ "" = "$2" ]
  then
    echo "dot without positions, arguments: $1 '$2'"
    dot -Tpdf -o ${1//.dot/.pdf1} $1
  elif [ "neato" = "$2" ]
  then
    echo "neato without positions, arguments: $1 $2"
    neato -Goverlap=compress -Gmodel=subset -Gsep=+0.5 -Tpdf -o ${1//.dot/.pdf1} $1
  else
    echo "$2 without positions, arguments: $1 $2"
    $2 -Tpdf -o ${1//.dot/.pdf1} $1
  fi
fi
gs -o ${1//.dot/.pdf} -sDEVICE=pdfwrite -dColorConversionStrategy=/sRGB -dProcessColorModel=/DeviceRGB ${1//.dot/.pdf1}
rm ${1//.dot/.pdf1}
