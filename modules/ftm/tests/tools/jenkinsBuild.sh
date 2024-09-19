#! /bin/bash -x

# start this script somewhere inside the git repository. WORKSPACE is set if not set before.

# with set -e the script fails on the first failing command. This marks the jenkins job as failed.
set -e
if [[ ! -v WORKSPACE ]]
then
  export WORKSPACE=$(git rev-parse --show-toplevel)
fi
echo $WORKSPACE
cd $WORKSPACE
# script for automated datamaster tests with jenkins, including make of test prerequisites
# ./jenkinsBuild 8 for firmware with 8 threads
# ./jenkinsBuild 32 for firmware with 32 threads
uname -a
ssh root@fel0069.acc.gsi.de 'uname -a'
date
export DATAMASTER=tcp/fel0069.acc.gsi.de
if [ $# -eq 1 ]
then
  THR_QTY=$1
else
  THR_QTY=8
fi
# create links needed for Rocky-9 environment
cd res/rocky-9
test -L libmpfr.so.4 || ./generate_soft_links.sh
export PATH=$PATH:$(pwd)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
cd ../..
# make all prerequisites: 1. hdlmake and lm32-toolchain, 2. etherbone, 3. eb-tools, 4. test-tools for ftm.
make
make etherbone
make tools
cd modules/ftm/tests
date
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$WORKSPACE/ip_cores/etherbone-core/api/.libs/:$WORKSPACE/modules/ftm/lib/
EBPATH1=../../../ip_cores/etherbone-core/api make prepare
export PATH=$PATH:$WORKSPACE/tools/:$WORKSPACE/modules/ftm/bin/:$WORKSPACE/modules/ftm/analysis/scheduleCompare/main/
# build ftm lm32 firmware
THR_QTY=$THR_QTY PATH=$PATH:$HOME/.local/bin:$WORKSPACE/lm32-toolchain/bin/ make -C $WORKSPACE/syn/gsi_pexarria5/ftm/ ftm.bin
# load the required lm32 firmware into fel0069
# use eb-ls in fwload_all.sh from workspace
export PATH=$PATH:$WORKSPACE/ip_cores/etherbone-core/api/tools/
$WORKSPACE/syn/gsi_pexarria5/ftm/fwload_all.sh $DATAMASTER $WORKSPACE/syn/gsi_pexarria5/ftm/ftm.bin
# run all tests; date: timestamp to get duration of tests.
date
eb-info -w $DATAMASTER | grep --binary-files=text -E "ThreadQty|Version     :" -m 2
OPTIONS='--runslow' make remote
date
