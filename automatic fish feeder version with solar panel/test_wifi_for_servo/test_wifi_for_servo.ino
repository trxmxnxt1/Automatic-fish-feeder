#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESP32_Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);
#define relay1 26
#define relay2 25

// some var
const char* ssid = "thiramanat_2.4G";
const char* password = "newin11111";

// create server
AsyncWebServer server(80);

Servo myservo; //ประกาศตัวแปรแทน Servo
int servoPin = 13;  // Replace with the actual pin connected to your servo

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     7    // UTC offset for your time zone (UTC+7)
#define UTC_OFFSET_DST 0    // Adjust if your location observes daylight saving time
void spinner() {
  static int8_t counter = 0;
  const char* glyphs = "\xa1\xa5\xdb";
  LCD.setCursor(15, 1);
  LCD.print(glyphs[counter++]);
  if (counter == strlen(glyphs)) {
    counter = 0;
  }
}

int firstOnTimeHr = 20;
int firstOnTimeMin = 54;
int secondOnTimeHr = 20;
int secondOnTimeMin = 55;

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    LCD.setCursor(0, 1);
    LCD.println("Connection Err");
    return;
  }

  LCD.setCursor(8, 0);
  LCD.println(&timeinfo, "%H:%M:%S");

  LCD.setCursor(0, 1);
  LCD.println(&timeinfo, "%d/%m/%Y   %Z");
  // do something here
  
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  
 if (timeinfo.tm_hour == firstOnTimeHr && timeinfo.tm_min == firstOnTimeMin) {
  // First condition: Turn on relay1, turn off relay2
  digitalWrite(relay1, LOW);

} if (timeinfo.tm_hour == firstOnTimeHr && timeinfo.tm_min == firstOnTimeMin && timeinfo.tm_sec > 4) {
  // Second condition: Turn off relay1, turn off relay2 (after 30 seconds)
  Serial.println("First condition met!");
  digitalWrite(relay1, HIGH);
}

if (timeinfo.tm_hour == secondOnTimeHr && timeinfo.tm_min == secondOnTimeMin) {
  // Third condition: Turn off relay2, turn on relay1
  digitalWrite(relay2, LOW);
} if (timeinfo.tm_hour == secondOnTimeHr && timeinfo.tm_min == secondOnTimeMin && timeinfo.tm_sec > 4) {
  // Fourth condition: Turn off relay2, turn off relay1 (after 10 seconds)
  Serial.println("Second condition met!");
  digitalWrite(relay2, HIGH);
}
  
}


void setup() {
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  myservo.attach(13, 544, 2400); // Servo
  digitalWrite(13, LOW);
  // begin
  
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("Connecting to ");
  LCD.setCursor(0, 1);
  LCD.print("WiFi ");

  // start WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String message = "Hello! Use /on or /off to control the servo.";
    request->send(SPIFFS, "/index.html", String(), false); // ลบ processor ออก
  });

  // Route to load bg.jpg file
  server.on("/bg.jpg", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/bg.jpg", "jpg");
  });

  // Route to load style.css file
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/styles.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest * request) {
    myservo.write(180); // Set servo to 180 degrees (on)
    request->send(SPIFFS, "/index.html", String(), false); // ลบ processor ออก
  });

  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest * request) {
    myservo.write(0); // Set servo to 0 degrees (off)
    request->send(SPIFFS, "/index.html", String(), false); // ลบ processor ออก
  });

// Route to set GPIO to HIGH
  server.on("/relay1", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, HIGH);
    request->send(SPIFFS, "/index.html", String(), false); // ลบ processor ออก
  });

  // Route to set GPIO to LOW
  server.on("/relay2", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(relay2, LOW);
    digitalWrite(relay1, HIGH);
    request->send(SPIFFS, "/index.html", String(), false); // ลบ processor ออก
  });
  server.begin();

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Online");
  LCD.setCursor(0, 1);
  LCD.println("Updating time...");

  configTime(UTC_OFFSET * 3600, UTC_OFFSET_DST, NTP_SERVER); // Multiply by 3600 to convert hours to seconds
}

void loop() {
  printLocalTime();
  delay(250);
  
}
