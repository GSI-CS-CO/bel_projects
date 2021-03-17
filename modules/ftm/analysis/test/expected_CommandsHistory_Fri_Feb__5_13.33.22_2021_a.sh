#! /bin/bash

# Usage: CommandsHistory_Fri_Feb__5_13.33.22_2021_a.sh [options]
#        where options are used for dm-cmd and dm-sched
#        Data Master name is expected in $DM.
# Example: ./commands.sh -v
OPTIONS=$1
# entry 000000
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000000.dot
# entry 000001, table check result: true
# entry 000002
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000002.dot
# entry 000003, table check result: true
# entry 000004
dm-cmd $DM $OPTIONS halt
dm-sched $DM $OPTIONS clear
# entry 000005
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000005.dot
# entry 000006, table check result: true
# entry 000007
dm-cmd $DM $OPTIONS halt
dm-sched $DM $OPTIONS clear
# entry 000008
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000008.dot
# entry 000009, table check result: true
# entry 000010
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000010.dot
# entry 000011, table check result: true
# entry 000012
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000012.dot
# entry 000013, table check result: true
# entry 000014
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000014.dot
# entry 000015
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000015.dot
# entry 000016, table check result: true
# entry 000017
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000017.dot
# entry 000018, table check result: true
# entry 000019
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000019.dot
# entry 000020, table check result: true
# entry 000021
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000021.dot
# entry 000022
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000022.dot
# entry 000023, table check result: true
# entry 000024
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000024.dot
# entry 000025, table check result: true
# entry 000026
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000026.dot
# entry 000027
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000027.dot
# entry 000028, table check result: true
# entry 000029
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000029.dot
# entry 000030
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000030.dot
# entry 000031
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000031.dot
# entry 000032, table check result: true
# entry 000033
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000033.dot
# entry 000034
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000034.dot
# entry 000035, table check result: true
# entry 000036
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000036.dot
# entry 000037
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000037.dot
# entry 000038
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000038.dot
# entry 000039, table check result: true
# entry 000040
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000040.dot
# entry 000041
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000041.dot
# entry 000042, table check result: true
# entry 000043
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000043.dot
# entry 000044
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000044.dot
# entry 000045
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000045.dot
# entry 000046, table check result: true
# entry 000047
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000047.dot
# entry 000048
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000048.dot
# entry 000049, table check result: true
# entry 000050
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000050.dot
# entry 000051
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000051.dot
# entry 000052
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000052.dot
# entry 000053
dm-cmd $DM $OPTIONS chkrem SIS18_FAST_HHD_20210201_085701
# entry 000054, result:  -> true
# entry 000055
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000055.dot
# entry 000056
dm-cmd $DM $OPTIONS chkrem SIS18_SLOW_HFS_20210114_132445
# entry 000057, result:  -> true
# entry 000058
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000058.dot
# entry 000059
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000059.dot
# entry 000060, table check result: true
# entry 000061
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000061.dot
# entry 000062
dm-cmd $DM $OPTIONS chkrem SL_ESR_DRYRUN20_2CRYRING_no_line
# entry 000063, result:  -> true
# entry 000064
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000064.dot
# entry 000065
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000065.dot
# entry 000066
dm-cmd $DM $OPTIONS chkrem SIS100_TEST_PROTON_20210114
# entry 000067, result:  -> true
# entry 000068
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000068.dot
# entry 000069
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000069.dot
# entry 000070
dm-cmd $DM $OPTIONS chkrem SIS18_FAST_HHT_20201211_132855
# entry 000071, result:  -> true
# entry 000072
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000072.dot
# entry 000073
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000073.dot
# entry 000074
dm-cmd $DM $OPTIONS chkrem SIS18_FAST_HHT_20210201_132940
# entry 000075, result:  -> true
# entry 000076
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000076.dot
# entry 000077
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000077.dot
# entry 000078, table check result: true
# entry 000079
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000079.dot
# entry 000080
dm-cmd $DM $OPTIONS chkrem CRYRING_D_local
# entry 000081, result:  -> true
# entry 000082
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000082.dot
# entry 000083
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000083.dot
# entry 000084
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000084.dot
# entry 000085
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000085.dot
# entry 000086
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000086.dot
# entry 000087
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000087.dot
# entry 000088
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000088.dot
# entry 000089
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000089.dot
# entry 000090, table check result: true
# entry 000091
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000091.dot
# entry 000092
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000092.dot
# entry 000093, table check result: true
# entry 000094
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000094.dot
# entry 000095
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000095.dot
# entry 000096
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000096.dot
# entry 000097, table check result: true
# entry 000098
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000098.dot
# entry 000099
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000099.dot
# entry 000100, table check result: true
# entry 000101
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000101.dot
