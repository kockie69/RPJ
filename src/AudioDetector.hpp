#include "rack.hpp"

const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
const double kSmallestNegativeFloatValue = -1.175494351e-38;         /* min negative value */

// --- constants
const unsigned int TLD_AUDIO_DETECT_MODE_PEAK = 0;
const unsigned int TLD_AUDIO_DETECT_MODE_MS = 1;
const unsigned int TLD_AUDIO_DETECT_MODE_RMS = 2;
const double TLD_AUDIO_ENVELOPE_ANALOG_TC = -0.99967234081320612357829304641019; // ln(36.7%)

struct AudioDetectorParameters
{
	AudioDetectorParameters();
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	//AudioDetectorParameters& operator=(const AudioDetectorParameters& params);
	// --- individual parameters
	double attackTime_mSec = 0.0; ///< attack time in milliseconds
	double releaseTime_mSec = 0.0;///< release time in milliseconds
	unsigned int  detectMode = 0;///< detect mode, see TLD_ constants above
	bool detect_dB = false;	///< detect in dB  DEFAULT  = false (linear NOT log)
	bool clampToUnityMax = true;///< clamp output to 1.0 (set false for true log detectors)
};

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
struct AudioDetector
{
public:
	AudioDetector();	/* C-TOR */
	~AudioDetector();	/* D-TOR */

public:
	/** set sample rate dependent time constants and clear last envelope output value */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	// --- process audio: detect the log envelope and return it in dB
	/**
	\param xn input
	\return the processed sample
	*/
	virtual rack::simd::float_4 processAudioSample(rack::simd::float_4 );

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return AudioDetectorParameters custom data structure
	*/
	AudioDetectorParameters getParameters();

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param AudioDetectorParameters custom data structure
	*/
	void setParameters(const AudioDetectorParameters& );

	/** set sample rate - our time constants depend on it */
	virtual void setSampleRate(double );

protected:
	AudioDetectorParameters audioDetectorParameters; ///< parameters for object
	double attackTime = 0.0;	///< attack time coefficient
	double releaseTime = 0.0;	///< release time coefficient
	double sampleRate = 44100;	///< stored sample rate
	rack::simd::float_4 lastEnvelope = 0.0;	///< output register

	/** set our internal atack time coefficients based on times and sample rate */
	void setAttackTime(double, bool = false);

	/** set our internal release time coefficients based on times and sample rate */
	void setReleaseTime(double, bool = false);
};

