@echo off
REM Tüm C dosyalarını derleyip ana klasörde Project.exe adında bir çalıştırılabilir dosya oluşturur.
REM Ek olarak memory kullanımı için psapi kütüphanesini bağlar.
echo Derleniyor...
gcc -O3 -Wall main.c preprocess.c storage.c search.c compression.c utils.c libs/md5.c libs/miniz.c -o ..\Project.exe -lpsapi

if %ERRORLEVEL% EQU 0 (
    echo Basarili! Program 'Project.exe' olarak bir ust dizinde olusturuldu.
    echo Calistirmak icin: cd .. ^&^& Project.exe
) else (
    echo Derleme sirasinda hata olustu.
)
pause