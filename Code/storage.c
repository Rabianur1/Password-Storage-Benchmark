/*storage.c: Processed klasöründeki temiz veriyi üç farklı formatta Storage klasörüne kaydeder:
Raw: düz metin olarak kopyalar.
Hashed: her parolanın MD5 hash'ini hesaplar ve sıralı olarak yazar.
Compressed: raw dosyasını zlib ile sıkıştırır.*/

#include "storage.h"
#include "libs/md5.h"
#include "compression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

//Hash'leri sıralamak için
static int cmp_hash(const void *a, const void *b){
    return strcmp((const char *)a, (const char *)b);
}

int build_storage(const char *processed_path,
                  const char *raw_dir,
                  const char *hashed_dir,
                  const char *comp_dir){
    printf("[2/3] Veri saklama yapıları oluşturuluyor...\n");

    char raw_path[512], hash_path[512], comp_path[512];
    snprintf(raw_path, sizeof(raw_path), "%s\\passwords.txt", raw_dir);
    snprintf(hash_path, sizeof(hash_path), "%s\\passwords_md5.txt", hashed_dir);
    snprintf(comp_path, sizeof(comp_path), "%s\\passwords.zlib", comp_dir);

    FILE *fin = fopen(processed_path, "r");
    if(!fin){
        perror("Dosya açılamadı");
        return -1;
    }

    //1. Raw Format
    printf("  -> Raw Storage yaziliyor...\n");
    FILE *fraw = fopen(raw_path, "w");
    if(!fraw){
        fclose(fin);
        return -1;
    }

    //Hash'leri de aynı döngüde hesaplayıp belleğe alırız.
    size_t cap = 65536;
    size_t count = 0;
    char(*hashes)[33] = malloc(cap * 33);
    if(!hashes){
        fclose(fin); fclose(fraw); return -1;
    }

    char line[MAX_LINE];
    while(fgets(line, sizeof(line), fin)){
        fputs(line, fraw);  //Raw dosyasına olduğu gibi yaz

        //Hash hesabı için satır sonunu temizler.
        size_t len = strlen(line);
        while(len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')){
            line[--len] = '\0';
        }
        if(len == 0) continue;

        //Dizi doluysa büyütür.
        if(count >= cap){
            cap *= 2;
            char(*tmp)[33] = realloc(hashes, cap * 33);
            if(!tmp){
                free(hashes); fclose(fin); fclose(fraw); return -1;
            }
            hashes = tmp;
        }

        md5_string(line, len, hashes[count]);
        count++;
    }
    fclose(fin);
    fclose(fraw);

    //2. Hash Formatı
    printf("  -> Hashed Storage yazılıyor (arama hızlandırması için sıralanarak)...\n");
    qsort(hashes, count, 33, cmp_hash);

    FILE *fhash = fopen(hash_path, "w");
    if(!fhash){
        free(hashes); return -1;
    }
    for(size_t i = 0; i < count; i++){
        fprintf(fhash, "%s\n", hashes[i]);
    }
    fclose(fhash);
    free(hashes);

    //3. Sıkıştırılmış Format
    printf("  -> Compressed Storage (zlib) oluşturuluyor...\n");
    if(compress_file(raw_path, comp_path) != 0){
        fprintf(stderr, "HATA: zlib işlemi başarısız.\n");
        return -1;
    }
    return 0;
}