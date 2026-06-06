#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // Larghezza del display in pixel
#define SCREEN_HEIGHT 64 // Altezza del display in pixel

// Configurazione pin I2C per il tuo ESP32-C3 Super Mini
#define I2C_SDA 8
#define I2C_SCL 9

// Crea l'oggetto display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  // Inizializza il bus I2C sui pin 8 e 9
  Wire.begin(I2C_SDA, I2C_SCL);

  // Inizializza il display (indirizzo 0x3C è quello standard)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("ERRORE: Display non trovato. Controlla le saldature!"));
    for(;;); // Blocca tutto se il display non risponde
  }

  // Pulisce il buffer (lo schermo all'inizio ha il logo Adafruit)
  display.clearDisplay();

  // Scriviamo qualcosa!
  display.setTextSize(1);             // Dimensione testo piccola
  display.setTextColor(SSD1306_WHITE); // Colore bianco
  display.setCursor(0,0);             // Inizia dall'angolo in alto a sinistra
  display.println("NeuroHearh32 V1.0");
  display.println("---------------------");
  
  display.setTextSize(2);             // Testo più grande
  display.setCursor(0, 30);
  display.println("ONLINE");

  display.setTextSize(1);
  display.setCursor(0, 55);
  display.print("SDA: Pin 8 | SCL: Pin 9");

  // COMANDO FONDAMENTALE: Senza .display() non vedrai mai nulla!
  display.display();
}

void loop() {
  // Per ora non facciamo nulla nel loop
}
