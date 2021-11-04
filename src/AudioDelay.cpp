#include "AudioDelay.hpp"

inline double doLinearInterpolation(double y1, double y2, double fractional_X)
{
	// --- check invalid condition
	if (fractional_X >= 1.0) return y2;

	// --- use weighted sum method of interpolating
	return fractional_X*y2 + (1.0 - fractional_X)*y1;
}

inline void boundValue(double& value, double minValue, double maxValue)
{
	value = fmin(value, maxValue);
	value = fmax(value, minValue);
}

AudioDelay::AudioDelay() {
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
	LPFafp.algorithm=filterAlgorithm::kLPF1;
	HPFafp.algorithm=filterAlgorithm::kHPF1;
} /* C-TOR */

AudioDelay::~AudioDelay() {}	/* D-TOR */


/** reset members to initialized state */
bool AudioDelay::reset(double _sampleRate)
{
	// --- if sample rate did not change
	if (sampleRate == _sampleRate)
	{
		// --- just flush buffer and return
		delayBuffer_L.flushBuffer();
		delayBuffer_R.flushBuffer();
		return true;
	}

	// --- create new buffer, will store sample rate and length(mSec)
	createDelayBuffers(_sampleRate, bufferLength_mSec);

	return true;
}

rack::simd::float_4 AudioDelay::processAudioSample(rack::simd::float_4 xn)
{
	// --- read delay
	rack::simd::float_4 yn = delayBuffer_L.readBuffer(delayInSamples_L);

	// --- create input for delay buffer
	rack::simd::float_4 dn = xn + (parameters.feedback_Pct / 100.0) * yn;

	// --- write to delay buffer
	delayBuffer_L.writeBuffer(dn);

	// --- form mixture out = dry*xn + wet*yn
	rack::simd::float_4 output = dryMix*xn + wetMix*yn;

	return output;
}

/** return true: this object can also process frames */
bool AudioDelay::canProcessAudioFrame() { return true; }

