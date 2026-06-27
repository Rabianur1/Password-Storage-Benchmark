//utils.h: Proje genelinde kullanılan yardımcı fonksiyonların bildirimleri. Zaman, bellek, CPU ölçümü ve dosya boyutu işlemleri burada tanımlanıyor.

#ifndef UTILS_H
#define UTILS_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>

//CPU kullanımını hesaplamak için iki anlık görüntü arasındaki farkı kullanıyoruz.
typedef struct {
    FILETIME proc_kernel;  //kernel modda harcanan süre
    FILETIME proc_user;    //user modda harcanan süre 
    LARGE_INTEGER wall;    //duvar saati                
} CpuSnapshot;

long long get_file_size(const char *path);  //Dosya boyutunu byte cinsinden verir.
double get_time_us(void);                   //Şu anki zamanı mikrosaniye olarak verir.   
double get_ram_mb(void);                    //Ne kadar RAM kullanıldığını hesaplar(MB).
void take_cpu_snapshot(CpuSnapshot *snap);  //CPU ölçümü için anlık durumu kaydeder.
double calc_cpu_percent(const CpuSnapshot *start, const CpuSnapshot *end);  //İki zaman arasındaki CPU kullanım yüzdesini hesaplar.
void format_size(long long bytes, char *out_str, size_t max_len);  //Dosya boyutunu okunaklı yazar.
#endif