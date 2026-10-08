# Cluster research — overnight scan 2026-09-28

Method: `tools/cc_scanner/` firmware (copy of the real sketch, SimHub protocol replaced
by serial test commands) + `scan.py` driving it and photographing the cluster with the
webcam. Each `0x5c0` check-control code is forced ON alone (`{0x40, lo, hi, 0x29, FF…}`),
photographed, then OFF (`0x28`). Speed is 0 during the scan. Photos:
`tools/cc_scanner/captures/` (`mid/` zoomed MID per hit, `sheets/` contact sheets,
`scan_log.csv` = code, changed pixels, hit, settle seconds, time).

Note: the code field is 16 bit (byte1 low, byte2 high), so codes > 255 exist.
Codes 0–49 were scanned twice; the second pass (rolling reference, locked exposure)
confirmed every code listed below and showed 23/26/44/46–49 were false hits.

## 0x5c0 check-control codes (Turkish cluster texts)

| Code | Hex | Shows |
|---|---|---|
| 1, 2 | 0x01–02 | Aktif hız kontrolü. Mesafeye dikkat. |
| 3 | 0x03 | Aktif hız kontrolü. Mesafe bırakın. |
| 4, 5 | 0x04–05 | Römork, sol park ışığı |
| 6, 7 | 0x06–07 | Römork. Sinyal lambası arızalı |
| 8 | 0x08 | Römork. Fren lambası arızalı |
| 9 | 0x09 | Römork, arka sis lambası arızalı |
| 10, 12 | 0x0A, 0x0C | Yürüyen aksam. Dikkatli sürün (yellow) |
| 11 | 0x0B | Yürüyen aksam. Dikkatlice durun (red) |
| 14–17 | 0x0E–0x11 | Kapı açık (car picture shows which door) |
| 18 | 0x12 | Motor kaputu açık |
| 19 | 0x13 | Bagaj bölümü açık |
| 21 | 0x15 | Marş motoru/kontak |
| 22 | 0x16 | Marş motoru. Motoru durdurmayın |
| 24 | 0x18 | Fren sistemi. Dikkatli sürün (yellow) — used for ETS2 air pressure < 80 |
| 25 | 0x19 | Ön ısıtma. Lütfen bekleyin (glow plug) |
| 28 | 0x1C | Motor yağı seviyesi. Motor yağı ilave edin |
| 29 | 0x1D | **Tahrik. Dikkatli sürün** (drivetrain malfunction) |
| 32 | 0x20 | Yakıt deposu kapağı açık |
| 34 | 0x22 | **Yellow engine-shaped MIL lamp** (no text) |
| 35 | 0x23 | Sürüş stabilizasyonu. Dikkatli sürün |
| 36 | 0x24 | DSC devre dışı + DSC OFF lamp |
| 37 | 0x25 | Tetikleme… |
| 38 | 0x26 | Uzaktan kumanda tanınmadı |
| 39 | 0x27 | Motor aşırı ısınmış. Dikkatlice durun |
| 40 | 0x28 | Motoru çalıştırmak için frene basın |
| 41 | 0x29 | Servis gerekli |
| 42 | 0x2A | Fren sistemi. Dikkatli sürün + red brake lamp |
| 43, 45 | 0x2B, 0x2D | Yürüyen aksam. Dikkatli sürün |

