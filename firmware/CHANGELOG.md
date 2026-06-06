# Changelog

## 0.0.1

- Rinominato il progetto in NeuroHearh32.
- Configurata la build PlatformIO per ESP32-C3 SuperMini.
- Aggiunto logger centralizzato tramite `LoggerInterface` e `SerialLogger`.
- Estratta la gestione OLED in `OledMonitor`.
- Configurati i pin OLED da `main.cpp`: SDA su GPIO5 e SCL su GPIO6.
- Aggiunto logging selettivo per modulo.
- Estratti i moduli architetturali per WiFi, MQTT, menu ed EEG simulato.
- Definita l'interfaccia pluggable `EegProvider` con output comune `EegSample`.
- Rimossa la logica joystick.
- Portato il firmware in modalita OLED bring-up: solo OLED attivo.
- Aggiunta schermata OLED iniziale con versione firmware e pin del display.
- Aggiunto flag di logging `main` separato dal logging interno OLED.
- Aggiunti prefissi di canale alle righe seriali (`MAIN`, `OLED`, `WIFI`, `MQTT`, `MENU`).
- Aggiunto indirizzo I2C OLED nella configurazione esplicita (`0x3C`).
- Separato il begin secco dell'OLED dal metodo diagnostico `scanBus()`.
- Aggiunto `AppController` testabile per orchestrare il boot del firmware.
- Aggiunta suite test PlatformIO/Unity con ambiente `native`.
- Aggiunti test per logging seriale e ordine di inizializzazione applicativa.
