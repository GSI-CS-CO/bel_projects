
FEC Changing the DM for Gateway
===============================

For switching to  another DM you just have to  rename (!) the symbolic
link in the nfs-init configuration. Example for INT.

ll                                                                                    // check current config
...
30_dmunipz-dev_dmunipz-tsl008-int-config -> ../global/timing-rte-db-yocto             // tsl008 as datamaster
...

mv 30_dmunipz-dev_dmunipz-tsl008-int-config 30_dmunipz-dev_dmunipz-tsl020-int-config  // change to tsl020

ll                                                                                    // verify config
...
30_dmunipz-dev_dmunipz-tsl020-int-config -> ../global/timing-rte-db-yocto             // tsl020 as datamaster
...

Reboot FEC.


EVERYTHING BELOW THIS LINE IS JUST BACKGROUND INFORMATION

The firmware of the datamaster <-> unipz gateway needs to know the MAC
and IP  addresses of the  data master. Up  to now, these  adresses are
configured  at boot-time  via startup  scripts.  We do  not intend  to
change this  as these  changes need  a change in  DM firmware  and the
end-of-life of the gateway will be reached soon.

However, we  need to  be able  to switch between  data master  and its
backup  without  fiddeling  with  MAC  addresses  in  scripts.   As  a
workaround, two startup scripts and  two nfs-init scripts are provided
per environment (PRO,  INT). The startup scripts  and nfs-init scripts
have the name of the data master server as part of their name.

startup scripts
===============

For the startup script, there is one 'master' script that includes MAC
addresses of  all datamaster as  commented lines. The scripts  for PRO
and INT are created via copying and copying the relevant line.

- x86/dm-unipz-env_start.sh         // master startup script 
- x86/dm-unipz-tsl008-int_start.sh  // startup script for INT for tsl008
- x86/dm-unipz-tsl020-int_start.sh  // startup script for INT for tsl020

nfsinit scripts
===============

The startup  script must  be selected and  called during  the nfs-init
process. This is achieved via nfs-init configurations.

- nfs-init/int/dmunipz-tsl008-int.config  // nfs init config for INT for tsl008
- nfs-init/int/dmunipz-tsl020-int.config  // nfs init config for INT for tsl020

Remark: These configurations are used to autogenerate nfs-init scripts [1, 2]

deployment
==========
Example for INT. For the ACC7 ramdisk, this must done on the ASL7 cluster.

It is very important to clean everything prior build
  'make clean'                                   

Then build firmware, software and generate nfs-init scrips for the ACC7 ramdisk.
  'make MASP=YES ENV=int SYSENV=ACC7 PREFIX= all'

Then deploy
  'make PREFIX= SYSENV=ACC7 STAGING=/common/export/timing-rte/dmunipz-dev deploy'

Have a look at the 'Makefile' here for examples when building for PRO.

FEC config
==========

This    does   NOT    (!)     require   any    specific   script    in
'/common/export/nfsinit/global'.  The  magic happens  via the  name of
nfs-init links. Example for INT.

10_timing-rte -> ../global/timing-rte-tg-fallout-v6.2.0-no-graylog           // timing RTE
20_dmunipz-dev_dmunipz-tools -> ../global/timing-rte-db-yocto                // this will copy the relevant tools to the FEC
30_dmunipz-dev_dmunipz-tsl008-int-config -> ../global/timing-rte-db-yocto    // this will configure the FEC
40_dmunipz-dev_dmunipz-int-systemd -> ../global/timing-rte-db-yocto          // this will fire up software on the host


[1] https://github.com/GSI-CS-CO/ci_cd/tree/master/scripts/yocto_helper/nfsinit
[2] https://www-acc.gsi.de/wiki/Timing/Intern/TimingSystemHowToHintsForFECS#B2B
