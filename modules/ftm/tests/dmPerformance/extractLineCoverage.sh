#! /bin/sh

# extract all lines with line coverage
grep -rnH --include=*.html  lineCov . > lineCoverage.txt
# extract all source lines for the carpeDMimpl
grep bel_projects lineCoverage.txt > lineCoverage-bel_projects.txt
# remove snippets with <a name="[0-9]*>"
sed -i -e 's/<a name="[0-9]*">//g' lineCoverage-bel_projects.txt
# sort lines with highest line coverage first
sort -r -k 6 -n lineCoverage-bel_projects.txt --output lineCoverage-bel_projects-sortet.txt

# remove snippets with <a name="[0-9]*>"
sed -i -e 's/<a name="[0-9]*">//g' lineCoverage.txt
# sort lines with highest line coverage first
sort -r -k 6 -n lineCoverage.txt --output lineCoverage-sortet.txt
