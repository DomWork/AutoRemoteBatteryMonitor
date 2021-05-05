/*
 * Created by Dominic Mikulin with original code snippets from
 * https://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/ for deep sleep
 * and Uteh Str called Arduino | Send Data to Google Sheets with LoLon NodeMCU ESP8266 V3 and Pushingbox API is at
 * https://www.youtube.com/watch?v=4e9hE34RMZM
 *
 * PLEASE EXCUSE THE MESSY CODE, I AM JUST AN AMATEUR CODER AND THIS IS THE STATE OF THE CODE WHEN I GOT IT ALL TO WORK WELL
 * THERE IS REDUNDANT CODE AND PLENTY OF COMMENTED OUT CODE YOU CAN GET RID OF IF YOU LIKE A NEAT PROGRAM.
 * FOR ME, THAT WOULD BE NICE, BUT THE KEY THING IS THAT IT DOES WHAT I WANT IT TO. 
 *
 * YOU WILL NEED TO ADD YOUR OWN WIFI SSID AND PASSWORDS, EMAIL ADDRESS AND PUSHING BOX UNIQUE FILE REFERENCE NUMBER
 * AS WELL AS CREATING YOUR OWN POTENTIAL DIVIDER AND CALIBRATING IT, PUTTING THE CALIBRATION INTO THE VARIABLE "FACTOR"
 * AND SETTING THE THRESHOLD TO SEND AN ALERT EMAIL
 * 
 */
#include <base64.h>
#include <ESP8266WiFi.h>

const char* filename =  "FILENAME COPY HERE";     //Enables you to know what file is loaded by looking at the serial monitor
const char* _ssid =  "SSID1";     // Enter your WiFi Network's SSID
const char* _password =  "PWD1"; // Enter your WiFi Network's Password
const char* _ssid2 =  "SSID2";     // Enter your WiFi Network's SSID
const char* _password2 =  "PWD2"; // Enter your WiFi Network's Password
const char* _GMailServer = "smtp.gmail.com";
const char* _mailUser = "YOURUSERNAME@gmail.com";
const char* _mailPassword = "YOUR EMAIL PASSWORD";
const char* ssid = _ssid;
const char *host = "api.pushingbox.com";                  // pushingbox Host
WiFiClientSecure client;
// Forward declarations of functions (only required in Eclipse IDE)
byte response();
byte sendEmail();

//#define ledPin 5 /* GPIO5 = D1 on board.   (onboard LED connected to GPIO 2, pin D4 on the NodeMCU) */
#define ledPin 14 /* GPIO14 = D5 on board.   (onboard LED connected to GPIO 2, pin D4 on the NodeMCU) */
#define analogPin A0 /* NodeMCU ESP8266 Analog Pin ADC0 = A0 */

// HERE ARE THE PARAMETERS YOU MIGHT WANT TO CHANGE FOR TIME, THRESHOLD OR CORRECTION FACTOR
float sleepFor = 3.7e9; //7.25e9 is near the max stable number of microseconds to go into deep sleep: 30e6 is 30seconds, 60e7=10min
float threshold = 12.15; // the voltage above which nothing happens, below which it sends an alert email.
float factor = 19.104244;  // the potential divider factor: ADJUST SO THAT THE MAXIMUM SOURCE VOLTAGE produces 3V out (which becomes 1V at the ESP chip)
int adcValue = 0;  /* Variable to store Output of ADC */
float volt =0.0;
float batt_ADCinput; // in range 0-1023
float batt_Volts; // actual battery voltage calculated from ADC & factor

//SETUP SETUP SETUP
void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  Serial.setTimeout(2000);
  // Wait for serial to initialize.
  while(!Serial) { }
  Serial.println(" ");
  Serial.println("- - - - - - - - - - - -");
  Serial.println(filename);
  Serial.println("- - - - - - - - - - - -");
  //Brief_flash();
  Brief_flash_rpt(1);
  Connect_wifi(); //try connecting wifi here to pull some power and stabilise before taking voltage reading
  //Long_flash();
  Brief_flash_rpt(1);
  delay(500); //try to pull some power and stabilise for a few seconds
  
  // Now to read the voltage and send it as an email
  //    Take an average of 5 readings.
  for (int i = 0; i<5; i++) {
    adcValue = analogRead (analogPin);
    batt_ADCinput = batt_ADCinput + adcValue;
    Serial.print(int(adcValue));
    Serial.print(" ");
    Serial.print(float(adcValue));
    Serial.print(" ");
    Serial.println(int(batt_ADCinput));
    Serial.print(" ");
    Serial.println(batt_ADCinput);
    delay(100);
  }
  batt_ADCinput = batt_ADCinput / 5;
  // batt_ADCinput = analogRead (analogPin);
  batt_Volts = (batt_ADCinput / 1024) * factor;
  Serial.print("ADC input value: ");
  Serial.println(int(batt_ADCinput));
  Serial.print("Battery Voltage: ");
  Serial.print(batt_Volts);
  Serial.println(" Volts");
  Serial.print("Battery LOW voltage alert threshold: ");
  Serial.print(threshold);
  Serial.println(" Volts");
  Brief_flash_rpt(1);

