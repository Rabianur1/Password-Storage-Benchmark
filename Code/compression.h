//compression.h: zlib uyumlu miniz kütüphanesi üzerine yazılmış sıkıştırma arayüzüdür.Tüm dosya belleğe alınarak tek seferde sıkıştırılır veya açılır.

#ifndef COMPRESSION_H
#define COMPRESSION_H

//Dosyayı sıkıştırıp output_path'e yazar. 0 başarı, -1 hata
int compress_file(const char *input_path, const char *output_path);

/*Sıkıştırılmış dosyayı belleğe açar.
out_buf: çağıran tarafından free edilmeli
out_size: açık verinin byte cinsinden boyutu döndürür: 0 başarı, -1 hata*/
int decompress_to_memory(const char *input_path, char **out_buf, long long *out_size);

//compressed_size / original_size oranını hesaplar, 1'den küçük olması iyidir.
double get_compression_ratio(const char *original_path, const char *compressed_path);
#endif 