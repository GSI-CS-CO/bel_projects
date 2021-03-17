#! /bin/bash

# Usage: CommandsHistory_Fri_Feb__5_13.33.22_2021_c.sh [options]
#        where options are used for dm-cmd and dm-sched
#        Data Master name is expected in $DM.
# Example: ./commands.sh -v
OPTIONS=$1
# entry 000084
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000084.dot
# entry 000085
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000085.dot
# entry 000086
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000086.dot
# entry 000087
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000087.dot
# entry 000088
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000088.dot
# entry 000089
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000089.dot
# entry 000090, table check result: true
# entry 000091
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000091.dot
# entry 000092
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000092.dot
# entry 000093, table check result: true
# entry 000094
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000094.dot
# entry 000095
dm-sched $DM $OPTIONS remove CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000095.dot
# entry 000096
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000096.dot
# entry 000097, table check result: true
# entry 000098
dm-sched $DM $OPTIONS add CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000098.dot
# entry 000099
dm-sched $DM $OPTIONS status CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000099.dot
# entry 000100, table check result: true
# entry 000101
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_c/graph-entry-000101.dot