// Send the data to the spreadsheet
  String batt_Volts_Str = String(batt_Volts);                   //integer to String conversion
  String batt_ADCinput_Str = String(batt_ADCinput);                   //integer to String conversion
  String threshold_Str = String(threshold);                   //integer to String conversion
//  Connect_wifi();
  
  Serial.println("");
  Serial.println("----------------------------------------------");
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    Long_flash_rpt(3);
    return;  //If the program hangs any more I should replace this return with deep sleep just like I did below!
  }
  Brief_flash_rpt(1);
 
  // We now create a URI for the request
  String url = "/pushingbox?";
  url += "devid=";
  url += "ENTER YOUR OWN DEVID UNIQUE REFERENCE NUMBER HERE";
  url += "&value1="+batt_Volts_Str;
  url += "&value2="+batt_ADCinput_Str;
  url += "&value3="+threshold_Str;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 4000) {
      Serial.println(">>> Client Timeout !"); // HERE IS WHERE IT REGULARLY CRASHED FOR ME!
      client.stop();
      //return;  //AS THIS IS WHY IT FREEZES, SO I REPLACED IT WITH A SLEEP WHICH EFFECTIVELY REBOOTS IT TO TRY AGAIN
      Long_flash_rpt(9);
      Serial.println(F("FAILED TO TALK TO PUSHINGBOX so having a 1 minute nap then trying again"));
      ESP.deepSleep(6e7);  //if it fails to connect it goes to sleep for 1 minute.
    }
  }
  Brief_flash_rpt(1);
// I HAVE COMMENTED OUT THIS SECTION AS IT SEEMED TO TAKE SOME TIME TO DO AND POSSIBLY CONTRIBUTED TO THE LOCKING UP
  // Read all the lines of the reply from server and print them to Serial
//  while(client.available()){
//    String line = client.readStringUntil('\r');
//    Serial.print(line);
//    Serial.print(" ......Data Sent!");
//  }

  Serial.println("....-");
  Serial.println("About to stop client");
  Serial.println("----------------------------------------------");
  //Brief_flash();
  client.stop();
  Serial.println("Stopped client");
  Serial.println("----------------------------------------------");

  
// Send an alert email if the battery voltage has dropped low.
  if (batt_Volts <= threshold) {
    Long_flash_rpt(1);
    Serial.println("BATTERY VOLTAGE HAS DROPPED BELOW ALERT THRESHOLD");
    Serial.print("Battery Voltage: ");
    Serial.print(batt_Volts);
    Serial.println(" Volts");
    Serial.println("BATTERY VOLTAGE HAS DROPPED BELOW ALERT THRESHOLD");
    //Brief_flash();
    digitalWrite(ledPin, HIGH);
    //Connect_wifi();  //don't need this if it is connecting everytime to send data ***JUST COMMENTED THIS OUT SO UNDO IF IT NO LONGER SENDS EMAILS
    //Brief_flash_rpt(1);
    //Long_flash_rpt(2);
    // Send the email
    Serial.println("");
    Serial.println("SENDING EMAIL");
    sendEmail(batt_Volts, filename); 
    Serial.println("");
    Serial.println("EMAIL SENT!!");
    Brief_flash_rpt(1);
  }
  


  // Now get ready to go to sleep
  Serial.println("");
  Serial.println("----------------------------------------------");
  Serial.print("Going into deep sleep mode for ");
//  Serial.print(int(sleepFor/3.6e9));  //1e6/60/60
  Serial.print(sleepFor/6e7);  //3.6e9 is 1e6/60/60 6e7 is 1e6*60
  Serial.println(" minutes.");
  // Deep sleep mode for x seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
  //Long_flash();
  Brief_flash_rpt(1);
  ESP.deepSleep(sleepFor); 
}


// Main program loop empty. Code never gets this far as goes to deep sleep above.
void loop() {
}


/*
 * Here are all the subroutines to keep the main loop code short and clean
 */

 
void Brief_flash_rpt(int iRepeat) {
  for (int i=0; i<=iRepeat; i++){
    digitalWrite(ledPin, LOW);
    delay(10);
    digitalWrite(ledPin, HIGH);
    delay(500); 
  } 
}

void Long_flash_rpt(int iRepeat) {
  for (int i=0; i<=iRepeat; i++){
    digitalWrite(ledPin, LOW);
    delay(500);
    digitalWrite(ledPin, HIGH);
    delay(500);
  }  
}

void Brief_flash() {
  for (int i=0; i<=5; i++){
    digitalWrite(ledPin, LOW);
    delay(50);
    digitalWrite(ledPin, HIGH);
    delay(20); 
  } 
}

void Long_flash() {
  for (int i=0; i<=3; i++){
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(500);
  }  
}