| 50 | 0x32 | Lastik hasarı göstergesi arızalı |
| 52 | 0x34 | PARK. Aracın kaymaması için önlem alın |
| 54 | 0x36 | PARK. Park freni kısıtlı olarak kullanılabilir |
| 55 | 0x37 | PARK. Park frenini çözün |
| 60 | 0x3C | Gösterge paneli |
| 62, 78 | 0x3E, 0x4E | **Hız uyarısı** |
| 63 | 0x3F | Lastik basınç kaybı. Dikkatli durun |
| 66 | 0x42 | Uz. kumanda yok. Motor çalıştırılamaz |
| 67 | 0x43 | Uzaktan kumanda. Pili değiştirin |
| 68 | 0x44 | Bağımsız fonksiyonlar için uzaktan kumanda |
| 69, 85 | 0x45, 0x55 | Aktif hız kontrolü (mesafe) |
| 70 | 0x46 | Direksiyon. Dikkatli sürün + yellow lamp |
| 71 | 0x47 | Fren sistemi. Dikkatli sürün + red lamp (was mislabeled "seatbelt") |
| 74 | 0x4A | Fren sistemi. Dikkatlice durun + red lamp |
| 75 | 0x4B | Römork bağlantısı elektriği |
| 77 | 0x4D | Red seatbelt lamp (no text) |
| 79 | 0x4F | Dış sıcaklık (ice warning) |
| 87 | 0x57 | Sağ arka lamba arızalı |
| 88, 89 | 0x58, 0x59 | Sol / sağ kısa huzmeli far arızalı |
| 90 | 0x5A | Römork. Geri vites ışığı arızalı |
| 91 | 0x5B | Emniyet kemerini takın |
| 94, 95 | 0x5E, 0x5F | Arka emniyet sistemi + airbag lamp |
| 97 | 0x61 | Emniyet sistemleri + airbag lamps |
| 103 | 0x67 | Şanzıman çok ısınmış |
| 104, 105 | 0x68, 0x69 | Şanzıman sıcaklığı. Dikkatlice durun (red) |
| 108 / 109 | 0x6C / 0x6D | Sürücü / Ön yolcu emniyet sistemi + airbag lamp |
| 113 | 0x71 | Park ışığı açık |
| 114, 129 | | Arka sis lambası arızalı |
| 115, 117 | | Geri vites lambası arızalı |
| 116 / 125 | | Sol / sağ arka sinyal lambası arızalı |
| 119 / 122 | | Sağ / sol ön sinyal lambası arızalı |
| 118, 123, 133 | | Sağ / sol arka lamba arızalı |
| 120 / 121 | | Sol / sağ kısa huzmeli far arızalı |
| 126, 138 | | Sis farı arızalı |
| 128 / 130 | | Sol / sağ uzun far arızalı |
| 131, 132 | | Park ışığı arızalı |
| 134 / 136 | | Sağ / sol fren lambası arızalı |
| 139–141, 143, 147 | 0x8B–0x93 | Lastik basınç kaybı. Dikkatli durun (car picture marks the wheel) |
| 142 | 0x8E | Lastik Bas. Kontr. edin. RDC başl. ayarını yapın |
| 144 | 0x90 | Lastik Basıncı Kontrolü arızalı |
| 145, 149 | 0x91, 0x95 | Lastik Basıncı Kontrolü devre dışı |
| 148 | 0x94 | Fren lambası kontrol sistemi arızalı |
| 150 | 0x96 | Bagaj bölümünün zırhlı kapısı açık |
| 151 | 0x97 | Gaz alarmı! Hava sistemi açık |
| 152 | 0x98 | Akü takviye kablosunu ayırın |
| 153 | 0x99 | Silah mesnedi hatası |
| 154 | 0x9A | Göz yaşartıcı gaz sezici hatası |
| 156 / 157 | 0x9C / 0x9D | Hava sistemi çalışmıyor / Hava sistemi aktif |
| 159 | 0x9F | Kapılar kilitli değil! |
| 160 | 0xA0 | Cam krikosu acil fonksiyonu kullanın |
| 161 | 0xA1 | İkinci akü arızalı |
| 162 | 0xA2 | Flaş lambası, lambayı kontrol edin |
| 163 | 0xA3 | Yangın söndürme sistemi aktif |
| 164 | 0xA4 | **Yıkama sıvısı az** |
| 165 | 0xA5 | Dış sıcaklık (ice) |
| 166 | 0xA6 | **Soğutma sıvısı ekle** |
| 170 | 0xAA | **Motor arızalı** |
| 173, 179 | 0xAD, 0xB3 | **Şanzıman. Dikkatli sürün** |
| 174 | 0xAE | Vites P konumu sadece dururken mümkün |
| 175, 203 | 0xAF, 0xCB | Aracın kaymaması için önlem alın |
| 176, 177 | 0xB0, 0xB1 | Aktif hız kontrolü. Mesafe bırakın |
| 180, 204 | 0xB4, 0xCC | Yürüyen aksam. Dikkatli sürün |
| 181 | 0xB5 | Red airbag lamp (no text) |
| 183 | 0xB7 | Silecek |
| 184 | 0xB8 | Dinamik çekiş kontrolü (DTC) aktif + lamp |
| 185 | 0xB9 | Vites göstergesi |
| 186 | 0xBA | Direksiyon kilidi arızalı |
| 192 | 0xC0 | RDC sıfırlama yürütülüyor… |
| 193 | 0xC1 | Acil çıkış hatası |
| 195 | 0xC3 | Park desteği. Kendiniz park ediniz |
| 201 | 0xC9 | Automatic Hold. Kendiniz frenleyin |
| 206 | 0xCE | Bir sonraki tuşa basma motoru çalıştırır |
| 209 | 0xD1 | Uzaktan kumanda aracın içinde |
| 210 | 0xD2 | Park freni arızalı |
| 212 | 0xD4 | **Motor yağı basıncı. Dikkatlice durun** (red) |
| 213 | 0xD5 | Akü şarj edilemiyor |
| 215 | 0xD7 | nothing visible (the reference project: "DSC") |
| 216 | 0xD8 | **Yakıt beslemesi. Dikkatli sürün** |
| 217 | 0xD9 | Uz. kumandayı direksiyon kolonuna tutun |
| 221 | 0xDD | Gaz alarmı. Yeri terk edin |
| 223 | 0xDF | Elektrik hatası. Dikkatli sürün |
| 224 | 0xE0 | İzolasyon hatası |
| 225 | 0xE1 | Ön camın kilidi açık |
| 226 | 0xE2 | Yangın söndürme sistemi hatası |
| 229 | 0xE5 | Akümülatör şarj edilmelidir |
| 231 | 0xE7 | Far sistemi. Dikkatlice durun |
| 232 | 0xE8 | Koruma sistemleri hatalı |
| 233 | 0xE9 | İletişim hatası |
| 234 | 0xEA | Saldırı alarm sistemi hatalı |
| 236 | 0xEC | Sürüş stabilizasyonu. Dikkatli sürün + lamp |
| 237 | 0xED | Sürüş stabilizasyonu. Dikkatli sürün |
| 239–243 | 0xEF–0xF3 | Park freni arızalı |
| 244, 250 | 0xF4, 0xFA | Vitese takmak için frene basın |
| 245 | 0xF5 | Yürüyen aksam. Dikkatli sürün |
| 257 | 0x101 | **Motor çok sıcak. Dikkatli sürün** |
| 259, 261 | 0x103, 0x105 | Sıkışmaya karşı koruma (elektr. cam) |
| 262 | 0x106 | Sıkışmaya karşı koruma (sürgülü tavan) |
| 265 | 0x109 | Lastik basıncını kontrol edin |
| 266 | 0x10A | Devrilmeye karşı koruma hatalı |
| 271 | 0x10F | Tavan! Daha yavaş sürün |
| 273 | 0x111 | Direksiyon. Dikkatli sürün + lamp |
| 275 | 0x113 | **Yakıt rezervi** |
| 277 | 0x115 | Aktif hız kontrolü. Mesafeye dikkat |
| 278 | 0x116 | **Düşük vites seçin** |
| 279 / 280 | 0x117 / 0x118 | Sürücü / Ön yolcu sırt dayama bölümü kilitli değil + red lamp |
| 281, 284, 285 | 0x119, 0x11C, 0x11D | **SERVICE + car picture** (like the startup service screen) |
| 286 | 0x11E | Menzil --- |
| 287 | 0x11F | **Debriyaj sıcaklığı. Dikkatli sürün** |
| 295 | 0x127 | Viraj farı arızalı |
| 299 | 0x12B | Acil çağrı sistem hatası |
| 301 | 0x12D | Sırt dayama bölümü denetimi |
| 303 | 0x12F | **Motoru çalıştırmak START için debriyaja basın** |
| 305 | 0x131 | Gerilim beslemesi |
| 320 | 0x140 | Tavan hareketi tamamlandı |
| 321 | 0x141 | Direksiyon. Dikkatli sürün + lamp |
| 326 | 0x146 | Aracın kayması için önlem alın |
| 327 | 0x147 | RDC sıfırlama yürütülüyor… |
| 328 | 0x148 | **Fren balatası aşınma göstergesi** |
| 330 / 333 | 0x14A / 0x14D | Eğim iniş kontrolü şu anda kullanılamaz / şu an devre dışı |
| 331 / 332 | 0x14B / 0x14C | Eğim iniş kontrolü (HDC) aktif / devre dışı |
| 335 | 0x14F | Kontak açık (START/STOP icon) |
| 337 | 0x151 | **Hız kontrolü arızalı** |
| 339–342 | 0x153–0x156 | **Aktif hız kontrolü devre dışı** |
| 345 / 346 | 0x159 / 0x15A | Sağ / sol fren/arka lamba arızalı |
| 347 | 0x15B | Sürüş kademesi değiştirilemez |
| 349 | 0x15D | Kapalı kontak sdc. vites P'deyken mümk. |
| 350, 351 | 0x15E, 0x15F | Sürüş stabilizasyonu. Dikkatli sürün |
| 354 | 0x162 | Kalkış yardımı devre dışı (red) |
| 356 | 0x164 | Gece Görüş Sistemi arızalı |
| 364 | 0x16C | RPA sıfırlama yürütülüyor… |
| 367 | 0x16F | **Tahrik. Dikkatli sürün** (temperature icon) |
| 369 | 0x171 | Sürüş stabilizasyonu. Dikkatli sürün + lamp |
| 370 | 0x172 | Fren sistemi. Dikkatli sürün + lamp |
| 371 | 0x173 | Plaka lambası arızalı |
| 372 / 373 | 0x174 / 0x175 | İki kademeli sol / sağ fren lambası arızalı |
| 374, 376 | 0x176, 0x178 | Kameralı asistan sistemi arızalı / hatalı |
| 377 | 0x179 | Hassasiyet ayarı değiştirildi |
| 378, 379 | 0x17A, 0x17B | Park ışığı/gündüz farı arızalı |
| 380 / 381 | 0x17C / 0x17D | Sol / sağ viraj farı arızalı |
| 389, 390 | 0x185, 0x186 | Emniyet kemerini takın + red lamp |
| 393 | 0x189 | Vites kolunu otomatik geçidine geri alın |
| 394 | 0x18A | Sürüş kademesi değiştirilemez |
| 395 | 0x18B | PARK. Aracın kaymaması için önlem alın |
| 397 | 0x18D | Start/Stop otomatiği fonksiyon arızalı |
| 400 | 0x190 | Motor yağı seviyesi çok yüksek |
| 402 | 0x192 | İlaveten ayak frenine basın |
| 403 | 0x193 | Römork, arka ve plaka lambası |
| 404 / 405 | 0x194 / 0x195 | Römork, sinyal/fren lambası, sol / sağ |
| 406 | 0x196 | **Römork, elektrikli freni kontrol edin** |
| 410 | 0x19A | PARK. Park freni kısıtlı olarak kullanılabilir |
| 414 | 0x19E | Arka koltuk kilitli değil + red lamp |
| 415 | 0x19F | Akümülatör. Duran araçta akü deşarjı |
| 416 | 0x1A0 | Bagaj bölümü ayrımı |
| 419, 420, 427, 448 | | Tahrik. Dikkatli sürün |
| 422 | 0x1A6 | Küçük bagaj kapağı |
| 424 / 425 | 0x1A8 / 0x1A9 | Geri vites kamerası: objektifi kontrol edin / arızalı |
| 428 | 0x1AC | Yan kamera arızalı |
| 430 | 0x1AE | Nakliye modu |
| 431 | 0x1AF | **Yüksek fren yüklenmesi** |
| 435 | 0x1B3 | Şeritten çıkma ikazı arızalı |
| 436 | 0x1B4 | Koltuk pozisyonu algılaması hatalı |
| 437 | 0x1B5 | Park ışığı devrede |
| 439 | 0x1B7 | PARK. Aracın kaymaması için önlem alın |
| 441 | 0x1B9 | Aktif hız kontrolü devre dışı |
| 442 | 0x1BA | Aracın kaymaması için önlem alın |
| 443 | 0x1BB | Aktif hız kontrolü. Kendiniz frenleyin |
| 444 | 0x1BC | Arka koltuk kemerini takın |
| 446 | 0x1BE | Launch Control aktif |
| 449 | 0x1C1 | **Römork tertibatı. Dikkatlice durun** |
| 450 | 0x1C2 | Start/Stop otomatiği. Fonksiyon devre dışı |
| 451, 488–490 | | Park asistanı. Park ediniz (488/489 with white lamp) |
| 452 | 0x1C4 | İlave olarak fren pedalına basın |
| 453 | 0x1C5 | Yakın mesafe sezicileri devre dışı |
| 454, 469, 478, 483 | | Aktif hız kontrolü devre dışı |
| 455 | 0x1C7 | Vites N konumunda, kontak açık |
| 456 | 0x1C8 | Kontak kapalı. Gerekirse P'ye takın |
| 457 | 0x1C9 | Römork. Aydınlatma arızalı |
| 458 | 0x1CA | Yaya koruma sistemi |
| 460 | 0x1CC | Akümülatörü değiştirin |
| 461 | 0x1CD | Akümülatör. Şarj durumu --- |
| 464 | 0x1D0 | Test-SW: Airbag-ECU cannot deploy |
| 466 | 0x1D2 | Gece görüş kamerası temizlenmelidir |
| 468, 487 | 0x1D4, 0x1E7 | Şeritten çıkma ikazı devre dışı / hatalı |
| 470–473 | | Kamera mesajları (dış ayna, kamera sistemi, yan görünüş, tampon) |
| 474 | 0x1DA | Sürüş stabilizasyonu. Dikkatli sürün |
| 475 | 0x1DB | Acil çağrı sınırlı |
| 476 / 477 | 0x1DC / 0x1DD | Şerit değiştirme ikazı arızalı / devre dışı |
| 479 | 0x1DF | Aktif hız kontrolü. Mesafe bırakın |
| 484 | 0x1E4 | Akümülatör OK |
| 491 | 0x1EB | Yürüyen aksam. Dikkatlice durun (red) |
| 495 | 0x1EF | Kaçış hızı kullanıldı! |
| 496–498 | 0x1F0–0x1F2 | Direksiyon. Dikkatli sürün + lamp |
| 499 | 0x1F3 | **Kar zincirleri. Hızı azaltın** |
| 501–504 | 0x1F5–0x1F8 | Direksiyon. Dikkatli sürün + lamp |
| 505 | 0x1F9 | Yürüyen aksam. Dikkatli sürün |
| 507 | 0x1FB | Tehlike! Araçta yüksek CO2 değeri! |
| 508 | 0x1FC | Flaş lambaları açık! |
| 509 | 0x1FD | Siren hazır! |
| 510 | 0x1FE | Kapılar kilitli değil! |
| 511 | 0x1FF | Dikkat! Koruma sistemleri etkin değil! |
| 512 | 0x200 | Fren sistemi. Dikkatli sürün |
| 516 / 519 | 0x204 / 0x207 | Tavan! Yalnızca araç dururken / Tavan! |
| 520 | 0x208 | SERVICE + car picture, "Mevcut değil" |
| 521–525 | 0x209–0x20D | Hız sınırı bilgisi şu anda mevcut değil (different sign icons: round, square, white, yellow, gauge) |
| 526–530 | 0x20E–0x212 | Bu ülkede hız sınırı bilgisi yok (same icon variants) |
| 531 / 532 | 0x213 / 0x214 | Mesafe ayarı devre dışı / aktif |
| 533 | 0x215 | Sistem ---'dan itibaren kullanılabilir |
| 534 | 0x216 | Hız kontrolü etkinleştirilemiyor |
| 535, 544 | 0x217, 0x220 | **Tehlikeli trafik durumu!** |
| 536 | 0x218 | Şerit değiştirme ikazı devre dışı |
| 537 | 0x219 | Klima otomatiği |
| 545, 546 | 0x221, 0x222 | **Yol üzerinde nesneler var!** |
| 547, 548 | 0x223, 0x224 | **Karşı yönden gelen sürücü tehlikesi!** |
| 549, 550 | 0x225, 0x226 | **Trafik sıkışıklığı!** |
| 551, 552 | 0x227, 0x228 | **Yol kapalı!** |
| 553, 554 | 0x229, 0x22A | **Kaza!** |
| 555, 556 | 0x22B, 0x22C | **Yol üzerinde insanlar!** |
| 557, 565 | 0x22D, 0x235 | Aracın kaymaması için önlem alın / Vites P konumu sadece dururken mümkün |
| 558, 561, 562 | | Tavan kilidi mesajları |
| 563 | 0x233 | Havalandırma |
| 566 | 0x236 | Servis verilerinin güncelleştirilmesi |
| 567, 573 | 0x237, 0x23D | Tahrik. Dikkatli sürün |
| 568, 569 | 0x238, 0x239 | **Tahrik. Dikkatlice durun** (red) |
| 570 | 0x23A | Tahrik! Çok sınırlı menzil |
| 574 | 0x23E | RBS. Fren enerjisi geri kazanımı mümkün değil |
| 575 | 0x23F | Yakıt dolumu, servis hazırlığı oranı: ---% |
| 576 | 0x240 | **Yakıt dolumu sadece araç dururken** |
| 577 | 0x241 | **Yakıt dolumu mümkün** |
| 578 | 0x242 | Yakıt deposu dolum kapağı açık! |
| 579, 580 | 0x243, 0x244 | Yakıt sistemi |
| 581 | 0x245 | Yellow MIL-style lamp (no text) |
| 582, 598, 599 | | Şerit değiştirme ikazı arızalı / devre dışı |
| 583 | 0x247 | Eğim iniş kontrolü etkinleştirilemiyor |
| 584 | 0x248 | Nakliye modu |
| 585, 586 | 0x249, 0x24A | **Motor. Tekrar çalıştırılamaz** |
| 587 | 0x24B | Araç sürüşe hazır değil |
| 588–591 | 0x24C–0x24F | Şarj mesajları (kontağı açın, yüksek gerilim, sonlandırıldı, iptal edildi) |
| 592, 593 | 0x250, 0x251 | Aktif hız kontrolü (mesafe) |
| 594 / 595 | 0x252 / 0x253 | Çarpışma ikazı devre dışı / arızalı |
| 596, 597 | 0x254, 0x255 | Şeritten çıkma ikazı devre dışı / arızalı |
| 601–603 | 0x259–0x25B | Fren sistemi. Dikkatli sürün |
| 604, 628, 629, 638 | | Fren sistemi. Dikkatlice durun (red) |
| 608–611 | 0x260–0x263 | Lastik basınç kaybı. Dikkatli durun (per wheel) |
| 626 | 0x272 | Çarpışma ikazı devre dışı! |
| 627 | 0x273 | Direksiyon. Dikkatli sürün + lamp |
| 631, 632 | 0x277, 0x278 | **Hız kontrolü hazır** (white cruise icon) |
| 633–635 | 0x279–0x27B | Tahrik. Dikkatli sürün |
| 636 | 0x27C | Yüksek gerilim sistemi kapalı |
| 637 | 0x27D | Uzaktan diyagnoz aktif! |
| 646 | 0x286 | **AdBlue rezerv** |
| 647, 648 | 0x287, 0x288 | **AdBlue ilave edin. Menzil: ---** |
| 649 | 0x289 | AdBlue: Yanlış sıvı. Menzil: --- |
| 650 | 0x28A | AdBlue: Sistem arızalı. Menzil: --- |
| 652 | 0x28C | Uz. kumandayı direksiyon kolonuna tutun |
| 653 | 0x28D | **Uz. kumanda algılandı. Motor çalıştırılabilir** |
| 654 | 0x28E | Kemerinizi takın ve kapıyı kapatın |
| 655 | 0x28F | İlave olarak fren pedalına basın |
| 656 | 0x290 | Sistem ---'dan itibaren kullanılabilir |
| 657 | 0x291 | Sürüş stabilizasyonu. Dikkatli sürün |
| 659, 662 | 0x293, 0x296 | Direksiyon. Dikkatli sürün + lamp |
| 663 / 664 | 0x297 / 0x298 | Lastik Bas. Kontr. edin, RDC başl. / Lastik Basıncı Kontrolü arızalı |
| 665 | 0x299 | **AdBlue rezervi. Menzil: ---** |
| 666, 667 | 0x29A, 0x29B | **AdBlue ilave edin. Menzil: ---** |
| 668 | 0x29C | AdBlue yanlış sıvı |
| 669 | 0x29D | Kar zinciri ile kullanım |
| 685–687 | 0x2AD–0x2AF | Hibrit akümülatörü (şarj durumu / OK / değiştirin) |
| 688–690 | 0x2B0–0x2B2 | Şerit değiştirme / şeritten çıkma ikazı devre dışı |
| 698 | 0x2BA | Vites P konumu hatalı! |
| 703 | 0x2BF | Lütfen kalkış yapın! |
| 705 | 0x2C1 | Park freni. P'ye geçin |
| 706 | 0x2C2 | **Ayak frenine veya debriyaja basın** |
| 718 | 0x2CE | Motoru çalıştırmak mümkün değil |
| 719 | 0x2CF | **Motor çalıştırma. Start'a basın** |
| 720 | 0x2D0 | Aktif hız kontrolü. Mesafe bırakın |
| 726, 727 | 0x2D6, 0x2D7 | PARK. Park freni arızalı |
| 732 | 0x2DC | PARK. Aracın kaymaması için önlem alın |
| 733 | 0x2DD | Vites P konumunda değil! |
| 734 / 735 | 0x2DE / 0x2DF | Start/Stop otomatiği arızalı / devre dışı |
| 737 | 0x2E1 | Motor bölmesi sıcak. Açarken dikkat edin |
| 755 | 0x2F3 | Yellow lamp (no text) |
| 757 | 0x2F5 | Mesafe bilgisi arızalı |
| 758 | 0x2F6 | Yumuşak kapama otomatiği devre dışı! |
| 759 | 0x2F7 | Kapıların güvenlik fonksiyonu arızalı |
| 761 | 0x2F9 | **Sınırlı kalan sürüş zamanı --- dk.** |
| 762, 792, 795 | | Tahrik. Dikkatli sürün |
| 764 | 0x2FC | Test modu sürüş başlangıcında sona erer |
| 765, 783 | 0x2FD, 0x30F | Çarpışma ikazı sınırlı / görüş alanını kontrol edin |
| 766 | 0x2FE | **LIM Hız sınırlaması hazır** |
| 767 | 0x2FF | LIM Hız sınırlaması hatalı |
| 768 | 0x300 | **LIM Hız sınırı aşıldı!** |
| 769 | 0x301 | LIM Hız sınırı etkinleştirilemiyor |
| 771 | 0x303 | Kontak açık |
| 773 | 0x305 | Motor. Tekrar çalıştırılamaz |
| 775 | 0x307 | Bağımsız kalorifer/havalandırma devre dışı |
| 776 | 0x308 | Motor sadece rölantide çalışır |
| 777 | 0x309 | **Debriyajın soğuması beklenmelidir** |
| 780, 781 | 0x30C, 0x30D | **Arka aks kilitli diferansiyel** |
| 784, 785 | 0x310, 0x311 | Ön kamera |
| 786 | 0x312 | Yaya koruma sistemi |
| 789 | 0x315 | Sürüş stabiliz. başlan. ayar işlemi yürütül. |
| 790 | 0x316 | Emniyet sürgüsü. Sadece manevra için |
| 791 | 0x317 | Tahrik. Dikkatlice durun (red) |
| 800 | 0x320 | Tahrik. Dikkatli sürün |
| 801 | 0x321 | Fren sistemi. Dikkatli sürün |
| 802–809 | 0x322–0x329 | EV charging messages (şarj kablosu, şebeke gerilimi, menzil, P vites) |
| 810 | 0x32A | Motor arızalı |
| 811 | 0x32B | Motor kaputu açık. Dikkatlice durun (red) |
| 812 | 0x32C | Mesafe bilgisi mevcut değil |
| 813, 814 | 0x32D, 0x32E | Dynamic Light Spot arızalı |
| 815–818, 826, 827 | | Aydınlatma sistemi sol/sağ ön arızalı |
| 819 | 0x333 | Far sistemi. Dikkatlice durun |
| 820, 822 | 0x334, 0x336 | Arka spoyler |
| 824 | 0x338 | Aracı çalıştırmak için "P"yi seçin |
| 828, 834 | 0x33C, 0x342 | **Araç sürüşe hazır** |
| 829 | 0x33D | Aktif hız kontrolü. Mesafe bırakın |
| 830 | 0x33E | Nakliye modu |
| 831 | 0x33F | Sadece far şalteri "A" konumunda mümkün |
| 832 | 0x340 | Yüksek gerilim sistemi kapalı |
| 833 | 0x341 | Akümülatör şarj durumu kontrol edilmelidir |
| 835 | 0x343 | Park desteği. Kendiniz park ediniz |
| 839–841, 843 | | Trafik tıkanıklığı asistanı (devre dışı / mevcut / etkinleştirilemiyor) |
| 842 | 0x34A | **Ellerinizi direksiyondan ayırmayın** |
| 844 | 0x34C | Tahrik. Menzile dikkat edin |
| 845–847 | 0x34D–0x34F | AdBlue yanlış sıvı / Exhaust Fluid System / AdBlue sistemi |
| 849 | 0x351 | Yaya uyarısı devre dışı |
| 850, 851 | 0x352, 0x353 | Yaya uyarısı kısıtlı / görüş alanı kontrol edilmelidir |
| 853 | 0x355 | Yükleme yardımı. Dikkatlice durun |
| 854, 855 | 0x356, 0x357 | Toplam menzil --- |
| 857 | 0x359 | Kumanda eksik. Kapatma mümkün değil |
| 858 | 0x35A | **Römork tanımlandı. Manüel açınız** |
| 859, 864, 873–875, 884, 885, 887 | | EV charging messages |
| 862, 863 | 0x35E, 0x35F | Menzil --- |
| 868, 903 | 0x364, 0x387 | Park desteği. Frenleyin |
| 869 | 0x365 | **Tavan. Bagajda yük yanlış yerleştirilmiş** |
| 870, 871 | 0x366, 0x367 | Egzoz gazı ölçüm modu aktifleştirildi / mümkün değil |
| 872 | 0x368 | Aracı indirmek mümkün değil |
| 876 | 0x36C | Sürücü asistan sistemi. Mesafeye dikkat edin |
| 877 | 0x36D | Akustik yaya koruması devre dışı kaldı |
| 878 | 0x36E | Panoramik görüş. Objektifi kontrol edin |
| 879 | 0x36F | Mot. --- dakika sonra çalıştırılabilir |
| 880, 881 | 0x370, 0x371 | Bağımsız ısıtma/soğutma devre dışı |
| 882 | 0x372 | Aracın kaymaması için önlem alın |
| 883 | 0x373 | Park desteği. Kendiniz park ediniz |
| 886 | 0x376 | Hız kontrolü. Mesafeye dikkat edin |
| 888 | 0x378 | AutoPDC devre dışı |
| 890, 891 | 0x37A, 0x37B | Trafik tıkan. asistanı sdc. otoyollarda |
| 892, 893 | 0x37C, 0x37D | Önce hız kontrolünü aktifleştirin |
| 894, 895 | 0x37E, 0x37F | **Yağ incelmesi. Dikkatli sürün** |
| 904–999 | | nothing |
| 1000 | 0x3E8 | Motor yağı seviyesi. Motor yağı ilave edin |

