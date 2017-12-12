#!/bin/bash
# Script  Flashaufruf.sh
# Verfasser: Karlheinz Kaiser    13.Januar 2017


echo    ""
echo    "Flashaufruf.sh flasht alle im Script eingetragenen SCU Slaves     "
echo    ""
echo    "Folgende Files werden hierfür im gleichen Directory benötigt      "
echo    "   addac.rpd  Programmierfile für die ADDAC (CID Grp:x3)          "
echo    "   addac2.rpd Programmierfile für die ADAC2 (CID Grp:x26)         "
echo    "   diob.rpd   Programmierfile für die DIOB  (CID Grp:x1A)         "
echo    "   eb_slave_flash_Bash.sh und natürlich Flashaufruf.sh            "
echo    ""  
echo    "Das Script erzeugt ein Log/Error file >prgmresults*.txt<          "
echo    "Vor dem Programmieren wird die SCU angepingt.                     "
echo    "Script stoppt nicht bei abgeschaltete SCUen oder Etherbone Errors."
echo    "Script wurde für voll-und teilbestückte 12er Crates getestet.     "
echo    "Die Crates werden gleichzeitig programmiert (Prozesse laufen bg). "
echo    ""
echo    "Folgende CRYRING SCU Slaves werden programmiert                   "
echo    ""
echo    "scuxl0005"
echo    "scuxl0006"
echo    "scuxl0013"
echo    "scuxl0024"
echo    "scuxl0027"
echo    "scuxl0028"
echo    "scuxl0039"
echo    "scuxl0056"
echo    "scuxl0057"
echo    "scuxl0070"
echo    "scuxl0075"
echo    "scuxl0076"
echo    "scuxl0086"
echo    "scuxl0113"
echo    "scuxl0118"
echo    "scuxl0122"
echo    "scuxl0127"
echo    "scuxl0136"
echo    "scuxl0145"
echo    ""


abfrage=""
read -p "Weiter mit >ja<, Abbruch bei allen anderen Eingaben: " abfrage
if [[  $abfrage != "ja"  ]] ; then
 exit
fi


./eb_sflash_DIOB_Bash.sh       "scuxl0003" & # not a CRYRING SCU

#./eb_slave_flash_Bash.sh      "scuxl0005" &
#./eb_slave_flash_Bash.sh      "scuxl0006" &
#./eb_slave_flash_Bash.sh      "scuxl0013" &
#./eb_slave_flash_Bash.sh      "scuxl0024" &
#./eb_slave_flash_Bash.sh      "scuxl0027" &
#./eb_slave_flash_Bash.sh      "scuxl0028" &
#./eb_slave_flash_Bash.sh      "scuxl0039" &
#./eb_slave_flash_Bash.sh      "scuxl0056" &
#./eb_slave_flash_Bash.sh      "scuxl0057" &
#./eb_slave_flash_Bash.sh      "scuxl0070" &
#./eb_slave_flash_Bash.sh      "scuxl0075" &
#./eb_slave_flash_Bash.sh      "scuxl0076" &
#./eb_slave_flash_Bash.sh      "scuxl0086" &
#./eb_slave_flash_Bash.sh      "scuxl0113" &
#./eb_slave_flash_Bash.sh      "scuxl0118" &
#./eb_slave_flash_Bash.sh      "scuxl0122" &
#./eb_slave_flash_Bash.sh      "scuxl0127" &
#./eb_slave_flash_Bash.sh      "scuxl0136" &
#./eb_slave_flash_Bash.sh      "scuxl0145" &



echo ""
echo "From Flashaufruf.sh: Programmierscripts aufgerufen"
echo ""

# Dieser Wait wartet bis der letzte Subprozess abgeschlossen ist
wait

# Zusammenfassen der Ergebnisfiles
cat           \
scuxl0003.txt \
#scuxl0005.txt \
#scuxl0006.txt \
#scuxl0013.txt \
#scuxl0024.txt \
#scuxl0027.txt \
#scuxl0028.txt \
#scuxl0039.txt \
#scuxl0056.txt \
#scuxl0057.txt \
#scuxl0070.txt \
#scuxl0075.txt \
#scuxl0076.txt \
#scuxl0086.txt \
#scuxl0113.txt \
#scuxl0118.txt \
#scuxl0122.txt \
#scuxl0127.txt \
#scuxl0136.txt \
#scuxl0145.txt \
> prgmresults_"$(date +%Y-%m-%d_%Hh%Mm%Ss)".txt 2>/dev/null

#rm  scuxl*.txt

echo "From Flashaufruf.sh: Ergebnisse stehen im File prgmresults*.txt"
echo "From Flashaufruf.sh: Programmierscripts beendet"

wait
ls


