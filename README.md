# Password-Storage-Benchmark

Bu proje, parolaların farklı yöntemlerle saklanmasının performans üzerindeki etkilerini ölçmek ve karşılaştırmak için geliştirilmiş bir **File Organization (Dosya Organizasyonu)** uygulamasıdır. Proje C dili ile yazılmıştır.

## Proje Ne Yapıyor?

Program çalıştığında sırasıyla şu 3 ana adımı gerçekleştirir:

1. **Veri Ön İşleme (Preprocessing):** `RawData` klasöründeki ham parola listelerini okur, tekrar eden ve geçersiz verileri temizleyerek `Processed` klasörüne yazar.
2. **Farklı Formatlarda Saklama (Storage):** İşlenmiş parolaları 3 farklı yöntemle `Storage` klasörüne kaydeder:
   - **Raw:** Düz metin olarak (`passwords.txt`)
   - **Hashed:** MD5 şifreleme algoritması ile hash'lenerek (`passwords_md5.txt`)
   - **Compressed:** Zlib kullanılarak sıkıştırılmış formatta (`passwords.zlib`)
3. **Performans Testi (Benchmarking):** İşlenmiş veriler arasından rastgele 100 parola seçer ve bu parolaları 3 farklı depolama formatında arayarak arama sürelerini ölçer. Sonuçlar `Results/results_summary.txt` dosyasına kaydedilir.

## Proje Yapısı

* **Code/**: Projenin C kaynak kodlarını (`.c`, `.h`), `Makefile` ve dış kütüphaneleri (md5, miniz) içerir.
* **RawData/**: İşlenecek olan ham parola veri setlerinin bulunduğu klasör.
* **Processed/**: Temizlenmiş ve işlenmiş parolaların tutulduğu klasör.
* **Storage/**: Parolaların 3 farklı formatta (Raw, Hashed, Compressed) saklandığı dizin.
* **Results/**: Arama ve performans testi sonuçlarının kaydedildiği klasör.
* **Report/**: Proje raporu (`Report.pdf`).

## Nasıl Çalıştırılır?

1. Projeyi derlemek için `Code` klasörü içindeki `Makefile` veya `compile.bat` dosyasını kullanabilirsiniz.
2. Derleme sonrası oluşan çalıştırılabilir dosyayı (`Project.exe`) ana dizinde çalıştırın.
3. Test sonuçlarını incelemek için işlem bittikten sonra `Results/results_summary.txt` dosyasına göz atın.
