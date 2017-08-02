#!/bin/bash
# Script  eb_slave_flash_bash.sh


# $1 ist der erste Übergabeparameter des Scriptaufrufs

debugmode=off
if [ "$debugmode" = "on" ] ; then  echo "Printmode = on im File eb_slave_flash_bash";  fi;
if [ "$debugmode" = "on" ] ; then  echo "------------------------------------------";  fi;

scuname=$1

RESULT="1" # when pingcheck on "## received"  is successful
PING=$(ping $scuname.acc -c1 | grep -E -o '[0-9]+ received'|cut -f1 -d' ' )

if [ "$RESULT" = "$PING" ] ; then
  echo "SCU=" $scuname " Ping successful"
  scuslave_baseadr=$(eb-find tcp/$scuname.acc 0x651 0x9602eb6f)
  if [ "$debugmode" = "on" ] ; then echo "SCU="  $scuname  "scuslave_baseadr  =" ${scuslave_baseadr} ;  fi;

  if [[ -z "$scuslave_baseadr" ]]; then # on zero response
   echo "SCU=" $scuname " SCU pingable, but etherbone problem"
   echo "SCU=" $scuname " SCU pingable, but etherbone problem" >$scuname.txt
  fi
  
  
  
###############################################################################################################
# Commented Section works with CID System and CID Group from SCU Slave Standard Registers
# For all SCU Slaves which have Slave User Prom, it is strongly recommended to use Information from there.
#   #calculate scu slave addresses for CID System and CID Group
#   i=1
#   while [ $i -lt 13 ]; do
#   
#     calculatedslotaddress="$((10#$(($scuslave_baseadr+0x20000*i+0x8)) ))"
#     slaveadresse0[i]="0x$( printf "%X\n" $calculatedslotaddress )"
#     #echo "calculated slot address CID System  =slaveaddresse0  =" ${slaveadresse0[i]} " i=" $i
#   
#     calculatedslotaddress="$((10#$(($scuslave_baseadr+0x20000*i+0xa)) ))"
#     slaveadresse1[i]="0x$( printf "%X\n" $calculatedslotaddress )"
#     #echo "calculated slot address CID Group   =slaveaddresse1  =" ${slaveadresse1[i]} " i=" $i
#   
#     i=$(($i + 1))
#   done
# 
# 
#   # Get Values for CID System and CID Group ( only occupied slots echoes with values)
#   i=1
#   while [ $i -lt 13 ]; do
#     lesewert1[i]=$(eb-read  tcp/$scuname.acc   ${slaveadresse0[i]}/2 2>/dev/null)
#     lesewert2[i]=$(eb-read  tcp/$scuname.acc   ${slaveadresse1[i]}/2 2>/dev/null)
#     i=$(($i + 1))
#   done
# 
# 
#   # Assign Program file names to Slave Slots. Program Files of same name must be in working directory.
#   i=1
#   while [ $i -lt 13 ]; do
#     if [ -n "${lesewert1[i]}" ]; then
#       #echo    "CID System Registeradr.  Slave" $i " = "${slaveadresse0[i]}/2
#       #echo    "CID System           SCU Slave" $i " = "${lesewert1[i]}
#       #echo    "CID Group            SCU Slave" $i " = "${lesewert2[i]}
# 
#     if [ "${lesewert1[i]}" = "0037" ] && [ "${lesewert2[i]}" = "001a" ]; then
#       Progfile[i]="diob.rpd"
#     fi
# 
#     if [ "${lesewert1[i]}" = "0037" ] && [ "${lesewert2[i]}" = "0026" ]; then
#       Progfile[i]="addac2.rpd"
#     fi
#     #echo "----------------------------------------------------------------------"
#   fi
#   i=$(($i + 1))
#   done
###############################################################################################################



i=0
while [ $i -lt 13 ]; do

screendump=""
stripped_screendump=""

screendump=$(eb-info -s$i tcp/$scuname.acc  2> /dev/null)
stripped_screendump=$(echo $screendump | sed 's/[[:space:]]//g' )

     if    [[ "$stripped_screendump" = *"scu_diobDIOBArriaIIGX"*       ]]; then
       if [ "$debugmode" = "on" ] ; then echo "SCU="$scuname " DIOB found";  fi;
       Progfile[i]="diob.rpd"
     fi
        

i=$(($i + 1))
done




  #Display program files found for each slave slot
  echo "--------------------------------------------------------"
  i=1
  while [ $i -lt 13 ]; do
    if  [[ ! -z ${Progfile[i]} ]] ; then echo "SCU=" $scuname "Slave="$i " " ${Progfile[i]}"-Progfile found" ; fi;
    #echo "SCU=" $scuname " slave="$i " Progfile="${Progfile[i]}
    i=$(($i + 1))
  done
  echo "--------------------------------------------------------"


  # Programming the SCU Slaves starts here
  i=1
  while [ $i -lt 13 ]; do
    # if  [[ ! -z ${Progfile[i]} ]] ; then echo "SCU=" $scuname "Slave="$i " Progfile="${Progfile[i]} ; fi;

    if    [ "${Progfile[i]}" = "diob.rpd"   ]; then
    
       if [ "$debugmode" = "on" ] ; then echo "SCU "$scuname"dummy programming for diob for slave "$i" would take place here"; fi;
       if [ "$debugmode" = "on" ] ; then echo "SCU "$scuname"dummy programming for diob for slave "$i" successful">>$scuname.txt; fi
       echo ""  



       screendump=""
       screendump=$(eb-sflash tcp/$scuname.acc -s$i -dw ./diob.rpd 3>&1 2>&1)
 
       if  [[ "$screendump" == *"New image written to epcs"* ]] ; then
         echo "Flashing DIOB Slave "$i" was successful"
         echo "SCU=" $scuname " Progfile="${Progfile[i]} "   Flashing DIOB Slave "$i " successful" >>$scuname.txt
       else
         echo "Flashing DIOB Slave "$i" failed See next line: "
         echo "SCU=" $scuname " Progfile="${Progfile[i]} "   Flashing DIOB Slave "$i " failed"     >>$scuname.txt
         echo $screendump
       fi;
 


    else
      echo " "
      #echo "SCU=" $scuname " No flash due to slave missed on slave="$i
    fi

    #echo "--------------------------------------------------------"
    i=$(($i + 1))
  done



  echo "SCU=" $scuname " finished"      >>$scuname.txt

else # Ping not successful

  echo "SCU=" $scuname " not reachable"
  echo "SCU=" $scuname " not reachable" >>$scuname.txt

fi

echo "##################################################################################"


#end of script



