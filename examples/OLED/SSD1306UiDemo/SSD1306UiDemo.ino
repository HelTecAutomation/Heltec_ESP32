/*
 * HelTec Automation(TM) ESP32 Series Dev boards OLED draw UI test code
 *
 * - Some OLED draw UI function test;
 *
 * by Aaron.Lee from HelTec AutoMation, ChengDu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.cn
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
*/

#include "Arduino.h"
#include "heltec.h"
#include "oled/OLEDDisplayUi.h"
#include "images.h"


#define DEMO_DURATION 3000
typedef void (*Demo)(void);

extern Heltec_ESP32 Heltec;
OLEDDisplayUi ui( Heltec.display );

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
	display->setTextAlignment(TEXT_ALIGN_RIGHT);
	display->setFont(ArialMT_Plain_10);
	display->drawString(128, 0, String(millis()));
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->drawXbm(x, y, BT_width, BT_height, BT_bits);
	display->drawXbm(x + 12 + 1, y, WIFI_width, WIFI_height, WIFI_bits);
	display->drawXbm(x + 108, y, BAT_width, BAT_height, BAT_bits);
	display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->drawXbm(x, y, BT_width, BT_height, BT_bits);
	display->drawXbm(x + 12 + 1, y, WIFI_width, WIFI_height, WIFI_bits);
	display->drawXbm(x + 108, y, BAT_width, BAT_height, BAT_bits);
	display->drawXbm(x + 34, y + 12, LoRa_Logo_width, LoRa_Logo_height, LoRa_Logo_bits);
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->drawXbm(x, y + 5, HelTec_LOGO_width, HelTec_LOGO_height, HelTec_LOGO_bits);
}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->setFont(ArialMT_Plain_16);
	display->drawString(x, y, "HelTec");
	display->setFont(ArialMT_Plain_10);
	display->drawString(x, y + 25, "HelTec AutoMation");
	display->drawString(x, y + 35, "www.heltec.cn");
}

FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4 };

int frameCount = 4;

void setup() {
	Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

	ui.setTargetFPS(30);

	// Customize the active and inactive symbol
	ui.setActiveSymbol(activeSymbol);
	ui.setInactiveSymbol(inactiveSymbol);

	// You can change this to
	// TOP, LEFT, BOTTOM, RIGHT
	ui.setIndicatorPosition(BOTTOM);

	// Defines where the first frame is located in the bar.
	ui.setIndicatorDirection(LEFT_RIGHT);

	// You can change the transition that is used
	// SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
	ui.setFrameAnimation(SLIDE_LEFT);

	// Add frames
	ui.setFrames(frames, frameCount);

	// Initialising the UI will init the display too.
	ui.init();

	Heltec.display->flipScreenVertically();
}

void loop() {
	int remainingTimeBudget = ui.update();

	if (remainingTimeBudget > 0) {
		// You can do some work here
		// Don't do stuff if you are below your
		// time budget.
		delay(remainingTimeBudget);
	}
 }
