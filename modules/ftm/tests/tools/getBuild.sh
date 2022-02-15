#! /bin/bash
# hint: add alias getBuild='~/scripts/getBuild.sh <user:token>' to login script.
echo -e get status of last build in json format
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <user:token> <job name, replace blanks by %20>" >&2
  echo "Job names are:"
  echo "   build_schedule_compare"
  echo "   build_tools"
  echo "   check%20remote%20snoop"
  echo "   test_module_ftm_datamaster"
  echo "   test_module_ftm_datamaster_long_run"
  echo "   test_module_ftm_datamaster_long_run%20dm-fallout-tests-2"
  exit 1
fi

curl -X POST -L --user $1 http://tsl025.acc.gsi.de:8080/job/$2/lastBuild/api/json | python3 -m json.tool > buildResult_$2.json
