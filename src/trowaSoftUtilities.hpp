#ifndef TROWASOFT_UTILITIES_HPP
#define TROWASOFT_UTILITIES_HPP

#include "rack.hpp"
using namespace rack;

#include <string.h>
#include <vector>
#include <sstream> // std::istringstream

#include "math.hpp"

#define TROWA_DEBUG_LVL_HIGH		100
#define TROWA_DEBUG_LVL_MED			 50
#define TROWA_DEBUG_LVL_LOW			  1
#define TROWA_DEBUG_LVL_OFF			  0
#define TROWA_DEBUG_MSGS		TROWA_DEBUG_LVL_MED


// Index not defined value.
#define TROWA_INDEX_UNDEFINED		-1 // Value for undefined index.
#define TROWA_DISP_MSG_SIZE			30 // For local buffers of strings
#define TROWA_SEQ_NUM_PATTERNS		64 // Number of patterns for sequencers.
#define TROWA_SEQ_PATTERN_MIN_V		-10 // Min voltage input / output for controlling pattern index and BPM
#define TROWA_SEQ_PATTERN_MAX_V	 	 10 // Max voltage input / output for controlling pattern index and BPM
#define TROWA_SEQ_NUM_NOTES			12 // Num notes per octave (1 V per octave)
#define TROWA_SEQ_ZERO_OCTAVE		4  // Octave for voltage 0 -- Was 5, now 4
#define TROWA_SEQ_NUM_OCTAVES	   10  // Number of total octaves

#define TROWA_NUM_GLOBAL_EFFECTS	11

#define TROWA_ANGLE_STRAIGHT_UP_RADIANS			(1.5*NVG_PI) // Angle for straight up (svg angles start from positive x and go clockwise)
#define TROWA_ANGLE_STRAIGHT_DOWN_RADIANS		(0.5*NVG_PI) // Angle for straight down

// Fonts:
#define TROWA_DIGITAL_FONT		"res/Fonts/Digital dream Fat.ttf"
#define TROWA_LABEL_FONT		"res/Fonts/ZeroesThree-Regular.ttf"

extern const char * TROWA_NOTES[TROWA_SEQ_NUM_NOTES]; // Our note labels.

// Given some input voltage, convert to our Pattern index [0-63].
inline int VoltsToPattern(float voltsInput)
{	
	return clampi(roundf(rescalef(voltsInput, TROWA_SEQ_PATTERN_MIN_V, TROWA_SEQ_PATTERN_MAX_V, 1, TROWA_SEQ_NUM_PATTERNS)), 1, TROWA_SEQ_NUM_PATTERNS);
}
// Pattern index [0-63] to output voltage.
inline float PatternToVolts(int patternIx)
{
	return rescalef(patternIx + 1, 1, TROWA_SEQ_NUM_PATTERNS, TROWA_SEQ_PATTERN_MIN_V, TROWA_SEQ_PATTERN_MAX_V);
}
// Voltage [-5 to 5] to Octave 0 to 10
inline int VoltsToOctave(float v)
{
	return (int)(v + TROWA_SEQ_ZERO_OCTAVE);
}
// Note index 0 to 11 (to TROWA_NOTES array).
inline int VoltsToNoteIx(float v)
{
	// This doesn't work all the time.
	//(v - floorf(v))*TROWA_SEQ_NUM_NOTES
	// (-4.9 - -5) * 12 = 0.1*12 = int(1.2) = 1 [C#]
	// (-0.33 - -1) * 12 = 0.67*12 = int(8.04) = 8 [G#]
	return (int)(round((v + TROWA_SEQ_ZERO_OCTAVE)*TROWA_SEQ_NUM_NOTES)) % TROWA_SEQ_NUM_NOTES;
}
// Floating point hue [0-1.0] to color.
NVGcolor inline HueToColor(float hue)
{
	return nvgHSLA(hue, 0.5, 0.5, /*alpha 0-255*/ 0xff);
}
// Floating point hue [0-1.0] to color.
NVGcolor inline HueToColor(float hue, float sat, float light)
{
	return nvgHSLA(hue, sat, light, /*alpha 0-255*/ 0xff);
}

// Floating point hue [0-1.0] to color for our color gradient.
NVGcolor inline HueToColorGradient(float hue)
{
	return nvgHSLA(hue, 1.0, 0.5, /*alpha 0-255*/ 0xff);
}
NVGcolor inline ColorInvertToNegative(NVGcolor color)
{ // Keep alpha the same.
	return nvgRGBAf(1.0 - color.r, 1.0 - color.g, 1.0 - color.b, color.a);
}

// Split a string
std::vector<std::string> str_split(const std::string& s, char delimiter);

struct TSColorHSL {
	union {
		float hsl[3];
		struct {
			float h, s, lum;
		};
	};
};
typedef struct TSColorHSL TSColorHSL;

namespace trowaSoft
{
	void TSColorToHSL(NVGcolor color, TSColorHSL* hsv);
}

