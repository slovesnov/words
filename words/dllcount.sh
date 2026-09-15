#!/bin/bash

SCRIPT_NAME=$(basename "$0")

# 1. Check if the command line argument is provided
if [ -z "$1" ]; then
    echo "Error: Target executable file is not specified!"
    echo "Usage: $SCRIPT_NAME <filename> or <filename.exe>"
    exit 1
fi

TARGET_EXE="$1"

if [[ "$TARGET_EXE" != *.exe ]]; then
    TARGET_EXE="${TARGET_EXE}.exe"
fi

# 2. Check if the target file exists
if [ ! -f "$TARGET_EXE" ]; then
    echo "Error: File '$TARGET_EXE' not found in the current directory!"
    exit 1
fi

total_bytes=0
counter=0

# Beautiful table header
printf "%-4s %-35s %s\n" "No." "DLL Name" "Size"
printf "%-4s %-35s %s\n" "---" "-----------------------------------" "------"

# 3. Collect unique DLL paths, sorting them alphabetically
while read -r file; do
    if [ -f "$file" ]; then
        counter=$((counter + 1))
        name=$(basename "$file")
        size_human=$(ls -lh "$file" | awk '{print $5}')
        size_bytes=$(stat -c %s "$file")
        
        # Output formatted table row
        printf "%-4d %-35s %s\n" "$counter" "$name" "$size_human"
        
        # Accumulate total bytes
        total_bytes=$((total_bytes + size_bytes))
    fi
# Added sort and uniq to eliminate duplicates during the reading stage
done < <(ldd "$TARGET_EXE" | awk -F '=> ' '{print $2}' | awk '{print $1}' | grep "/mingw64/" | sort | uniq)

# 4. Output the total summary
printf "%-4s %-35s %s\n" "---" "-----------------------------------" "------"
printf "%-4s %-35s " "" "TOTAL unique dependencies:"
echo "$total_bytes" | awk '{printf "%.2f MB\n", $1 / 1048576}'
