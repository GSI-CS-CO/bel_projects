#!/bin/bash

# Deploy temporary firmware, test scripts

# Prerequisites:
# - must be invoked only from the devel-host (ie., acopc061)
# - target SCUs run yocto-based RAM disk

tgt_scus="scuxl0264 scuxl0321 scuxl0339"
scp_opts="-o StrictHostKeyChecking=no"
rsync_opts="-Pauvh"
ssh_pass_opts="-p password"              # change password!

module_dir="${PWD/fbas*/fbas}"           # bel_projects/modules/fbas
build_dir="$module_dir/fw"               # location of FBAS firmware
scu_tools_dir="$module_dir/test/scu"     # location of client test script (for target SCUs)
x86_tools_dir="$module_dir/test/tools"   # location of server test script (for test host)

# test host
rsync $rsync_opts $x86_tools_dir/*.sh tsl101:./fbas_test/tools/

# test targets
for scu in $tgt_scus; do
  sshpass $ssh_pass_opts scp $scp_opts $build_dir/fbas??.scucontrol.bin root@$scu.acc.gsi.de:.
  sshpass $ssh_pass_opts scp $scp_opts $scu_tools_dir/setup_local.sh root@$scu.acc.gsi.de:/usr/bin/
done
