find . -type f -name "*.o" ! -name "app.o" -printf '%T@ %T+ %p\n' | sort -n | awk '
NR==1 {
    start_sec=$1; 
    start_time=substr($2, 1, 19); 
    start_file=$3
} 
END {
    if (NR>0) {
        end_sec=$1; 
        end_time=substr($2, 1, 19); 
        end_file=$3; 
        
        diff=int(end_sec-start_sec); 
        min=int(diff/60); 
        sec=diff%60; 
        
        printf "First .o file (excluding app.o): %s (%s)\n", start_file, start_time
        printf "Last .o file:                   %s (%s)\n", end_file, end_time
        print "--------------------------------------------------"
        printf "Compilation duration: %d min %d sec (%d sec)\n", min, sec, diff
    } else {
        print "No *.o files found (excluding app.o)!"
    }
}'
