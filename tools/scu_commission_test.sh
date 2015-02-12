#!/bin/bash
# Functional Test SCU3, created 2015-02-10 by Karlheinz Kaiser (k.kaiser@gsi.de)

clear
echo "-------------------Funktionstest Visatronic Fertigungslos SCU3--------------------"
echo "--------------       Abbruch des Scripts mit CTRL-Z        -----------------------"
echo "--Testaufbau vorzugsweise: Voller FG900.901 mit 12 Slaves und SCU mit MIL Option--"

whotest[0]='test' || (echo 'Failure: arrays not supported in this version of
bash.' && exit 2)


# In diesem Script verwendete Adressen

slaveadresse[1]='0x420020'
slaveadresse[2]='0x440020'
slaveadresse[3]='0x460020'
slaveadresse[4]='0x480020'
slaveadresse[5]='0x4a0020'
slaveadresse[6]='0x4c0020'
slaveadresse[7]='0x4e0020'
slaveadresse[8]='0x500020'
slaveadresse[9]='0x520020'
slaveadresse[10]='0x540020'
slaveadresse[11]='0x560020'
slaveadresse[12]='0x580020'

master_global_irq='0x400004'
master_en_slave_srq='0x40000c'
master_srq_active='0x400010'

oled_ctrl_adresse='0x80000'
oled_data_adresse='0x90000'

mil_option_ctrl_adresse='0x8004'
mil_option_data_adresse='0x8000'

Onewire_Carrierboard_MSW_Adr='0x1c0100'
Onewire_Carrierboard_LSW_Adr='0x1c0104'

Onewire_Extensionboard_MSW_Adr='0x1c0110'
Onewire_Extensionboard_LSW_Adr='0x1c0114'
Onewire_Backplane_MSW_Adr='0x1c0120'
Onewire_Backplane_LSW_Adr='0x1c0124'

# Fehlerflags für Zusammenfassung Testergebnisse

SCU_Verbindungstest_ebinfo="Fehlerhaft"
WhiteRabbit_Sekundenpuls="Fehlerhaft"
wr_trackingphase_test="Fehlerhaft"
Interfacekartentest="Fehlerhaft"
slavebustest_1[1]="Fehlerhaft"
slavebustest_1[2]="Fehlerhaft"
slavebustest_1[3]="Fehlerhaft"
slavebustest_1[4]="Fehlerhaft"
slavebustest_1[5]="Fehlerhaft"
slavebustest_1[6]="Fehlerhaft"
slavebustest_1[7]="Fehlerhaft"
slavebustest_1[8]="Fehlerhaft"
slavebustest_1[9]="Fehlerhaft"
slavebustest_1[10]="Fehlerhaft"
slavebustest_1[11]="Fehlerhaft"
slavebustest_1[12]="Fehlerhaft"

slavebustest_2[1]="Fehlerhaft"
slavebustest_2[2]="Fehlerhaft"
slavebustest_2[3]="Fehlerhaft"
slavebustest_2[4]="Fehlerhaft"
slavebustest_2[5]="Fehlerhaft"
slavebustest_2[6]="Fehlerhaft"
slavebustest_2[7]="Fehlerhaft"
slavebustest_2[8]="Fehlerhaft"
slavebustest_2[9]="Fehlerhaft"
slavebustest_2[10]="Fehlerhaft"
slavebustest_2[11]="Fehlerhaft"
slavebustest_2[12]="Fehlerhaft"
slavebustest_irq="Fehlerhaft"

displaytest="Fehlerhaft"
onewiretest="Fehlerhaft"
temperaturanzeigen="Fehlerhaft"
usb_schnittstellen="Fehlerhaft"
lemo_buchse_b1="Fehlerhaft"
board_reset="Fehlerhaft"

# Start der Testroutinen

eingabefehler=j
while eingabefehler=j 
 do
  read -p "Geben Sie die SCU Nummer ein (z.B. 0022 für scuxl0022):" scuname
  echo ""
  echo -e "Der verwendete Name ist\e[7m" scuxl$scuname "\e[0m"
  echo ""
  read -p  "Ist der Name richtig ( j / n ):" eingabefehler 
   if [ "$eingabefehler" == "j" ] 
    then
     break
    else
     echo -e "\e[7mSie können den Namen nun aendern\e[0m"
   fi
done



# Loesche alte File Leichen mit dem gleichen SCU Rechnernamen
file="1W_scuxl$scuname.txt"
[[ -f "$file" ]] && rm -f "$file"


file="scuxl$scuname.txt"
[[ -f "$file" ]] && rm -f "$file"

echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo "Funktionaltest SCU : "                      >> ./scuxl$scuname.txt
echo "Durchgefuehrt am "`date +%Y-%m-%d:%H:%M:%S` >> ./scuxl$scuname.txt
echo "Filename: " $file                           >> ./scuxl$scuname.txt
echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
clear
echo "----------------------------Ping Check auf SCU (4 Pings)--------------------------"
ping scuxl$scuname.acc -c 4
ping scuxl$scuname.acc -c 4                       >> ./scuxl$scuname.txt
echo ""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear









echo "--------------------------------eb-info ------------------------------------------"
echo "--------------------------------eb-info ------------------------------------------">> ./scuxl$scuname.txt
eb-info tcp/scuxl$scuname.acc
echo ""
sleep 1s
eb-info tcp/scuxl$scuname.acc >> ./scuxl$scuname.txt
echo ""
echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt


read -p "Sind Sie mit dem richtigen SCU Typ - z.B. SCU2 - verbunden ( j / n ): " scu_connected
  if [ "$scu_connected" == "j" ] 
  then
    echo "Erfolgreich Ethernetverbindung und eb-info"
    echo "Erfolgreich Ethernetverbindung und eb-info" `date +%Y-%m-%d:%H:%M:%S`          >> ./scuxl$scuname.txt
    SCU_Verbindungstest_ebinfo="Erfolgreich"


  else
    echo "Fehlerhaft:Ethernet Verbindung zur SCU und eb-info" scuxl$scuname
    echo "Fehlerhaft:Ethernet Verbindung zur SCU und eb-info" `date +%Y-%m-%d:%H:%M:%S`  >> ./scuxl$scuname.txt
  fi
echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear




echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo "----------- eb-ls Test durch Herstellen einer SCU Verbindung----------------------"

eb-ls tcp/scuxl$scuname.acc
echo "Erfolgreich: eb-ls Test :" `date +%Y-%m-%d:%H:%M:%S`                               >> ./scuxl$scuname.txt
eb-ls tcp/scuxl$scuname.acc                                                              >> ./scuxl$scuname.txt

echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear



echo "-------------White Rabbit Sekundenpuls--------------------------------------------"

echo "Erwartete Reaktionen: Sekundenpuls an blauer LED U2, SFP Activity und SFP Link LEDs sind aktiv"
echo ""
read -p "Ist WhiteRabbit synchron und zeigen WR LEDs die erwartete Reaktion ( j / n ):" wr_pulsed
if [ "$wr_pulsed" == "j" ] 
  then
    echo "Erfolgreich: WR LED U2 zeigt  Sekundenpuls, WR Link ok"
    echo "Erfolgreich: WR LED U2 zeigt  Sekundenpuls, WR Link ok" `date +%Y-%m-%d:%H:%M:%S`                 >>./scuxl$scuname.txt
    WhiteRabbit_Sekundenpuls="Erfolgreich"
  else
    echo "Fehlerhaft: WR LED U2 zeigt keinen Sekundenpuls"  
    echo "Fehlerhaft: WR LED U2 zeigt keinen Sekundenpuls" `date +%Y-%m-%d:%H:%M:%S`     >>./scuxl$scuname.txt
  fi
echo "----------------------------------------------------------------------------------">>./scuxl$scuname.txt
clear


echo "----------------White Rabbit Ausgabe auf RS232 Console COM1------------------------"
echo "COM1 ist z.B. mit TeraTerm Console verbunden(115200Bd/8bit/NoPar/No HW FlowControl)"
echo -e "Die Console zeigt die Eingabeaufforderung \e[7mwrc#\e[0m als Prompt"
echo ""
echo -e "Geben Sie folgenden Befehl auf der \e[7mCONSOLE\e[0m ein:  \e[7mgui\e[0m"
echo ""



read -p "Wird  > TRACK_PHASE < auf Console angezeigt, leuchtet gruene LED U3 ( j / n ): " wr_tracked
if [ "$wr_tracked" == "j" ] 
  then
    echo "Erfolgreich: WR TRACKED und COM1 Port o.k."
    echo "Erfolgreich: WR TRACKED und COM1 Port o.k." `date +%Y-%m-%d:%H:%M:%S`          >>./scuxl$scuname.txt
    wr_trackingphase_test="Erfolgreich"
  else
    echo "Fehlerhaft:  WR not TRACKED bzw. COM1 Port defekt"  
    echo "Fehlerhaft:  WR not TRACKED bzw. COM1 Port defekt" `date +%Y-%m-%d:%H:%M:%S`   >>./scuxl$scuname.txt
  fi
echo "----------------------------------------------------------------------------------">>./scuxl$scuname.txt

echo ""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear



echo""
echo "-------------Echoregister Interfacekarte mit Adresse 79 -------------------------"
echo "--- Der Test dauert etwas, während des Tests blinken die TX/RX Leds der IFK -----"
echo "--------------- Test Echo Register der Interfacekarte----------------------------">>./scuxl$scuname.txt
schreibwert=cafe



eb-write tcp/scuxl$scuname.acc             ${mil_option_data_adresse}/4 0x$schreibwert
eb-write tcp/scuxl$scuname.acc             ${mil_option_ctrl_adresse}/4 0x1379
sleep 1s
eb-write tcp/scuxl$scuname.acc             ${mil_option_ctrl_adresse}/4 0x8979
sleep 1s
lesewert=$(eb-read  tcp/scuxl$scuname.acc  ${mil_option_ctrl_adresse}/4)
#echo " Gesendeter Wert Interface Karte #79" $schreibwert
#echo " Gelesener  Wert Interface Karte #79" $lesewert

if [ 0x$lesewert == 0x0000$schreibwert ] 
  then
    echo "Erfolgreich:WR/RD auf Interface Karte #79 mit 0xCAFE" 
    echo "Erfolgreich:WR/RD auf Interface Karte #79 mit 0xCAFE" `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    Interfacekartentest="Erfolgreich"
  else
    echo "Fehlerhaft:WR/RD auf Interface Karte #79 mit 0xCAFE"
    echo "Fehlerhaft:WR/RD auf Interface Karte #79 mit 0xCAFE"  `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi

echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo ""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear

echo "---------------------  Test Slavebuskarten   ----------------------------"
echo "--------------- Test Echo Register der Slavebuskarten ----------------------------">> ./scuxl$scuname.txt

schreibwert=0x5555

for ((i=1;i<=12;i++))
do


eb-write tcp/scuxl$scuname.acc  ${slaveadresse[i]}/2  $schreibwert


lesewert=$(eb-read  tcp/scuxl$scuname.acc   ${slaveadresse[i]}/2) 


#echo " Gesendeter Wert Slavebuskarte :" $i "ist"    $schreibwert
#echo " Gelesener  Wert Slavebuskarte :" $i "ist"    $lesewert

if [ 0x$lesewert == $schreibwert ] 
  then
    echo "Erfolgreich:WR/RD auf Slavebuskarte:" $i" mit " $schreibwert
    echo "Erfolgreich:WR/RD auf Slavebuskarte:" $i" mit " $schreibwert `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    slavebustest_1[i]="Erfolgreich"
  else
    echo "Fehlerhaft: WR/RD auf Slavebuskarte:" $i" mit " $schreibwert
    echo "Fehlerhaft: WR/RD auf Slavebuskarte:" $i" mit " $schreibwert `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi


done


schreibwert=0xaaaa

for ((i=1;i<=12;i++))
do


eb-write tcp/scuxl$scuname.acc  ${slaveadresse[i]}/2  $schreibwert


lesewert=$(eb-read  tcp/scuxl$scuname.acc   ${slaveadresse[i]}/2) 


#echo " Gesendeter Wert Slavebuskarte :" $i "ist"    $schreibwert
#echo " Gelesener  Wert Slavebuskarte :" $i "ist"    $lesewert

if [ 0x$lesewert == $schreibwert ] 
  then
    echo "Erfolgreich:WR/RD auf Slavebuskarte:" $i" mit " $schreibwert
    echo "Erfolgreich:WR/RD auf Slavebuskarte:" $i" mit " $schreibwert `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    slavebustest_2[i]="Erfolgreich"
  else
    echo "Fehlerhaft: WR/RD auf Slavebuskarte:" $i" mit " $schreibwert
    echo "Fehlerhaft: WR/RD auf Slavebuskarte:" $i" mit " $schreibwert `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi


done

echo ""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear

echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo "-------------- Interrupt Test der SCU Slaves (nSRQ1 ..nSRQ12)- -------------------"

master_global_irq='0x400004'
master_en_slave_srq='0x40000c'
master_srq_active='0x400010'

eb-write tcp/scuxl$scuname.acc  ${master_global_irq}/2 0x20
eb-write tcp/scuxl$scuname.acc  ${master_en_slave_srq}/2 0xFFF

lesewert=$(eb-read  tcp/scuxl$scuname.acc   ${master_srq_active}/2) 

echo "Auslesen Master_SRQ_ACTIVE Register"  $lesewert

echo ""

#  0fff bedeutet, das jede Slavebuskarte eine 1 in das SRQ Register des Masters schreibt
testwert='0fff'

if [ $lesewert == $testwert  ] 
  then
    echo "Erfolgreich:SRQ Response der Slavebuskarten: " $i" mit " $testwert
    echo "Erfolgreich:SRQ Response der Slavebuskarten: " $i" mit " $testwert `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    slavebustest_irq="Erfolgreich"
  else
    echo "Fehlerhaft: SRQ Response der Slavebuskarten: " $i" mit " $testwert
    echo "Fehlerhaft: SRQ Response der Slavebuskarten: " $i" mit " $testwert `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi

eb-write tcp/scuxl$scuname.acc  ${master_global_irq}/2 0x0
eb-write tcp/scuxl$scuname.acc  ${master_en_slave_srq}/2 0x0

echo ""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed

clear




echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo "-------------------------- Displaytest OLED Display --------------------------------------"
echo -e "Displaytest 1: Checken Sie \e[7mHallo wie gehts\e[0m in der OLED Anzeige  "



eb-write tcp/scuxl$scuname.acc  ${oled_ctrl_adresse}/4 0x1 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0xC 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x48
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x61
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x6c 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x6c 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x6f 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x0d 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x77 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x69 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x65 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x0d 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x67 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x65 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x68 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x74 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x73                 
echo ""
read -p "Bitte drücken Sie die Returntaste um beim Displaytest fortzufahren" proceed
clear

 

echo -e "Displaytest 2: Checken Sie \e[7mSTART/12345....6789/ENDE\e[0m in der OLED Anzeige"
eb-write tcp/scuxl$scuname.acc  ${oled_ctrl_adresse}/4 0x1
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0xC 

eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x53
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x54
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x41
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x52
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x54

eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x31
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x32
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x33 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x34 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x35 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x36 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x37 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x38 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x39
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x31
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x32
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x33 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x34 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x35 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x36 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x37 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x38 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x39
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x31
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x32
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x33 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x34 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x35 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x36 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x37 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x38 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x39
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x31
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x32
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x33 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x34 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x35 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x36 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x37 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x38 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x39
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x31
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x32
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x33 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x34 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x35 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x36 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x37 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x38 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x39
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x31
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x32
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x33 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x34 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x35 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x36 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x37 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x38 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x39
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x31
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x32


eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x45 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x4E 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x44 
eb-write tcp/scuxl$scuname.acc  ${oled_data_adresse}/4 0x45

echo ""
echo ""
read -p "Waren beide Displaytest erfolgreich ( j / n ): " display_ok
echo ""

if [ "$display_ok" == "j" ] 
  then
    echo "Sie haben einen erfolgreichen Displaytest protokolliert" 
    echo "Erfolgreich:OLED Displaytest"  `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    displaytest="Erfolgreich"

  else
    echo "Sie haben einen fehlerhaften Displaytest protokolliert" 
    echo "Fehlerhaft: OLED Displaytest"  `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi

echo ""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear


echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo "-------------------------Test der OneWire IDs ------------------------------------"
echo "-------------------------Test der OneWire IDs ------------------------------------">> ./scuxl$scuname.txt

lesewert1=$(eb-read  tcp/scuxl$scuname.acc  ${Onewire_Backplane_MSW_Adr}/4)
lesewert2=$(eb-read  tcp/scuxl$scuname.acc  ${Onewire_Backplane_LSW_Adr}/4)
echo " 1Wire SCU Backplane        Temp ID:" $lesewert1$lesewert2

echo "1Wire SCU Backplane         Temp ID:" $lesewert1$lesewert2  >> ./scuxl$scuname.txt


lesewert1=$(eb-read  tcp/scuxl$scuname.acc  ${Onewire_Carrierboard_MSW_Adr}/4)
lesewert2=$(eb-read  tcp/scuxl$scuname.acc  ${Onewire_Carrierboard_LSW_Adr}/4)
echo    " 1Wire SCU Carrier Board U1=Temp ID:" $lesewert1$lesewert2
echo -n scuxl$scuname";">> ./1W_scuxl$scuname.txt
echo -n "One-Wire-U1;"$lesewert1$lesewert2";" >> ./1W_scuxl$scuname.txt

echo    "1Wire SCU Carrier Board  U1=Temp ID:" $lesewert1$lesewert2 >> ./scuxl$scuname.txt



#lesewert7=2>&1 eb-w1-write -v tcp/scuxl$scuname.acc -i 1 0 0 | sed -n '2p' | awk -F "" '{printf $3}'

echo -n " 1Wire SCU Carrier Board    PROM ID: " 
2>&1 eb-w1-write -v tcp/scuxl$scuname.acc -i 1 0 0 | sed -n '2p' | cut -d ' ' -f3 

echo -n "One-Wire-U2;" >> ./1W_scuxl$scuname.txt
2>&1 eb-w1-write -v tcp/scuxl$scuname.acc -i 1 0 0 | sed -n '2p' | cut -d ' ' -f3  | tr -d '\012\015' >> ./1W_scuxl$scuname.txt

echo -n  "1Wire SCU Carrier Board  U2=Prom ID: " >> ./scuxl$scuname.txt
2>&1 eb-w1-write -v tcp/scuxl$scuname.acc -i 1 0 0 | sed -n '2p' | cut -d ' ' -f3   >> ./scuxl$scuname.txt

lesewert1=$(eb-read  tcp/scuxl$scuname.acc  ${Onewire_Extensionboard_MSW_Adr}/4)
lesewert2=$(eb-read  tcp/scuxl$scuname.acc  ${Onewire_Extensionboard_LSW_Adr}/4)
echo " 1Wire SCU Extension Board  Temp ID:"  $lesewert1$lesewert2
echo -n ";One-Wire-ExtBrd;"$lesewert1$lesewert2";" >> ./1W_scuxl$scuname.txt
echo ""
echo  "1Wire Extension Board (MIL Option) : "$lesewert1$lesewert2 >> ./scuxl$scuname.txt



echo ""
echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
read -p "Enden die One-Wire-IDs mit 0x42 oder 0x43 ( j / n ): " onewire_ok
echo""

if [ "$onewire_ok" == "j" ] 
  then
    echo "Sie haben einen erfolgreichen 1Wire Test protokolliert" 
    echo "Erfolgreich: 1Wire Test"  `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    onewiretest="Erfolgreich"

  else
    echo "Sie haben einen fehlerhaften 1Wire Test protokolliert" 
    echo "Fehlerhaft: 1Wire Test"   `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi

echo ""

read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear

echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt

echo "Jetzt werden Ihnen noch die Temperaturen des Prüflings angezeigt"

sleep 2s

eb_temp tcp/scuxl$scuname.acc

read -p "Sind alle Temperatursensoren ok und werden IDs ohne Fehlermeldung angezeigt ( j/n ): " onewire2_ok
echo""
if [ "$onewire2_ok" == "j" ] 
  then
    echo "Sie haben einen erfolgreichen Sensor Test protokolliert" 
    echo "Erfolgreich: Sensor Test"  `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    temperaturanzeigen="Erfolgreich"

  else
    echo "Sie haben einen fehlerhaften Sensor Test protokolliert" 
    echo "Fehlerhaft: Sensor Test"   `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi

echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt
echo ""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear

    echo -e "\e[7mSie können den Namen nun aendern\e[0m"
echo "-------------------------Test der USB Buchsen ------------------------------------"
echo ""
echo "-----------------Bitte stecken Sie zwei USB Sticks in die Buchsen ----------------"
echo ""
echo -e "----------Sie müssen sich nun auf der SCU als \e[7mroot@scuxlxxxx\e[0m einloggen -----------"
echo -e "-----Nach der Eingabe des Passwords müssen Sie den Befehl \e[7mlsusb\e[0m eintippen-------"
echo ""
echo -e "-------Sie sollten dann die beiden USB \e[7mDevice IDs\e[0m Ihrer Sticks sehen--------------"
echo -e "-------Danach können Sie mit \e[7mexit\e[0m die SSH Verbindung wieder beenden ----------"



ssh root@scuxl$scuname.acc

read -p "Wurden die beiden USB Devices erkannt ( j/n ): " usb_ok
echo""
if [ "$usb_ok" == "j" ] 
  then
    echo "Sie haben einen erfolgreichen USB Test protokolliert" 
    echo "Erfolgreich: USB Test"  `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    usb_schnittstellen="Erfolgreich"

  else
    echo "Sie haben einen fehlerhaften USB Test protokolliert" 
    echo "Fehlerhaft: USB Test"   `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi

echo "-------Bitte entfernen Sie die USB Sticks wieder für die weiteren Tests-----------"
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed
clear

echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt

echo "Nun wird der LEMO B1 Ausgang oszillografisch getestet"
echo "Verbinden Sie LEMO B1 mit einem Oszilloskop"
echo ""
echo "Erwartete Reaktion: Rechtecksignal an B1,  rote B1Out aktiv, grüne B1act Led blinkt"
echo ""
read -p "Drücken Sie die Returntaste zum Weitertesten " proceed

echo ""

eca-15pps tcp/scuxl$scuname.acc

read -p "Konnten Sie das erwartete B1 Verhalten feststellen ( j/n ): " lemo1_ok
echo ""
if [ "$lemo1_ok" == "j" ] 
  then
    echo "Sie haben einen erfolgreichen LEMO B1 Test protokolliert" 
    echo "Erfolgreich: LEMO B1 Test"  `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    lemo_buchse_b1="Erfolgreich"

  else
    echo "Sie haben einen fehlerhaften LEMO B1 Test protokolliert" 
    echo "Fehlerhaft: LEMO B1 Test"   `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi

clear


echo "----------------------------------------------------------------------------------">> ./scuxl$scuname.txt

echo -e "Nun wird der Boardreset getestet. Drücken Sie hierfür den \e[7mlinken Resetbutton\e[0m"
echo -e "Die Ethernet Leds zeigen ca \e[7m17 sek\e[0m nach Reset durch kurzes Erlöschen den Restart an"


for ((i=1;i<=17;i++))
do
sleep 1s
echo "Wir warten auf die Ethernet LEDs :)" $i "Sekunden"

done

read -p "Konnten Sie das Erlöschen beider Ethernet LEDs feststellen ( j/n )" button_ok
echo""
if [ "$button_ok" == "j" ] 
  then
    echo "Sie haben einen erfolgreichen Board Reset protokolliert" 
    echo "Erfolgreich: Board Reset"  `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
    board_reset="Erfolgreich"

  else
    echo "Sie haben einen fehlerhaften Board Reset protokolliert" 
    echo "Fehlerhaft: Board Reset"   `date +%Y-%m-%d:%H:%M:%S` >>./scuxl$scuname.txt
fi

clear


echo -e "\e[7m Gratulation. Sie haben das Testende des SCU Tests erreicht\e[0m"
echo ""
echo -e " Folgende Tests wurden an der \e[7m"scuxl$scuname "\e[0m durchgeführt:"
echo ""
echo " SCU Verbindungstest mit eb-info   " $SCU_Verbindungstest_ebinfo
echo " White Rabbit Sekundenpuls Test    " $WhiteRabbit_Sekundenpuls
echo " COM1 Console / WR Tracking Phase  " $wr_trackingphase_test
echo " Interface Karte Echo Test         " $Interfacekartentest

echo " SCU Slave  1 Karte Echo Test 1    " ${slavebustest_1[1]}
echo " SCU Slave  2 Karte Echo Test 1    " ${slavebustest_1[2]}
echo " SCU Slave  3 Karte Echo Test 1    " ${slavebustest_1[3]}
echo " SCU Slave  4 Karte Echo Test 1    " ${slavebustest_1[4]}
echo " SCU Slave  5 Karte Echo Test 1    " ${slavebustest_1[5]}
echo " SCU Slave  6 Karte Echo Test 1    " ${slavebustest_1[6]}
echo " SCU Slave  7 Karte Echo Test 1    " ${slavebustest_1[7]}
echo " SCU Slave  8 Karte Echo Test 1    " ${slavebustest_1[8]}
echo " SCU Slave  9 Karte Echo Test 1    " ${slavebustest_1[9]}
echo " SCU Slave 10 Karte Echo Test 1    " ${slavebustest_1[10]}
echo " SCU Slave 11 Karte Echo Test 1    " ${slavebustest_1[11]}
echo " SCU Slave 12 Karte Echo Test 1    " ${slavebustest_1[12]}

echo " SCU Slave  1 Karte Echo Test 2    " ${slavebustest_2[1]}
echo " SCU Slave  2 Karte Echo Test 2    " ${slavebustest_2[2]}
echo " SCU Slave  3 Karte Echo Test 2    " ${slavebustest_2[3]}
echo " SCU Slave  4 Karte Echo Test 2    " ${slavebustest_2[4]}
echo " SCU Slave  5 Karte Echo Test 2    " ${slavebustest_2[5]}
echo " SCU Slave  6 Karte Echo Test 2    " ${slavebustest_2[6]}
echo " SCU Slave  7 Karte Echo Test 2    " ${slavebustest_2[7]}
echo " SCU Slave  8 Karte Echo Test 2    " ${slavebustest_2[8]}
echo " SCU Slave  9 Karte Echo Test 2    " ${slavebustest_2[9]}
echo " SCU Slave 10 Karte Echo Test 2    " ${slavebustest_2[10]}
echo " SCU Slave 11 Karte Echo Test 2    " ${slavebustest_2[11]}
echo " SCU Slave 12 Karte Echo Test 2    " ${slavebustest_2[12]}

echo " SCU Slavebustest IRQ              " $slavebustest_irq

echo " SCU OLED Displaytest              " $displaytest
echo " SCU One Wire Test                 " $onewiretest
echo " Temperaturanzeigen                " $temperaturanzeigen
echo " USB Schnittstellen erkannt        " $usb_schnittstellen
echo " Lemo Buchse B1                    " $lemo_buchse_b1
echo " board_reset                       " $board_reset
echo ""

echo -e " Detailergebnisse sind im File \e[7m" scuxl$scuname.txt "\e[0m protokolliert"
