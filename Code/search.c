/*search.c: Üç farklı saklama formatında (raw, hashed, compressed) arama yapan benchmark fonksiyonlarını içerir. 
Her yöntem için ortalama ve maksimum arama süresi, CPU ve RAM kullanımı ölçülür. Sonuçlar Results klasörüne yazılır.*/

#include "search.h"
#include "utils.h"
#include "libs/md5.h"
#include "compression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define MAX_LINE 1024

//Satır sonundaki boşlukları temizler.
static void trim_nl(char *s){
    size_t len = strlen(s);
    while(len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')){
        s[--len] = '\0';
    }
}

//İşlenmiş dosyadan rastgele count kadar parola seçip döndürür.
char **pick_random_passwords(const char *processed_path, int count){
    FILE *f = fopen(processed_path, "r");
    if(!f) return NULL;

    //Önce kaç satır olduğu bulunur.
    int total = 0;
    char line[MAX_LINE];
    while(fgets(line, sizeof(line), f)) total++;

    if(total == 0){fclose(f); return NULL;}

    int *indices = malloc(count * sizeof(int));
    char **queries = malloc(count * sizeof(char *));
    srand((unsigned int)time(NULL));

    //Rastgele indis üretir.
    for(int i = 0; i < count; i++){
        indices[i] = rand() % total;
    }

    //Seçtiğimiz yerlerdeki satırları dosyadan okur.
    for(int i = 0; i < count; i++){
        fseek(f, 0, SEEK_SET);
        int current = 0;
        while(fgets(line, sizeof(line), f)){
            if(current == indices[i]){
                trim_nl(line);
                queries[i] = _strdup(line);
                break;
            }
            current++;
        }
    }
    fclose(f);
    free(indices);
    return queries;
}

//1.Raw arama: En baştan tek tek okuyarak arar.
static double bench_raw(const char *path, char **queries, int qcount, double *max_t, double *std_dev){
    double total_us = 0.0;
    *max_t = 0.0;
    double *times = malloc(qcount * sizeof(double));

    for(int i = 0; i < qcount; i++){
        double start = get_time_us();
        FILE *f = fopen(path, "r");
        if(f){
            char line[MAX_LINE];
            while(fgets(line, sizeof(line), f)){
                trim_nl(line);
                if(strcmp(line, queries[i]) == 0) break;
            }
            fclose(f);
        }
        double elapsed = get_time_us() - start;
        times[i] = elapsed;
        total_us += elapsed;
        if(elapsed > *max_t) *max_t = elapsed;
    }
    double avg = total_us / qcount;
    double sum_sq = 0.0;
    for(int i = 0; i < qcount; i++){
        sum_sq += (times[i] - avg) * (times[i] - avg);
    }
    *std_dev = sqrt(sum_sq / qcount);
    free(times);
    return avg;
}

//2.Hash arama: Hash dosyası sıralı olduğu için ikili arama yapar.
static double bench_hashed(const char *path, char **queries, int qcount, double *max_t, double *std_dev){
    double total_us = 0.0;
    *max_t = 0.0;
    double *times = malloc(qcount * sizeof(double));

    long long fsize = get_file_size(path);
    FILE *f = fopen(path, "rb");
    if(!f) {free(times); return 0.0;}

    //İlk satırı okuyarak bir kaydın kaç byte tuttuğunu öğreniriz.
    char first[64];
    fgets(first, sizeof(first), f);
    long long rec_len = ftell(f);
    long long total_records = fsize / rec_len;

    for(int i = 0; i < qcount; i++){
        double start = get_time_us();

        //Aranan parolanın MD5'ini alır sonra dosyada o hash'i arar.
        char target_hash[33];
        md5_string(queries[i], strlen(queries[i]), target_hash);

        long long left = 0;
        long long right = total_records - 1;

        while(left <= right){
            long long mid = left + (right - left) / 2;
            fseek(f, (long)(mid * rec_len), SEEK_SET);
            char line[64];
            if(!fgets(line, sizeof(line), f)) break;
            trim_nl(line);

            int cmp = strcmp(line, target_hash);
            if (cmp == 0) { break; }
            else if (cmp < 0) left = mid + 1;
            else right = mid - 1;
        }

        double elapsed = get_time_us() - start;
        times[i] = elapsed;
        total_us += elapsed;
        if(elapsed > *max_t) *max_t = elapsed;
    }
    fclose(f);
    
    double avg = total_us / qcount;
    double sum_sq = 0.0;
    for(int i = 0; i < qcount; i++){
        sum_sq += (times[i] - avg) * (times[i] - avg);
    }
    *std_dev = sqrt(sum_sq / qcount);
    free(times);
    return avg;
}