Codes 1001–1632: nothing (scanned 2026-09-28 morning; every "hit" there was just
the now-running MID clock changing its minute digits). Codes above 1632 not scanned.
For future scans: the clock changes once a minute, so mask it like the info line.

## ETS2 mapping proposals (choose in the morning)

Property names follow the SCS telemetry layout SimHub exposes under
`DataCorePlugin.GameRawData.TruckValues…`; **verify each one live in SimHub's
Available Properties before wiring it** (wrong names silently read 0). Thresholds
are starting guesses. Messages should be shown briefly (a few seconds) unless they
describe a lasting state.

| ETS2 condition | Code | Cluster shows |
|---|---|---|
| Engine damage > ~30 % | 34 | yellow engine MIL lamp |
| Engine damage > ~60 % | 170 | Motor arızalı |
| Transmission damage > ~30 % | 29 / 173 | Tahrik / Şanzıman. Dikkatli sürün |
| Any damage > ~10 % | 41 | Servis gerekli |
| Any damage > ~10 % (at ignition on) | 281 | SERVICE + car picture |
| Wheel/tyre damage | 139–143 | Lastik basınç kaybı (per wheel) |
| Brake temperature high | 431 | Yüksek fren yüklenmesi |
| Air pressure < 80 (already wired) | 24 | Fren sistemi (yellow) |
| Air pressure emergency | 74 | Fren sistemi. Dikkatlice durun (red) |
| Fuel warning light | 275 | Yakıt rezervi |
| AdBlue warning / empty | 646 / 647 | AdBlue rezerv / AdBlue ilave edin |
| Oil pressure warning | 212 | Motor yağı basıncı. Dikkatlice durun |
| Water temperature warning | 39 / 257 | Motor aşırı ısınmış / Motor çok sıcak |
| Battery voltage warning | 213 | Akü şarj edilemiyor |
| Speed > speed limit | 768 | LIM Hız sınırı aşıldı! |
| Rest stop soon (fatigue) | 761 | Sınırlı kalan sürüş zamanı --- dk. |
| Differential lock on | 780 | Arka aks kilitli diferansiyel |
| Trailer just attached | 858 | Römork tanımlandı |
| Trailer damage | 449 | Römork tertibatı. Dikkatlice durun |
| Parking brake on while moving | 55 | PARK. Park frenini çözün |
| Ignition on, engine off | 719 | Motor çalıştırma. Start'a basın |
| Engine just started | 828 | Araç sürüşe hazır |
| Cruise control switched on / off | 631 / 339 | Hız kontrolü hazır / Aktif hız kontrolü devre dışı |
| Wipers on (optional) | 164 | Yıkama sıvısı az (only as an easter egg) |
| Hazard lights / hard braking | 535 | Tehlikeli trafik durumu! |
| Refuelling at a station | 577 | Yakıt dolumu mümkün |

