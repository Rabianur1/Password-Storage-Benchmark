//main.c: Programın başlangıç noktasıdır. Sırasıyla veri temizleme, saklama ve performans testlerini çalıştırır.

#include <stdio.h>
#include <stdlib.h>
#include "preprocess.h"
#include "storage.h"
#include "search.h"
#include "utils.h"

int main(){
    printf("--------FILE ORGANIZATION PROJECT--------\n");

    const char *rawdata_dir    = ".\\RawData";
    const char *processed_file = ".\\Processed\\passwords.txt";

    //Adım 1: Ham verileri okur, temizler, Processed klasörüne yazar.
    int unique_count = preprocess_rawdata(rawdata_dir, processed_file);
    if(unique_count <= 0){
        printf("\nHATA: Veriler işlenemedi. Program sonlanıyor.\n");
        return 1;}
    printf("\n");

    //Adım 2: İşlenmiş veriyi üç farklı formatta kaydeder.
    const char *raw_dir  = ".\\Storage\\Raw";
    const char *hash_dir = ".\\Storage\\Hashed";
    const char *comp_dir = ".\\Storage\\Compressed";

    if(build_storage(processed_file, raw_dir, hash_dir, comp_dir) != 0){
        printf("\nHATA: Dosyalar kaydedilemedi.\n");
        return 1;}
    printf("\n");

    //Adım 3: Her format için 100 parolayla arama yapıp süreleri ölçer.
    const int query_count = 100;
    char **queries = pick_random_passwords(processed_file, query_count);
    if(!queries){
        printf("HATA: Test için rastgele parola seçilemedi.\n");
        return 1;}

    const char *raw_file  = ".\\Storage\\Raw\\passwords.txt";
    const char *hash_file = ".\\Storage\\Hashed\\passwords_md5.txt";
    const char *comp_file = ".\\Storage\\Compressed\\passwords.zlib";
    const char *results_dir = ".\\Results";

    run_benchmarks(raw_file, hash_file, comp_file, results_dir, queries, query_count);

    //Bellekten temizler.
    for(int i = 0; i < query_count; i++){
        free(queries[i]);}
    free(queries);
    printf("\nİşlem bitti. Sonuçları Results/results_summary.txt dosyasından görebilirsiniz.\n");
    return 0;
}