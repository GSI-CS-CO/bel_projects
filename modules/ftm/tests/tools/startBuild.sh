#! /bin/bash
# hint: add alias startBuild='~/scripts/startBuild.sh <user:token>' to login script.
echo -e start job
date +'%Y-%M-%d %R:%S'
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <user:token> <job name, replace blanks by %20>"
  echo "Job names are:"
  echo "   build_schedule_compare"
  echo "   build_tools"
  echo "   check%20remote%20snoop"
  echo "   Datamaster_Tests_dm-summer-update-2022_singleTest"
  echo "   Datamaster_Tests_dm-summer-update-2022"
  echo "   test_module_ftm_datamaster"
  echo "   test_module_ftm_datamaster_long_run"
  echo "   test_module_ftm_datamaster_long_run%20dm-fallout-tests"

  exit 1
fi
curl -X POST -L --user $1 http://tsl025.acc.gsi.de:8080/job/$2/build/
