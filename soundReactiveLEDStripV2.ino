#include <FastLED.h>

// MSGEQ7 Pin Declarations
#define STROBE_PIN 					2
#define RESET_PIN  					3
// NEOPIXEL Pin Declarations
#define NUM_LEDS   					30
#define DATA_PIN   					5
#define PUSHBUTTON_SELECT_PIN		4
#define PARSEDANALOGDATA_PIN		0

#define NUMOFOPTION					5

const float beta = 0.3;
unsigned int currentValues[7] = {0,0,0,0,0,0,0};
unsigned int previousValues[7] = {0,0,0,0,0,0,0};
byte selectionState = 0, redColor, greenColor, blueColor;
bool buttonPressedAlready = false, previousSlopeIsPositive = false;
CRGB leds[NUM_LEDS];		// create an object named leds

unsigned long previousTime = 0;
const long interval = 200;

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
		*(bands + i) = map(analogRead(PARSEDANALOGDATA_PIN), 0, 1023, 0, 255);
		delayMicroseconds(37);
		digitalWrite(STROBE_PIN, HIGH);
		delayMicroseconds(19);
	}
}

void calibrateBandValues(unsigned int (&bands)[7])
{
	*(bands + 0) = constrain(	*(bands + 0), 35, 255);
	*(bands + 0) = map(			*(bands + 0), 35, 255, 0, 255);

	*(bands + 1) = constrain(	*(bands + 1), 62, 255);
	*(bands + 1) = map(			*(bands + 1), 62, 255, 0, 255);

	*(bands + 2) = constrain(	*(bands + 2), 52, 255);
	*(bands + 2) = map(			*(bands + 2), 52, 255, 0, 255);

	*(bands + 3) = constrain(	*(bands + 3), 65, 255);
	*(bands + 3) = map(			*(bands + 3), 65, 255, 0, 255);

	*(bands + 4) = constrain(	*(bands + 4), 72, 255);
	*(bands + 4) = map(			*(bands + 4), 72, 255, 0, 255);

	*(bands + 5) = constrain(	*(bands + 5), 65, 255);
	*(bands + 5) = map(			*(bands + 5), 65, 255, 0, 255);

	*(bands + 6) = constrain(	*(bands + 6), 75, 255);
	*(bands + 6) = map(			*(bands + 6), 75, 255, 0, 255);
}

void lpfFilter(unsigned int (&input)[7], unsigned int (&previousInput)[7])
{
	for(int i = 0; i < 7; i++) // current values = previous values + beta * (current input - previous values)
	{
		*(previousInput + i) = *(input + i); 													// get new previous value
		*(input + i) = *(previousInput + i) + beta * (*(input + i) - *(previousInput + i)); 	// get filtered output
	}
}

void setup()
{
	Serial.begin(4800);
	pinMode(STROBE_PIN, OUTPUT);
	pinMode(RESET_PIN, OUTPUT);
	pinMode(DATA_PIN, OUTPUT);

	digitalWrite(RESET_PIN, LOW);	// do not read audio
	digitalWrite(STROBE_PIN, HIGH);	// do not read audio

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

	if(selectionState == 0) // default sound reactive mode
	{
		getBandValues(currentValues);
		calibrateBandValues(currentValues);
		//lpfFilter(currentValues, previousValues);
		setLEDsToAppropiateColor();
		FastLED.show();
		// unsigned long currentTime = millis();
		// if(currentTime - previousTime >= interval)
		// {
		// 	previousTime = currentTime;
		// 	FastLED.show();
		// }
		// for(int i = 0; i < NUM_LEDS; i++)
		// 	leds[i] = CRGB::Black;	// Black
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

	unsigned long currentTime = millis();
	if(currentTime - previousTime >= interval)
	{
		previousTime = currentTime;
		FastLED.show();
	}
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

	// // if 63 Hz and 160 Hz are both saturateed (over 200), set R & G to 0, but set blue to 16 kHz to get the bells/cymbals
	// if(currentValues[0] >= 200 && currentValues[1] >= 200)
	// {
	// 	Serial.println("Saturated Bass");
	// 	redColor = 0; greenColor = 0; blueColor = currentValues[6];
	// }
	// else if(currentValues[0] >= 200 && currentValues[1] < 200)
	// {
	// 	Serial.println("Notable Highs");
	// 	redColor = currentValues[6]; greenColor = 0; blueColor = 0;
	// }
	// else
	// {
	// 	redColor 	= currentValues[6];
	// 	greenColor 	= currentValues[2];
	// 	if(currentValues[0] < 25)
	// 	{
	// 		Serial.println("Non-Averaged");
	// 		unsigned int mean = currentValues[1];
	// 		blueColor	= mean;
	// 	}
	// 	else
	// 	{
	// 		unsigned int mean = (currentValues[0] + currentValues[1]) / 2;
	// 		blueColor	= mean;
	// 	}
	// }

	redColor 	= currentValues[5];
	greenColor 	= currentValues[1];
	blueColor	= currentValues[0];

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
	// for(int i = 0; i < NUM_LEDS; i++)
	// 	leds[i].setRGB(redColor, greenColor, blueColor);

	Serial.print("R: ");
	Serial.print(redColor);
	Serial.print("		G: ");
	Serial.print(greenColor);
	Serial.print("		B: ");
	Serial.println(blueColor);
}