#include "rack.hpp"

/**
@sgn
\ingroup FX-Functions

@brief calculates sgn( ) of input
\param xn - the input value
\return -1 if xn is negative or +1 if xn is 0 or greater
*/
inline double sgn(double );


/**
@softClipWaveShaper
\ingroup FX-Functions

@brief calculates hyptan waveshaper
\param xn - the input value
\param saturation  - the saturation control
\return the waveshaped output value
*/
inline double softClipWaveShaper(double , double );


/**
@dBTo_Raw
\ingroup FX-Functions

@brief converts dB to raw value

\param dB - value to convert to raw
\return the raw value
*/
inline double dB2Raw(double );

/**
@rawTo_dB
\ingroup FX-Functions

@brief calculates dB for given input

\param raw - value to convert to dB
\return the dB value
*/
inline double raw2dB(double );

/**
@peakGainFor_Q
\ingroup FX-Functions

@brief calculates the peak magnitude for a given Q

\param Q - the Q value
\return the peak gain (not in dB)
*/
inline double peakGainFor_Q(double);

/**
@dBPeakGainFor_Q
\ingroup FX-Functions

@brief calculates the peak magnitude in dB for a given Q

\param Q - the Q value
\return the peak gain in dB
*/
inline double dBPeakGainFor_Q(double Q);

/**
\enum vaFilterAlgorithm
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the virtual analog filter algorithm

- enum class vaFilterAlgorithm { kLPF1, kHPF1, kAPF1, kSVF_LP, kSVF_HP, kSVF_BP, kSVF_BS };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum class vaFilterAlgorithm {
	kLPF1, kHPF1, kAPF1, kSVF_LP, kSVF_HP, kSVF_BP, kSVF_BS
}; // --- you will add more here...

/**
\struct ZVAFilterParameters
\ingroup FX-Objects
\brief
Custom parameter structure for the ZVAFilter object.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct ZVAFilterParameters
{
	ZVAFilterParameters();
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	//ZVAFilterParameters& operator=(const ZVAFilterParameters& );

	// --- individual parameters
	vaFilterAlgorithm filterAlgorithm = vaFilterAlgorithm::kSVF_LP;	///< va filter algorithm
	rack::simd::float_4 fc = 1000.0;						///< va filter fc
	double Q = 0.707;						///< va filter Q
	double filterOutputGain_dB = 0.0;		///< va filter gain (normally unused)
	bool enableGainComp = false;			///< enable gain compensation (see book)
	bool matchAnalogNyquistLPF = false;		///< match analog gain at Nyquist
	bool selfOscillate = false;				///< enable selfOscillation
	bool enableNLP = false;					///< enable non linear processing (use oversampling for best results)
};


/**
\class ZVAFilter
\ingroup FX-Objects
\brief
The ZVAFilter object implements multpile Zavalishin VA Filters.
Audio I/O:
- Processes mono input to mono output.

Control I/F:
- Use BiquadParameters structure to get/set object params.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class ZVAFilter
{
public:
	ZVAFilter();	/* C-TOR */
	~ZVAFilter();	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate);

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ZVAFilterParameters custom data structure
	*/
	ZVAFilterParameters getParameters();

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ZVAFilterParameters custom data structure
	*/
	void setParameters(const ZVAFilterParameters& );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the VA filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual rack::simd::float_4 processAudioSample(rack::simd::float_4 );
	/** recalculate the filter coefficients*/
	void calculateFilterCoeffs();

	/** set beta value, for filters that aggregate 1st order VA sections*/
	void setBeta(double );

	/** get beta value,not used in book projects; for future use*/
	double getBeta();

protected:
	ZVAFilterParameters zvaFilterParameters;	///< object parameters
	double sampleRate = APP->engine->getSampleRate();				///< current sample rate

	// --- state storage
	rack::simd::float_4 integrator_z[2];						///< state variables

	// --- filter coefficients
	rack::simd::float_4 alpha0 = 0.0;		///< input scalar, correct delay-free loop
	rack::simd::float_4 alpha = 0.0;			///< alpha is (wcT/2)
	rack::simd::float_4 rho = 0.0;			///< p = 2R + g (feedback)

	double beta = 0.0;			///< beta value, not used

	// --- for analog Nyquist matching
	rack::simd::float_4 analogMatchSigma = 0.0; ///< analog matching Sigma value (see book)
};