## 0x21A lights byte0 (sweep 0..255, byte1 0x00)

| Bit | Effect |
|---|---|
| 0x02 | Blue high-beam icon |
| 0x04 | Green "lights on" icon **and MID switches to the night (red) text theme** |
| 0x20 | Green front fog icon |
| 0x40 | Yellow rear fog icon |
| 0x01, 0x08, 0x10, 0x80 | no visible effect |

There is no separate low-beam icon on this cluster: the green 0x04 icon is the
"lights on" indicator (same as a real F30). The night theme follows bit 0x04 only;
`0x02` alone (high beam without 0x04) keeps white text. Photos:
`captures/sweep_lights0/`, per-bit sheet `captures/bits_lights0.jpg`.

Byte1 of 0x21A (0..255) and byte1 of 0x202 (0..255) were swept too: the MID
theme never changed, so the night theme can't be separated from the green icon.
Firmware now masks 0x04 (`nightThemeWithLights = false`) and shows the front-fog
icon 0x20 instead (`lightsIconViaFog = true`) — fog bits keep the text white.

## Clock and date: 0x39E works

One frame `0x39E { hour, minute, second, day, (month<<4)|0x0F, year lo, year hi, 0xF2 }`
(no CRC, layout from E87/E90 captures) set the MID clock and the date page
(28.09.26) and removed the yellow triangle that had been on the MID all along —
that triangle was the cluster's "set date/time" reminder. The cluster keeps time
afterwards (survives Arduino resets). Firmware sends it once when SimHub PC time
arrives and every 10 min. `simhub/custom_protocol.txt` had year on line 5 instead of 6; fixed.
Photos: `captures/clock/`.

