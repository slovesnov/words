#!/bin/bash

# Указываем ваш исполняемый файл
TARGET_EXE="words.exe"

if [ ! -f "$TARGET_EXE" ]; then
    echo "Ошибка: Файл $TARGET_EXE не найден в текущей папке!"
    exit 1
fi

total_bytes=0
counter=0

printf "%-4s %-25s %s\n" "№" "Имя DLL" "Размер"
printf "%-4s %-25s %s\n" "---" "-------------------------" "------"

# Проходим по всем DLL из mingw64
while read -r file; do
    if [ -f "$file" ]; then
        counter=$((counter + 1))
        name=$(basename "$file")
        size_human=$(ls -lh "$file" | awk '{print $5}')
        size_bytes=$(stat -c %s "$file")
        
        # Выводим строку с номером
        printf "%-4d %-25s %s\n" "$counter" "$name" "$size_human"
        
        # Суммируем байты
        total_bytes=$((total_bytes + size_bytes))
    fi
done < <(ldd "$TARGET_EXE" | awk -F '=> ' '{print $2}' | awk '{print $1}' | grep "/mingw64/")

# Выводим итоговую сумму
printf "%-4s %-25s %s\n" "---" "-------------------------" "------"
printf "%-4s %-25s " "" "ИТОГО:"
echo "$total_bytes" | awk '{printf "%.2f MB\n", $1 / 1048576}'
