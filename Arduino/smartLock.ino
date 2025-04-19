#include <Servo.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"
#define DEBOUNCE 20
#define SEGA 2
#define SEGB 3
#define SEGC 4
#define SEGD 5
#define SEGE 6
#define SEGF 7
#define SEGG 8
#define BUT1 9
#define BUT2 10
#define LOCK 11
#define BUZZ 12
#define GLED 13
#define RLED A0
#define DIG1 A1
#define SEN A5

char ssid[] = TEMP_SSID;
char pass[] = TEMP_PASS;
char server[] = SERVER_HOST;
int port = SERVER_PORT;
int status = WL_IDLE_STATUS;
WiFiServer ardServer(80);
WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, server, port);

// Network found
String networks[10];
int numNetworks = 0;

const int segmentPins[7] = { SEGA, SEGB, SEGC, SEGD, SEGE, SEGF, SEGG };
const byte digitPatt[10][7] = {
  { HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW },   // 0
  { LOW, HIGH, HIGH, LOW, LOW, LOW, LOW },       // 1
  { HIGH, HIGH, LOW, HIGH, HIGH, LOW, HIGH },    // 2
  { HIGH, HIGH, HIGH, HIGH, LOW, LOW, HIGH },    // 3
  { LOW, HIGH, HIGH, LOW, LOW, HIGH, HIGH },     // 4
  { HIGH, LOW, HIGH, HIGH, LOW, HIGH, HIGH },    // 5
  { HIGH, LOW, HIGH, HIGH, HIGH, HIGH, HIGH },   // 6
  { HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW },      // 7
  { HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH },  // 8
  { HIGH, HIGH, HIGH, HIGH, LOW, HIGH, HIGH }    // 9

};
volatile unsigned long lastInterruptTime0 = 0;
volatile unsigned long lastInterruptTime1 = 0;
volatile unsigned long lastInterruptTime2 = 0;
unsigned long ledTimer = 0, activeTimer = 0, lockStatusTimer = 0;
volatile uint16_t digit = 0;
volatile bool displayFlag = false, confirmFlag = false, activeFlag = false;
volatile bool ledOn = false, isLocked = true, isEnteringCode = false;
volatile uint8_t pos = 0;
String code = "", master = "1234";
Servo servo;


void ligthUp() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime0 >= DEBOUNCE) {
    lastInterruptTime0 = interruptTime;
    displayFlag = true;
    activeFlag = true;
    activeTimer = millis();
    digitalWrite(DIG1, LOW);
    Serial.println("Light Up");
  }
}

void buttonCycle() {

  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime1 >= DEBOUNCE) {
    lastInterruptTime1 = interruptTime;
    if (!activeFlag) return;
    displayFlag = true;
    digit++;
    if (digit > 9) {
      digit = 0;
    }
    activeTimer = millis();
  }
}

void buttonConfirm() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime2 >= DEBOUNCE) {
    lastInterruptTime2 = interruptTime;
    if (!activeFlag) {
      servo.write(0);
      isLocked = true;
      Serial.println("Locking...");
      Serial.println("Locked");
      return;
    }
    displayFlag = true;
    code += String(digit);
    pos++;
    digit = 0;
    if (pos >= 4) {
      confirmFlag = true;
      displayFlag = false;
      pos = 0;
    }
    activeTimer = millis();
  }
}

void displayDigit(int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], digitPatt[num][i]);
  }
}

void displayCode() {
  int digits[4] = { 0, 0, 0, digit };
  int length = code.length();

  for (int i = 0; i < length && i < 3; i++) {
    digits[i] = code[i] - '0';
  }
  displayDigit(digit);

  for (int i = 0; i < 4; i++) {
    Serial.print(String(digits[i]));
  }
  Serial.println();
}

void clearCode() {

  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], HIGH);
  }
  digitalWrite(DIG1, HIGH);
}

void checkCode() {
  // Send Code to check on the server
  String postData = "{\"code\":\"" + code + "\"}";
  client.post("/checkCode", "application/json", postData);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  if (statusCode == 200) {
    digitalWrite(GLED, HIGH);
    tone(BUZZ, 1000, 500);
    servo.write(180);
    ledOn = true;
    isLocked = false;
    Serial.println("Correct code, unlocked.");
  } else {
    digitalWrite(RLED, HIGH);
    tone(BUZZ, 200, 1000);
    ledOn = true;
    Serial.println("Incorrect code");
  }
  client.stop();

  // if (code == master) {
  //   digitalWrite(GLED, HIGH);
  //   tone(BUZZ, 1000, 500);
  //   servo.write(180);
  //   ledOn = true;
  //   isLocked = false;
  //   Serial.println("Correct code, unlocked.");
  // } else {
  //   digitalWrite(RLED, HIGH);
  //   tone(BUZZ, 200, 1000);
  //   ledOn = true;
  //   Serial.println("Incorrect code");
  // }

  ledTimer = millis();
  code = "";
  clearCode();
  activeFlag = false;
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  numNetworks = WiFi.scanNetworks();
  if (numNetworks > 0) {
    for (int i = 0; i < numNetworks && i < 10; i++) {
      networks[i] = WiFi.SSID(i);
    }
  }

  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // Start AP network
  WiFi.beginAP(ssid, pass);
  ardServer.begin();
  printWiFiStatus();
  doWifiConnection();

  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  pinMode(BUT1, INPUT);
  pinMode(BUT2, INPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);
  pinMode(DIG1, OUTPUT);
  pinMode(SEN, INPUT);
  servo.attach(LOCK);
  servo.write(0);

  // Button interrupt
  attachInterrupt(digitalPinToInterrupt(BUT1), buttonCycle, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUT2), buttonConfirm, FALLING);
  attachInterrupt(digitalPinToInterrupt(SEN), ligthUp, FALLING);

  //Initialize Serial Comm
  Serial.println("Serial Started");
}

