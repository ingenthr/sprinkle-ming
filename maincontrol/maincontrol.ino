/*
 * 
 * Based upon code found at 
 * http://www.arduino.cc/en/Tutorial/Blink
 * and https://randomnerdtutorials.com/esp8266-web-server-with-arduino-ide/
 *
 * driving this relay board:
 * https://www.amazon.com/gp/product/B00KTEN3TM/ref=oh_aui_search_detailpage?ie=UTF8&psc=1
 *
 * On that board, HIGH is deactivate, LOW is activate.
 *
 * Note to self: HSPI is hardware SPI, driving data to SD, which I don't care about.
 *
 *
*/

#include <ESP8266WiFi.h>
#include "wifiaccess.h"

WiFiServer server(80);

String req_header;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  
  pinMode(D7, OUTPUT);  // blue - grass
  pinMode(D6, OUTPUT);  // red - bubblers
  pinMode(D5, OUTPUT);  // green - drip manifold


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("\nWiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  digitalWrite(D7, HIGH);   // turn everything on to turn it off
  digitalWrite(D6, HIGH);
  digitalWrite(D5, HIGH);
  
}


void loop() {

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        req_header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (req_header.indexOf("POST /valve/grass") >= 0) {
              Serial.println("Turning grass on…");
              digitalWrite(D7, LOW);
            } else if (req_header.indexOf("POST /valve/bubbler") >= 0) {
              Serial.println("Turning bubbler on…");
              digitalWrite(D6, LOW);
            } else if (req_header.indexOf("POST /valve/drip") >= 0) {
              Serial.println("Turning drip on…");
              digitalWrite(D5, LOW);
            } else if (req_header.indexOf("POST /valve") >= 0) {  // TODO, check for off/on
              Serial.println("Turning all valves off…");
              for (uint8_t i=D5; i<=D7; i++) {
                digitalWrite(i, HIGH);
              }
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            client.println("<body><h1>Ingenthron Home Controller</h1>");

            client.println("Current state- ");
            client.print("grass: 0x"); client.print(digitalRead(D7), HEX);
            client.print(" bubblers: 0x"); client.print(digitalRead(D7), HEX);
            client.print(" drip: 0x"); client.print(digitalRead(D7), HEX);


            client.println("<p>This is an ESP8266 running a relay board as a controller for irrigation valves. Code written by Matt Ingenthron based on various bits on the web.</p>");

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    req_header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");

  }


}
