#include "ZVA.hpp"

/**
@sgn
\ingroup FX-Functions

@brief calculates sgn( ) of input
\param xn - the input value
\return -1 if xn is negative or +1 if xn is 0 or greater
*/
inline double sgn(double xn)
{
	return (xn > 0) - (xn < 0);
}

/**
@softClipWaveShaper
\ingroup FX-Functions

@brief calculates hyptan waveshaper
\param xn - the input value
\param saturation  - the saturation control
\return the waveshaped output value
*/
inline rack::simd::float_4 softClipWaveShaper(rack::simd::float_4 xn, double saturation)
{
	// --- un-normalized soft clipper from Reiss book
	return sgn(xn)*(1.0 - exp(-fabs(saturation*xn)));
}

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
\struct ZVAFilterParameters
\ingroup FX-Objects
\brief
Custom parameter structure for the ZVAFilter object.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

ZVAFilterParameters::ZVAFilterParameters() {}
/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
/*ZVAFilterParameters::ZVAFilterParameters& operator=(const ZVAFilterParameters& params)	// need this override for collections to work
{
	if (this == &params)
		return *this;

	filterAlgorithm = params.filterAlgorithm;
	fc = params.fc;
	Q = params.Q;
	filterOutputGain_dB = params.filterOutputGain_dB;
	enableGainComp = params.enableGainComp;
	matchAnalogNyquistLPF = params.matchAnalogNyquistLPF;
	selfOscillate = params.selfOscillate;
	enableNLP = params.enableNLP;
	return *this;
}*/

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
ZVAFilter::ZVAFilter() {}		/* C-TOR */
ZVAFilter::~ZVAFilter() {}		/* D-TOR */

/** reset members to initialized state */
bool ZVAFilter::reset(double _sampleRate)
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
ZVAFilterParameters ZVAFilter::getParameters()
{
	return zvaFilterParameters;
}

/** set parameters: note use of custom structure for passing param data */
/**
\param ZVAFilterParameters custom data structure
*/
void ZVAFilter::setParameters(const ZVAFilterParameters& params)
{
	rack::simd::float_4 mask = rack::simd::operator!=(params.fc,zvaFilterParameters.fc);

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
bool ZVAFilter::canProcessAudioFrame() { return false; }

/** process input x(n) through the VA filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
rack::simd::float_4 ZVAFilter::processAudioSample(rack::simd::float_4 xn)
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
		rack::simd::float_4 vn = (xn - integrator_z[0])*alpha;

		// --- form LP output
		rack::simd::float_4 lpf = ((xn - integrator_z[0])*alpha) + integrator_z[0];

		// double sn = integrator_z[0];

		// --- update memory
		integrator_z[0] = vn + lpf;

		// --- form the HPF = INPUT = LPF
		rack::simd::float_4 hpf = xn - lpf;

		// --- form the APF = LPF - HPF
		rack::simd::float_4 apf = lpf - hpf;

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
	rack::simd::float_4 hpf = alpha0*(xn - rho*integrator_z[0] - integrator_z[1]);

	// --- BPF Out
	rack::simd::float_4 bpf = alpha*hpf + integrator_z[0];
	if (zvaFilterParameters.enableNLP)
		bpf = softClipWaveShaper(bpf, 1.0);

	// --- LPF Out
	rack::simd::float_4 lpf = alpha*bpf + integrator_z[1];

	// --- BSF Out
	rack::simd::float_4 bsf = hpf + lpf;

	// --- finite gain at Nyquist; slight error at VHF
	rack::simd::float_4 sn = integrator_z[0];

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
void ZVAFilter::calculateFilterCoeffs()
{
	rack::simd::float_4 fc = zvaFilterParameters.fc;
	double Q = zvaFilterParameters.Q;
	vaFilterAlgorithm filterAlgorithm = zvaFilterParameters.filterAlgorithm;

	// --- normal Zavalishin SVF calculations here
	//     prewarp the cutoff- these are bilinear-transform filters
	rack::simd::float_4 wd = 2*M_PI*fc;
	double T = 1.0 / sampleRate;
	rack::simd::float_4 wa = (2.0 / T)*tan(wd*T / 2.0);
	rack::simd::float_4 g = wa*T / 2.0;

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
		rack::simd::float_4 f_o = (sampleRate / 2.0) / fc;
		analogMatchSigma = 1.0 / (alpha*f_o*f_o);
	}
}

/** set beta value, for filters that aggregate 1st order VA sections*/
void ZVAFilter::setBeta(double _beta) { beta = _beta; }

/** get beta value,not used in book projects; for future use*/
double ZVAFilter::getBeta() { return beta; }
