#include "rack.hpp"
#include "AudioFilter.hpp"

template <typename T>
class CircularBuffer
{
public:
	CircularBuffer();	/* C-TOR */
	~CircularBuffer();	/* D-TOR */

							/** flush buffer by resetting all values to 0.0 */
	void flushBuffer();

	/** Create a buffer based on a target maximum in SAMPLES
	//	   do NOT call from realtime audio thread; do this prior to any processing */
	void createCircularBuffer(unsigned int );

	/** Create a buffer based on a target maximum in SAMPLESwhere the size is
	    pre-calculated as a power of two */
	void createCircularBufferPowerOfTwo(unsigned int );

	/** write a value into the buffer; this overwrites the previous oldest value in the buffer */
	void writeBuffer(T );

	/** read an arbitrary location that is delayInSamples old */
	T readBuffer(int ); //, bool readBeforeWrite = true)


	/** read an arbitrary location that includes a fractional sample */
	T readBuffer(rack::simd::float_4 );

	/** enable or disable interpolation; usually used for diagnostics or in algorithms that require strict integer samples times */
	void setInterpolate(bool );

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

struct AudioDelay
{
public:
	AudioDelay();	/* C-TOR */
	~AudioDelay();	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double);
	
	/** process MONO audio delay */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual rack::simd::float_4 processAudioSample(rack::simd::float_4);
	
	/** return true: this object can also process frames */
	virtual bool canProcessAudioFrame();

	/** process STEREO audio delay in frames */
	virtual bool processAudioFrame(rack::simd::float_4* ,		/* ptr to one frame of data: pInputFrame[0] = left, pInputFrame[1] = right, etc...*/
		rack::simd::float_4* ,
		uint32_t ,
		uint32_t );

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return AudioDelayParameters custom data structure
	*/
	AudioDelayParameters getParameters();

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param AudioDelayParameters custom data structure
	*/
	void setParameters(AudioDelayParameters);
	
	/** creation function */
	void createDelayBuffers(double , double );

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
	CircularBuffer<rack::simd::float_4> delayBuffer_L;	///< LEFT delay buffer of doubles
	CircularBuffer<rack::simd::float_4> delayBuffer_C;	///< LEFT delay buffer of doubles
	CircularBuffer<rack::simd::float_4> delayBuffer_R;	///< RIGHT delay buffer of doubles

	AudioFilter LPFaudioFilter;
	AudioFilter HPFaudioFilter;
	AudioFilterParameters LPFafp,HPFafp;
};


