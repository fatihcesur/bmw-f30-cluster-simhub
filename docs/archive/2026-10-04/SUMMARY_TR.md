# 04.10.2026 oturumu - toplanan bilgiler

Bu klasör, 4 Ekim 2026 gecesi ve gündüzü yapılan tezgâh çalışmalarının yedeğidir.
Ayrıntılı teknik notlar: `docs/RESEARCH.md`, `CLAUDE.md`. Ham kamera kareleri (~350 MB)
`tools/cc_scanner/captures/` altında kalıyor (git'e dahil değil).

## 1. Yakıt göstergesi - ÇÖZÜLDÜ (tezgâhta doğrulandı, oyunda denenmedi)

Sorun: ETS2'de yakıt alınca ibre çok yavaş doluyordu.
Neden: ETS2 depoyu kademeli dolduruyor. Gösterge, yakıt hâlâ artarken "uyandırılırsa"
(7 sn CAN sessizliği) ibreyi oynatmıyor. Eski kod dolumun ortasında uyandırıyordu.
Düzeltme (`firmware/bmw_f30_cluster/SHCustomProtocol.h`, commit 38b4d27):
- Uyandırma, yakıt 2,5 sn sabit kaldıktan sonra (dolum bitince) yapılıyor.
- Hız > 2 km/h olursa sessizlik hemen bitiyor (sessizlikte yola çıkmak l/100km ibresini
  20'de kilitliyor ve "Şanzıman. Dikkatli sürün" uyarısı çıkarıyordu; 12V kesmek gerekti).
Kanıt: `fuel_test/` (refuel1-5 özet kareleri; 4 = kontak açık, 5 = kontak kapalı, ikisi de doğru).
Pratikte: yakıt aldıktan sonra kontağı açıp birkaç saniye bekle.

## 2. Hız limiti tabelası (SLI) - BULUNAMADI (7 tur tarama)

Göstergenin yazılımında SLI var (check-control 521-530, yuvarlak tabela simgeli
"Hız sınırı bilgisi şu anda mevcut değil"). Mesaj kimliği internette bulunamadı.
`tools/cc_scanner/slihunt.py` ile her CAN kimliğine periyodik çerçeve gönderilip kamerayla bakıldı:

| Tur | Aralık | Veri | Hız |
|---|---|---|---|
| 1 | 0A0-4FF | 50 x8 | 0 |
| 2 | 0A0-4FF | 10 x8 | 50 |
| 3 | 001-5FF | 50 x8 + sayaç | 50 |
| 4 | 001-5FF | 50 00.. + sayaç | 50 |
| 5 | 001-5FF | 00 F0 50 00.. (F-serisi düzeni) | 50 |
| 6 | 001-5FF | 00 F0 10 10.. | 50 |
| 7 | 001-248 (yarıda) | 3 desen: her yer / 3. bayt / 4. bayt = 50 | 50 |

Sonuç: tabela çıkmadı. Kalan ihtimaller: CRC gerekiyor, iki mesaj birlikte gerekiyor
(iDrive "göster" ayarı + veri), ya da gösterge SLI için kodlanmamış.
En iyi yol: SLI'lı gerçek bir F30'un CAN kaydı veya hatta gerçek bir NBT ünitesi.
Kayıtlar: `sli_scan/logs/`, görüntüler: `sli_scan/images/`.

Yan bulgular:
- **0x2A5** (ortam ışığı, BRIG_SURR): MID yazılarını turuncudan beyaza çeviriyor.
  Temayı farlardan bağımsız seçmek için kullanılabilir.
- Gösterge kendi parlaklığını oda ışığına göre otomatik ayarlıyor (0x35A/0x3AF/0x507
  "eşleşmeleri" bundan; 0x35A tek başına denendi, etkisi yok).
- Tarama sırasında MID'de geçici bir uyarı üçgeni çıktı (kaynağı belirlenemedi).

## 3. Göstergenin kimliği (UDS ile okundu, hiçbir şey yazılmadı)

`tools/cc_scanner/uds.py` - istek 0x6F1 (byte0 = 0x60 KOMBI), cevap 0x660.
- VIN: WBA3D31xxxxxxxxxx (gizlendi) (2012 F30)
- Seri no: (gizlendi), SGBD indeksi 0F 16 30
- Yazılım (SVK), programlama tarihi 21.09.2013:
  HWEL 088B 2.47.4, HWAP 0140, HWAP 0CE5, HWEL 0B09 2.47.0, BTLD 0E4E 7.51.2,
  SWFL 0E4F 7.55.100, SWFL 0E50 7.55.8, SWFL 0E52 7.55.2, SWFL 0E51 7.55.7,
  **CAFD 0760 7.0.29** (kodlama dosyası)
- Uyanırken gösterge kendiliğinden 0xF0 adresine `62 17 04 ...` (24 bayt) gönderiyor.

## 4. Işık / M Performance

- Kadran ışığı turuncu LED'lerle; renk yazılımla değişmez (beyaz için LED değişimi gerekir).
- MID yazıları beyaz olabiliyor (0x2A5, 0x21A).
- "M Performance" yazısı KOMBI kodlamasıyla açılıyor. Tezgâhta yazmak için CAFD 0760'ın
  bit haritası (E-Sys PSdZData) gerekiyor. Önce kodlama yedeği alınmalı (henüz alınmadı).

## 5. Diğer

- Kamera: Brio 500, indeks 1, görüntü 180° ters. Creative kamera bilgisayarda görünmüyor.
- `0x39E` saat ayarı oyunda çalışıyor (gösterge PC saatini gösterdi).
- Arduino'da şu an **tarama firmware'i** yüklü. Oyundan önce `firmware/bmw_f30_cluster` geri yüklenmeli.
- Megane 4 + BMW tek STM32 değerlendirmesi: mümkün, iki MCP2515 (ayrı hat),
  Blue Pill flash'ı yetmeyebilir, SimHub için iki sanal COM port gerekir.
