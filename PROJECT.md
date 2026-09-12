# Autonomous VTOL Project

## Proje Amacı

ESP32 tabanlı bir VTOL uçak geliştirmek.
Uzun vadede uçuş verilerini Java ile geliştirilecek bir uygulamadan takip etmek
ve kontrollü otonom uçuş özellikleri geliştirmek.

## Mevcut Donanım

- ESP32
- LSM6DS3
- Servo motorlar
- Buzzer
- RGB led
- BMP280
- FlySky-i6x ve 10 kanallı alıcısı
- Fırçasız motor (geçici olarak A2212 1400KV)
- Esc (40a geçici)
- Voltaj düşürücü
- Lipo batarya (3s geçici)

## Şu Ana Kadar Yapılanlar

- ESP32 çalıştırıldı.
- MPU6050 bağlandı.
- MPU6050'den sensör verileri okunuyor.
- Eksen yönleri kontrol edildi.
- İvme ve Gyro değerleri birleştirilerek PID için işe yarar veriler elde edildi.
- BMP280 ile irtifa, sıcaklık ve basınç bilgileri elde edildi.
- Alıcı ile kumandadan komut gönderme, arm ve failsafe modları eklendi.
- İlkel seviyede servo kontrolleri eklendi.

## Şu Anki Aşama

...

## Sonraki Aşama

...

## Daha Sonraki Aşamalar

1. PID kodlamaya giriş
2. PID stabilizasyon ve tekli fırçasız motorlu düzenek ile test.
3. GPS
4. Telemetri
5. Java takip uygulaması
6. Otonom uçuş sistemi

## Proje Durumu

