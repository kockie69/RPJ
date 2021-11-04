#include "EnvelopeFollower.hpp"

inline void boundValue(rack::simd::float_4& value, double minValue, double maxValue)
{
	value = fmin(value, maxValue);
	value = fmax(value, minValue);
}

inline rack::simd::float_4 doUnipolarModulationFromMin(rack::simd::float_4 unipolarModulatorValue, double minValue, double maxValue)
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

EnvelopeFollowerParameters::EnvelopeFollowerParameters() {}

EnvelopeFollower::EnvelopeFollower() {
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
EnvelopeFollower::~EnvelopeFollower() {}		/* D-TOR */

/** reset members to initialized state */
bool EnvelopeFollower::reset(double _sampleRate)
{		
	filter.reset(_sampleRate);
	detector.reset(_sampleRate);
	return true;
}

/** get parameters: note use of custom structure for passing param data */
/**
\return EnvelopeFollowerParameters custom data structure
*/
EnvelopeFollowerParameters EnvelopeFollower::getParameters() { return parameters; }

/** set parameters: note use of custom structure for passing param data */
/**
\param EnvelopeFollowerParameters custom data structure
*/
void EnvelopeFollower::setParameters(const EnvelopeFollowerParameters& params)
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
bool EnvelopeFollower::canProcessAudioFrame() { return false; }

/** process input x(n) through the envelope follower to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
rack::simd::float_4 EnvelopeFollower::processAudioSample(rack::simd::float_4 xn)
{
	// --- calc threshold
	double threshValue = pow(10.0, parameters.threshold_dB / 20.0);

	// --- detect the signal
	rack::simd::float_4 detect_dB = detector.processAudioSample(xn);
	rack::simd::float_4 detectValue = rack::simd::pow(10.0, detect_dB / 20.0);
	rack::simd::float_4 deltaValue = detectValue - threshValue;

	ZVAFilterParameters filterParams = filter.getParameters();
	filterParams.fc = parameters.fc;

	// --- if above the threshold, modulate the filter fc
	rack::simd::float_4 mask = rack::simd::operator>(deltaValue,0.f);// || delta_dB > 0.0)
	if (rack::simd::movemask(mask))
	{
		// --- fc Computer
		rack::simd::float_4 modulatorValue = 0.0;

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

