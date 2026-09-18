#!/bin/bash

# Гарантируем, что разделителем тысяч будет именно запятая (,)
export LC_NUMERIC=en_US.UTF-8

find . -type f \( -name "*.cpp" -o -name "*.h" \) -exec ls -l {} + | sort -rnk5 | awk '
{
    size += $5; 
    if ($9 ~ /\.cpp$/) cpp++; else h++; 
    
    printf "%'\''d\t%s\n", $5, $9
} 
END {
    print "\n=== TOTAL ==="; 
    print "Total .cpp files:", cpp; 
    print "Total .h files:", h; 
    
    printf "Total file size: %'\''d bytes\n", size
}'
