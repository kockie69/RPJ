#include "rack.hpp"

/**
@dBTo_Raw
\ingroup FX-Functions

@brief converts dB to raw value

\param dB - value to convert to raw
\return the raw value
*/
inline double dB2Raw(double dB)
{
	return pow(10.0, (dB / 20.0));
}



/**
@rawTo_dB
\ingroup FX-Functions

@brief calculates dB for given input

\param raw - value to convert to dB
\return the dB value
*/
inline double raw2dB(double raw)
{
	return 20.0*log10(raw);
}

/**
@peakGainFor_Q
\ingroup FX-Functions

@brief calculates the peak magnitude for a given Q

\param Q - the Q value
\return the peak gain (not in dB)
*/
inline double peakGainFor_Q(double Q)
{
	// --- no resonance at or below unity
	if (Q <= 0.707) return 1.0;
	return (Q*Q) / (pow((Q*Q - 0.25), 0.5));
}

/**
@dBPeakGainFor_Q
\ingroup FX-Functions

@brief calculates the peak magnitude in dB for a given Q

\param Q - the Q value
\return the peak gain in dB
*/
inline double dBPeakGainFor_Q(double Q)
{
	return raw2dB(peakGainFor_Q(Q));
}

/**
@softClipWaveShaper
\ingroup FX-Functions

@brief calculates hyptan waveshaper
\param xn - the input value
\param saturation  - the saturation control
\return the waveshaped output value
*/
template<typename T>
inline T softClipWaveShaper(T xn, double saturation)
{
	// --- un-normalized soft clipper from Reiss book
	return sgn(xn)*(1.0 - exp(-fabs(saturation*xn)));
}

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
template <typename T>
class ZVAFilter
{
public:
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
ZVAFilter() {}		/* C-TOR */
~ZVAFilter() {}		/* D-TOR */

/** reset members to initialized state */
bool reset(double _sampleRate)
{
	sampleRate = _sampleRate;
	integrator_z[0] = 0.0;
	integrator_z[1] = 0.0;

	return true;
}

/** get parameters: note use of custom structure for passing param data */
/**
\return ZVAFilterParameters custom data structure
*/
ZVAFilterParameters getParameters()
{
	return zvaFilterParameters;
}

/** set parameters: note use of custom structure for passing param data */
/**
\param ZVAFilterParameters custom data structure
*/
void setParameters(const ZVAFilterParameters& params)
{
	T mask = rack::simd::operator!=(params.fc,zvaFilterParameters.fc);

	if ((bool)rack::simd::movemask(mask) ||
		params.Q != zvaFilterParameters.Q ||
		params.selfOscillate != zvaFilterParameters.selfOscillate ||
		params.matchAnalogNyquistLPF != zvaFilterParameters.matchAnalogNyquistLPF)
	{
			zvaFilterParameters = params;
			calculateFilterCoeffs();
	}
	else
		zvaFilterParameters = params;
}

/** return false: this object only processes samples */
bool canProcessAudioFrame() { return false; }

/** process input x(n) through the VA filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
T processAudioSample(T xn)
{
	// --- with gain comp enabled, we reduce the input by
	//     half the gain in dB at resonant peak
	//     NOTE: you can change that logic here!
	vaFilterAlgorithm filterAlgorithm = zvaFilterParameters.filterAlgorithm;
	bool matchAnalogNyquistLPF = zvaFilterParameters.matchAnalogNyquistLPF;

	if (zvaFilterParameters.enableGainComp)
	{
		double peak_dB = dBPeakGainFor_Q(zvaFilterParameters.Q);
		if (peak_dB > 0.0)
		{
			double halfPeak_dBGain = dB2Raw(-peak_dB / 2.0);
			xn *= halfPeak_dBGain;
		}
	}

	// --- for 1st order filters:
	if (filterAlgorithm == vaFilterAlgorithm::kLPF1 ||
		filterAlgorithm == vaFilterAlgorithm::kHPF1 ||
		filterAlgorithm == vaFilterAlgorithm::kAPF1)
	{
		// --- create vn node
		T vn = (xn - integrator_z[0])*alpha;

		// --- form LP output
		T lpf = ((xn - integrator_z[0])*alpha) + integrator_z[0];

		// double sn = integrator_z[0];

		// --- update memory
		integrator_z[0] = vn + lpf;

		// --- form the HPF = INPUT = LPF
		T hpf = xn - lpf;

		// --- form the APF = LPF - HPF
		T apf = lpf - hpf;

		// --- set the outputs
		if (filterAlgorithm == vaFilterAlgorithm::kLPF1)
		{
			// --- this is a very close match as-is at Nyquist!
			if (matchAnalogNyquistLPF)
				return lpf + alpha*hpf;
			else
				return lpf;
		}
		else if (filterAlgorithm == vaFilterAlgorithm::kHPF1)
			return hpf;
		else if (filterAlgorithm == vaFilterAlgorithm::kAPF1)
			return apf;

		// --- unknown filter
		return xn;
	}

	// --- form the HP output first
	T hpf = alpha0*(xn - rho*integrator_z[0] - integrator_z[1]);

	// --- BPF Out
	T bpf = alpha*hpf + integrator_z[0];
	if (zvaFilterParameters.enableNLP)
		bpf = softClipWaveShaper(bpf, 1.0);

	// --- LPF Out
	T lpf = alpha*bpf + integrator_z[1];

	// --- BSF Out
	T bsf = hpf + lpf;

	// --- finite gain at Nyquist; slight error at VHF
	T sn = integrator_z[0];

	// update memory
	integrator_z[0] = alpha*hpf + bpf;
	integrator_z[1] = alpha*bpf + lpf;

	double filterOutputGain = pow(10.0, zvaFilterParameters.filterOutputGain_dB / 20.0);

	// return our selected type
	if (filterAlgorithm == vaFilterAlgorithm::kSVF_LP)
	{
		if (matchAnalogNyquistLPF)
			lpf += analogMatchSigma*(sn);
		return filterOutputGain*lpf;
	}
	else if (filterAlgorithm == vaFilterAlgorithm::kSVF_HP)
		return filterOutputGain*hpf;
	else if (filterAlgorithm == vaFilterAlgorithm::kSVF_BP)
		return filterOutputGain*bpf;
	else if (filterAlgorithm == vaFilterAlgorithm::kSVF_BS)
		return filterOutputGain*bsf;

	// --- unknown filter
	return filterOutputGain*lpf;
}

/** recalculate the filter coefficients*/
void calculateFilterCoeffs()
{
	T fc = zvaFilterParameters.fc;
	double Q = zvaFilterParameters.Q;
	vaFilterAlgorithm filterAlgorithm = zvaFilterParameters.filterAlgorithm;

	// --- normal Zavalishin SVF calculations here
	//     prewarp the cutoff- these are bilinear-transform filters
	T wd = 2*M_PI*fc;
	double Tee = 1.0 / sampleRate;
	T wa = (2.0 / Tee)*tan(wd*Tee / 2.0);
	T g = wa*Tee / 2.0;

	// --- for 1st order filters:
	if (filterAlgorithm == vaFilterAlgorithm::kLPF1 ||
		filterAlgorithm == vaFilterAlgorithm::kHPF1 ||
		filterAlgorithm == vaFilterAlgorithm::kAPF1)
	{
		// --- calculate alpha
		alpha = g / (1.0 + g);
	}
	else // state variable variety
	{
		// --- note R is the traditional analog damping factor zeta
		double R = zvaFilterParameters.selfOscillate ? 0.0 : 1.0 / (2.0*Q);
		alpha0 = 1.0 / (1.0 + 2.0*R*g + g*g);
		alpha = g;
		rho = 2.0*R + g;

		// --- sigma for analog matching version
		T f_o = (sampleRate / 2.0) / fc;
		analogMatchSigma = 1.0 / (alpha*f_o*f_o);
	}
}

/** set beta value, for filters that aggregate 1st order VA sections*/
void setBeta(double _beta) { beta = _beta; }

/** get beta value,not used in book projects; for future use*/
double getBeta() { return beta; }

protected:
	ZVAFilterParameters zvaFilterParameters;	///< object parameters
	double sampleRate = APP->engine->getSampleRate();				///< current sample rate

	// --- state storage
	T integrator_z[2];						///< state variables

	// --- filter coefficients
	T alpha0 = 0.0;		///< input scalar, correct delay-free loop
	T alpha = 0.0;			///< alpha is (wcT/2)
	T rho = 0.0;			///< p = 2R + g (feedback)

	double beta = 0.0;			///< beta value, not used

	// --- for analog Nyquist matching
	T analogMatchSigma = 0.0; ///< analog matching Sigma value (see book)
};