//3.Sıkıştırılmış arama. Zlib dosyasını belleğe açıp orada arar.
static double bench_comp(const char *path, char **queries, int qcount, double *max_t, double *decomp_time_out, double *std_dev){
    double total_us = 0.0;
    *max_t = 0.0;
    double *times = malloc(qcount * sizeof(double));

    //Açma süresini ayrıca ölçer.
    double d_start = get_time_us();
    char *buf = NULL;
    long long buf_size = 0;
    if(decompress_to_memory(path, &buf, &buf_size) != 0){
        free(times);
        return 0.0;
    }
    *decomp_time_out = get_time_us() - d_start;

    for(int i = 0; i < qcount; i++){
        double start = get_time_us();

        //Bellek üzerinde strstr ile aratıp tam eşleşmeyi kontrol eder.
        char *ptr  = buf;
        int  qlen  = strlen(queries[i]);
        while((ptr = strstr(ptr, queries[i])) != NULL){
            int start_ok = (ptr == buf || *(ptr-1) == '\n');
            int end_ok   = (*(ptr+qlen) == '\n' || *(ptr+qlen) == '\r' || *(ptr+qlen) == '\0');
            if(start_ok && end_ok) break;  //kelime tam eşleşti
            ptr += qlen;
        }

        double elapsed = get_time_us() - start;
        times[i] = elapsed;
        total_us += elapsed;
        if(elapsed > *max_t) *max_t = elapsed;
    }
    free(buf);
    
    double avg = total_us / qcount;
    double sum_sq = 0.0;
    for(int i = 0; i < qcount; i++){
        sum_sq += (times[i] - avg) * (times[i] - avg);
    }
    *std_dev = sqrt(sum_sq / qcount);
    free(times);
    return avg;
}

