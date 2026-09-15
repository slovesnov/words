#!/bin/bash

find . -type f \( -name "*.cpp" -o -name "*.h" \) -exec ls -l {} + | sort -rnk5 | awk '{size+=$5; if($9 ~ /\.cpp$/) cpp++; else h++; print $5 "\t" $9} END {print "\n=== ИТОГО ==="; print "Всего .cpp файлов:", cpp; print "Всего .h файлов:", h; print "Общий размер файлов:", size, "байт"}'