/** process STEREO audio delay in frames */
bool AudioDelay::processAudioFrame(rack::simd::float_4* inputFrame,		/* ptr to one frame of data: pInputFrame[0] = left, pInputFrame[1] = right, etc...*/
	rack::simd::float_4* outputFrame,
	uint32_t inputChannels,
	uint32_t outputChannels)
{
	// --- make sure we have input and outputs
	if (inputChannels == 0 || outputChannels == 0)
		return false;

	// --- make sure we support this delay algorithm
	if (parameters.algorithm != delayAlgorithm::kNormal &&
		parameters.algorithm != delayAlgorithm::kPingPong &&
		parameters.algorithm != delayAlgorithm::kLCRDelay &&
		parameters.algorithm != delayAlgorithm::kTapDelay)
		return false;

	// --- if only one output channel, revert to mono operation
	if (outputChannels == 1)
	{
		// --- process left channel only
		outputFrame[0] = processAudioSample(inputFrame[0]);
		return true;
	}

	// --- if we get here we know we have 2 output channels
	//
	// --- pick up inputs
	//
	// --- LEFT channel
	rack::simd::float_4 outputL, outputR;
	rack::simd::float_4 xnL = inputFrame[0];

	// --- RIGHT channel (duplicate left input if mono-in)
	rack::simd::float_4 xnR = inputChannels > 1 ? inputFrame[1] : xnL;

	// --- read delay LEFT
	rack::simd::float_4 ynL = delayBuffer_L.readBuffer(delayInSamples_L);

	// --- read delay LEFT
	rack::simd::float_4 ynC = delayBuffer_C.readBuffer(delayInSamples_C);

	// --- read delay RIGHT
	rack::simd::float_4 ynR = delayBuffer_R.readBuffer(delayInSamples_R);

	// --- create input for delay buffer with LEFT channel info
	rack::simd::float_4 dnL = xnL + (parameters.feedback_Pct / 100.0) * ynL;

	// --- create input for delay buffer with RIGHT channel info
	rack::simd::float_4 dnR = xnR + (parameters.feedback_Pct / 100.0) * ynR;

	// --- decode
	if (parameters.algorithm == delayAlgorithm::kNormal)
	{
		// --- write to LEFT delay buffer with LEFT channel info
		delayBuffer_L.writeBuffer(dnL);

		// --- write to RIGHT delay buffer with RIGHT channel info
		delayBuffer_R.writeBuffer(dnR);

		// --- form mixture out = dry*xn + wet*yn
		outputL = dryMix*xnL + wetMix*ynL;

		// --- form mixture out = dry*xn + wet*yn
		outputR = dryMix*xnR + wetMix*ynR;
	}
	else if (parameters.algorithm == delayAlgorithm::kPingPong)
	{
		// --- write to LEFT delay buffer with RIGHT channel info
		delayBuffer_L.writeBuffer(dnR);

		// --- write to RIGHT delay buffer with LEFT channel info
		delayBuffer_R.writeBuffer(dnL);

			// --- form mixture out = dry*xn + wet*yn
		outputL = dryMix*xnL + wetMix*ynL;

		// --- form mixture out = dry*xn + wet*yn
		outputR = dryMix*xnR + wetMix*ynR;
	}
	else if (parameters.algorithm == delayAlgorithm::kLCRDelay)
	{
		// --- create input for delay buffer with Left and Right channel info
		
		// TO DO: Check if LPF and HPF are enabled in the menu
		// if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected()) {
		LPFaudioFilter.setSampleRate(sampleRate);
		HPFaudioFilter.setSampleRate(sampleRate);
 	
 		LPFafp.fc = parameters.lpfFc;
		HPFafp.fc = parameters.hpfFc;
		LPFafp.dry = HPFafp.dry = 0;
		LPFafp.wet = HPFafp.wet = 1;

		// Check if LPF is enabled
		rack::simd::float_4 LPFOut = (parameters.feedback_Pct / 100.0) * ynC; 
		if (parameters.useLPF) {
			LPFaudioFilter.setParameters(LPFafp);
			LPFOut = LPFaudioFilter.processAudioSample(LPFOut);
		}

		// Check if HPF is enabled 
		rack::simd::float_4 HPFOut = LPFOut;
		if (parameters.useHPF) {
			HPFaudioFilter.setParameters(HPFafp);
			HPFOut = HPFaudioFilter.processAudioSample(HPFOut);
		}

		rack::simd::float_4 dnC = xnL + xnR + HPFOut;

		// --- write to LEFT delay buffer with LEFT channel info
		delayBuffer_L.writeBuffer(xnL);

		// --- write to CENTRE delay buffer with RIGHT channel info
		delayBuffer_C.writeBuffer(dnC);

		// --- write to RIGHT delay buffer with RIGHT channel info
		delayBuffer_R.writeBuffer(xnR);

		// --- form mixture out = dry*xn + wet*yn
		outputL = dryMix*xnL + wetMix*ynL + wetMix*ynC;

		// --- form mixture out = dry*xn + wet*yn
		outputR = dryMix*xnR + wetMix*ynR + wetMix*ynC;
	}
	else if (parameters.algorithm == delayAlgorithm::kTapDelay) {
				// --- write to LEFT delay buffer with LEFT channel info
		delayBuffer_L.writeBuffer(dnL);

		// --- write to RIGHT delay buffer with RIGHT channel info
		delayBuffer_R.writeBuffer(dnR);

		// --- form mixture out = dry*xn + wet*yn
		outputL = dryMix*xnL + wetMix*ynL + delayBuffer_L.readBuffer(delayInSamples_L * 0.75) + delayBuffer_L.readBuffer(delayInSamples_L * 0.5) + delayBuffer_L.readBuffer(delayInSamples_L * 0.25);

		// --- form mixture out = dry*xn + wet*yn
		outputR = dryMix*xnR + wetMix*ynR + delayBuffer_R.readBuffer(delayInSamples_R * 0.75) + delayBuffer_R.readBuffer(delayInSamples_R * 0.5) + delayBuffer_R.readBuffer(delayInSamples_R * 0.25);
	}

	// --- set left channel
	outputFrame[0] = outputL;

	// --- set right channel
	outputFrame[1] = outputR;

	return true;
}

/** get parameters: note use of custom structure for passing param data */
/**
\return AudioDelayParameters custom data structure
*/
AudioDelayParameters AudioDelay::getParameters() { return parameters; }

/** set parameters: note use of custom structure for passing param data */
/**
\param AudioDelayParameters custom data structure
*/
void AudioDelay::setParameters(AudioDelayParameters _parameters)
{
	// --- check mix in dB for calc
	if (_parameters.dryLevel_dB != parameters.dryLevel_dB)
		dryMix = pow(10.0, _parameters.dryLevel_dB / 20.0);
	if (_parameters.wetLevel_dB != parameters.wetLevel_dB)
		wetMix = pow(10.0, _parameters.wetLevel_dB / 20.0);

	// --- save; rest of updates are cheap on CPU
	parameters = _parameters;

	// --- check update type first:
	if (parameters.updateType == delayUpdateType::kLeftAndRight)
	{
		// --- set left and right delay times
		// --- calculate total delay time in samples + fraction
		double newDelayInSamples_L = parameters.leftDelay_mSec*(samplesPerMSec);
		double newDelayInSamples_C = parameters.centreDelay_mSec*(samplesPerMSec);
		double newDelayInSamples_R = parameters.rightDelay_mSec*(samplesPerMSec);

		// --- new delay time with fraction
		delayInSamples_L = newDelayInSamples_L;
		delayInSamples_C = newDelayInSamples_C;
		delayInSamples_R = newDelayInSamples_R;
	}
	else if (parameters.updateType == delayUpdateType::kLeftPlusRatio)
	{
		// --- get and validate ratio
		double delayRatio = parameters.delayRatio_Pct / 100.0;
		boundValue(delayRatio, 0.0, 1.0);

		// --- calculate total delay time in samples + fraction
		double newDelayInSamples = parameters.leftDelay_mSec*(samplesPerMSec);

		// --- new delay time with fraction
		delayInSamples_L = newDelayInSamples;
		delayInSamples_C = delayInSamples_L*(delayRatio/2.0);
		delayInSamples_R = delayInSamples_L*delayRatio;
	}
}

