#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN 4       // Pin conectado al DHT11
#define DHTTYPE DHT11  // Tipo de sensor
#define LDRPIN 34      // Pin analógico conectado a el LDR
#define FCPIN 32       // Pin analógico conectado al sensor FC-28
#define LEDPIN 25 // Pin digital donde conectaremos el LED

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

// Configuración de red
const char* ssid = "HOME OFFICE";     
const char* password = "AMSblue2023";  

bool necesitaRiego = false;

void setup() {

  pinMode(LEDPIN, OUTPUT); // Configurar el pin del LED como salida
  digitalWrite(LEDPIN, LOW); // Asegurarnos de que el LED está apagado al inicio

  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("ESP32 y DHT11");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando....");
  lcd.blink();
  delay(3000);

  // Conectar a red Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a Wi-Fi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  
  // Configurar las rutas del servidor web
  server.on("/", handleRoot);
  server.on("/getSensorData", HTTP_GET, handleGetSensorData); // Ruta para obtener datos de sensores
  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();
  
  // Espera unos segundos entre lecturas
  delay(2000);

  // Lee la humedad
  float humedad = dht.readHumidity();
  // Lee la temperatura en Celsius
  float temperatura = dht.readTemperature();

  int ldrValue = analogRead(LDRPIN);
  int ldrPercentage = map(ldrValue, 0, 1023, 0, 100);  // Conversión a porcentaje
  ldrPercentage = constrain(ldrPercentage, 0, 100);

  // Lectura de la humedad del suelo (FC-28)
  int humedadSuelo = analogRead(FCPIN);
  humedadSuelo = constrain(humedadSuelo, 2500, 4095);
  int humedadSueloPercentage = map(humedadSuelo, 2500, 4095, 100, 0);

   if (temperatura > 30 && humedadSueloPercentage < 40) {
    necesitaRiego = true;
  } else if (ldrPercentage > 80 && humedadSueloPercentage < 30) {
    necesitaRiego = true;
  } else {
    necesitaRiego = false;
  }

   

  // Verifica si las lecturas fallaron
  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error leyendo DHT11");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error leyendo");
    lcd.setCursor(0, 1);
    lcd.print("DHT11");
  } else {
    Serial.println("Humedad: " + String(humedad) + " %");
    Serial.println("Temperatura: " + String(temperatura) + " °C");
    Serial.println("Luz LDR: " + String(ldrPercentage));
    Serial.println("Humedad Suelo: " + String(humedadSueloPercentage));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperatura);
    lcd.print(char(223));  // Símbolo °C
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Hum: ");
    lcd.print(humedad);
    lcd.print(" %");

    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Luz: ");
    lcd.print(ldrPercentage);  // Valor del LDR
    lcd.print(" %");

    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Humedad Suelo:");
    lcd.setCursor(0, 1);
    lcd.print(humedadSueloPercentage);
    lcd.print(" %");

    if (necesitaRiego) {
    digitalWrite(LEDPIN, HIGH); // Enciende el LED
  } else {
    digitalWrite(LEDPIN, LOW); // Apaga el LED
  }

  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  if (necesitaRiego) {
    lcd.print("Regar: SI");
  } else {
    lcd.print("Regar: NO");
  }
  delay(2000);

  }

  delay(3000);
}

