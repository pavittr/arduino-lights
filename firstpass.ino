// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid     = "lavalamp";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "zzm313y29";


// {
//   "pin": 1,
//   "r": 255,
//   "g": 255,
//   "b": 255
// }
//
const int docSize = JSON_OBJECT_SIZE(4);
StaticJsonDocument<docSize> doc;

ESP8266WebServer server(80);

void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    12

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 61

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


void setup() {
  Serial.begin(9600);
   // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT);     // Initialize GPIO2 pin as an output  - TODO why are we doign this?

  delay(10);
  Serial.println('\n');


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  outputToDisplay("Starting up");

  delay(10);
  outputToDisplay("Connecting to " + String(ssid));
  WiFi.begin(ssid, password);      
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
   outputToDisplay(String(++i));
  }

  outputToDisplay("IP address: " + WiFi.localIP().toString());

  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  
  server.begin();
  Serial.println("HTTP server started");

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)


}


// loop() function -- runs repeatedly as long as board is on ---------------
void loop() {
  server.handleClient();
}

void fireLED(int pin, int r, int g, int b) {
  // LEDs are indexed from 0, but as this function returns 0 if the string isn't convertable, we'll need to assume human 1-base indexing
  // Clear outside the check, otherwise I'll neevr be able to swithc them back off
  strip.clear();
  if (pin > 0) {
    strip.setPixelColor(pin -1 , strip.Color(r, g, b)); // Set pixel 'c' to value 'color'
  }
  
  strip.show(); // Update strip with new contents
}

void outputToDisplay(String message) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.write(message.c_str());

  display.display();
}

void handleRoot() {
  DeserializationError error = deserializeJson(doc, server.arg("plain").c_str());
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    server.send(500, "text/plain", String("deserializeJson() failed: ") + error.f_str());
    return;
  }
  int pin = doc["pin"];
  int r = doc["r"];
  int g = doc["g"];
  int b = doc["b"];
  
  fireLED(pin, r, g, b);

  server.send(200, "text/plain", String("Increase pin to ") + String(pin));   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
