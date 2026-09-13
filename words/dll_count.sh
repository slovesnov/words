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
    echo "Ошибка: Файл '$TARGET_EXE' не найден в текущей папке!"
    exit 1
fi

total_bytes=0
counter=0

# Красивый заголовок таблицы
printf "%-4s %-35s %s\n" "№" "Имя DLL" "Размер"
printf "%-4s %-35s %s\n" "---" "-----------------------------------" "------"

# 3. Собираем уникальные пути к DLL, сразу сортируя их по алфавиту
while read -r file; do
    if [ -f "$file" ]; then
        counter=$((counter + 1))
        name=$(basename "$file")
        size_human=$(ls -lh "$file" | awk '{print $5}')
        size_bytes=$(stat -c %s "$file")
        
        # Выводим форматированную строку таблицы
        printf "%-4d %-35s %s\n" "$counter" "$name" "$size_human"
        
        # Суммируем байты
        total_bytes=$((total_bytes + size_bytes))
    fi
# Добавлен sort и uniq для исключения дубликатов на этапе чтения
done < <(ldd "$TARGET_EXE" | awk -F '=> ' '{print $2}' | awk '{print $1}' | grep "/mingw64/" | sort | uniq)

# 4. Выводим итоговую сумму
printf "%-4s %-35s %s\n" "---" "-----------------------------------" "------"
printf "%-4s %-35s " "" "ИТОГО уникальных зависимостей:"
echo "$total_bytes" | awk '{printf "%.2f MB\n", $1 / 1048576}'
