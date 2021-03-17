#! /bin/bash

# Usage: CommandsHistory_Fri_Feb__5_13.33.22_2021_b.sh [options]
#        where options are used for dm-cmd and dm-sched
#        Data Master name is expected in $DM.
# Example: ./commands.sh -v
OPTIONS=$1
# entry 000084
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_b/graph-entry-000084.dot
# entry 000085
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_b/graph-entry-000085.dot
# entry 000086
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_b/graph-entry-000086.dot
# entry 000087
dm-cmd $DM $OPTIONS -i CommandsHistory_Fri_Feb__5_13.33.22_2021_b/graph-entry-000087.dot
