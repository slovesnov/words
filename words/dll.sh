#!/bin/bash

SCRIPT_NAME=$(basename "$0")

# 1. Проверяем, передан ли аргумент командной строки
if [ -z "$1" ]; then
    echo "Ошибка: Не указан исполняемый файл!"
    echo "Использование: $SCRIPT_NAME <имя_файла> или <имя_файла.exe>"
    exit 1
fi

TARGET_EXE="$1"

if [[ "$TARGET_EXE" != *.exe ]]; then
    TARGET_EXE="${TARGET_EXE}.exe"
fi

# 2. Проверяем, существует ли файл
if [ ! -f "$TARGET_EXE" ]; then
    echo "Ошибка: Файл '$TARGET_EXE' не найден!"
    exit 1
fi

dll_list=()

# Собираем уникальные полные пути
while read -r file; do
    if [ -f "$file" ]; then
        dll_name=$(basename "$file")
        name_without_ext=${dll_name%.*}
        dll_list+=("$name_without_ext")
    fi
done < <(ldd "$TARGET_EXE" | awk -F '=> ' '{print $2}' | awk '{print $1}' | grep "/mingw64/" | sort | uniq)

# 3. СОРТИРОВКА ИМЁН ФАЙЛОВ ПО АЛФАВИТУ
# Пересобираем массив, сортируя его элементы встроенными средствами bash + sort
IFS=$'\n' sorted_dll_list=($(sort <<<"${dll_list[*]}"))
unset IFS

total_files=${#sorted_dll_list[@]}

# 4. Вывод результата в формате Inno Setup
echo "#dim Files[$total_files]"

for ((i=0; i<total_files; i++)); do
    echo "#define Files[$i] \"${sorted_dll_list[$i]}\""
done
