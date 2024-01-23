# create links needed for Rocky-9 environment
date
if [ $# -eq 1 ]
then
  THR_QTY=$1
else
  THR_QTY=8
fi
cd res/rocky-9
./generate_soft_links.sh
export PATH=$PATH:$(pwd)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
cd ../..
./fix-git.sh
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
$WORKSPACE/syn/gsi_pexarria5/ftm/fwload_all.sh tcp/fel0069.acc.gsi.de $WORKSPACE/syn/gsi_pexarria5/ftm/ftm.bin
# run all tests; date: timestamp to get duration of tests.
date
OPTIONS='--runslow' make remote
date
