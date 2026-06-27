//preprocess.h: RawData klasöründeki ham parola dosyalarını okuyup temizler ve Processed klasörüne kaydeder.

#ifndef PREPROCESS_H
#define PREPROCESS_H

int preprocess_rawdata(const char *rawdata_dir,  //ham txt dosyalarının bulunduğu klasördür.
                       const char *output_file);  //temizlenmiş ve birleştirilmiş çıktı dosyasının yolu döndürür.
#endif