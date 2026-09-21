
import fs from 'fs/promises';
import iconv from 'iconv-lite';

import fss from 'fs';
import path from 'path';

let dic = [], set = [];
const ENGLISH = 0;

main()

function tos(i) {
    return i.toLocaleString('en-US')
}

async function main() {
    let i, e, start, gstart, c;
    gstart = start = performance.now();
    await loadDictionary(`../words/words/words/${ENGLISH ? 'en' : 'ru'}/words.txt`, 0);

    // console.log(dic.length,dic[dic.length-1])

    console.log(`load time ${((performance.now() - start) / 1000).toFixed(3)}`);

    start = performance.now();
    dic.push('новые слова','новые слова')

    console.log(`check/add time ${((performance.now() - start) / 1000).toFixed(3)}`);

    start = performance.now();
    dic.sort((a, b) => a.localeCompare(b, ENGLISH ? 'en' : 'ru'));
    console.log(`sort time ${((performance.now() - start) / 1000).toFixed(3)}`);
    // console.log(dic.length,dic[dic.length-1])

    start = performance.now();

    try {
        const text = dic.join('\n');
        const buffer = ENGLISH ? Buffer.from(text) : iconv.encode(text, 'windows-1251');
        await fs.writeFile('words.txt', buffer);
    } catch (err) {
        console.error('Ошибка записи файла:', err);
    }

    console.log(`write time ${((performance.now() - start) / 1000).toFixed(3)}`);

    start = gstart
    console.log(`total time ${((performance.now() - start) / 1000).toFixed(3)}`);
}

async function loadDictionary(f, i) {
    try {
        let data, a
        const buffer = await fs.readFile(f);
        const decoder = new TextDecoder('windows-1251');
        data = decoder.decode(buffer);
        a = data.split(/\r?\n/);
        dic = a
    } catch (err) {
        console.error('Ошибка:', err, f, i);
    }
}

function checkdir(dirPath) {
    try {
        // Получаем список объектов dirent (directory entry)
        const files = fss.readdirSync(dirPath, { withFileTypes: true });

        files.forEach(file => {
            // Проверяем, что это именно файл, а не папка или ссылка
            if (file.isFile()) {

                // Если нужно получить полный путь к файлу:
                const fullPath = path.join(dirPath, file.name);
                if (checkCarriageReturn(fullPath)) {
                    console.log(file.name);
                }
            }
        });
    } catch (err) {
        console.error('Ошибка чтения каталога:', err.message);
    }
}

function checkCarriageReturn(filePath) {
    try {
        const content = fss.readFileSync(filePath, 'utf8');
        return content.includes('\r')
    } catch (error) {
        console.error(`Ошибка при чтении файла: ${error.message}`);
    }
}
