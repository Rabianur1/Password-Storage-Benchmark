/*compression.c: Bir dosyayı miniz (zlib uyumlu) ile sıkıştırıp diske yazar.
Açma işleminde ise sıkıştırılmış veriyi belleğe çözer.
Orijinal dosya boyutunu .zlib dosyasının başına 8 byte olarak saklıyoruz çünkü açarken ne kadar bellek ayıracağımızı bilmemiz gerekiyor.*/

#include "compression.h"
#include "utils.h"
#include "libs/miniz.h"
#include <stdio.h>
#include <stdlib.h>

int compress_file(const char *input_path, const char *output_path){
    long long in_size = get_file_size(input_path);
    if(in_size <= 0) return -1;

    FILE *fin = fopen(input_path, "rb");
    if(!fin) return -1;

    //Dosyanın tamamını belleğe alır.
    unsigned char *in_buf = (unsigned char *)malloc((size_t)in_size);
    if(!in_buf) {fclose(fin); return -1;}

    if(fread(in_buf, 1, (size_t)in_size, fin) != (size_t)in_size){
        free(in_buf); fclose(fin); return -1;}
    fclose(fin);

    //Ne kadar yer kaplayacağını önceden hesaplıyoruz.
    unsigned long cmp_len = compressBound((unsigned long)in_size);
    unsigned char *cmp_buf = (unsigned char *)malloc(cmp_len);
    if(!cmp_buf){free(in_buf); return -1;}

    //Sıkıştırma işlemi
    int status = compress(cmp_buf, &cmp_len, in_buf, (unsigned long)in_size);
    free(in_buf);

    if(status != Z_OK){
        free(cmp_buf);
        return -1;}

    FILE *fout = fopen(output_path, "wb");
    if(!fout){free(cmp_buf); return -1;}

    //Açarken lazım olacağı için asıl dosya boyutunu en başa kaydediyoruz.
    unsigned long long orig_size = (unsigned long long)in_size;
    fwrite(&orig_size, sizeof(orig_size), 1, fout);
    fwrite(cmp_buf, 1, cmp_len, fout);

    fclose(fout);
    free(cmp_buf);
    return 0;
}

int decompress_to_memory(const char *input_path, char **out_buf, long long *out_size){
    long long file_size = get_file_size(input_path);
    if(file_size <= (long long)sizeof(unsigned long long)) return -1;

    FILE *fin = fopen(input_path, "rb");
    if(!fin) return -1;

    //İlk 8 byte'ta orijinal boyut saklı
    unsigned long long orig_size = 0;
    if(fread(&orig_size, sizeof(orig_size), 1, fin) != 1){
        fclose(fin); return -1;}

    //Geri kalanı sıkıştırılmış veri
    long long cmp_len = file_size - sizeof(unsigned long long);
    unsigned char *cmp_buf = (unsigned char *)malloc((size_t)cmp_len);
    if(!cmp_buf){fclose(fin); return -1;}

    if(fread(cmp_buf, 1, (size_t)cmp_len, fin) != (size_t)cmp_len){
        free(cmp_buf); fclose(fin); return -1;}
    fclose(fin);

    //Çözülmüş veriye alan açar ve çözer.
    unsigned char *uncomp_buf = (unsigned char *)malloc((size_t)orig_size + 1);
    if(!uncomp_buf){free(cmp_buf); return -1;}

    unsigned long uncomp_len = (unsigned long)orig_size;
    int status = uncompress(uncomp_buf, &uncomp_len, cmp_buf, (unsigned long)cmp_len);
    free(cmp_buf);

    if(status != Z_OK){
        free(uncomp_buf);
        return -1;}

    uncomp_buf[uncomp_len] = '\0';  //string işlevleri için sona null koyulur.
    *out_buf  = (char *)uncomp_buf;
    *out_size = (long long)uncomp_len;
    return 0;
}

//Sıkıştırma oranı
double get_compression_ratio(const char *original_path, const char *compressed_path){
    long long orig = get_file_size(original_path);
    long long comp = get_file_size(compressed_path);
    if(orig <= 0 || comp <= 0) return 0.0;
    return(double)comp / (double)orig;
}