#include <FastLED.h>

// MSGEQ7 Pin Declarations
#define STROBE_PIN 					2
#define RESET_PIN  					3
// NEOPIXEL Pin Declarations
#define NUM_LEDS   					30
#define DATA_PIN   					5
#define PUSHBUTTON_SELECT_PIN		4
#define PARSEDANALOGDATA_PIN		0

#define NUMOFOPTION					9

const float beta = 0.3;
unsigned int currentValues[7] = {0,0,0,0,0,0,0};
byte selectionState = 0, redColor, greenColor, blueColor;
bool buttonPressedAlready = false;
CRGB leds[NUM_LEDS];		// create an object named leds

unsigned long previousTime = 0;
const long interval = 200;

// threshold calibration variables
unsigned int maxValues[7] = {0,0,0,0,0,0,0};

void getBandValues(unsigned int (&bands)[7])
{
	// Reset HIGH reads & latches values
	// STROBE LOW take the read values to OUTPUT pin A0
	// Reference waveform in datasheet
	digitalWrite(RESET_PIN, HIGH);
	digitalWrite(RESET_PIN, LOW);
	delayMicroseconds(73);	// trs (reset to strobe) = 73 us for stable read
	for(int i = 0; i < 7; i++)
	{
		digitalWrite(STROBE_PIN, LOW);
		delayMicroseconds(37);	// to (output settle time) = 37 us for stable read
		*(bands + i) = analogRead(PARSEDANALOGDATA_PIN);
		delayMicroseconds(37);
		digitalWrite(STROBE_PIN, HIGH);
		delayMicroseconds(19);
	}
}

void calibrateBandValues(unsigned int (&bands)[7])
{
	*(bands + 0) = constrain(	*(bands + 0), maxValues[0], 1023);
	*(bands + 0) = map(			*(bands + 0), maxValues[0], 1023, 0, 255);

	*(bands + 1) = constrain(	*(bands + 1), maxValues[1], 1023);
	*(bands + 1) = map(			*(bands + 1), maxValues[1], 1023, 0, 255);

	*(bands + 2) = constrain(	*(bands + 2), maxValues[2], 1023);
	*(bands + 2) = map(			*(bands + 2), maxValues[2], 1023, 0, 255);

	*(bands + 3) = constrain(	*(bands + 3), maxValues[3], 1023);
	*(bands + 3) = map(			*(bands + 3), maxValues[3], 1023, 0, 255);

	*(bands + 4) = constrain(	*(bands + 4), maxValues[4], 1023);
	*(bands + 4) = map(			*(bands + 4), maxValues[4], 1023, 0, 255);

	*(bands + 5) = constrain(	*(bands + 5), maxValues[5], 1023);
	*(bands + 5) = map(			*(bands + 5), maxValues[5], 1023, 0, 255);

	*(bands + 6) = constrain(	*(bands + 6), maxValues[6], 1023);
	*(bands + 6) = map(			*(bands + 6), maxValues[6], 1023, 0, 255);
}

void setup()
{
	Serial.begin(4800);
	pinMode(STROBE_PIN, OUTPUT);
	pinMode(RESET_PIN, OUTPUT);
	pinMode(DATA_PIN, OUTPUT);

	digitalWrite(RESET_PIN, LOW);	// do not read audio
	digitalWrite(STROBE_PIN, HIGH);	// do not read audio

	while(millis() < 5000)
	{
		getBandValues(currentValues);

		for(int i = 0; i < 7; i++)
		{
			if(currentValues[i] > maxValues[i])
				maxValues[i] = currentValues[i];
		}
	}

	FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
	FastLED.setBrightness(255);
}

