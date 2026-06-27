//utils.c: Proje genelinde kullanılan yardımcı fonksiyonlardır.Zaman ölçümü için QueryPerformanceCounter, bellek kullanımı için GetProcessMemoryInfo, CPU için GetProcessTimes kullanılıyor.

#include "utils.h"
#include <stdio.h>
#include <psapi.h>

//İşlemci frekansı program boyunca değişmez,başta bir kez alıp saklanır.
static LARGE_INTEGER g_freq = {0};

static void init_freq(void){
    if(g_freq.QuadPart == 0)
        QueryPerformanceFrequency(&g_freq);
}

//Mikrosaniye cinsinden anlık zamanı döndürür.
double get_time_us(void){
    init_freq();
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1e6 / (double)g_freq.QuadPart;
}

//Verilen dosyanın byte cinsinden boyutunu döndürür.
long long get_file_size(const char *path){
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h == INVALID_HANDLE_VALUE) return -1LL;
    LARGE_INTEGER sz = {0};
    BOOL ok = GetFileSizeEx(h, &sz);
    CloseHandle(h);
    return ok ? (long long)sz.QuadPart : -1LL;
}

//Byte değerini okunabilir formata çevirir.
void format_size(long long bytes, char *buf, size_t buflen){
    if(bytes < 0)
        snprintf(buf, buflen, "N/A");
    else if(bytes < 1024LL)
        snprintf(buf, buflen, "%lld B", bytes);
    else if(bytes < 1024LL * 1024)
        snprintf(buf, buflen, "%.2f KB", bytes / 1024.0);
    else
        snprintf(buf, buflen, "%.2f MB", bytes / (1024.0 * 1024.0));
}

//Programın şu an kullandığı RAM miktarını MB olarak döndürür.
double get_ram_mb(void){
    PROCESS_MEMORY_COUNTERS pmc;
    pmc.cb = sizeof(pmc);
    if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        return (double)pmc.WorkingSetSize / (1024.0 * 1024.0);
    return 0.0;
}

//CPU kullanım hesabı için o anki işlem zamanını ve duvar saatini kaydeder.
void take_cpu_snapshot(CpuSnapshot *s){
    FILETIME creation, exit_t;
    GetProcessTimes(GetCurrentProcess(), &creation, &exit_t,
                    &s->proc_kernel, &s->proc_user);
    QueryPerformanceCounter(&s->wall);
}

//FILETIME yapısını 64-bit tam sayıya çevirir.
static unsigned long long ft64(FILETIME ft){
    return ((unsigned long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

//İki anlık görüntü arasında işlemcinin yüzde kaç kullanıldığını hesaplar.
double calc_cpu_percent(const CpuSnapshot *before, const CpuSnapshot *after){
    init_freq();

    //Kernel + user modunda harcanan toplam CPU süresi farkı
    unsigned long long proc_delta =
        (ft64(after->proc_kernel) - ft64(before->proc_kernel)) +
        (ft64(after->proc_user)   - ft64(before->proc_user));

    long long wall_ticks = after->wall.QuadPart - before->wall.QuadPart;
    if(wall_ticks <= 0) return 0.0;

    //Duvar saati farkını 100 ns birimine çevir.
    unsigned long long wall_100ns = (unsigned long long)(
        (double)wall_ticks / (double)g_freq.QuadPart * 1e7);
    if(wall_100ns == 0) return 0.0;

    //Çok çekirdekli sistemlerde çekirdek sayısına bölerek normalize eder.
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    double pct = (double)proc_delta /
                 (double)(wall_100ns * (unsigned long long)si.dwNumberOfProcessors) * 100.0;
    return (pct > 100.0) ? 100.0 : pct;
}