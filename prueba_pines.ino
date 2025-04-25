#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd1(0x27, 20, 4);
LiquidCrystal_I2C lcd2(0x26, 20, 4);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 50);
EthernetServer server(80);
const int chipSelectSD = 4;
char letra;
const int pinPulsador = 46;
unsigned long tiempoPresionado = 0;
bool enMenu = false;
unsigned long ultimoCambio = 0;
unsigned long tiempoLectura = 2;
unsigned long ultimoPulsador = 0;
unsigned long ultimaInteraccion = 0;
bool estadoPrevio = HIGH;
unsigned long ultimaLectura = 0;
bool mostrandoGuardado = false;
unsigned long tiempoGuardado = 0;
unsigned long ultimaActualizacionLCD1 = 0;
unsigned long ultimaActualizacionWeb = 0;

void setup() {
  lcd1.init();
  lcd1.backlight();

  lcd2.init();
  lcd2.backlight();

  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("Pantalla 2 xd");

  Serial.begin(9600);

  pinMode(pinPulsador, INPUT_PULLUP);

  if (!rtc.begin()) {
    Serial.println("Fallo al inicializar el RTC");
    lcd1.setCursor(0, 0);
    lcd1.print("Fallo RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Ethernet.begin(mac, ip);
  server.begin();

  if (!SD.begin(chipSelectSD)) {
    Serial.println("Fallo al iniciar SD");
    lcd1.setCursor(0, 0);
    lcd1.print("Fallo SD");
    while (1);
  }else {
    // Si la SD se inicializa correctamente, borra el archivo
    SD.remove("lecturas.txt");
  }
}

void loop() {
  unsigned long ahora = millis();
  bool estadoActual = digitalRead(pinPulsador);

  if (estadoPrevio == HIGH && estadoActual == LOW && (ahora - ultimoPulsador > 50)) {
    tiempoPresionado = ahora;
    ultimoPulsador = ahora;
  }

  if (estadoActual == LOW) {
    if (!enMenu && (ahora - tiempoPresionado >= 5000)) {
      enMenu = true;
      lcd1.clear();
      lcd1.setCursor(0, 0);
      lcd1.print("Configurar tiempo:");
      lcd1.setCursor(0, 1);
      lcd1.print(tiempoLectura);
      lcd1.print(" segundos");
      ultimaInteraccion = ahora;
      delay(500);
    } else if (enMenu && (ahora - ultimoPulsador > 300)) {
      tiempoLectura++;
      if (tiempoLectura > 60) tiempoLectura = 1;
      lcd1.setCursor(0, 1);
      lcd1.print("                ");
      lcd1.setCursor(0, 1);
      lcd1.print(tiempoLectura);
      lcd1.print(" segundos");
      ultimaInteraccion = ahora;
      ultimoPulsador = ahora;
    }
  }

  if (enMenu && (ahora - ultimaInteraccion > 5000)) {
    enMenu = false;
        mostrandoGuardado = true;
    tiempoGuardado = ahora;
    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Guardado:");
    lcd1.setCursor(0, 1);
    lcd1.print(tiempoLectura);
    lcd1.print(" segundos");

    // Guardar en SD al salir del menú
    File dataFile = SD.open("lecturas.txt", FILE_WRITE);
    if (dataFile) {
      for (int i = 0; i < 12; i++) {
        int valor = analogRead(i);
        dataFile.print(valor * (5.0 / 1023.0));
        dataFile.print(" ; ");
      }

      DateTime now = rtc.now();
      char fecha[11], hora[9];
      sprintf(fecha, "%04d-%02d-%02d", now.year(), now.month(), now.day());
      sprintf(hora, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

      dataFile.print(fecha);
      dataFile.print(" ; ");
      dataFile.println(hora);
      dataFile.close();
      mostrarWeb();
    }
  }

  if (mostrandoGuardado && (ahora - tiempoGuardado >= 5000)) {
    mostrandoGuardado = false;
    lcd1.clear();
    ultimaLectura = ahora;

    // Mostrar lecturas inmediatamente
    for (int linea = 0; linea < 4; linea++) {
      lcd1.setCursor(0, linea);
      for (int j = 0; j < 3; j++) {
        int canal = linea * 3 + j;
        int lectura = analogRead(canal);
        float voltaje = lectura * (5 / 1023.0);
        
        switch (canal){
          case 0: letra='A';
                  break;
          case 1: letra='B';
                  break;
          case 2: letra='C';
                  break;
          case 3: letra='D';
                  break;
          case 4: letra='E';
                  break;
          case 5: letra='F';
                  break;
          case 6: letra='G';
                  break;
          case 7: letra='H';
                  break;
          case 8: letra='I';
                  break;
          case 9: letra='J';
                  break;
          case 10: letra='K';
                  break;
          case 11: letra='L';
                  break;
        }

        lcd1.print(letra);
        lcd1.print(":");
        lcd1.print(voltaje, 1);
        lcd1.print(" ");
      }
    }
  }

  estadoPrevio = estadoActual;

  // No leer si está en menú o mostrando "Guardado"
if (!enMenu && !mostrandoGuardado && (ahora - ultimaActualizacionLCD1 >= 1000)) {
  ultimaActualizacionLCD1 = ahora;
  // Código para actualizar el LCD
  lcd1.clear();
  for (int linea = 0; linea < 4; linea++) {
    lcd1.setCursor(0, linea);
    for (int j = 0; j < 3; j++) {
      int canal = linea * 3 + j;
      int lectura = analogRead(canal);
      float voltaje = lectura * (5 / 1023.0);

      switch (canal){
        case 0: letra='A';
                break;
        case 1: letra='B';
                break;
        case 2: letra='C';
                break;
        case 3: letra='D';
                break;
        case 4: letra='E';
                break;
        case 5: letra='F';
                break;
        case 6: letra='G';
                break;
        case 7: letra='H';
                break;
        case 8: letra='I';
                break;
        case 9: letra='J';
                break;
        case 10: letra='K';
                break;
        case 11: letra='L';
                break;
      }

      lcd1.print(letra);
      lcd1.print(":");
      lcd1.print(voltaje, 1);
      lcd1.print(" ");
    }
  }
}

if (!enMenu && !mostrandoGuardado && (ahora - ultimaLectura >= tiempoLectura * 1000)) {
  ultimaLectura = ahora;
  // Código para guardar en SD
  // Guardar en SD
  File dataFile = SD.open("lecturas.txt", FILE_WRITE);
  if (dataFile) {
    for (int i = 0; i < 12; i++) {
      int valor = analogRead(i);
      dataFile.print(valor * (5.0 / 1023.0));
      dataFile.print(" ; ");
    }

    DateTime now = rtc.now();
    char fecha[11], hora[9];
    sprintf(fecha, "%04d-%02d-%02d", now.year(), now.month(), now.day());
    sprintf(hora, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    dataFile.print(fecha);
    dataFile.print(" ; ");
    dataFile.println(hora);
    dataFile.close();
  }
}

if (ahora - ultimaActualizacionWeb >= 1000) {
  ultimaActualizacionWeb = ahora;
  // Código para actualizar el contenido del servidor web
  EthernetClient cliente = server.available();
    if (cliente) {
      boolean currentLineIsBlank = true;
      while (cliente.connected()) {
        if (cliente.available()) {
          char c = cliente.read();
          if (c == '\n' && currentLineIsBlank) {
            cliente.println("HTTP/1.1 200 OK");
            cliente.println("Content-Type: text/html");
            cliente.println("Connection: close");
            cliente.println("Refresh: 3");
            cliente.println();
            cliente.println("<!DOCTYPE HTML><html><body>");
            cliente.println("<h1>Entradas Analogicas</h1><hr>");
            for (int i = 0; i < 12; i++) {
              int val = analogRead(i);
              float volt = val * 5.0 / 1023.0;
              cliente.print("A");
              cliente.print(i);
              cliente.print(": ");
              cliente.print(val);
              cliente.print(" (");
              cliente.print(volt, 2);
              cliente.println(" V)<br>");
            }
            cliente.println("</body></html>");
            break;
          }
          if (c == '\n') {
            currentLineIsBlank = true;
          } else if (c != '\r') {
            currentLineIsBlank = false;
          }
        }
      }
      delay(15);
      cliente.stop();
    }

}


}

void mostrarWeb() {
  EthernetClient cliente = server.available();
  if (cliente) {
    bool currentLineIsBlank = true;
    while (cliente.connected()) {
      if (cliente.available()) {
        char c = cliente.read();
        if (c == '\n' && currentLineIsBlank) {
          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Content-Type: text/html");
          cliente.println("Connection: close");
          cliente.println("Refresh: 3");
          cliente.println();
          cliente.println("<!DOCTYPE HTML><html><body>");
          cliente.println("<h1>Entradas Analogicas</h1><hr>");
          for (int i = 0; i < 12; i++) {
            int val = analogRead(i);
            float volt = val * 5.0 / 1023.0;
            cliente.print("A");
            cliente.print(i);
            cliente.print(": ");
            cliente.print(val);
            cliente.print(" (");
            cliente.print(volt, 2);
            cliente.println(" V)<br>");
          }
          cliente.println("</body></html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(15);
    cliente.stop();
  }
}
