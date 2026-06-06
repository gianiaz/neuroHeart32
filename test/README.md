# Test suite

Questa cartella contiene i test automatici del progetto.

## Tipi di test

- `native`: test eseguiti sul computer, senza ESP32 collegata. Sono adatti alla logica pura: configurazioni, stato menu, serializzazione dati, provider simulati.
- test su scheda: utili per driver e hardware reali, come OLED/I2C, bottoni, WiFi e MQTT. Questi non dovrebbero essere il primo livello di test.

## Comandi

Esegui i test nativi:

```powershell
pio test -e native
```

Compila il firmware ESP32:

```powershell
pio run -e esp32-c3-supermini
```

## Regola architetturale

La logica che vogliamo testare spesso deve stare in classi/header senza dipendenze da `Arduino.h`.
I moduli hardware dovrebbero restare sottili e delegare le decisioni a codice testabile.