void loop() {
  getLockStatus();
  if (displayFlag) {
    displayFlag = false;
    displayCode();
  }

  if (confirmFlag) {
    confirmFlag = false;
    Serial.println("Checking code...");
    checkCode();
  }

  if (ledOn && millis() - ledTimer >= 250) {
    digitalWrite(GLED, LOW);
    digitalWrite(RLED, LOW);
    noTone(BUZZ);
    ledOn = false;
  }

  if (activeFlag) {
    if (millis() - activeTimer >= 2000) {
      activeTimer = millis();
      digitalWrite(DIG1, HIGH);
      displayFlag = false;
      activeFlag = false;
      pos = 0;
      digit = 0;
      code = "";
      Serial.println("Ligth Down");
    }
  }
}

void doWifiConnection() {
  while (true) {
    wifiClient = ardServer.available();

    if (wifiClient) {
      Serial.println("New Client Connected");
      String request = "";
      while (wifiClient.connected()) {
        if (wifiClient.available()) {
          char c = wifiClient.read();
          request += c;
          if (request.endsWith("\r\n\r\n") || request.endsWith("\n\n")) break;
        }
      }
      // Check for the request if its a POST
      if (request.indexOf("POST /") >= 0) {
        Serial.println("Processing POST request...");

        // Wait for data
        while (wifiClient.available() == 0) delay(10);

        // past the data to the postData
        String postData = "";
        while (wifiClient.available()) {
          char c = wifiClient.read();
          postData += c;
        }
        Serial.println("Received POST Data ");
        // Serial.println(postData);

        // Parse SSID and Password from POST data
        int ssidStart = postData.indexOf("ssid=") + 5;
        int ssidEnd = postData.indexOf("&pass=");
        int passStart = ssidEnd + 6;

        String newSsid = postData.substring(ssidStart, ssidEnd);
        String newPass = postData.substring(passStart);
        // Serial.println("newSSID: " + newSsid + ", newPass: " + newPass);

        newSsid.toCharArray(ssid, 32);
        newPass.toCharArray(pass, 32);

        wifiClient.println("HTTP/1.1 200 OK");
        wifiClient.println("Content-Type: text/html");
        wifiClient.println();

        wifiClient.println("<html><body><h2>WiFi Credentials Saved! Rebooting...</h2></body></html>");
        delay(2000);

        // Connecting to new WiFi
        WiFi.begin(ssid, pass);
        int attempt = 0;
        Serial.print("Connecting to WiFi");
        while (WiFi.status() != WL_CONNECTED && attempt < 10) {
          delay(1000);
          Serial.print(".");
          attempt++;
        }

        // Check Wifi connection status
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nConnected to WiFi succesfully!");
          printWiFiStatus();
          sendDataToServer();
          break;
        } else {
          Serial.println("\nCouldn't connected to WiFi, Check credentials!");
          WiFi.disconnect();
          delay(1000);
          // Reset Credentials and start AP
          strcpy(ssid, TEMP_SSID);
          strcpy(pass, TEMP_PASS);
          WiFi.beginAP(ssid, pass);
          Serial.println("Switching to Access Point");
          printWiFiStatus();
        }

      } else {
        // WiFi configuration form with drop box
        wifiClient.println("HTTP/1.1 200 OK");
        wifiClient.println("Content-Type: text/html");
        wifiClient.println();
        wifiClient.println("<html><body>");
        wifiClient.println("<h2>Select WiFi Network</h2>");

        if (numNetworks == 0) {
          wifiClient.println("<p>Error scanning networks. Please try again.</p>");
        } else {
          wifiClient.println("<form action='/' method='POST'>");
          wifiClient.println("SSID: <select name='ssid'><br>");
          Serial.println("Network Options: ");

          for (int i = 0; i < numNetworks && i < 10; i++) {
            wifiClient.println("<option value='" + networks[i] + "'>" + networks[i] + "</option>");
            Serial.println("option " + String(i + 1) + ": " + networks[i]);
          }
          wifiClient.println("</select><br>");
          wifiClient.println("Password: <input type='password' name='pass'><br>");
          wifiClient.println("<input type='submit' value='Connect'>");
          wifiClient.println("</form>");
        }
        wifiClient.println("</body></html>");
      }
      wifiClient.stop();
    }
  }
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void getLockStatus(){
  if (millis() - lockStatusTimer > 5000){
    lockStatusTimer = millis();
    Serial.println("Checking Lock Status");
    client.get("/getLockStatus");
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    if(statusCode == 200){
      int statusStart = response.indexOf("status\":\"") + 9;
      int statusEnd = response.indexOf("\"", statusStart)
      Serial.print("Lock status: ");
      Serial.println(response.substring(statusStart, statusEnd));
    }else{
      Serial.println("Failed to get lock status");
    }
    client.stop();
  }
}

void sendDataToServer() {
  Serial.println("Sending data to server...");
  String postData = "{\"data\":\"Hello from the Arduino\"}";
  client.post("/data", "application/json", postData);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
  client.stop();
}