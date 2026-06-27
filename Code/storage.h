//storage.h: İşlenmiş veriyi Raw, Hashed ve Compressed formatlarında Storage klasörü altına kaydeden fonksiyonun arayüzüdür.

#ifndef STORAGE_H
#define STORAGE_H

int build_storage(const char *processed_path,  //temizlenmiş parolaların bulunduğu dosya
                  const char *raw_dir,         //düz metin kopyasının yazılacağı klasör
                  const char *hashed_dir,      //MD5 hash'lerinin yazılacağı klasör
                  const char *comp_dir);       //sıkıştırılmış dosyanın yazılacağı klasör

#endif