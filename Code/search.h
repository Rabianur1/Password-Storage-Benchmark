//search.h: Benchmark fonksiyonlarının dışarıya açılan arayüzüdür. pick_random_passwords test sorgularını seçer,run_benchmarks ise üç format üzerinde ölçüm yapar.

#ifndef SEARCH_H
#define SEARCH_H

//processed dosyasından rastgele count kadar parola seçer ve dizi olarak döndürür.
char **pick_random_passwords(const char *processed_path, int count);

//Testleri çalıştırır ve sonuçları dosyaya kaydeder.
void run_benchmarks(const char *raw_path,
                    const char *hash_path,
                    const char *comp_path,
                    const char *results_dir,
                    char **queries, int query_count);
#endif