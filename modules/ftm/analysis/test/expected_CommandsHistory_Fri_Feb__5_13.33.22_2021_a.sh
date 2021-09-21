#! /bin/bash

# Usage: CommandsHistory_Fri_Feb__5_13.33.22_2021_a.sh [options]
#        where options are used for dm-cmd and dm-sched
#        Data Master name is expected in $DM.
# Example: ./commands.sh -v
OPTIONS=$1
# entry 000000 2021-02-05 13:36:22
echo entry 000000 2021-02-05 13:36:22
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000000.dot
# entry 000001 2021-02-05 13:36:22
echo entry 000001 2021-02-05 13:36:22, table check result: true
# entry 000002 2021-02-05 13:36:22
echo entry 000002 2021-02-05 13:36:22
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000002.dot
# entry 000003 2021-02-05 13:36:22
echo entry 000003 2021-02-05 13:36:22, table check result: true
# entry 000004 2021-02-05 13:37:28
echo entry 000004 2021-02-05 13:37:28
dm-cmd $DM $OPTIONS halt
dm-sched $DM $OPTIONS clear
# entry 000005 2021-02-05 13:37:28
echo entry 000005 2021-02-05 13:37:28
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000005.dot
# entry 000006 2021-02-05 13:37:28
echo entry 000006 2021-02-05 13:37:28, table check result: true
# entry 000007 2021-02-05 13:37:28
echo entry 000007 2021-02-05 13:37:28
dm-cmd $DM $OPTIONS halt
dm-sched $DM $OPTIONS clear
# entry 000008 2021-02-05 13:37:28
echo entry 000008 2021-02-05 13:37:28
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000008.dot
# entry 000009 2021-02-05 13:37:28
echo entry 000009 2021-02-05 13:37:28, table check result: true
# entry 000010 2021-02-05 13:37:29
echo entry 000010 2021-02-05 13:37:29
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000010.dot
# entry 000011 2021-02-05 13:37:29
echo entry 000011 2021-02-05 13:37:29, table check result: true
# entry 000012 2021-02-05 13:37:29
echo entry 000012 2021-02-05 13:37:29
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000012.dot
# entry 000013 2021-02-05 13:37:29
echo entry 000013 2021-02-05 13:37:29, table check result: true
# entry 000014 2021-02-05 13:37:29
echo entry 000014 2021-02-05 13:37:29
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000014.dot
# entry 000015 2021-02-05 13:37:29
echo entry 000015 2021-02-05 13:37:29
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000015.dot
# entry 000016 2021-02-05 13:37:29
echo entry 000016 2021-02-05 13:37:29, table check result: true
# entry 000017 2021-02-05 13:37:33
echo entry 000017 2021-02-05 13:37:33
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000017.dot
# entry 000018 2021-02-05 13:37:33
echo entry 000018 2021-02-05 13:37:33, table check result: true
# entry 000019 2021-02-05 13:37:33
echo entry 000019 2021-02-05 13:37:33
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000019.dot
# entry 000020 2021-02-05 13:37:33
echo entry 000020 2021-02-05 13:37:33, table check result: true
# entry 000021 2021-02-05 13:37:33
echo entry 000021 2021-02-05 13:37:33
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000021.dot
# entry 000022 2021-02-05 13:37:33
echo entry 000022 2021-02-05 13:37:33
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000022.dot
# entry 000023 2021-02-05 13:37:33
echo entry 000023 2021-02-05 13:37:33, table check result: true
# entry 000024 2021-02-05 13:37:33
echo entry 000024 2021-02-05 13:37:33
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000024.dot
# entry 000025 2021-02-05 13:37:33
echo entry 000025 2021-02-05 13:37:33, table check result: true
# entry 000026 2021-02-05 13:37:33
echo entry 000026 2021-02-05 13:37:33
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000026.dot
# entry 000027 2021-02-05 13:37:33
echo entry 000027 2021-02-05 13:37:33
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000027.dot
# entry 000028 2021-02-05 13:37:33
echo entry 000028 2021-02-05 13:37:33, table check result: true
# entry 000029 2021-02-05 13:37:33
echo entry 000029 2021-02-05 13:37:33
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000029.dot
# entry 000030 2021-02-05 13:37:33
echo entry 000030 2021-02-05 13:37:33
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000030.dot
# entry 000031 2021-02-05 13:37:33
echo entry 000031 2021-02-05 13:37:33
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000031.dot
# entry 000032 2021-02-05 13:37:33
echo entry 000032 2021-02-05 13:37:33, table check result: true
# entry 000033 2021-02-05 13:37:33
echo entry 000033 2021-02-05 13:37:33
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000033.dot
# entry 000034 2021-02-05 13:37:33
echo entry 000034 2021-02-05 13:37:33
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000034.dot
# entry 000035 2021-02-05 13:37:33
echo entry 000035 2021-02-05 13:37:33, table check result: true
# entry 000036 2021-02-05 13:37:33
echo entry 000036 2021-02-05 13:37:33
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000036.dot
# entry 000037 2021-02-05 13:37:34
echo entry 000037 2021-02-05 13:37:34
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000037.dot
# entry 000038 2021-02-05 13:37:34
echo entry 000038 2021-02-05 13:37:34
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000038.dot
# entry 000039 2021-02-05 13:37:34
echo entry 000039 2021-02-05 13:37:34, table check result: true
# entry 000040 2021-02-05 13:37:34
echo entry 000040 2021-02-05 13:37:34
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000040.dot
# entry 000041 2021-02-05 13:37:34
echo entry 000041 2021-02-05 13:37:34
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000041.dot
# entry 000042 2021-02-05 13:37:34
echo entry 000042 2021-02-05 13:37:34, table check result: true
# entry 000043 2021-02-05 13:37:34
echo entry 000043 2021-02-05 13:37:34
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000043.dot
# entry 000044 2021-02-05 13:37:34
echo entry 000044 2021-02-05 13:37:34
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000044.dot
# entry 000045 2021-02-05 13:37:34
echo entry 000045 2021-02-05 13:37:34
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000045.dot
# entry 000046 2021-02-05 13:37:34
echo entry 000046 2021-02-05 13:37:34, table check result: true
# entry 000047 2021-02-05 13:37:35
echo entry 000047 2021-02-05 13:37:35
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000047.dot
# entry 000048 2021-02-05 13:37:35
echo entry 000048 2021-02-05 13:37:35
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000048.dot
# entry 000049 2021-02-05 13:37:35
echo entry 000049 2021-02-05 13:37:35, table check result: true
# entry 000050 2021-02-05 13:37:35
echo entry 000050 2021-02-05 13:37:35
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000050.dot
# entry 000051 2021-02-05 13:37:35
echo entry 000051 2021-02-05 13:37:35
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000051.dot
# entry 000052 2021-02-05 13:37:35
echo entry 000052 2021-02-05 13:37:35
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000052.dot
# entry 000053 2021-02-05 13:37:37
echo entry 000053 2021-02-05 13:37:37
dm-cmd $DM $OPTIONS chkrem SIS18_FAST_HHD_20210201_085701
# entry 000054 2021-02-05 13:37:37
echo entry 000054 2021-02-05 13:37:37, result:  -> true
# entry 000055 2021-02-05 13:37:48
echo entry 000055 2021-02-05 13:37:48
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000055.dot
# entry 000056 2021-02-05 13:37:48
echo entry 000056 2021-02-05 13:37:48
dm-cmd $DM $OPTIONS chkrem SIS18_SLOW_HFS_20210114_132445
# entry 000057 2021-02-05 13:37:48
echo entry 000057 2021-02-05 13:37:48, result:  -> true
# entry 000058 2021-02-05 13:37:57
echo entry 000058 2021-02-05 13:37:57
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000058.dot
# entry 000059 2021-02-05 13:37:57
echo entry 000059 2021-02-05 13:37:57
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000059.dot
# entry 000060 2021-02-05 13:37:57
echo entry 000060 2021-02-05 13:37:57, table check result: true
# entry 000061 2021-02-05 13:37:58
echo entry 000061 2021-02-05 13:37:58
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000061.dot
# entry 000062 2021-02-05 13:37:58
echo entry 000062 2021-02-05 13:37:58
dm-cmd $DM $OPTIONS chkrem SL_ESR_DRYRUN20_2CRYRING_no_line
# entry 000063 2021-02-05 13:37:58
echo entry 000063 2021-02-05 13:37:58, result:  -> true
# entry 000064 2021-02-05 13:38:07
echo entry 000064 2021-02-05 13:38:07
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000064.dot
# entry 000065 2021-02-05 13:38:07
echo entry 000065 2021-02-05 13:38:07
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000065.dot
# entry 000066 2021-02-05 13:38:07
echo entry 000066 2021-02-05 13:38:07
dm-cmd $DM $OPTIONS chkrem SIS100_TEST_PROTON_20210114
# entry 000067 2021-02-05 13:38:08
echo entry 000067 2021-02-05 13:38:08, result:  -> true
# entry 000068 2021-02-05 13:38:08
echo entry 000068 2021-02-05 13:38:08
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000068.dot
# entry 000069 2021-02-05 13:38:08
echo entry 000069 2021-02-05 13:38:08
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000069.dot
# entry 000070 2021-02-05 13:38:08
echo entry 000070 2021-02-05 13:38:08
dm-cmd $DM $OPTIONS chkrem SIS18_FAST_HHT_20201211_132855
# entry 000071 2021-02-05 13:38:08
echo entry 000071 2021-02-05 13:38:08, result:  -> true
# entry 000072 2021-02-05 13:38:15
echo entry 000072 2021-02-05 13:38:15
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000072.dot
# entry 000073 2021-02-05 13:38:15
echo entry 000073 2021-02-05 13:38:15
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000073.dot
# entry 000074 2021-02-05 13:38:15
echo entry 000074 2021-02-05 13:38:15
dm-cmd $DM $OPTIONS chkrem SIS18_FAST_HHT_20210201_132940
# entry 000075 2021-02-05 13:38:15
echo entry 000075 2021-02-05 13:38:15, result:  -> true
# entry 000076 2021-02-05 13:38:21
echo entry 000076 2021-02-05 13:38:21
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000076.dot
# entry 000077 2021-02-05 13:38:21
echo entry 000077 2021-02-05 13:38:21
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000077.dot
# entry 000078 2021-02-05 13:38:21
echo entry 000078 2021-02-05 13:38:21, table check result: true
# entry 000079 2021-02-05 13:38:21
echo entry 000079 2021-02-05 13:38:21
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000079.dot
# entry 000080 2021-02-05 13:38:21
echo entry 000080 2021-02-05 13:38:21
dm-cmd $DM $OPTIONS chkrem CRYRING_D_local
# entry 000081 2021-02-05 13:38:21
echo entry 000081 2021-02-05 13:38:21, result:  -> true
# entry 000082 2021-02-05 13:38:24
echo entry 000082 2021-02-05 13:38:24
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000082.dot
# entry 000083 2021-02-05 13:39:40
echo entry 000083 2021-02-05 13:39:40
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000083.dot
# entry 000084 2021-02-05 13:40:42
echo entry 000084 2021-02-05 13:40:42
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000084.dot
# entry 000085 2021-02-05 13:40:42
echo entry 000085 2021-02-05 13:40:42
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000085.dot
# entry 000086 2021-02-05 13:40:42
echo entry 000086 2021-02-05 13:40:42
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000086.dot
# entry 000087 2021-02-05 13:40:42
echo entry 000087 2021-02-05 13:40:42
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000087.dot
# entry 000088 2021-02-05 14:04:29
echo entry 000088 2021-02-05 14:04:29
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000088.dot
# entry 000089 2021-02-05 14:04:29
echo entry 000089 2021-02-05 14:04:29
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000089.dot
# entry 000090 2021-02-05 14:04:29
echo entry 000090 2021-02-05 14:04:29, table check result: true
# entry 000091 2021-02-05 14:04:29
echo entry 000091 2021-02-05 14:04:29
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000091.dot
# entry 000092 2021-02-05 14:04:30
echo entry 000092 2021-02-05 14:04:30
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000092.dot
# entry 000093 2021-02-05 14:04:30
echo entry 000093 2021-02-05 14:04:30, table check result: true
# entry 000094 2021-02-05 14:04:30
echo entry 000094 2021-02-05 14:04:30
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000094.dot
# entry 000095 2021-02-05 14:07:46
echo entry 000095 2021-02-05 14:07:46
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000095.dot
# entry 000096 2021-02-05 14:07:47
echo entry 000096 2021-02-05 14:07:47
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000096.dot
# entry 000097 2021-02-05 14:07:47
echo entry 000097 2021-02-05 14:07:47, table check result: true
# entry 000098 2021-02-05 14:07:47
echo entry 000098 2021-02-05 14:07:47
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000098.dot
# entry 000099 2021-02-05 14:07:47
echo entry 000099 2021-02-05 14:07:47
dm-sched $DM $OPTIONS status -o CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000099.dot
# entry 000100 2021-02-05 14:07:47
echo entry 000100 2021-02-05 14:07:47, table check result: true
# entry 000101 2021-02-05 14:07:52
echo entry 000101 2021-02-05 14:07:52
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_a/graph-entry-000101.dot