//Tüm testleri çalıştırıp sonuçları yazar.
void run_benchmarks(const char *raw_path,
                    const char *hash_path,
                    const char *comp_path,
                    const char *results_dir,
                    char **queries, int query_count){

    printf("[3/3] Performans testleri çalıştırılıyor (%d parola)...\n", query_count);

    CpuSnapshot cpu_start, cpu_end;
    char res_file[512];
    snprintf(res_file, sizeof(res_file), "%s\\results_summary.txt", results_dir);
    FILE *fout = fopen(res_file, "w");
    if(!fout) return;

    fprintf(fout, "--------FILE ORGANIZATION PROJECT BENCHMARK RESULTS--------\n\n");

    printf("  -> Raw Storage test ediliyor...\n");
    take_cpu_snapshot(&cpu_start);
    double max_raw = 0, std_raw = 0;
    double avg_raw = bench_raw(raw_path, queries, query_count, &max_raw, &std_raw);
    take_cpu_snapshot(&cpu_end);
    double cpu_raw = calc_cpu_percent(&cpu_start, &cpu_end);
    double ram_raw = get_ram_mb();
    long long size_raw = get_file_size(raw_path);
    char s_raw[32]; format_size(size_raw, s_raw, sizeof(s_raw));

    printf("  -> Hashed Storage test ediliyor...\n");
    take_cpu_snapshot(&cpu_start);
    double max_hash = 0, std_hash = 0;
    double avg_hash = bench_hashed(hash_path, queries, query_count, &max_hash, &std_hash);
    take_cpu_snapshot(&cpu_end);
    double cpu_hash = calc_cpu_percent(&cpu_start, &cpu_end);
    double ram_hash = get_ram_mb();
    long long size_hash = get_file_size(hash_path);
    char s_hash[32]; format_size(size_hash, s_hash, sizeof(s_hash));

    printf("  -> Compressed Storage test ediliyor...\n");
    take_cpu_snapshot(&cpu_start);
    double max_comp = 0, decomp_time = 0, std_comp = 0;
    double avg_comp = bench_comp(comp_path, queries, query_count, &max_comp, &decomp_time, &std_comp);
    take_cpu_snapshot(&cpu_end);
    double cpu_comp = calc_cpu_percent(&cpu_start, &cpu_end);
    double ram_comp = get_ram_mb();
    long long size_comp = get_file_size(comp_path);
    char s_comp[32]; format_size(size_comp, s_comp, sizeof(s_comp));

    double ratio = get_compression_ratio(raw_path, comp_path);

    //Sonuçları Yazdırma
    fprintf(fout, "+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+\n");
    fprintf(fout, "| Method          | Disk Size       | Avg Search (us) | Std Dev (us)    | Max Search (us) | CPU Usage       | RAM Usage       |\n");
    fprintf(fout, "+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+\n");
    
    fprintf(fout, "| Raw Storage     | %-15s | %-15.2f | %-15.2f | %-15.2f | %%%-14.2f | %-6.2f MB       |\n", 
            s_raw, avg_raw, std_raw, max_raw, cpu_raw, ram_raw);
            
    fprintf(fout, "| Hashed Storage  | %-15s | %-15.2f | %-15.2f | %-15.2f | %%%-14.2f | %-6.2f MB       |\n", 
            s_hash, avg_hash, std_hash, max_hash, cpu_hash, ram_hash);
            
    fprintf(fout, "| Compressed      | %-15s | %-15.2f | %-15.2f | %-15.2f | %%%-14.2f | %-6.2f MB       |\n", 
            s_comp, avg_comp, std_comp, max_comp, cpu_comp, ram_comp);
            
    fprintf(fout, "+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+\n\n");

    fprintf(fout, "ADDITIONAL METRICS:\n");
    fprintf(fout, " - Compression Ratio (Compressed / Raw): %.4f\n", ratio);
    fprintf(fout, " - Decompression Time: %.2f ms\n\n", decomp_time / 1000.0);

    fprintf(fout, "EVALUATION AND COMPARISON:\n");
    fprintf(fout, "1. SPEED (Search Time): Hashed Storage provides the fastest direct lookup (avg %u us) because it uses\n", (unsigned int)avg_hash);
    fprintf(fout, "   Binary Search without loading the whole file. ");
    
    if (avg_comp < avg_hash) {
        fprintf(fout, "While Compressed Storage might seem faster (avg %u us),\n", (unsigned int)avg_comp);
        fprintf(fout, "   this is only because it searches entirely in RAM *after* a massive %.2f ms decompression penalty.\n", decomp_time / 1000.0);
    } else {
        fprintf(fout, "Compressed Storage (avg %u us) also suffers from a massive\n", (unsigned int)avg_comp);
        fprintf(fout, "   %.2f ms decompression penalty before it can even begin searching.\n", decomp_time / 1000.0);
    }
    
    fprintf(fout, "   Raw Storage is the slowest method due to its reliance on linear disk reading.\n\n");

    fprintf(fout, "2. DISK USAGE: Compressed Storage occupies the least disk space (%s) thanks to the zlib algorithm.\n", s_comp);
    fprintf(fout, "   Hashed Storage is the most disadvantageous in terms of disk space (%s) due to the 32-character\n", s_hash);
    fprintf(fout, "   length of MD5 hashes.\n\n");
    
    fprintf(fout, "3. RAM AND CPU: Since the Compressed method requires decompression into memory before searching (%.2f ms),\n", decomp_time / 1000.0);
    fprintf(fout, "   it incurs additional time and RAM overhead, especially during the first run. While Raw storage reads directly\n");
    fprintf(fout, "   from the disk, Hashed storage requires instantaneous CPU processing power to calculate the MD5 hash before searching.\n");
    
    fclose(fout);
    printf("  -> Results saved to %s\\results_summary.txt\n", results_dir);
}