struct GlobalEffect {
	NVGcompositeOperation compositeOperation = NVG_SOURCE_OVER;
	const char* label;
	GlobalEffect(const char* label, NVGcompositeOperation compositeOperation)
	{
		this->label = label;
		this->compositeOperation = compositeOperation;
		return;
	}
};



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// ValueSequencerMode
// Information and methods for translating knob input voltages to output voltages
// and for display strings.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct ValueSequencerMode
{
	// Min value in voltage
	float voltageMin;
	// Max value in voltage
	float voltageMax;
	// Force whole / integer values 
	bool wholeNumbersOnly;
	
	// The angle that represents 0 radians.
	float zeroPointAngle_radians;
	// Output voltage 
	float outputVoltageMin;
	float outputVoltageMax;
	
	// Min value (what it means)
	float minDisplayValue;
	// Max value (what it means)
	float maxDisplayValue;
	
	bool needsTranslationDisplay;
	bool needsTranslationOutput;
	
	float roundNearestDisplay = 0;
	float roundNearestOutput = 0;
	// Format string for the display value
	const char * displayFormatString;
	// The display name.
	const char * displayName;
	
	float zeroValue;
	
	ValueSequencerMode()
	{
		return;
	}
	
	ValueSequencerMode(const char* displayName, float minDisplayValue, float maxDisplayValue, float min_V, float max_V, 
		float outVoltageMin, float outVoltageMax,
		bool wholeNumbersOnly, float zeroPointAngle, const char * formatStr,
		float roundDisplay, float roundOutput, float zeroValue)
	{
		this->displayName = displayName;
		this->displayFormatString = formatStr;
		this->minDisplayValue = minDisplayValue; // I.e. 1
		this->maxDisplayValue = maxDisplayValue; // I.e. 64 
		this->voltageMin = min_V;  // I.e. -10 Volts
		this->voltageMax = max_V;  // I.e. +10 Volts
		this->outputVoltageMin = outVoltageMin;
		this->outputVoltageMax = outVoltageMax;
		this->wholeNumbersOnly = wholeNumbersOnly; // Force whole numbers
		this->zeroPointAngle_radians = zeroPointAngle;
		this->roundNearestDisplay = roundDisplay;
		this->roundNearestOutput = roundOutput;
		this->zeroValue = zeroValue;
		
		needsTranslationDisplay = minDisplayValue != voltageMin || maxDisplayValue != voltageMax;
		needsTranslationOutput = outputVoltageMin != voltageMin || outputVoltageMax != voltageMax;
		return;
	}

	
	virtual void GetDisplayString(/*in*/ float val, /*out*/ char* buffer)
	{
		float dVal = val;
		if (needsTranslationDisplay)
		{
			dVal = rescalef(val, voltageMin, voltageMax, minDisplayValue, maxDisplayValue);
		}
		if (roundNearestDisplay > 0)
		{
			dVal = static_cast<int>(dVal  / roundNearestDisplay) * roundNearestDisplay;
		}
		sprintf(buffer, displayFormatString, dVal);	
		return;
	}

	
	virtual float GetOutputValue(float val)
	{
		float oVal = val;
		if (needsTranslationOutput)
		{
			oVal = rescalef(val, voltageMin, voltageMax, outputVoltageMin, outputVoltageMax);
		}
		if (roundNearestOutput > 0)
		{ // Round this
			oVal = static_cast<int>(round(oVal  / roundNearestOutput)) * roundNearestOutput;
		}
		return oVal;
	}
};
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// NoteValueSequencerMode
// Special sequencer mode for displaying human friendly Note labels instead of voltages.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct NoteValueSequencerMode : ValueSequencerMode
{
	NoteValueSequencerMode(const char* displayName,
		float min_V, float max_V)
	{
		this->displayName = displayName;
		//this->displayFormatString = formatStr;
		//this->minDisplayValue = minDisplayValue; // I.e. 0
		//this->maxDisplayValue = maxDisplayValue; // I.e. 15 
		
		this->minDisplayValue = -TROWA_SEQ_ZERO_OCTAVE; // -4
		this->maxDisplayValue = TROWA_SEQ_NUM_OCTAVES - TROWA_SEQ_ZERO_OCTAVE; // 10-4 = 6
	
		this->voltageMin = min_V;  // I.e. -10 Volts
		this->voltageMax = max_V;  // I.e. +10 Volts
		
		this->outputVoltageMin = -TROWA_SEQ_ZERO_OCTAVE;
		this->outputVoltageMax = TROWA_SEQ_NUM_OCTAVES - TROWA_SEQ_ZERO_OCTAVE; // 10-4 = 6
		
		this->wholeNumbersOnly = false; // Force whole numbers
		// Zero is no longer straight up now that we are going -4 to +6
		// Knob goes from 0.67*NVG_PI to 2.33*NVG_PI (1 and 2/3 Pi)
		//this->zeroPointAngle_radians = 1.5*NVG_PI; // Straight up
		//this->zeroValue = (max_V + min_V) / 2.0;
		this->zeroPointAngle_radians = 0.67*NVG_PI + TROWA_SEQ_ZERO_OCTAVE *1.67*NVG_PI / TROWA_SEQ_NUM_OCTAVES;
		this->zeroValue = rescalef(0, this->minDisplayValue, this->maxDisplayValue, min_V, max_V);
		this->roundNearestDisplay = 1.0/TROWA_SEQ_NUM_NOTES;
		this->roundNearestOutput = 1.0/TROWA_SEQ_NUM_NOTES;
		
		needsTranslationDisplay = minDisplayValue != voltageMin || maxDisplayValue != voltageMax;
		needsTranslationOutput = outputVoltageMin != voltageMin || outputVoltageMax != voltageMax;
		return;
	}
	// Overriden display string to show notes instead of output voltage values.
	void GetDisplayString(/*in*/ float val, /*out*/ char* buffer) override
	{
		int octave = VoltsToOctave(val);
		int noteIx = VoltsToNoteIx(val);
		if (noteIx > TROWA_SEQ_NUM_NOTES - 1)
			noteIx = TROWA_SEQ_NUM_NOTES - 1;
		else if (noteIx < 0)
			noteIx = 0;
		sprintf(buffer, "%s%d", TROWA_NOTES[noteIx], octave);
		return;
	}
};


#endif // end if not defined