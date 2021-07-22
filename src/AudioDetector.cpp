#include "AudioDetector.hpp"

/**
@checkFloatUnderflow
\ingroup FX-Functions

@brief Perform underflow check; returns true if we did underflow (user may not care)

\param value - the value to check for underflow
\return true if overflowed, false otherwise
*/
inline bool checkFloatUnderflow(double& value)
{
	bool retValue = false;
	if (value > 0.0 && value < kSmallestPositiveFloatValue)
	{
		value = 0;
		retValue = true;
	}
	else if (value < 0.0 && value > kSmallestNegativeFloatValue)
	{
		value = 0;
		retValue = true;
	}
	return retValue;
}

/**
\struct AudioDetectorParameters
\ingroup FX-Objects
\brief
Custom parameter structure for the AudioDetector object. NOTE: this object uses constant defintions:

- const unsigned int TLD_AUDIO_DETECT_MODE_PEAK = 0;
- const unsigned int TLD_AUDIO_DETECT_MODE_MS = 1;
- const unsigned int TLD_AUDIO_DETECT_MODE_RMS = 2;
- const double TLD_AUDIO_ENVELOPE_ANALOG_TC = -0.99967234081320612357829304641019; // ln(36.7%)

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

AudioDetectorParameters::AudioDetectorParameters() {}

/**
\class AudioDetector
\ingroup FX-Objects
\brief
The AudioDetector object implements the audio detector defined in the book source below.
NOTE: this detector can receive signals and transmit detection values that are both > 0dBFS

Audio I/O:
- Processes mono input to a detected signal output.

Control I/F:
- Use AudioDetectorParameters structure to get/set object params.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

AudioDetector::AudioDetector() {}	/* C-TOR */
AudioDetector::~AudioDetector() {}	/* D-TOR */


/** set sample rate dependent time constants and clear last envelope output value */
bool AudioDetector::reset(double _sampleRate)
{
	setSampleRate(_sampleRate);
	lastEnvelope = 0.0;
	return true;
}

/** return false: this object only processes samples */
bool AudioDetector::canProcessAudioFrame() { return false; }

// --- process audio: detect the log envelope and return it in dB
/**
\param xn input
\return the processed sample
*/
double AudioDetector::processAudioSample(double xn)
{
	// --- all modes do Full Wave Rectification
	double input = fabs(xn);

	// --- square it for MS and RMS
	if (audioDetectorParameters.detectMode == TLD_AUDIO_DETECT_MODE_MS ||
		audioDetectorParameters.detectMode == TLD_AUDIO_DETECT_MODE_RMS)
		input *= input;

	// --- to store current
	double currEnvelope = 0.0;

	// --- do the detection with attack or release applied
	if (input > lastEnvelope)
		currEnvelope = attackTime * (lastEnvelope - input) + input;
	else
		currEnvelope = releaseTime * (lastEnvelope - input) + input;

	// --- we are recursive so need to check underflow
	checkFloatUnderflow(currEnvelope);

	// --- bound them; can happen when using pre-detector gains of more than 1.0
	if (audioDetectorParameters.clampToUnityMax)
		currEnvelope = fmin(currEnvelope, 1.0);

	// --- can not be (-)
	currEnvelope = fmax(currEnvelope, 0.0);

	// --- store envelope prior to sqrt for RMS version
	lastEnvelope = currEnvelope;

	// --- if RMS, do the SQRT
	if (audioDetectorParameters.detectMode == TLD_AUDIO_DETECT_MODE_RMS)
		currEnvelope = pow(currEnvelope, 0.5);

	// --- if not dB, we are done
	if (!audioDetectorParameters.detect_dB)
		return currEnvelope;

	// --- setup for log( )
	if (currEnvelope <= 0)
	{
		return -96.0;
	}

	// --- true log output in dB, can go above 0dBFS!
	return 20.0*log10(currEnvelope);
}

AudioDetectorParameters AudioDetector::getParameters() { return audioDetectorParameters; }

/** set parameters: note use of custom structure for passing param data */
/**
\param AudioDetectorParameters custom data structure
*/
void AudioDetector::setParameters(const AudioDetectorParameters& parameters)
{
	audioDetectorParameters = parameters;

	// --- update structure
	setAttackTime(audioDetectorParameters.attackTime_mSec, true);
	setReleaseTime(audioDetectorParameters.releaseTime_mSec, true);

}

void AudioDetector::setAttackTime(double attack_in_ms, bool forceCalc = false) {};

void AudioDetector::setReleaseTime(double release_in_ms, bool forceCalc = false) {};

/** set sample rate - our time constants depend on it */
void AudioDetector::setSampleRate(double _sampleRate)
{
	if (sampleRate == _sampleRate)
		return;

	sampleRate = _sampleRate;

	// --- recalculate RC time-constants
	setAttackTime(audioDetectorParameters.attackTime_mSec, true);
	setReleaseTime(audioDetectorParameters.releaseTime_mSec, true);
}

/** set our internal atack time coefficients based on times and sample rate */
//void setAttackTime(double attack_in_ms, bool forceCalc = false);

/** set our internal release time coefficients based on times and sample rate */
//void setReleaseTime(double release_in_ms, bool forceCalc = false);


