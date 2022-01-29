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
template <typename T>
struct AudioDetector
{
	public:
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

		AudioDetector() {}	/* C-TOR */
		~AudioDetector() {}	/* D-TOR */

public:
	/** set sample rate dependent time constants and clear last envelope output value */
	bool reset(double _sampleRate)
	{
		setSampleRate(_sampleRate);
		lastEnvelope = 0.0;
		return true;
	}

	/** return false: this object only processes samples */
	bool canProcessAudioFrame() { return false; }

	/**
	@checkFloatUnderflow
	\ingroup FX-Functions

	@brief Perform underflow check; returns true if we did underflow (user may not care)

	\param value - the value to check for underflow
	\return true if overflowed, false otherwise
	*/
	inline bool checkFloatUnderflow(T& value)
	{
		bool retValue = false;
		T tmpValue = ifelse(value > 0.f, ifelse(value < kSmallestPositiveFloatValue,0.f,value), ifelse(value > kSmallestNegativeFloatValue,0.f,value));
		value = tmpValue;
		return retValue = !(bool)rack::simd::movemask(value);
	}

	// --- process audio: detect the log envelope and return it in dB
	/**
	\param xn input
	\return the processed sample
	*/
	T processAudioSample(T xn)
	{
		// --- all modes do Full Wave Rectification
		T input = rack::simd::fabs(xn);

		// --- square it for MS and RMS
		if (audioDetectorParameters.detectMode == TLD_AUDIO_DETECT_MODE_MS ||
			audioDetectorParameters.detectMode == TLD_AUDIO_DETECT_MODE_RMS)
			input *= input;

		// --- to store current
		T currEnvelope = 0.0;

		// --- do the detection with attack or release applied
		currEnvelope = rack::simd::ifelse(input > lastEnvelope,attackTime * (lastEnvelope - input) + input,releaseTime * (lastEnvelope - input) + input);

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
			currEnvelope = rack::simd::sqrt(currEnvelope);

		// --- if not dB, we are done
		if (!audioDetectorParameters.detect_dB)
			return currEnvelope;

		// --- setup for log( )
		T mask = rack::simd::operator<=(currEnvelope,0.f);
		if ((bool)rack::simd::movemask(mask))
		{
			return -96.0;
		}

		// --- true log output in dB, can go above 0dBFS!
		return 20.0*log10(currEnvelope);
	}

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return AudioDetectorParameters custom data structure
	*/
	AudioDetectorParameters getParameters() { return audioDetectorParameters; }

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param AudioDetectorParameters custom data structure
	*/
	void setParameters(const AudioDetectorParameters& parameters)
	{
		audioDetectorParameters = parameters;

		// --- update structure
		setAttackTime(audioDetectorParameters.attackTime_mSec, true);
		setReleaseTime(audioDetectorParameters.releaseTime_mSec, true);

	}

	/** set sample rate - our time constants depend on it */
	void setSampleRate(double _sampleRate)
	{
		if (sampleRate == _sampleRate)
			return;

		sampleRate = _sampleRate;

		// --- recalculate RC time-constants
		setAttackTime(audioDetectorParameters.attackTime_mSec, true);
		setReleaseTime(audioDetectorParameters.releaseTime_mSec, true);
	}

protected:
	AudioDetectorParameters audioDetectorParameters; ///< parameters for object
	double attackTime = 0.0;	///< attack time coefficient
	double releaseTime = 0.0;	///< release time coefficient
	double sampleRate = 44100;	///< stored sample rate
	T lastEnvelope = 0.0;	///< output register

	void setAttackTime(double attack_in_ms, bool forceCalc)
	{
		if (!forceCalc && audioDetectorParameters.attackTime_mSec == attack_in_ms)
			return;

		audioDetectorParameters.attackTime_mSec = attack_in_ms;
		attackTime = exp(TLD_AUDIO_ENVELOPE_ANALOG_TC / (attack_in_ms * sampleRate * 0.001));
	}


	void setReleaseTime(double release_in_ms, bool forceCalc)
	{
		if (!forceCalc && audioDetectorParameters.releaseTime_mSec == release_in_ms)
			return;

		audioDetectorParameters.releaseTime_mSec = release_in_ms;
		releaseTime = exp(TLD_AUDIO_ENVELOPE_ANALOG_TC / (release_in_ms * sampleRate * 0.001));
	}
};

