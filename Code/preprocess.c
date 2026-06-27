//preprocess.c: RawData klasöründeki tüm .txt dosyalarını okuyup birleştirir.Boş satırları atar, alfabetik sıraya koyar ve tekrar edenleri siler.Temiz hali Processed/passwords.txt olarak kaydeder.

#include "preprocess.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE  1024   //tek satır için yeterli uzunluk 
#define INIT_CAP  65536  //başlangıçta bu kadar satır için yer ayır

//Dinamik büyüyebilen string dizisi
typedef struct {
    char  **data;
    size_t  count;
    size_t  cap;
} StrArr;

static int arr_init(StrArr *a){
    a->data  = (char **)malloc(INIT_CAP * sizeof(char *));
    a->count = 0;
    a->cap   = INIT_CAP;
    return a->data ? 0 : -1;
}

static int arr_push(StrArr *a, const char *s){
    if(a->count >= a->cap){
        a->cap *= 2;  //dizi doluysa kapasiteyi iki katına çıkarır.
        char **tmp = (char **)realloc(a->data, a->cap * sizeof(char *));
        if(!tmp) return -1;
        a->data = tmp;
    }
    a->data[a->count] = _strdup(s);
    return a->data[a->count] ? (int)(a->count++, 0) : -1;
}

static void arr_free(StrArr *a){
    for(size_t i = 0; i < a->count; i++) free(a->data[i]);
    free(a->data);
    a->data  = NULL;
    a->count = a->cap = 0;
}

//Satır sonundaki \n, \r ve boşlukları temizler.
static void rtrim(char *s){
    int len = (int)strlen(s);
    while(len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' ||
                       s[len-1] == ' '  || s[len-1] == '\t'))
        s[--len] = '\0';
}

//qsort için string karşılaştırma fonksiyonudur.
static int cmp_str(const void *a, const void *b){
    return strcmp(*(const char **)a, *(const char **)b);
}

//RawData klasöründeki tüm .txt dosyalarını satır satır okuyup diziye ekler.
static int read_all_txt(const char *rawdata_dir, StrArr *arr){
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*.txt", rawdata_dir);

    WIN32_FIND_DATAA ffd;
    HANDLE h = FindFirstFileA(pattern, &ffd);

    if(h == INVALID_HANDLE_VALUE){
        fprintf(stderr, "\nUYARI: '%s' klasöründe .txt dosyası yok!\n", rawdata_dir);
        return 0;
    }

    int file_count = 0;

    do {
        if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s\\%s", rawdata_dir, ffd.cFileName);

        FILE *f = fopen(filepath, "r");
        if(!f){
            fprintf(stderr, "UYARI: %s açılamadı, geçtim.\n", ffd.cFileName);
            continue;
        }

        printf("  Okunuyor : %s\n", ffd.cFileName);
        char line[MAX_LINE];
        int line_cnt = 0;

        while(fgets(line, sizeof(line), f)){
            rtrim(line);
            if(strlen(line) == 0) continue;  //boş satırı atlar.
            arr_push(arr, line);
            line_cnt++;
        }
        printf("           -> %d satır\n", line_cnt);
        fclose(f);
        file_count++;

    } while(FindNextFileA(h, &ffd));

    FindClose(h);
    return file_count;
}

int preprocess_rawdata(const char *rawdata_dir, const char *output_file){
    printf("[1/3] Veriler temizleniyor...\n");

    StrArr arr;
    if(arr_init(&arr) != 0){
        fprintf(stderr, "HATA: RAM yetersiz.\n");
        return -1;
    }

    int file_count = read_all_txt(rawdata_dir, &arr);

    if(arr.count == 0){
        fprintf(stderr, "HATA: Okunacak hiçbir şey yok.\n");
        arr_free(&arr);
        return -1;
    }

    size_t total = arr.count;
    printf("  Toplam   : %llu satir (%d dosya)\n",
           (unsigned long long)total, file_count);
    printf("  Siralaniyor...\n");

    //Aynı olanları bulmak için önce sıralıyoruz.
    qsort(arr.data, arr.count, sizeof(char *), cmp_str);

    FILE *fout = fopen(output_file, "w");
    if(!fout){
        perror("Dosya oluşturulamadı");
        arr_free(&arr);
        return -1;
    }

    int unique = 0;
    const char *prev = NULL;

    //Aynı kelime bir daha gelirse yazmıyoruz.
    for(size_t i = 0; i < arr.count; i++){
        if(prev == NULL || strcmp(arr.data[i], prev) != 0){
            fprintf(fout, "%s\n", arr.data[i]);
            prev = arr.data[i];
            unique++;
        }
    }
    fclose(fout);

    double dup_rate = (total > 0) ? (1.0 - (double)unique / (double)total) * 100.0 : 0.0;
    printf("  Tekil    : %d parola (silinen kopya oranı: %.1f%%)\n", unique, dup_rate);

    arr_free(&arr);
    return unique;
}