// Función para manejar la ruta raíz del servidor
void handleRoot() {
 String html = "<!DOCTYPE html>"
              "<html lang='en'>"
              "<head>"
              "<meta charset='UTF-8'>"
              "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
              "<title>Monitoreo Riego</title>"
              "<style>"
              "body {"
              "  display: flex;"
              "  justify-content: center;"
              "  align-items: center;"
              "  height: 100vh;"
              "  margin: 0;"
              "  background-color: #f0f0f0;"
              "  font-family: Arial, sans-serif;"
              "} "
              ".container {"
              "  display: flex;"
              "  gap: 2rem;"
              "  width: 90vw;"
              "  max-width: 1200px;"
              "} "
              ".contentL, .contentR {"
              "  border-radius: 1rem;"
              "  box-shadow: rgb(38, 57, 77) 0px 20px 30px -10px;"
              "  background-color: #fff;"
              "  padding: 2rem;"
              "} "
              ".contentL {"
              "  width: 70%;"
              "} "
              ".contentR {"
              "  width: 30%;"
              "  display: flex;"
              "  justify-content: center;"
              "  align-items: center;"
              "} "
              ".contentL h1 {"
              "  text-align: center;"
              "  margin-bottom: 2rem;"
              "} "
              ".card-container {"
              "  display: grid;"
              "  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));"
              "  gap: 1rem;"
              "} "
              ".card {"
              "  border-radius: 1rem;"
              "  padding: 1.5rem;"
              "  text-align: center;"
              "  transition: transform 0.3s, box-shadow 0.3s;"
              "  box-shadow: rgb(38, 57, 77) 0px 20px 30px -10px;"
              "} "
              ".card:hover {"
              "  transform: scale(1.05);"
              "  box-shadow: rgba(50, 50, 93, 0.25) 0px 30px 60px -12px inset, rgba(0, 0, 0, 0.3) 0px 18px 36px -18px inset;"
              "} "
              ".card h2 {"
              "  font-size: 1.5rem;"
              "  margin-bottom: 1rem;"
              "} "
              ".card p {"
              "  font-size: 1rem;"
              "} "
              ".progress-bar-container {"
              "  width: 100%;"
              "  background-color: #e0e0e0;"
              "  border-radius: 1rem;"
              "  overflow: hidden;"
              "} "
              ".progress-bar {"
              "  height: 20px;"
              "  border-radius: 1rem;"
              "  transition: width 0.5s;"
              "} "
              ".progress-bar-green {"
              "  background-color: #4caf50;"
              "} "
              ".progress-bar-blue {"
              "  background-color: #4caf50;"
              "} "
              ".progress-bar-red {"
              "  background-color: #4caf50;"
              "} "
              "@media (max-width: 768px) {"
              "  .container {"
              "    flex-direction: column;"
              "  }"
              "  .contentL, .contentR {"
              "    width: 100%;"
              "  }"
              "} "
              "</style>"
              "</head>"
              "<body>"
              "<div class='container'>"
              "<div class='contentL'>"
              "<h1>Monitoreo de Riego</h1>"
              "<div class='card-container'>"
              "<div class='card'>"
              "<h2>Temperatura</h2>"
              "<p id='temperatura'>0 °C</p>"
              "<div class='progress-bar-container'>"
              "<div id='progress-temperatura' class='progress-bar progress-bar-green'></div>"
              "</div>"
              "</div>"
              "<div class='card'>"
              "<h2>Humedad</h2>"
              "<p id='humedad'>0 %</p>"
              "<div class='progress-bar-container'>"
              "<div id='progress-humedad' class='progress-bar progress-bar-blue'></div>"
              "</div>"
              "</div>"
              "<div class='card'>"
              "<h2>Luz</h2>"
              "<p id='ldr'>0 %</p>"
              "<div class='progress-bar-container'>"
              "<div id='progress-ldr' class='progress-bar progress-bar-red'></div>"
              "</div>"
              "</div>"
              "<div class='card'>"
              "<h2>Humedad Suelo</h2>"
              "<p id='suelo'>0 %</p>"
              "<div class='progress-bar-container'>"
              "<div id='progress-suelo' class='progress-bar progress-bar-green'></div>"
              "</div>"
              "</div>"
              "<div class='card'>"
              "<h2>Regar Planta </h2>"
              "<p id='regar'>no</p>"
              "<div class='progress-bar-container'>"
              "<div id='progress-regar' class='progress-bar progress-bar-green'></div>"
              "</div>"
              "</div>"
              "</div>"
              "</div>"
              "<div class='contentR'>"
              "<iframe src='https://my.spline.design/floatingplant-5e1b63cae65ac6b3f681beef7a136c92/' frameborder='0' width='100%' height='100%'></iframe>"
              "</div>"
              "</div>"
              "<script>"
              "function updateSensorData() {"
              "  fetch('/getSensorData')"
              "  .then(response => response.json())"
              "  .then(data => {"
              "    document.getElementById('temperatura').innerText = data.temperatura + ' °C';"
              "    document.getElementById('humedad').innerText = data.humedad + ' %';"
              "    document.getElementById('ldr').innerText = data.ldr + ' %';"
              "    document.getElementById('suelo').innerText = data.suelo + ' %';"
              "    document.getElementById('regar').innerText = data.regar ;"
              "    document.getElementById('progress-temperatura').style.width = data.temperatura + '%';"
              "    document.getElementById('progress-humedad').style.width = data.humedad + '%';"
              "    document.getElementById('progress-ldr').style.width = data.ldr + '%';"
              "    document.getElementById('progress-suelo').style.width = data.suelo + '%';"
              "  });"
              "} "
              "setInterval(updateSensorData, 2000);"
              "</script>"
              "</body>"
              "</html>";
  server.send(200, "text/html", html);

}

// Función para manejar la ruta de datos de sensores
void handleGetSensorData() {
  // Leer sensores
  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature();
  int ldrValue = analogRead(LDRPIN);
  int ldrPercentage = map(ldrValue, 0, 1023, 0, 100);
  int humedadSuelo = analogRead(FCPIN);
  humedadSuelo = constrain(humedadSuelo, 2500, 4095);
  int humedadSueloPercentage = map(humedadSuelo, 2500, 4095, 100, 0);

  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error leyendo DHT11");
    humedad = 0;
    temperatura = 0;
  }

  if (temperatura > 30 && humedadSueloPercentage < 40) {
    necesitaRiego = true;
  } else if (ldrPercentage > 80 && humedadSueloPercentage < 30) {
    necesitaRiego = true;
  } else {
    necesitaRiego = false;
  }


  String jsonResponse = "{\"temperatura\": " + String(temperatura) + ","
                         "\"humedad\": " + String(humedad) + ","
                         "\"ldr\": " + String(ldrPercentage) + ","
                         "\"suelo\": " + String(humedadSueloPercentage) + "}";
                         "\"regar\": \"" + String(necesitaRiego ? "si" : "no") + "\"}";

  server.send(200, "application/json", jsonResponse);
}