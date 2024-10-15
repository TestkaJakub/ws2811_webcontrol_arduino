#include <PololuLedStrip.h>

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<6> ledStrip;

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 1

// Create Color class to fix ordering for write method of PololuLedStrip class
class Color
{
  public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    Color() : red(0), green(0), blue(0) {}

    Color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

    rgb_color toPololuRgbColor() const {
      return rgb_color(green, red, blue);
    }

    static void ConvertArray(const Color* colorArray, rgb_color* rgbColorArray, uint16_t numLEDs)
    {
      for (uint16_t i = 0; i < numLEDs; ++i) {
        rgbColorArray[i] = colorArray[i].toPololuRgbColor();
      }
    }
};

Color colors[LED_COUNT];
rgb_color rgbColors[LED_COUNT];

void setup()
{
  
}

void loop()
{
  for(uint16_t i = 0; i < LED_COUNT; i++)
  {
    colors[i] = Color(255, 1, 1);
  }

  Color::ConvertArray(colors, rgbColors, LED_COUNT);

  ledStrip.write(rgbColors, LED_COUNT);
}