## Cruise set speed (0x289): not found

Swept bytes 1–6 of our 0x289 payload at 0 km/h and again at a fake 80 km/h /
2000 rpm with a clean MID. Findings: byte4 bit7 = cruise icon on/off (0x96 on);
byte6 = status (0 ok, 24–88 "Hız kontrolü arızalı", ≥96 icon off); byte1 36–44
show a small arc + "1"/"2" icon on the MID (probably ACC distance). No value
showed a set-speed marker or number. The set speed probably lives in another
message; would need sniffing a real car.

## Tach (0xF3): fixed

Swept 0–7000 rpm on camera. Old frame (RPM low byte in byte1, double send, CRC over
the whole frame / previous message's CRC): needle right at 500/2000/4000/6000 but
**0 at 1500/3000/3500/5000** — only some values happened to pass. Single frame with
proper CRC (seeds 0x7A and 0x2C) but no alive counter: rejected at every rpm.
Working frame: `{crc, 0x60|counter4Bit, rpm*1.557/256, 0xC0, eco, 0x00, 0xFF, 0xFF}`,
CRC `get_crc8(bytes1-7, 0x7A)`. Needle hits the printed marks at every rpm. byte3
does not add resolution (~164 rpm per byte2 step). Photos `captures/morning/tach*.jpg`.
Note: `simhub/custom_protocol.txt` multiplies RPM by 1.2, probably to compensate the old frame.

## Consumption / range: SOLVED (2026-09-28 afternoon)

**0x2C4 byte0 is the injected-fuel counter.** The cluster integrates its deltas
against 0x2BB distance for (a) the MID average consumption, (b) the range, and
(c) its own fuel-quantity estimate (fused with the 0x349 float sensor). Proof on
camera after a MENU long-press reset of the average: counter frozen -> 0.0 l/100km;
+1/s -> ~19 and falling; +5/s -> ~10–12. The old code sent `count` (+1 every Loop,
~70/s, wrapping at 0x77), i.e. far too much fuel: the average sat at the 29.5 display
cap and range was ~185 km on a full tank.

Calibration (scanner `fk` mode, same maths now in the real firmware): every 100 ms
`fuel += Speed * consX10 * 114 / 1e6`, byte0 = fuel mod 256. With consX10 = 100 the
MID average reads 8.8 at factor 100 and **10.0 at factor 114**, at both 40 and
80 km/h (speed-independent as expected). Photos `captures/drain/avgcal_*.jpg`.

customprotocol field 34 now sends ETS2 consumption scaled to a car tank:
`AverageConsumption(l/km) * 1000 * 57 / tank capacity` (l/100km ×10). Then the
cluster's average is a car-like number and its range = fuel ÷ consumption equals
ETS2's own range. **To confirm in game:** the property
`TruckValues.ConstantsValues.CapacityValues.Fuel` (fallback 400 L) and the 57 L
guess for the cluster's full tank (read 0x330 byte3 at full on a freshly
power-cycled cluster).

