The OLED library is migration from [https://github.com/ThingPulse/esp8266-oled-ssd1306](https://github.com/ThingPulse/esp8266-oled-ssd1306)



## Features

* Draw pixels at given coordinates
* Draw lines from given coordinates to given coordinates
* Draw or fill a rectangle with given dimensions
* Draw Text at given coordinates:
 * Define Alignment: Left, Right and Center
 * Set the Fontface you want to use (see section Fonts below)
 * Limit the width of the text by an amount of pixels. Before this widths will be reached, the renderer will wrap the text to a new line if possible
* Display content in automatically side scrolling carousel
 * Define transition cycles
 * Define how long one frame will be displayed
 * Draw the different frames in callback methods
 * One indicator per frame will be automatically displayed. The active frame will be displayed from inactive once

## API

### Display Control

```C++
// Initialize the display
void init();

// Free the memory used by the display
void end();

// Cycle through the initialization
void resetDisplay(void);

// Connect again to the display through I2C
void reconnect(void);

// Turn the display on
void displayOn(void);

// Turn the display offs
void displayOff(void);

// Clear the local pixel buffer
void clear(void);

// Write the buffer to the display memory
void display(void);

// Inverted display mode
void invertDisplay(void);

// Normal display mode
void normalDisplay(void);

// Set display contrast
// really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
// normal brightness & contrast:  contrast = 100
void setContrast(uint8_t contrast, uint8_t precharge = 241, uint8_t comdetect = 64);

// Convenience method to access
void setBrightness(uint8_t);

// Turn the display upside down
void flipScreenVertically();

// Draw the screen mirrored
void mirrorScreen();
```

## Pixel drawing

```C++

/* Drawing functions */
// Sets the color of all pixel operations
// color : BLACK, WHITE, INVERSE
void setColor(OLEDDISPLAY_COLOR color);

// Draw a pixel at given position
void setPixel(int16_t x, int16_t y);

// Draw a line from position 0 to position 1
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

// Draw the border of a rectangle at the given location
void drawRect(int16_t x, int16_t y, int16_t width, int16_t height);

// Fill the rectangle
void fillRect(int16_t x, int16_t y, int16_t width, int16_t height);

// Draw the border of a circle
void drawCircle(int16_t x, int16_t y, int16_t radius);

// Fill circle
void fillCircle(int16_t x, int16_t y, int16_t radius);

// Draw a line horizontally
void drawHorizontalLine(int16_t x, int16_t y, int16_t length);

// Draw a lin vertically
void drawVerticalLine(int16_t x, int16_t y, int16_t length);

// Draws a rounded progress bar with the outer dimensions given by width and height. Progress is
// a unsigned byte value between 0 and 100
void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);

// Draw a bitmap in the internal image format
void drawFastImage(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *image);

// Draw a XBM
void drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, const char* xbm);
```

## Text operations

``` C++
void drawString(int16_t x, int16_t y, String text);

// Draws a String with a maximum width at the given location.
// If the given String is wider than the specified width
// The text will be wrapped to the next line at a space or dash
void drawStringMaxWidth(int16_t x, int16_t y, int16_t maxLineWidth, String text);

// Returns the width of the const char* with the current
// font settings
uint16_t getStringWidth(const char* text, uint16_t length);

// Convencience method for the const char version
uint16_t getStringWidth(String text);

// Specifies relative to which anchor point
// the text is rendered. Available constants:
// TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER, TEXT_ALIGN_RIGHT, TEXT_ALIGN_CENTER_BOTH
void setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment);

// Sets the current font. Available default fonts
// ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
// Or create one with the font tool at http://oleddisplay.squix.ch
void setFont(const uint8_t* fontData);
```

## Ui Library (OLEDDisplayUi)

The Ui Library is used to provide a basic set of Ui elements called, `Frames` and `Overlays`. A `Frame` is used to provide
information the default behaviour is to display a `Frame` for a defined time and than move to the next. The library also provides an `Indicator` that will be updated accordingly. An `Overlay` on the other hand is a pieces of information (e.g. a clock) that is displayed always at the same position.


```C++
/**
 * Initialise the display
 */
void init();

/**
 * Configure the internal used target FPS
 */
void setTargetFPS(uint8_t fps);

/**
 * Enable automatic transition to next frame after the some time can be configured with
 * `setTimePerFrame` and `setTimePerTransition`.
 */
void enableAutoTransition();

/**
 * Disable automatic transition to next frame.
 */
void disableAutoTransition();

/**
 * Set the direction if the automatic transitioning
 */
void setAutoTransitionForwards();
void setAutoTransitionBackwards();

/**
 *  Set the approx. time a frame is displayed
 */
void setTimePerFrame(uint16_t time);

/**
 * Set the approx. time a transition will take
 */
void setTimePerTransition(uint16_t time);

/**
 * Draw the indicator.
 * This is the default state for all frames if
 * the indicator was hidden on the previous frame
 * it will be slided in.
 */
void enableIndicator();

/**
 * Don't draw the indicator.
 * This will slide out the indicator
 * when transitioning to the next frame.
 */
void disableIndicator();

/**
 * Enable drawing of all indicators.
 */
void enableAllIndicators();

/**
 * Disable drawing of all indicators.
 */
void disableAllIndicators();

/**
 * Set the position of the indicator bar.
 */
void setIndicatorPosition(IndicatorPosition pos);

/**
 * Set the direction of the indicator bar. Defining the order of frames ASCENDING / DESCENDING
 */
void setIndicatorDirection(IndicatorDirection dir);

/**
 * Set the symbol to indicate an active frame in the indicator bar.
 */
void setActiveSymbol(const char* symbol);

/**
 * Set the symbol to indicate an inactive frame in the indicator bar.
 */
void setInactiveSymbol(const char* symbol);

/**
 * Configure what animation is used to transition from one frame to another
 */
void setFrameAnimation(AnimationDirection dir);

/**
 * Add frame drawing functions
 */
void setFrames(FrameCallback* frameFunctions, uint8_t frameCount);

/**
 * Add overlays drawing functions that are draw independent of the Frames
 */
void setOverlays(OverlayCallback* overlayFunctions, uint8_t overlayCount);

/**
 * Set the function that will draw each step
 * in the loading animation
 */
void setLoadingDrawFunction(LoadingDrawFunction loadingDrawFunction);

/**
 * Run the loading process
 */
void runLoadingProcess(LoadingStage* stages, uint8_t stagesCount);

// Manuell Controll
void nextFrame();
void previousFrame();

/**
 * Switch without transition to frame `frame`.
 */
void switchToFrame(uint8_t frame);

/**
 * Transition to frame `frame`, when the `frame` number is bigger than the current
 * frame the forward animation will be used, otherwise the backwards animation is used.
 */
void transitionToFrame(uint8_t frame);

// State Info
OLEDDisplayUiState* getUiState();

// This needs to be called in the main loop
// the returned value is the remaining time (in ms)
// you have to draw after drawing to keep the frame budget.
int8_t update();
```

