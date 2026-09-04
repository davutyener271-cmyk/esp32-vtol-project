# Autonomous VTOL Project

## Proje Amacı

ESP32 tabanlı bir VTOL uçak geliştirmek.
Uzun vadede uçuş verilerini Java ile geliştirilecek bir uygulamadan takip etmek
ve kontrollü otonom uçuş özellikleri geliştirmek.

## Mevcut Donanım

- ESP32
- MPU6050
- Servo motorlar
- Buzzer
- RGB led
- BMP280

## Şu Ana Kadar Yapılanlar

- ESP32 çalıştırıldı.
- MPU6050 bağlandı.
- MPU6050'den sensör verileri okunuyor.
- Eksen yönleri kontrol edildi.
- İvme ve Gyro değerleri birleştirilerek PID için işe yarar veriler elde edildi.

## Şu Anki Aşama

BMP280 kullanarak basınç ve irtifa verisi elde etmek.

## Sonraki Aşama

Kumanda alıcısı ile kumanda komut göndeebilmek.

## Daha Sonraki Aşamalar


1. Servo kontrolü
2. PID kodlamaya giriş
4. PID stabilizasyon ve tekli fırçasız motorlu düzenek ile test.
6. GPS
7. Telemetri
8. Java takip uygulaması
9. Otonom uçuş sistemi

## Proje Durumu

Bmo280 irtifa verisi üzerine geliştirmeler devam ediyor.
