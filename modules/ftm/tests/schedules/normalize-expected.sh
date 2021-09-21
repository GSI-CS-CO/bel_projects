sed -i -e 's/, fontname="Times-Bold", fontcolor = "blue2", fontsize="16"//g' $1
sed -i -e 's/flags="0x00020007"/flags="0x00000007"/g' $1
sed -i -e 's/flags="0x00022007"/flags="0x00002007"/g' $1
sed -i -e 's/flags="0x00128007"/flags="0x00108007"/g' $1
sed -i -e 's/flags="0x00120007"/flags="0x00100007"/g' $1
sed -i -e 's/fillcolor = "green"/fillcolor = "white"/g' $1
sed -i -e 's/flags="0x00100107"/flags="0x00100007"/g' $1
sed -i -e 's/flags="0x00708107"/flags="0x00708007"/g' $1
sed -i -e 's/flags="0x00002107"/flags="0x00002007"/g' $1
sed -i -e 's/flags="0x00000107"/flags="0x00000007"/g' $1
sed -i -e 's/flags="0x00108107"/flags="0x00108007"/g' $1
