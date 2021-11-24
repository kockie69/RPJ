#include "rack.hpp"
#include "AudioFilter.hpp"

template <typename T>
class CircularBuffer
{
public:
	CircularBuffer() {}		/* C-TOR */
	~CircularBuffer() {}	/* D-TOR */

	/** flush buffer by resetting all values to 0.0 */
	void flushBuffer(){ memset(&buffer[0], 0, bufferLength * sizeof(T)); }


	/** Create a buffer based on a target maximum in SAMPLES
	//	   do NOT call from realtime audio thread; do this prior to any processing */

	void createCircularBuffer(unsigned int _bufferLength)
	{
		// --- find nearest power of 2 for buffer, and create
		createCircularBufferPowerOfTwo((unsigned int)(pow(2, ceil(log(_bufferLength) / log(2)))));
	}

	/** Create a buffer based on a target maximum in SAMPLESwhere the size is
		pre-calculated as a power of two */
	void createCircularBufferPowerOfTwo(unsigned int _bufferLengthPowerOfTwo)
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
	void writeBuffer(T input)
	{
		// --- write and increment index counter
		buffer[writeIndex++] = input;

		// --- wrap if index > bufferlength - 1
		writeIndex &= wrapMask;
	}

	/** read an arbitrary location that is delayInSamples old */
	T readBuffer(int delayInSamples)//, bool readBeforeWrite = true)
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
	T readBuffer(T delayInFractionalSamples)
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
		T fraction = delayInFractionalSamples - delayInFractionalSamples;

		// --- do the interpolation (you could try different types here)
		return doLinearInterpolation(y1, y2, fraction);
	}
	
	/** enable or disable interpolation; usually used for diagnostics or in algorithms that require strict integer samples times */
	void setInterpolate(bool b) { interpolate = b; }

private:
	std::unique_ptr<T[]> buffer = nullptr;	///< smart pointer will auto-delete
	unsigned int writeIndex = 0;		///> write index
	unsigned int bufferLength = 1024;	///< must be nearest power of 2
	unsigned int wrapMask = 1023;		///< must be (bufferLength - 1)
	bool interpolate = true;			///< interpolation (default is ON)
};

enum class delayUpdateType { kLeftAndRight, kLeftPlusRatio };

enum class delayAlgorithm { kNormal, kPingPong, kLCRDelay, kTapDelay, numDelayAlgorithms };

struct AudioDelayParameters
{
	AudioDelayParameters();
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	
	// --- individual parameters
	delayAlgorithm algorithm; ///< delay algorithm

	double wetLevel_dB;	///< wet output level in dB
	double dryLevel_dB;	///< dry output level in dB
	double feedback_Pct;	///< feedback as a % value

	delayUpdateType updateType;///< update algorithm
	double leftDelay_mSec;	///< left delay time
	double centreDelay_mSec;	///< left delay time
	double rightDelay_mSec;	///< right delay time
	double delayRatio_Pct;	///< dela ratio: right length = (delayRatio)*(left length)

	double lpfFc, hpfFc;
	bool useLPF, useHPF;
};

template <typename T>
struct AudioDelay
{
public:

	AudioDelay() {
		LPFaudioFilter_L.reset(APP->engine->getSampleRate());
		HPFaudioFilter_L.reset(APP->engine->getSampleRate());
		LPFaudioFilter_C.reset(APP->engine->getSampleRate());
		HPFaudioFilter_C.reset(APP->engine->getSampleRate());
		LPFaudioFilter_R.reset(APP->engine->getSampleRate());
		HPFaudioFilter_R.reset(APP->engine->getSampleRate());
		LPFafp.algorithm=filterAlgorithm::kLPF1;
		HPFafp.algorithm=filterAlgorithm::kHPF1;
	} /* C-TOR */

	~AudioDelay() {}	/* D-TOR */

public:
	/** reset members to initialized state */
	/** reset members to initialized state */
	bool reset(double _sampleRate)
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

