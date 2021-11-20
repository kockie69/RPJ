#include "ZVA.hpp"
#include "AudioDetector.hpp"

const double kMaxFilterFrequency = 20480.0; // 10 octaves above 20 Hz

template <typename T>
inline void boundValue(T& value, double minValue, double maxValue)
{
	value = fmin(value, maxValue);
	value = fmax(value, minValue);
}

template <typename T>
inline T doUnipolarModulationFromMin(T unipolarModulatorValue, double minValue, double maxValue)
{
	// --- UNIPOLAR bound
	boundValue(unipolarModulatorValue, 0.0, 1.0);

	// --- modulate from minimum value upwards
	return unipolarModulatorValue*(maxValue - minValue) + minValue;
}

/**
\struct EnvelopeFollowerParameters
\ingroup FX-Objects
\brief
Custom parameter structure for the EnvelopeFollower object.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct EnvelopeFollowerParameters
{
	EnvelopeFollowerParameters();
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	//EnvelopeFollowerParameters& operator=(const EnvelopeFollowerParameters& params);

	// --- individual parameters
	double fc = 0.0;				///< filter fc
	double Q = 0.707;				///< filter Q
	double attackTime_mSec = 10.0;	///< detector attack time
	double releaseTime_mSec = 10.0;	///< detector release time
	double threshold_dB = 0.0;		///< detector threshold in dB
	double sensitivity = 1.0;		///< detector sensitivity
};

/**
\class EnvelopeFollower
\ingroup FX-Objects
\brief
The EnvelopeFollower object implements a traditional envelope follower effect modulating a LPR fc value
using the strength of the detected input.

Audio I/O:
- Processes mono input to mono output.

Control I/F:
- Use EnvelopeFollowerParameters structure to get/set object params.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
template <typename T>
struct EnvelopeFollower
{
public:

EnvelopeFollower() {
	// --- setup the filter
	ZVAFilterParameters filterParams;
	filterParams.filterAlgorithm = vaFilterAlgorithm::kSVF_LP;
	filterParams.fc = 1000.0;
	filterParams.enableGainComp = true;
	filterParams.enableNLP = true;
	filterParams.matchAnalogNyquistLPF = true;
	filter.setParameters(filterParams);

	// --- setup the detector
	AudioDetectorParameters adParams;
	adParams.attackTime_mSec = -1.0;
	adParams.releaseTime_mSec = -1.0;
	adParams.detectMode = TLD_AUDIO_DETECT_MODE_RMS;
	adParams.detect_dB = true;
	adParams.clampToUnityMax = false;
	detector.setParameters(adParams);

}		/* C-TOR */

~EnvelopeFollower() {}		/* D-TOR */

/** reset members to initialized state */

bool reset(double _sampleRate)
{		
	filter.reset(_sampleRate);
	detector.reset(_sampleRate);
	return true;
}

/** get parameters: note use of custom structure for passing param data */
/**
\return EnvelopeFollowerParameters custom data structure
*/

EnvelopeFollowerParameters getParameters() { return parameters; }

/** set parameters: note use of custom structure for passing param data */
/**
\param EnvelopeFollowerParameters custom data structure
*/

void setParameters(const EnvelopeFollowerParameters& params)
{
	ZVAFilterParameters filterParams = filter.getParameters();
	AudioDetectorParameters adParams = detector.getParameters();

	if (params.fc != parameters.fc || params.Q != parameters.Q)
	{
		filterParams.fc = params.fc;
		filterParams.Q = params.Q;
		filter.setParameters(filterParams);
	}
	if (params.attackTime_mSec != parameters.attackTime_mSec ||
		params.releaseTime_mSec != parameters.releaseTime_mSec)
	{
		adParams.attackTime_mSec = params.attackTime_mSec;
		adParams.releaseTime_mSec = params.releaseTime_mSec;
		detector.setParameters(adParams);
	}

	// --- save
	parameters = params;
}

/** return false: this object only processes samples */

bool canProcessAudioFrame() { return false; }

/** process input x(n) through the envelope follower to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/

T processAudioSample(T xn)
{
	// --- calc threshold
	double threshValue = pow(10.0, parameters.threshold_dB / 20.0);

	// --- detect the signal
	T detect_dB = detector.processAudioSample(xn);
	T detectValue = rack::simd::pow(10.0, detect_dB / 20.0);
	T deltaValue = detectValue - threshValue;

	ZVAFilterParameters filterParams = filter.getParameters();
	filterParams.fc = parameters.fc;

	// --- if above the threshold, modulate the filter fc
	T mask = rack::simd::operator>(deltaValue,0.f);// || delta_dB > 0.0)
	if (rack::simd::movemask(mask))
	{
		// --- fc Computer
		T modulatorValue = 0.0;

		// --- best results are with linear values when detector is in dB mode
		modulatorValue = (deltaValue * parameters.sensitivity);

		// --- calculate modulated frequency
		filterParams.fc = doUnipolarModulationFromMin(modulatorValue, parameters.fc, kMaxFilterFrequency);
	}

	// --- update with new modulated frequency
	filter.setParameters(filterParams);

	// --- perform the filtering operation
	return filter.processAudioSample(xn);
}


protected:
	EnvelopeFollowerParameters parameters; ///< object parameters

	// --- 1 filter and 1 detector
	ZVAFilter<T> filter;		///< filter to modulate
	AudioDetector<T> detector; ///< detector to track input signal
};
