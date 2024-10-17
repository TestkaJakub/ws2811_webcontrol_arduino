#include <SPI.h>
#include <Ethernet.h>
#include <PololuLedStrip.h>
#include <ArduinoJson.h> // Make sure you have installed the ArduinoJson library

// MAC address for the Ethernet shield (you can pick a unique one)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Static IP address for your Arduino (adjust based on your network)
IPAddress ip(192, 168, 0, 177);

// IP address of your Deno API server (adjust to your Deno server's IP)
IPAddress server(192, 168, 0, 2); // Replace with your server's IP

// The port on which your Deno server is running
int port = 8000;
bool isConnected = true;

// Initialize the Ethernet client
EthernetClient client;

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<6> ledStrip;

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 1

// Create Color class to fix ordering for write method of PololuLedStrip class
class Color {
  public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    Color() : red(0), green(0), blue(0) {}

    Color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

    rgb_color toPololuRgbColor() const {
      return rgb_color(green, red, blue);
    }

    static void ConvertArray(const Color* colorArray, rgb_color* rgbColorArray, uint16_t numLEDs) {
      for (uint16_t i = 0; i < numLEDs; ++i) {
        rgbColorArray[i] = colorArray[i].toPololuRgbColor();
      }
    }
};

Color colors[LED_COUNT];
rgb_color rgbColors[LED_COUNT];

unsigned long previousMillis = 0;  // stores the last time a request was made
const long interval = 1000;        // interval at which to make the GET request (1000 milliseconds = 1 second)

void setup() {
  // Start the serial communication
  Serial.begin(9600);

  // Start Ethernet connection with a static IP
  Ethernet.begin(mac, ip);

  // Give the Ethernet shield a second to initialize
  delay(1000);

  // Print the IP address for debugging
  Serial.print("Assigned IP: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  unsigned long currentMillis = millis();

  // Only send a GET request every second
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Reconnect to the server for each request
    if (client.connect(server, port)) {
      Serial.println("Connected to the server");

      // Make a GET request to the /led endpoint
      client.println("GET /led HTTP/1.1");
      client.println("Host: 192.168.0.2"); // Replace with your server's IP
      client.println("Connection: close");
      client.println(); // End of request
    }
  }

  // If there's data from the server, read it and print it to the serial monitor
  if (client.available()) {
    String jsonString = "";
    bool isJson = false;

    // Read the server's response character by character
    while (client.available()) {
      char c = client.read();
      
      // Look for the start of the JSON data (after headers)
      if (c == '{') {
        isJson = true;  // Start reading the JSON part
      }

      // Only append characters to jsonString once we encounter the JSON data
      if (isJson) {
        jsonString += c;
      }
    }

    // Print the received response for debugging
    Serial.print("Received JSON: ");
    Serial.println(jsonString);

    // Parse the JSON data
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    // Extract the values from the JSON response
    bool state = doc["state"]; // true or false
    int brightness = doc["brightness"]; // 0-255
    int red = doc["red"];
    int green = doc["green"];
    int blue = doc["blue"];
    
    Serial.print("Received color: ");
    Serial.print("R: "); Serial.print(red);
    Serial.print(" G: "); Serial.print(green);
    Serial.print(" B: "); Serial.println(blue);
    Serial.print("State: "); Serial.println(state);
    Serial.print("Brightness: "); Serial.println(brightness);

    // If the state is false, turn off the LED (set all colors to 0)
    if (!state) {
      red = 0;
      green = 0;
      blue = 0;
    }

    // Scale the brightness of each color component
    red = (red * brightness) / 255;
    green = (green * brightness) / 255;
    blue = (blue * brightness) / 255;

    // Update the LED strip with the new colors and brightness
    for (uint16_t i = 0; i < LED_COUNT; i++) {
      colors[i] = Color(red, green, blue);
    }

    Color::ConvertArray(colors, rgbColors, LED_COUNT);
    ledStrip.write(rgbColors, LED_COUNT);
  }
}