Side effects seen while calibrating: a high consumption drags the cluster's fuel
estimate down (range warning "Menzil 8 km" with 0x349 at 100 %); it recovers only
slowly. A 12V power cycle resets it.

The small l/100km needle never followed the counter on the bench (it sat at 0,
~5 or ~10 depending on the session); it may need a longer history. Low priority.

### Range behaviour (in-game + bench, 2026-09-28 afternoon)

- The MID **range is recomputed only when the cluster wakes** (ignition-on / after
  ≥6 s CAN silence) and held in between — during 10-min drives it stayed on one value
  even while the cluster's own fuel estimate changed. The firmware's 7 s wake on
  ignition-on therefore also refreshes the range.
- Range uses a separate, slow long-term consumption average (the MID trip average
  follows what we send within minutes; the range average took ~300 fake km).
  One-off teaching with `teach.py` (fake 250 km/h, distance ×2.7) moved it from
  ~21 to ~3.6 l/100km: range went 170 -> 296 -> 325 -> 498 -> 500 -> **1002 km**.
  (Commit bb605ae's message claims a ~7 l/100km floor — that was wrong; the value
  only shows up at the next wake.) The learned average is kept by the cluster.
- Distance: in game one Loop takes several hundred ms, so counters must scale with
  elapsed time (fixed in firmware).

- Correction: range is recomputed on the **ignition OFF -> ON transition** in 0x12F
  (e.g. an Arduino reset sends ignition-off frames until the host says "on"), not by
  CAN silence alone - 8 s and 30 s mutes with ignition kept "on" did not refresh it.
  In game every key cycle produces this transition.
- Tank constant: the cluster read back 43 L at 82 % via 0x330, i.e. ~52 L full.
  customprotocol field 34 uses 52 (57 and 35 were wrong guesses). Teaching 2.9 l/100km
  gave 1209 km at 37 L cluster fuel (avg 3.06).

- Tried refreshing range while driving with a short ignition-off blip in 0x12F only
  (300 ms and 1 s, everything else kept running): range did NOT refresh, and the
  cluster flashed warning lamps plus a "mevcut değil" MID message for ~5 s. Rejected
  by the user; not in the firmware. Range refreshes only on real key cycles.

- Range-while-driving hunt (`rangehunt.py`, fake 80 km/h, stale range on the MID):
  single-bit flips + 00/FF of 0x12F b2-b7, 0xF3 b3-b7, 0x3F9 b0-b7, 0x2C4 b2-b7
  never refreshed the range. Only 0x12F b2 (ignition state) changed the MID, via
  off/on cycles (SERVICE screen), not a live refresh. The live-range enable signal is
  probably in a frame we don't send at all (DME engine status etc.) -> needs a real
  F30 CAN log.

### Fuel gauge calibration (this cluster)

The reference project's `{0,50,100}% -> {37,18,4}` (different cluster series) was wrong. A first
calibration from the oblique webcam angle was also off (user checked head-on: 81.7 %
showed ~57 %). Redone with the camera facing the gauge, one wake per value
(`fuelcal.py`): the needle is non-linear in the raw value. Firmware table:
`{0,25,50,60,75,100} % -> {37,27,18,11,8,4}`; verified head-on at
0/10/25/40/50/60/70/75/82/90/100 %. Raw > 37 stays at empty; raw 0 is not "full".

### Fuel adaptation side effect (2026-09-28 evening)

After the range-teaching runs (~600 fake km, ~17 L of counter consumption while
0x349 stayed at 82 %) the cluster started showing the fuel level ~20-25 % too high,
persistent across a 12 V cycle, and it no longer goes below ~20 % (raw >= 38 all
show ~20 %). Most likely BMW's fuel-level adaptation (sensor vs. consumption) learned
a correction. The table was re-measured for this state:
`{0,20,25,37,50,60,75,80,92,100} % -> {45,40,35,29,23,17,14,12,10,8}`, verified at
25/50/75/81/100 %. Below 20 % cannot be displayed. If the cluster un-learns it in
normal play the table will drift again; re-run the head-on sweep then.
**Lesson: never fake consumption with a constant 0x349 level.** An "un-teach" (sensor
falling with zero consumption) was proposed but not tried.
Also: `tools/cc_scanner/fakesimhub.py` emulates SimHub's ArqSerial framing to drive
the real firmware from Python.