void loop()
{
	if(digitalRead(PUSHBUTTON_SELECT_PIN) == HIGH && buttonPressedAlready == false)
	{
		buttonPressedAlready = true;
		selectionState++;
		if(selectionState == NUMOFOPTION)
			selectionState = 0;
	}
	else if(digitalRead(PUSHBUTTON_SELECT_PIN) == LOW && buttonPressedAlready == true)
	{
		buttonPressedAlready = false;
	}

	if(selectionState == 0 || selectionState == 5 || selectionState == 6 || selectionState == 7 || selectionState == 8) // default sound reactive mode
	{
		getBandValues(currentValues);
		calibrateBandValues(currentValues);
		setLEDsToAppropiateColor();
	}
	else if(selectionState == 1) // RED
	{
		for(int i = 0; i < NUM_LEDS; i++)
			leds[i] = CRGB::Red;
	}
	else if(selectionState == 2) // GREEN
	{
		for(int i = 0; i < NUM_LEDS; i++)
			leds[i] = CRGB::Green;
	}
	else if(selectionState == 3) // BLUE
	{
		for(int i = 0; i < NUM_LEDS; i++)
			leds[i] = CRGB::Blue;
	}
	else if(selectionState == 4) // WHITE
	{
		for(int i = 0; i < NUM_LEDS; i++)
			leds[i] = CRGB::White;
	}

	// unsigned long currentTime = millis();
	// if(currentTime - previousTime >= interval)
	// {
	// 	previousTime = currentTime;
	// 	FastLED.show();
	// }
	FastLED.show();
}

// 63 Hz, 160 Hz, 400 Hz denote bass range, 16 kHz denotes bells/cymbals
void setLEDsToAppropiateColor()
{
	// for(int i = 0; i < 7; i++)
	// {
	// 	Serial.print(currentValues[i]);
	// 	Serial.print("		");
	// }
	// Serial.println();

	// lowFrequencies 	= max(currentValues[0], currentValues[1]); // max of 63 Hz, 160 Hz
	// midFrequencies 	= max(currentValues[2], currentValues[3]); // max of 400 Hz, 1 kHz, 2.5 kHz
	// midFrequencies  = max(midFrequencies, currentValues[4]);
	// highFrequencies = max(currentValues[5], currentValues[6]); // max of 6.25 kHz and 16 kHz

	redColor 	= currentValues[5];
	greenColor 	= currentValues[1];
	blueColor	= currentValues[0];

	if(selectionState == 0)
	{
		// Nice RGB rainbow mode
		for(int i = 0; i < 30; i+=3)
		{
			leds[i].setRGB(redColor, 0, 0);
		}
		for(int i = 1; i < 30; i+=3)
		{
			leds[i].setRGB(0, greenColor, 0);
		}
		for(int i = 2; i < 30; i+=3)
		{
			leds[i].setRGB(0, 0, blueColor);
		}
	}
	else if(selectionState == 5)
	{
		// Every 10 LEDS do R, G, B respectively
		for(int i = 0; i < 10; i++)
		{
			leds[i].setRGB(redColor, 0, 0);
		}
		for(int i = 10; i < 20; i++)
		{
			leds[i].setRGB(0, 0, blueColor);

		}
		for(int i = 20; i < 30; i++)
		{
			leds[i].setRGB(0, greenColor, 0);
		}
	}
	else if(selectionState == 6)
	{
		// Reactive Red only
		for(int i = 0; i < 30; i++)
		{
			leds[i].setRGB(redColor, 0, 0);
		}
		// // only update the first LED, push the previous colors to the next LED and so on
		// static unsigned int LEDtracker = 0;
		// if(LEDtracker <= 29)
		// {
		// 	leds[LEDtracker++].setRGB(redColor, 0, 0);
		// }
		// else 
		// {
		// 	LEDtracker = 0;
		// }
	}
	else if(selectionState == 7)
	{
		// Reactive Green only
		for(int i = 0; i < 30; i++)
		{
			leds[i].setRGB(0, greenColor, 0);
		}
	}
	else if(selectionState == 8)
	{
		// Reactive Blue only
		for(int i = 0; i < 30; i++)
		{
			leds[i].setRGB(0, 0, blueColor);
		}
	}

	Serial.print("R: ");
	Serial.print(redColor);
	Serial.print("		G: ");
	Serial.print(greenColor);
	Serial.print("		B: ");
	Serial.println(blueColor);
}