void Connect_wifi() {
  WiFi.mode(WIFI_OFF);                                    // Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);                                    // This line hides the viewing of ESP as wifi hotspot
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(_ssid, _password);
  int retries = 0;
  int tryAgain = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    retries++;  //additional code to prevent the controller getting stuck in a permanent loop if it cannot connect to the wifi
    delay(250);
    Serial.print(".");
    Brief_flash_rpt(1);
  }
  if (retries > 14) {
    Serial.println(F("Wifi connection Failed"));
    Serial.println(F("Will attempt to connect to alternative SSID"));
    Long_flash_rpt(6);
    WiFi.mode(WIFI_OFF);
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(_ssid2);
    WiFi.begin(_ssid2, _password2);
    while ((WiFi.status() != WL_CONNECTED) && (tryAgain < 15)) {
      tryAgain++;  
      delay(250);
      Serial.print(".");
      Brief_flash_rpt(1);
    }
    if (tryAgain > 14) {
      Serial.println(F("Wifi connection Failed"));
    }
    Long_flash_rpt(6);
    Serial.println(F("Having a 5 minute nap then trying again"));
    ESP.deepSleep(3e8);  //if it fails to connect to wifi after 15 tries, it goes to sleep for 5 minutes so it can then try again.
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("My IP address: ");
    Serial.println(WiFi.localIP());
  }
}


// Function send a secure email via Gmail
byte sendEmail(float batt_Volts, const char* filename)
{
  Serial.print("Email Battery Voltage: ");
  Serial.print(batt_Volts);
  Serial.println(" Volts");
    
  Serial.println("Attempting to connect to GMAIL server");
  if (client.connect(_GMailServer, 465) == 1) {
    Serial.println(F("Connected"));
  } else {
    Serial.print(F("Connection failed:"));
    return 0;
  }
  if (!response())
    return 0;

  Serial.println(F("Sending Extended Hello"));
  client.println("EHLO gmail.com");
  if (!response())
    return 0;
  
  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if (!response())
    return 0;

  Serial.println(F("Sending User"));
  // Change to your base64, ASCII encoded user
  client.println(base64::encode(_mailUser));
  if (!response())
    return 0;

  Serial.println(F("Sending Password"));
  // change to your base64, ASCII encoded password
  client.println(base64::encode(_mailPassword));
  if (!response())
    return 0;

  Serial.println(F("Sending From"));
  // your email address (sender) - MUST include angle brackets
  client.println(F("MAIL FROM: <YOUR_EMAIL@gmail.com>"));
  if (!response())
    return 0;

  // change to recipient address - MUST include angle brackets
  Serial.println(F("Sending To"));
  client.println(F("RCPT To: <YOUR_EMAIL2@gmail.com>"));
  // Repeat above line for EACH recipient
  if (!response())
    return 0;

  Serial.println(F("Sending DATA"));
  client.println(F("DATA"));
  if (!response())
    return 0;

  Serial.println(F("Sending email"));
  // recipient address (include option display name if you want)
  client.println(F("To: Battery Monitor<YOUR_EMAIL2@gmail.com>"));

  // Sender, recipient, subject and contents of the email being sent
  client.println(F("From: YOUR_EMAIL@gmail.com"));
  client.print(F("Subject: Battery Voltage \r\n"));
  client.print(batt_Volts);
  client.println(F(" Volts.\n"));
  client.println(F("---- \n"));
//  client.print(threshold);
  client.println(F(" Battery voltage has dropped below the threshold set for an alert. See Battery Monitor in Google Sheets for logged data. \n"));
//  client.println(batt_ADCinput);
//  client.println(F("..-.."));
  client.print(F("The file running on this NodeMCU ESP12E ESP8266 via Arduino IDE software is:   \n"));
  client.println(filename); //THIS IS REALLY USEFUL TO HELP YOU IF YOU NEED TO EDIT OR UPDATE THE CODE MONTHS OR YEARS LATER TO KNOW WHICH FILE IS RUNNING ON THE ARDUINO
  client.println(F("--Using PushingBox API to enter data into a Google Sheet called BatteryMonitor and then going into DeepSleep-- \n"));
//  client.println(F("."));
//  client.println(F("Using the ESP8266 connecting to Gmail."));

  // IMPORTANT you must send a complete line containing just a "." to end the conversation
  // So the PREVIOUS line to this one must be a println not just a print
  client.println(F("."));
  if (!response())
    return 0;

  Serial.println(F("Sending QUIT"));
  client.println(F("QUIT"));
  if (!response())
    return 0;

  client.stop();
  Serial.println(F("Disconnected"));
  return 1;
}

// Check response from SMTP server
byte response()
{
  // Wait for a response for up to X seconds
  int loopCount = 0;
  while (!client.available()) {
    delay(1);
    loopCount++;
    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  // Take a snapshot of the response code
  byte respCode = client.peek();
  while (client.available())
  {
    Serial.write(client.read());
  }

  if (respCode >= '4')
  {
    Serial.print("Failed in eRcv with response: ");
    Serial.print(respCode);
    return 0;
  }
  return 1;
}