### SimHub "Invalid version" (2026-09-28 night)

With ~482 bytes free RAM the firmware stopped passing SimHub's detection ("Arduino
scan COMx ... Unrecognized, Invalid version" in SimHub/Logs/SimHub.txt), although a
Python emulation of the handshake still got answers. The 17:34 build (490 bytes free)
connected. Fixed by dropping the idle-fuel-burn experiment and moving the setup
Serial.println strings to flash (F()): 546 bytes free. **Keep free RAM above ~500
bytes.** SimHub also stops scanning a port after "Maximum connection attempts"
failures per session (default 5) - restart SimHub after fixing.

### Correction came back out (2026-09-29)

Next day, with the game tank full (raw 8), the needle showed ~76 % = 3/4, i.e. the
original head-on mapping again: the learned +20 % correction was gone (overnight power
off and/or rewiring). The original table `{0,25,50,60,75,100} -> {37,27,18,11,8,4}` is
back in the firmware. If the gauge drifts again: first check one known point (full
tank should be exactly 1) before re-measuring the whole scale.

## Fuel gauge speed

Bench measurements (camera time-lapses, `captures/fuelfill/`):
- Standstill: empty -> full in ~30 s.
- Driving: ~2°/min (heavy anti-slosh filter).
- After a CAN silence of ≥ 6 s (`v mute`) the cluster "wakes" and the needle jumps
  to the real level in < 5 s. A 3 s silence was not enough.
- Pushing 0x349 far past "empty" latched the gauge (frozen) until a long silence.
In ETS2 refuelling happens parked with the ignition off, then the truck drives off
at once -> the needle is still filtering heavily. Fix (implemented 2026-09-28,
reworked 2026-10-04): stay silent on CAN 7 s on ignition-on and after a refuel while
stopped. Silence while Speed > 0 latches the l/100km needle at 20.

Refuel replay on the bench (2026-10-04, `refueltest.py` + `fuelsheet.py`, fake SimHub
driving the real firmware, captures/refuel1-5):
- ETS2 fills gradually. A wake while the level is still rising does not move the
  needle; only a wake after the level stopped changing jumps it. The old trigger
  (+3 % -> mute at once) therefore usually woke mid-fill, and with the ignition kept
  on nothing else fixed it: the needle crawled at ~2 deg/min after driving off.
  Now the refuel wake waits for 2.5 s of steady fuel.
- With the ignition OFF a wake does not move the needle either; the ignition-on
  wake does (needle at full ~6 s after ignition-on).
- With the ignition ON and stopped, the needle followed the fill by itself (30 s ramp).
- Driving off during the silence pinned the l/100km needle at 20, left the speedo
  at 0 and raised "Sanziman. Dikkatli surun" (CC 173/179); it survived Arduino
  resets and needed a 12 V cycle. The silence now ends as soon as Speed > 2.
- After that, the main firmware with fake data still showed CC 173/179 while the
  scanner firmware did not. Not yet checked with SimHub + ETS2.

## Consumption / range: earlier dead ends

0x2C4 byte0 is a counter (`count`, +1 every Loop, wraps at 0x77). Tried replacing
it with a speed×consumption fuel counter (`fk` in the scanner firmware) and
sweeping bytes 1–6 at 80 km/h: the l/100km needle did not follow any of it in
5–60 s windows. During the sweeps the needle sat at 0, then jumped to 20 and
stayed there for every later setting (even ~zero fuel) — it looks latched, like
the average consumption stuck at 29.5. Needs a 12V power cycle and slow,
one-at-a-time tests with the user present. Range on the MID is computed by the
cluster (fuel ÷ its average), so it can only follow ETS2 once consumption is
controllable. The cluster broadcasts its range in 0x330 (output only).
Helpers: `sweep.py c2c4N`, `fueltest.py`, `needle.py` (needle angle from photos).
Morning retest (`fueltest2.py`) with a valid 0xF3 at 80 km/h / 2000 rpm: the needle
stayed at 0 for the old counter, fk 1/3000/30000, and 0xF3 bytes 3/4/5 at several
values. The two times it rose overnight (to ~10 and to 20) are not reproduced and
not explained. Earlier "2000 rpm" tests actually ran at 0 rpm (the scanner's
`v rpm` set RPM2, which Loop overwrites from RPM; fixed).

Morning, second round (all with a valid 0xF3 at 80 km/h / 2000 rpm):
- **The "needle at 20" is an error state, not consumption.** Reproduced: restarting
  the Arduino while the fake speed is > 0 (a few seconds of CAN silence while
  "moving") pins the needle at 20; it stays latched across further restarts and
  clears only after a long silence (a firmware upload). Both overnight "rises"
  were this. Keep speed at 0 before closing a test session.
- Tried with no needle movement: fine 16-bit fuel level in 0x349 drained at
  5–1000 raw units / 10 s; one-off jumps of the 0x2C4 byte0 counter and of the
  0x2BB distance counter; rising 16-bit counters in every 0x2C4 byte pair; static
  0x2C4 bytes 1–6 (00/40/80/C0/FF, 10 s each); E90 `0x1D0` with a rising counter in
  bytes 4–5; cruise on. Old vs new 0xF3 frame: no difference.
- After ~100 fake km with constant fuel, the cluster's average (29.5) and range
  never changed, so it doesn't derive consumption from fuel level or distance.
- The cluster may reject our whole 0x2C4 frame (wrong CRC seed or missing
  alive-counter pattern, like 0xF3 was). Brute-forced it (`seedscan.py`): byte1 =
  0xF0|counter, 0x60|counter and 0x10|counter, bytes 2–7 = 0xFF, every CRC seed
  0–255 (768 combinations, 3.5 s each) at 80 km/h / 2000 rpm: the needle never
  left 0. So either consumption is not in 0x2C4 or the layout differs again.
  Most reliable next step: log a real F30's CAN bus (or ask on the reference project's repo) for
  the message that carries fuel rate.
- **User report (2026-09-28 10:40): in ETS2 with the real firmware the needle DOES
  move.** So the real firmware already sends something the needle follows; the bench
  tests never reproduced it (constant or ramped speed/rpm did nothing). Leading
  hypothesis: 0x2C4 byte0 `count` (+1 per Loop, wraps at 0x77) is the fuel counter,
  and its rate is only plausible when Loop is slowed down by SimHub packet parsing;
  the scanner's fast Loop makes it implausible. Test in-game: watch the needle with
  the camera while noting throttle/rpm, and compare with a firmware that counts at a
  fixed rate (e.g. `count` per 100 ms scaled by ETS2 consumption).
- Useful tool: the cluster broadcasts **0x330** with its own numbers:
  bytes 3–5 fuel quantity (byte3 ≈ litres, falls when 0x349 falls), bytes 6–7 range
  ×16 little-endian (`5C 0D` = 213.75 km, matches the MID). `sniff.py` logs it, so
  range/fuel reactions can be read as numbers instead of from photos.
  0x2C5 (≈11 Hz, CRC + F0|counter + `2A F3 C6 70 C8`) is another cluster output.

## Side findings