/** creation function */
void AudioDelay::createDelayBuffers(double _sampleRate, double _bufferLength_mSec)
{
	// --- store for math
	bufferLength_mSec = _bufferLength_mSec;
	sampleRate = _sampleRate;
	samplesPerMSec = sampleRate / 1000.0;

	// --- total buffer length including fractional part
	bufferLength = (unsigned int)(bufferLength_mSec*(samplesPerMSec)) + 1; // +1 for fractional part

																			   // --- create new buffer
	delayBuffer_L.createCircularBuffer(bufferLength);
	delayBuffer_C.createCircularBuffer(bufferLength);
	delayBuffer_R.createCircularBuffer(bufferLength);
}

AudioDelayParameters::AudioDelayParameters() {
	algorithm = delayAlgorithm::kNormal; ///< delay algorithm

	wetLevel_dB = -3.0;	///< wet output level in dB
	dryLevel_dB = -3.0;	///< dry output level in dB
	feedback_Pct = 0.0;	///< feedback as a % value

	updateType = delayUpdateType::kLeftAndRight;///< update algorithm
	leftDelay_mSec = 0.0;	///< left delay time
	centreDelay_mSec = 0.0;	///< left delay time
	rightDelay_mSec = 0.0;	///< right delay time
	delayRatio_Pct = 100.0;	///< dela ratio: right length = (delayRatio)*(left length)

	lpfFc = 100.0;
	hpfFc = 100.0;
	useLPF = true;
	useHPF = true;
}

template <typename T>
CircularBuffer<T>::CircularBuffer() {}		/* C-TOR */

template <typename T>
CircularBuffer<T>::~CircularBuffer() {}	/* D-TOR */

/** flush buffer by resetting all values to 0.0 */
template <typename T>
void CircularBuffer<T>::flushBuffer(){ memset(&buffer[0], 0, bufferLength * sizeof(T)); }

/** Create a buffer based on a target maximum in SAMPLES
//	   do NOT call from realtime audio thread; do this prior to any processing */
template <typename T>
void CircularBuffer<T>::createCircularBuffer(unsigned int _bufferLength)
{
	// --- find nearest power of 2 for buffer, and create
	createCircularBufferPowerOfTwo((unsigned int)(pow(2, ceil(log(_bufferLength) / log(2)))));
}

/** Create a buffer based on a target maximum in SAMPLESwhere the size is
	pre-calculated as a power of two */
template <typename T>	
void CircularBuffer<T>::createCircularBufferPowerOfTwo(unsigned int _bufferLengthPowerOfTwo)
{
	// --- reset to top
	writeIndex = 0;

	// --- find nearest power of 2 for buffer, save it as bufferLength
	bufferLength = _bufferLengthPowerOfTwo;

	// --- save (bufferLength - 1) for use as wrapping mask
	wrapMask = bufferLength - 1;

	// --- create new buffer
	buffer.reset(new T[bufferLength]);

	// --- flush buffer
	flushBuffer();
}

/** write a value into the buffer; this overwrites the previous oldest value in the buffer */
template <typename T>
void CircularBuffer<T>::writeBuffer(T input)
{
	// --- write and increment index counter
	buffer[writeIndex++] = input;

	// --- wrap if index > bufferlength - 1
	writeIndex &= wrapMask;
}

/** read an arbitrary location that is delayInSamples old */
template <typename T>
T CircularBuffer<T>::readBuffer(int delayInSamples)//, bool readBeforeWrite = true)
{
	// --- subtract to make read index
	//     note: -1 here is because we read-before-write,
	//           so the *last* write location is what we use for the calculation
	int readIndex = (writeIndex - 1) - delayInSamples;

	// --- autowrap index
	readIndex &= wrapMask;

	// --- read it
	return buffer[readIndex];
}

/** read an arbitrary location that includes a fractional sample */
template <typename T>
T CircularBuffer<T>::readBuffer(rack::simd::float_4 delayInFractionalSamples)
{
	// --- truncate delayInFractionalSamples and read the int part
	T y1 = readBuffer(delayInFractionalSamples);

	// --- if no interpolation, just return value
	if (!interpolate) return y1;

	// --- else do interpolation
	//
	// --- read the sample at n+1 (one sample OLDER)
	T y2 = readBuffer(delayInFractionalSamples + 1);

	// --- get fractional part
	double fraction = delayInFractionalSamples - (int)delayInFractionalSamples;

	// --- do the interpolation (you could try different types here)
	return doLinearInterpolation(y1, y2, fraction);
}

/** enable or disable interpolation; usually used for diagnostics or in algorithms that require strict integer samples times */
template <typename T>
void CircularBuffer<T>::setInterpolate(bool b) { interpolate = b; }






