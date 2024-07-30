#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

Servo myservo;
Servo myservo2;

//konfigurasi wifi
const char* ssid = "Ihsan";
const char* password = "qwertyuiop";

//alamat ip server
const char* server = "172.20.10.6";
const int httpPort = 80;

#define PIR_PIN 19   
#define SERVO_PIN 2  
#define SERVO_PIN2 4
#define Buzzer 18
#define Yellow 26
#define Green 25

unsigned long previousMillis = 0;  
unsigned long ledMillis = 0;       
const long interval = 5000;        
const long ledInterval = 100;   

String urlencode(String input) {
  String encoded = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c == ' ') {
      encoded += "%20";
    } else if (c == '!') {
      encoded += "%21";
    } else if (c == '"') {
      encoded += "%22";
    } else if (c == '#') {
      encoded += "%23";
    } else if (c == '$') {
      encoded += "%24";
    } else if (c == '%') {
      encoded += "%25";
    } else if (c == '&') {
      encoded += "%26";
    } else if (c == '\'') {
      encoded += "%27";
    } else if (c == '(') {
      encoded += "%28";
    } else if (c == ')') {
      encoded += "%29";
    } else if (c == '*') {
      encoded += "%2A";
    } else if (c == '+') {
      encoded += "%2B";
    } else if (c == ',') {
      encoded += "%2C";
    } else if (c == '-') {
      encoded += "%2D";
    } else if (c == '.') {
      encoded += "%2E";
    } else if (c == '/') {
      encoded += "%2F";
    } else if (c >= '0' && c <= '9') {
      encoded += c;
    } else if (c >= 'A' && c <= 'Z') {
      encoded += c;
    } else if (c >= 'a' && c <= 'z') {
      encoded += c;
    } else if (c == ':') {
      encoded += "%3A";
    } else if (c == ';') {
      encoded += "%3B";
    } else if (c == '<') {
      encoded += "%3C";
    } else if (c == '=') {
      encoded += "%3D";
    } else if (c == '>') {
      encoded += "%3E";
    } else if (c == '?') {
      encoded += "%3F";
    } else if (c == '@') {
      encoded += "%40";
    } else if (c == '[') {
      encoded += "%5B";
    } else if (c == '\\') {
      encoded += "%5C";
    } else if (c == ']') {
      encoded += "%5D";
    } else if (c == '^') {
      encoded += "%5E";
    } else if (c == '_') {
      encoded += "%5F";
    } else if (c == '`') {
      encoded += "%60";
    } else if (c == '{') {
      encoded += "%7B";
    } else if (c == '|') {
      encoded += "%7C";
    } else if (c == '}') {
      encoded += "%7D";
    } else if (c == '~') {
      encoded += "%7E";
    } else {
      // For unhandled characters, encode them as %xx
      encoded += String("%") + String(c, HEX);
    }
  }
  return encoded;
}

void connectToWiFi() {
   Serial.print("Connecting to WiFi...");
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("");
   Serial.println("WiFi connected");
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());
}
void setup() {
  Serial.begin(9600);

  pinMode(PIR_PIN, INPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Yellow, OUTPUT);
  pinMode(Green, OUTPUT);
  myservo.attach(SERVO_PIN);
  myservo2.attach(SERVO_PIN2);
  connectToWiFi();

}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
     Serial.println("WiFi disconnected! Trying to reconnect...");
     connectToWiFi();
   }

  unsigned long currentMillis = millis();  

  int pirState = digitalRead(PIR_PIN);
  String status;
  int timespray;

  Serial.print("Status PIR: ");
  Serial.println(pirState);

  if (pirState == HIGH) {  
    myservo.write(90);
    status = "Tikus Terdeteksi, Kandang Tertutup";
    Serial.println(status);
    digitalWrite(Yellow, HIGH);
    digitalWrite(Green, LOW);
    digitalWrite(Buzzer, HIGH);

    delay(500); 

  } else {  
    status = "Tidak Ada Tikus, Kandang Terbuka";
    Serial.println(status);
    digitalWrite(Green, HIGH);
    digitalWrite(Buzzer, LOW);
    myservo.write(0); 

  }

  //servo spray

  static unsigned long sprayMillis = 0;
  static int countdown = interval / 1000; // hitung mundur dalam detik

  if (currentMillis - sprayMillis >= 1000) {
    sprayMillis = currentMillis;
    if (countdown > 0) {
      countdown--;
      Serial.print("Hitung Mundur: ");
      timespray = countdown+1;
      Serial.println(timespray);
    } else {
      myservo2.write(120);  
      delay(1000);  
      myservo2.write(180);  
      countdown = interval / 1000; // reset hitung mundur
    }
  }

  //lampu
  if (currentMillis - ledMillis >= ledInterval) {
    ledMillis = currentMillis;  

    int yellowState = digitalRead(Yellow);
    int greenState = digitalRead(Green);

    digitalWrite(Yellow, !yellowState);  
    digitalWrite(Green, !greenState);
  }
  
  //pengiriman data sensor
  WiFiClient wClient ;
   //cek koneksi ke web server
  if(!wClient.connect(server, httpPort)) 
  {
      Serial.println("Gagal Konek Ke Web Server");
      delay(100);
      return;
  }

  //proses pengiriman data
  String url = "http://" + String(server) + "/PerangkapTikus/public/simpan/" + urlencode(status) + "/" + String(timespray);
  
  Serial.print("Requesting URL: ");
  Serial.println(url);

  HTTPClient http;
  http.begin(wClient, url);
  int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending GET: ");
      Serial.println(httpResponseCode);
    }

  http.end();

  delay(1000);
}
