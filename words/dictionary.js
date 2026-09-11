
import fs from 'fs/promises';
import iconv from 'iconv-lite';

let dic = [], set = [];
const ENGLISH = 1;

main()

function tos(i) {
    return i.toLocaleString('en-US')
}

async function main() {
    let i, e, start, gstart, c;
    gstart = start = performance.now();
    //
    let D = ENGLISH ? ['words_alpha.txt'/*, 'enindex.dic'*/] : ['Russian.dic', 'ru_RU.txt', 'index.dic'];
    D.unshift(`../site2mar/words/words/${ENGLISH ? 'en' : 'ru'}/words.txt`)
    const promises = D.map(async (e, i) => await loadDictionary(e, i));

    await Promise.all(promises);

    console.log(`load time ${((performance.now() - start) / 1000).toFixed(3)}`);

    start = performance.now();
    const s = ENGLISH ? '[a-z]' : '[-а-я]';
    const re = new RegExp(`^${s}+$`);

    for (i = 1; i < D.length; i++) {
        c = 1;
        for (e of dic[i]) {
            if (!re.test(e)) {
                console.log('error', e, i, e.length, `line ` + c)
                process.exit(1);
            }
            c++;
        }
    }

    // return;

    dic.forEach((e, index) => {
        if (!ENGLISH && index < 2) {//index=1 bad dictionary
            return
        }
        i = set[0].size;
        e.forEach(e => {
            if (!e.length) {
                console.log('err')
            }
            set[0].add(e)
        })
        console.log(index + ' ' + tos(i) + ' => ' + tos(set[0].size) + ', inserted ' + tos(set[0].size - i) + ', dictionary size ' + tos(e.length))
    })

    console.log(`check/add time ${((performance.now() - start) / 1000).toFixed(3)}`);

    start = performance.now();
    dic[0] = [...set[0]].sort((a, b) => a.localeCompare(b, ENGLISH ? 'en' : 'ru'));
    console.log(`sort time ${((performance.now() - start) / 1000).toFixed(3)}`);

    start = performance.now();

    try {
        const text = dic[0].join('\n') + '\n';
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
        if (i) {
            data = await fs.readFile(f, 'utf8');
            if (ENGLISH) {
                data = data.trim()
            }
            if (i == 1) {
                data = data.replace(/\/[\d,]+$/gm, '')
            }
            else {
                if (ENGLISH) {
                    if (i == 2) {
                        data = data.replace(/\/[a-z!]+$/gmi, '')

                    }
                }
                else {
                    if (i == 3) {
                        data = data.replace(/\/[a-z]+$/gmi, '')
                    }
                    data = data.toLocaleLowerCase().replaceAll('ё', 'е');
                }
            }
        }
        else {
            const buffer = await fs.readFile(f);
            const decoder = new TextDecoder('windows-1251');
            data = decoder.decode(buffer);
        }
        a = data.split(/\r?\n/);
        if (i) {
            if (ENGLISH) {
                if ([2].includes(i)) {
                    a = a.slice(1)
                    a = a.filter(e => !/[\d']/.test(e)).map(e=>e.toLowerCase())
                }
            }
            else {
                if ([1, 3].includes(i)) {
                    a = a.slice(1)
                }
                a = a.filter(e => e.length && !e.includes('.'))
            }
        }
        else {
            a = a.slice(0, -1)
        }
        dic[i] = a
        set[i] = new Set(a)
    } catch (err) {
        console.error('Ошибка:', err, f, i);
    }
}