- Cluster transmits: 660 1B3 328 362 336 338 205 2C5 2F7 2CA 560 35C 393 5E0 367 330.
  No 0x2F8 → it does not broadcast the clock; its own date shows **06.05.22**.
- A MENU press (`0x1EE`) cycles the MID info line: date, average speed, range (**442 km**).
- Webcam auto exposure must be locked (`CAP_PROP_AUTO_EXPOSURE 0.25`, exposure −5),
  otherwise every MID change re-balances the picture and looks like a change everywhere.
- The CH340 USB link dropped once mid-scan (port renumbered COM30 → COM8 after replug).

## Speed limit info (SLI) sign hunt (2026-10-04)

The cluster firmware knows SLI: CC 521-530 show "Hiz siniri bilgisi su anda mevcut
degil" with round sign icons. No CAN id for the sign was found online (forums paywalled,
not in superwofy's FXX DBC, not in his F11 525d traces - that car has no 8TH).
`slihunt.py` sends one id at a time every 100 ms (scanner `P` command) and diffs the
camera (MID clock / average / odometer lines masked):
- pass 1: ids 0x0A0-0x4FF, all bytes 0x50, speed 0
- pass 2: same ids, all bytes 0x10, fake 50 km/h
Ids we send and ids the cluster transmits were skipped. No sign appeared. Only hit:
**0x2A5 (BRIG_SURR, ambient brightness) switches the MID from the orange to the white
theme** - possibly a way to choose the theme independently of the 0x21A headlight bit.
0x181 scored just below the threshold in pass 1 (not looked at yet).
Remaining ideas: the message may need an alive counter + CRC, a specific layout, or the
KOMBI is not coded for SLI (8TH / SPEED_LIMIT=aktiv). A real F30 CAN log with SLI is
the reliable next step.

## KOMBI over UDS (2026-10-04, `tools/cc_scanner/uds.py`, read-only)

BMW extended addressing works on the bench: request on 0x6F1 (byte0 = 0x60 KOMBI, then
ISO-TP), answer on 0x660 (byte0 = 0xF1). Frames padded to 8 bytes. Rig.cmd() must not be
used for the request (it swallows the answer while waiting for "OK"); flow control with
STmin 80 ms, our own traffic muted for 3 s per request (`v mute 3000`) and up to 6
retries: the Nano drops consecutive frames otherwise (with STmin 10 ms it failed even muted).
On every wake the KOMBI itself pushes `62 17 04 ...` (24 bytes, entries E1 14 5C / 60 / 64)
to address 0xF0 and waits for flow control from 0x6F0.

- VIN (F190): WBA3D31xxxxxxxxxx (redacted) (2012 F30); serial (F18C) (redacted); SGBD index (F150) 0F 16 30
- SVK (F101), programmed 2013-09-21: HWEL 088B 2.47.4, HWAP 0140, HWAP 0CE5,
  HWEL 0B09 2.47.0, BTLD 0E4E 7.51.2, SWFL 0E4F 7.55.100, SWFL 0E50 7.55.8,
  SWFL 0E52 7.55.2, SWFL 0E51 7.55.7, **CAFD 0760 7.0.29** (coding data)

## "Sanziman. Dikkatli surun" (CC 173) and gear display (2026-10-04)

User in ETS2: the warning comes when shifting D -> N while moving; stopping first (with
brake) does not raise it. Bench (`shifttest.py`, fake SimHub): D at 30 km/h for 20 s after
an ignition cycle was clean once, then D -> N at 30 km/h raised it; at standstill
N/P -> D also raised it. Results are not fully repeatable (one run had it from the first
frame), so the KOMBI most likely checks gear changes against a brake-pedal signal we
don't send (shift lock); that message is unknown (not in superwofy's FXX DBC; 0x197
ST_GWS = lever position, 0x3FD DT_DISP_GRDT: byte2 bits 0-2 program, 5-7 position).
Firmware now: one continuous alive counter in 0x3FD byte0 low nibble (it used to jump
between three counters on every shift), N is shown as D while Speed > 3, plain D in
Eco Pro/Comfort/Traction and S1..S9 only in Sport/Sport+/DSC off, P = N + parking brake
+ stopped. KOMBI fault memory had ~25 entries (mostly E114xx message timeouts from the
day's CAN silences); cleared with `uds.py --clear-dtc 14FFFFFF`.
Next: find the brake pedal / brake light switch message (scan ids while doing P -> D).

## Refuel in game, round 2 (2026-10-04 evening)

`gamelog.py` (SimHub web API, localhost:8888) logged a real ETS2 refuel: ignition OFF and
engine off during the fill, fill ~5 s (48 -> 100 %), drive-off ~20 s later. Before that,
switching trucks made SimHub send fuel **0 for ~15 s** (loading screen) and again for 1 s.
The needle then sat at 1/2 after the refuel.
Bench replay (`switchtest.py`): the 0 -> 48 % jump makes the cluster mis-read the level
afterwards (full shows 1/2). Firmware now keeps the last good level when fuel drops to 0
from > 2 %. A refuel with the ignition off still did not move the needle on the bench
(switch2/3): the cluster, put to sleep in its ignition-off state, wakes on the old level.
Tried showing "ignition on" for 2 s before the refuel wake (switch3): no better, but by
then the bench cluster had drifted (48 % shown as ~1/4) - every fake drive with constant
fuel and `cons=29` feeds the cluster's fuel adaptation, so later bench results are not
comparable. Needs a 12 V cycle and an in-game test before more changes.
After the 12 V cycle (evening) the needle stayed at EMPTY for 48 % and 99 %, also after a
40 s CAN silence - same firmware that showed 99 % correctly earlier the same evening. Most
likely the cluster's fuel adaptation learned a large correction from the day's ~15 fake
refuel/drive replays (constant 0x349 while the 0x2C4 counter said fuel was being used).
In September such a correction survived a 12 V cycle and was gone the next day.
`fakesimhub.fields()` now defaults to cons=0 so replays stop feeding the adaptation.
0x330 readout right after (scanner, no driving, cons 0): sender 99 % -> cluster quantity
byte3 = 5 L (range 94 km); 48 % and 10 % -> 0 L. Bytes 4-5 do follow the sender. So the
needle is not stuck: the cluster's fused quantity is ~47 L below the sender. An ignition-off
refuel 10 -> 99 % followed by ignition-on did NOT reset it (still 5 L). 0x330 comes only
every ~5-10 s. Idea for a real fix: drive the 0x2C4 fuel counter from the actual ETS2 fuel
decrease (scaled to the 52 L car tank) instead of consumption x speed, so the cluster's
consumption integral always agrees with the sender and no correction is ever learned.

## Warning gong (2026-10-04)

The KOMBI's own buzzer is loud and clear on the Brio mic (turn-signal ticks peak ~0.41 vs
~0.01 room). Sound scan (`tools/cc_scanner/gongscan.py`) over 376 of the 581 known
check-control codes: no code reproducibly makes a sound; the few hits (104, 122, 224...)
did not repeat in quiet retests -> room noise. Matches what F30 owners report: the gong is
played by the head unit through the door speakers, the KOMBI only ticks. So the chime is
played on the PC instead: `tools/gong/gong.py` (SimHub web API; chime.wav = recorded BMW
warning chime, kept out of git) on fuel reserve and damage >= 20 / 40 %.
