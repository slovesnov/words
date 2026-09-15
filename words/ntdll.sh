#!/bin/bash

#use ntdll instead of ldd

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
    echo "Error: File '$TARGET_EXE' not found!"
    exit 1
fi

dll_list=()

# Collect unique full paths using ntldd
# 1. ntldd -R gets all dependencies recursively
# 2. sed cleans up any square brackets [ ] or "=>" signs, leaving only the raw Windows path
# 3. grep filters for MinGW64/UCRT64 libraries
while read -r win_file; do
    if [ -n "$win_file" ]; then
        # Convert Windows path (e.g. C:\msys64\...) to POSIX path (/mingw64/...)
        file=$(cygpath -u "$win_file")
        
        if [ -f "$file" ]; then
            dll_name=$(basename "$file")
            name_without_ext=${dll_name%.*}
            dll_list+=("$name_without_ext")
        fi
    fi
done < <(ntldd -R "$TARGET_EXE" | sed -E 's/.*(=>|\[| )([a-zA-Z]:\\[^] \t]*).*/\2/' | grep -i "mingw64" | sort | uniq)

# 3. Sort DLL names alphabetically
IFS=$'\n' sorted_dll_list=($(sort <<<"${dll_list[*]}"))
unset IFS

total_files=${#sorted_dll_list[@]}

# 4. Output the result in Inno Setup format
echo "#dim Files[$total_files]"

for ((i=0; i<total_files; i++)); do
    echo "#define Files[$i] \"${sorted_dll_list[$i]}\""
done
