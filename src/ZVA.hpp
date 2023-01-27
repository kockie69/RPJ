#include "rack.hpp"

template<typename T>
inline T dB2Raw(T dB)
{
	return pow(10.0, (dB / 20.0));
}

template<typename T>
inline T raw2dB(T raw)
{
	return 20.0*log10(raw);
}

template<typename T>
inline T peakGainFor_Q(T Q)
{
	// --- no resonance at or below unity
	//if (Q <= 0.707) return 1.0;
	//return (Q*Q) / (pow((Q*Q - 0.25), 0.5));

	return rack::simd::ifelse(Q <= 0.707,1.0,(Q*Q) / (pow((Q*Q - 0.25), (rack::simd::float_4)0.5)));

}

template<typename T>
inline T dBPeakGainFor_Q(T Q)
{
	return raw2dB(peakGainFor_Q(Q));
}

template<typename T>
inline T softClipWaveShaper(T xn, double saturation)
{
	// --- un-normalized soft clipper from Reiss book
	return sgn(xn)*(1.0 - exp(-fabs(saturation*xn)));
}

inline double sgn(double );

inline double softClipWaveShaper(double , double );

inline double dB2Raw(double );

inline double raw2dB(double );

inline double peakGainFor_Q(double);

inline double dBPeakGainFor_Q(double Q);

enum class vaFilterAlgorithm {
	kLPF1, kHPF1, kAPF1, kSVF_LP, kSVF_HP, kSVF_BP, kSVF_BS
};

struct ZVAFilterParameters
{
	ZVAFilterParameters();

	vaFilterAlgorithm filterAlgorithm = vaFilterAlgorithm::kSVF_LP;	///< va filter algorithm
	rack::simd::float_4 fc = 1000.0;						///< va filter fc
	rack::simd::float_4 Q = 0.707;						///< va filter Q
	rack::simd::float_4 filterOutputGain_dB = 0.0;		///< va filter gain (normally unused)
	bool enableGainComp = false;			///< enable gain compensation (see book)
	bool matchAnalogNyquistLPF = false;		///< match analog gain at Nyquist
	bool selfOscillate = false;				///< enable selfOscillation
	bool enableNLP = false;					///< enable non linear processing (use oversampling for best results)
};


template <typename T>
class ZVAFilter
{
public:
ZVAFilter() {}		/* C-TOR */
~ZVAFilter() {}		/* D-TOR */

bool reset(double _sampleRate)
{
	sampleRate = _sampleRate;
	integrator_z[0] = 0.0;
	integrator_z[1] = 0.0;

	return true;
}

ZVAFilterParameters getParameters()
{
	return zvaFilterParameters;
}

void setParameters(const ZVAFilterParameters& params)
{
	//T mask = rack::simd::operator!=(params.fc,zvaFilterParameters.fc);

//	if ((bool)rack::simd::movemask(mask) ||
//		params.Q != zvaFilterParameters.Q ||
//		params.selfOscillate != zvaFilterParameters.selfOscillate ||
//		params.matchAnalogNyquistLPF != zvaFilterParameters.matchAnalogNyquistLPF)
//	{
			zvaFilterParameters = params;
			calculateFilterCoeffs();
//	}
//	else
//		zvaFilterParameters = params;
}

bool canProcessAudioFrame() { return false; }

T processAudioSample(T xn)
{
	vaFilterAlgorithm filterAlgorithm = zvaFilterParameters.filterAlgorithm;
	bool matchAnalogNyquistLPF = zvaFilterParameters.matchAnalogNyquistLPF;
	
	// --- with gain comp enabled, we reduce the input by
	//     half the gain in dB at resonant peak
	//     NOTE: you can change that logic here!
	if (zvaFilterParameters.enableGainComp)
	{
		T peak_dB = dBPeakGainFor_Q(zvaFilterParameters.Q);
		
		xn *= ifelse(peak_dB > 0.0,dB2Raw(-peak_dB / 2.0),1);
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

	T filterOutputGain = pow(10.0, zvaFilterParameters.filterOutputGain_dB / 20.0);

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
	T Q = zvaFilterParameters.Q;
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
		T R = zvaFilterParameters.selfOscillate ? 0.0 : 1.0 / (2.0*Q);
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