		LPFaudioFilter_L.reset(APP->engine->getSampleRate());
		HPFaudioFilter_L.reset(APP->engine->getSampleRate());
		LPFaudioFilter_C.reset(APP->engine->getSampleRate());
		HPFaudioFilter_C.reset(APP->engine->getSampleRate());
		LPFaudioFilter_R.reset(APP->engine->getSampleRate());
		HPFaudioFilter_R.reset(APP->engine->getSampleRate());
		return true;
	}
	
	/** process MONO audio delay */
	/**
	\param xn input
	\return the processed sample
	*/
	T processAudioSample(T xn)
	{
		// --- read delay
		T yn = delayBuffer_L.readBuffer(delayInSamples_L);

		// --- create input for delay buffer
		T dn = xn + (parameters.feedback_Pct / 100.0) * yn;

		// --- write to delay buffer
		delayBuffer_L.writeBuffer(dn);

		// --- form mixture out = dry*xn + wet*yn
		T output = dryMix*xn + wetMix*yn;

		return output;
	}

	/** return true: this object can also process frames */
	bool canProcessAudioFrame() { return true; }

	/** process STEREO audio delay in frames */
	bool processAudioFrame(T* inputFrame,		/* ptr to one frame of data: pInputFrame[0] = left, pInputFrame[1] = right, etc...*/
		T* outputFrame,
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
		T outputL, outputR;
		T xnL = inputFrame[0];

		// --- RIGHT channel (duplicate left input if mono-in)
		T xnR = inputChannels > 1 ? inputFrame[1] : xnL;

		// --- read delay LEFT
		T ynL = delayBuffer_L.readBuffer(delayInSamples_L);

		// --- read delay Center
		T ynC = delayBuffer_C.readBuffer(delayInSamples_C);

		// --- read delay RIGHT
		T ynR = delayBuffer_R.readBuffer(delayInSamples_R);




 		LPFafp.fc = parameters.lpfFc;
		HPFafp.fc = parameters.hpfFc;
		LPFafp.dry = HPFafp.dry = 0;
		LPFafp.wet = HPFafp.wet = 1;

		// Check if LPF is enabled
		T LPFOutL = (parameters.feedback_Pct / 100.0) * ynL;
		T LPFOutC = (parameters.feedback_Pct / 100.0) * ynC;
		T LPFOutR = (parameters.feedback_Pct / 100.0) * ynR;

		if (parameters.useLPF) {
			LPFaudioFilter_L.setParameters(LPFafp);
			LPFOutL = LPFaudioFilter_L.processAudioSample(LPFOutL);
			LPFaudioFilter_C.setParameters(LPFafp);
			LPFOutC = LPFaudioFilter_C.processAudioSample(LPFOutC);
			LPFaudioFilter_R.setParameters(LPFafp);
			LPFOutR = LPFaudioFilter_R.processAudioSample(LPFOutR);
		}

		// Check if HPF is enabled 
		T HPFOutL = LPFOutL;
		T HPFOutC = LPFOutC;
		T HPFOutR = LPFOutR;
		if (parameters.useHPF) {
			HPFaudioFilter_L.setParameters(HPFafp);
			HPFOutL = HPFaudioFilter_L.processAudioSample(HPFOutL);
			HPFaudioFilter_C.setParameters(HPFafp);
			HPFOutC = HPFaudioFilter_C.processAudioSample(HPFOutC);
			HPFaudioFilter_R.setParameters(HPFafp);
			HPFOutR = HPFaudioFilter_R.processAudioSample(HPFOutR);
		}

		// --- create input for delay buffer with LEFT channel info
		T dnL = xnL + HPFOutL;

		// T dnC = xnL + xnR + HPFOutC;

		// --- create input for delay buffer with RIGHT channel info
		T dnR = xnR + HPFOutR;
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


			T dnC = xnL + xnR + HPFOutC;

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
	AudioDelayParameters getParameters() { return parameters; }

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
	
	/** set parameters: note use of custom structure for passing param data */
	/**
	\param AudioDelayParameters custom data structure
	*/
	void setParameters(AudioDelayParameters _parameters)
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
			// --- set left, right and centre delay times
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
	void createDelayBuffers(double _sampleRate, double _bufferLength_mSec)
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
	
	std::string delayAlgorithmTxt[static_cast<int>(delayAlgorithm::numDelayAlgorithms)] = { "Normal", "PingPong", "LCRDelay", "TapDelay"};

private:
	AudioDelayParameters parameters; ///< object parameters

	double sampleRate = 0.0;		///< current sample rate
	double samplesPerMSec = 0.0;	///< samples per millisecond, for easy access calculation
	double delayInSamples_L = 0.0;	///< double includes fractional part
	double delayInSamples_C = 0.0;	///< double includes fractional part
	double delayInSamples_R = 0.0;	///< double includes fractional part
	double bufferLength_mSec = 0.0;	///< buffer length in mSec
	unsigned int bufferLength = 0;	///< buffer length in samples
	double wetMix = 0.707; ///< wet output default = -3dB
	double dryMix = 0.707; ///< dry output default = -3dB
	bool enableLPF, enableHPF = true;

	// --- delay buffer of doubles
	CircularBuffer<T> delayBuffer_L;	///< LEFT delay buffer of doubles
	CircularBuffer<T> delayBuffer_C;	///< LEFT delay buffer of doubles
	CircularBuffer<T> delayBuffer_R;	///< RIGHT delay buffer of doubles

	AudioFilter<T> LPFaudioFilter_L;
	AudioFilter<T> HPFaudioFilter_L;
	AudioFilter<T> LPFaudioFilter_C;
	AudioFilter<T> HPFaudioFilter_C;
	AudioFilter<T> LPFaudioFilter_R;
	AudioFilter<T> HPFaudioFilter_R;
	AudioFilterParameters LPFafp,HPFafp;
};
