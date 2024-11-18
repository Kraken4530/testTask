# testTask
Данный проект позволяет выполнить одновременную выгрузку множества файлов из интернет-ресурсов (Допустимы адреса с http и https)

Программа данного проекта успешно компилируется на ОС Linux, Windows при помощи компиляторов GCC и MSVC соответсвенно

## Запуск проекта
Сборка проекта производтся при помощи системы сборки CMake, с помощью команды: 

`cmake --build .`

После чего в дирректории со сборкой появится исполняемый файл `downloader`

## Входные параметры

Программе в параметрах запуска передаётся две строки. (строки не должны содержать пробельных символов внутри себя) и число в деястичной форме. Параметры позиционные. Должны присутствовать всегда.

Первая строка содержит путь (абсолютный либо относительный) до файла со списком URL. 

Вторая строка содержит путь до каталога, в который необходимо разместить загруженные файлы. 

Третий параметр - положительное целове число представленное в десятичной форме в диапазоне [1..999]

## Файл с URL

Во входном файле в кодировке utf-8 содержатся URLы по одному на строку. 

Пример:

`https://bolid.ru/favicon.ico

https://bolid.ru/bld/images/logo.png`

## Выходные данные

Именем сохраняемого файла берётся имя из заголовка "Content-Disposition" типа "attachment". В случае отсутствия заголовка именем файла берётся часть строки изначального URL после последнего символа "/". Невалидные для имени символы заменяются символом "_".

В случае если файл с таким именем уже существует - к имени файла добавляется номер (1), (2), и так далее в случае необходимости.

В случае неверного ввода, будет выводится ошибка и выполнение программы остановится.

В случае если код ответа от сервера отличается от 200 ОК - код ошибки выводится в консоль, URL игнорируется. 
