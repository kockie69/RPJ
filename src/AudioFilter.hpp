#include "BiQuad.hpp"

const double kSqrtTwo = pow(2.0, 0.5);

enum class filterAlgorithm {
	kLPF1, kHPF1, kLPF2, kHPF2, kBPF2, kBSF2, kButterLPF2, kButterHPF2, kButterBPF2,
	kButterBSF2, kMMALPF2, kMMALPF2B, kLowShelf, kHiShelf, kNCQParaEQ, kCQParaEQ, kLWRLPF2, kLWRHPF2,
	kAPF1, kAPF2, kResonA, kResonB, kImpInvLP1, kImpInvLP2, numFilterAlgorithms }; 


struct AudioFilterParameters
{
	AudioFilterParameters(){}
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	/*AudioFilterParameters& operator=(const AudioFilterParameters& params)	// need this override for collections to work
	{
		if (this == &params)
			return *this;
		algorithm = params.algorithm;
		fc = params.fc;
		Q = params.Q;
		boostCut_dB = params.boostCut_dB;

		return *this;
	}*/

	// --- individual parameters
	filterAlgorithm algorithm = filterAlgorithm::kLPF1; ///< filter algorithm

	rack::simd::float_4 fc = 100.0; ///< filter cutoff or center frequency (Hz)
	rack::simd::float_4 Q = 0.707; ///< filter Q
	rack::simd::float_4 boostCut_dB = 0.0; ///< filter gain; note not used in all types
	rack::simd::float_4 wet = 0.f;
	rack::simd::float_4 dry = 1.f;
	rack::simd::float_4 drive = 1.f;
	biquadAlgorithm bqa = kDirect;
};

template <typename T>
struct AudioFilter {
public:
	AudioFilter() {}		/* C-TOR */
	~AudioFilter() {}		/* D-TOR */

	// --- IAudioSignalProcessor
	/** --- set sample rate, then update coeffs */

	bool reset(double _sampleRate) {

		bqp = biquad.getParameters();

		bqp.biquadCalcType = audioFilterParameters.bqa; 
		biquad.setParameters(bqp); 

		sampleRate = _sampleRate;
		return biquad.reset(_sampleRate);
	}

	/** return false: this object only processes samples */
	bool canProcessAudioFrame() { 
		return false; 
	}

	/** process input x(n) through the filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	T processAudioSample(T xn)
	{
		// --- let biquad do the grunt-work
		//
		// return (dry) + (processed): x(n)*d0 + y(n)*c0
		return coeffArray[d0] * xn + coeffArray[c0] * biquad.processAudioSample(xn);
	}

	/** --- sample rate change necessarily requires recalculation */
	void setSampleRate(double _sampleRate) {
		if (sampleRate!=_sampleRate) {
			sampleRate = _sampleRate;
			calculateFilterCoeffs();
		}
	}

	/** --- get parameters */
	AudioFilterParameters getParameters() { 
		return audioFilterParameters;
	}

	/** --- set parameters */
	void setParameters(const AudioFilterParameters& parameters) {

		audioFilterParameters = parameters;
		bqp.biquadCalcType = audioFilterParameters.bqa;
		biquad.setParameters(bqp);

		// --- don't allow 0 or (-) values for Q
		rack::simd::ifelse(audioFilterParameters.Q <= 0, 0.707,audioFilterParameters.Q);

		// --- update coeffs
		calculateFilterCoeffs();

	}

	/** --- helper for Harma filters (phaser) */
	double getG_value() { 
		return biquad.getG_value(); 
	}

	/** --- helper for Harma filters (phaser) */
	double getS_value();

protected:
	// --- our calculator
	Biquad<T> biquad; ///< the biquad object

	// --- array to hold coeffs (we need them too)
	rack::simd::float_4 coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; ///< our local copy of biquad coeffs

	// --- object parameters
	AudioFilterParameters audioFilterParameters; ///< parameters
	double sampleRate = 44100.0; ///< current sample rate

	/** --- function to recalculate coefficients due to a change in filter parameters */
	bool calculateFilterCoeffs()
	{
		// --- clear coeff array
		memset(&coeffArray[0], 0, sizeof(rack::simd::float_4)*numCoeffs);

		// --- set default pass-through
		coeffArray[a0] = 1.0;

		coeffArray[c0] = audioFilterParameters.wet;
		coeffArray[d0] = audioFilterParameters.dry;

		// --- grab these variables, to make calculations look more like the book
		filterAlgorithm algorithm = audioFilterParameters.algorithm;
		rack::simd::float_4 fc = audioFilterParameters.fc;
		rack::simd::float_4 Q = audioFilterParameters.Q;
		rack::simd::float_4 boostCut_dB = audioFilterParameters.boostCut_dB;

		// --- decode filter type and calculate accordingly
		// --- impulse invariabt LPF, matches closely with one-pole version,
		//     but diverges at VHF
		if (algorithm == filterAlgorithm::kImpInvLP1)
		{
			double Tee = 1.0 / sampleRate;
			rack::simd::float_4 omega = 2.0*M_PI*fc;
			rack::simd::float_4 eT = rack::simd::exp(-Tee*omega);

			coeffArray[a0] = 1.0 - eT; // <--- normalized by 1-e^aT
			coeffArray[a1] = 0.0;
			coeffArray[a2] = 0.0;
			coeffArray[b1] = -eT;
			coeffArray[b2] = 0.0;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;

		}
		else if (algorithm == filterAlgorithm::kImpInvLP2)
		{
			rack::simd::float_4 alpha = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 p_Re = -alpha / (2.0*Q);
			rack::simd::float_4 zeta = 1.0 / (2.0 * Q);
			rack::simd::float_4 p_Im = alpha*pow((1.0 - (zeta*zeta)), 0.5);
			double c_Re = 0.0;
			rack::simd::float_4 c_Im = alpha / (2.0*pow((1.0 - (zeta*zeta)), 0.5));

			rack::simd::float_4 eP_re = rack::simd::exp(p_Re);
			coeffArray[a0] = c_Re;
			coeffArray[a1] = -2.0*(c_Re*cos(p_Im) + c_Im*sin(p_Im))*exp(p_Re);
			coeffArray[a2] = 0.0;
			coeffArray[b1] = -2.0*eP_re*cos(p_Im);
			coeffArray[b2] = eP_re*eP_re;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		// --- kMatchLP2A = TIGHT fit LPF vicanek algo
		/*else if (algorithm == filterAlgorithm::kMatchLP2A)
		{
			// http://vicanek.de/articles/BiquadFits.pdf
			double theta_c = 2.0*M_PI*fc / sampleRate;

			double q = 1.0 / (2.0*Q);

			// --- impulse invariant
			double b_1 = 0.0;
			double b_2 = exp(-2.0*q*theta_c);
			if (q <= 1.0)
			{
				b_1 = -2.0*exp(-q*theta_c)*cos(pow((1.0 - q*q), 0.5)*theta_c);
			}
			else
			{
				b_1 = -2.0*exp(-q*theta_c)*cosh(pow((q*q - 1.0), 0.5)*theta_c);
			}

			// --- TIGHT FIT --- //
			double B0 = (1.0 + b_1 + b_2)*(1.0 + b_1 + b_2);
			double B1 = (1.0 - b_1 + b_2)*(1.0 - b_1 + b_2);
			double B2 = -4.0*b_2;

			double phi_0 = 1.0 - sin(theta_c / 2.0)*sin(theta_c / 2.0);
			double phi_1 = sin(theta_c / 2.0)*sin(theta_c / 2.0);
			double phi_2 = 4.0*phi_0*phi_1;

			double R1 = (B0*phi_0 + B1*phi_1 + B2*phi_2)*(Q*Q);
			double A0 = B0;
			double A1 = (R1 - A0*phi_0) / phi_1;

			if (A0 < 0.0)
				A0 = 0.0;
			if (A1 < 0.0)
				A1 = 0.0;

			double a_0 = 0.5*(pow(A0, 0.5) + pow(A1, 0.5));
			double a_1 = pow(A0, 0.5) - a_0;
			double a_2 = 0.0;

			coeffArray[a0] = a_0;
			coeffArray[a1] = a_1;
			coeffArray[a2] = a_2;
			coeffArray[b1] = b_1;
			coeffArray[b2] = b_2;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		// --- kMatchLP2B = LOOSE fit LPF vicanek algo
		else if (algorithm == filterAlgorithm::kMatchLP2B)
		{
			// http://vicanek.de/articles/BiquadFits.pdf
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double q = 1.0 / (2.0*Q);

			// --- impulse invariant
			double b_1 = 0.0;
			double b_2 = exp(-2.0*q*theta_c);
			if (q <= 1.0)
			{
				b_1 = -2.0*exp(-q*theta_c)*cos(pow((1.0 - q*q), 0.5)*theta_c);
			}
			else
			{
				b_1 = -2.0*exp(-q*theta_c)*cosh(pow((q*q - 1.0), 0.5)*theta_c);
			}

			// --- LOOSE FIT --- //
			double f0 = theta_c / M_PI; // note f0 = fraction of pi, so that f0 = 1.0 = pi = Nyquist

			double r0 = 1.0 + b_1 + b_2;
			double denom = (1.0 - f0*f0)*(1.0 - f0*f0) + (f0*f0) / (Q*Q);
			denom = pow(denom, 0.5);
			double r1 = ((1.0 - b_1 + b_2)*f0*f0) / (denom);

			double a_0 = (r0 + r1) / 2.0;
			double a_1 = r0 - a_0;
			double a_2 = 0.0;

			coeffArray[a0] = a_0;
			coeffArray[a1] = a_1;
			coeffArray[a2] = a_2;
			coeffArray[b1] = b_1;
			coeffArray[b2] = b_2;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		// --- kMatchBP2A = TIGHT fit BPF vicanek algo
		else if (algorithm == filterAlgorithm::kMatchBP2A)
		{
			// http://vicanek.de/articles/BiquadFits.pdf
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double q = 1.0 / (2.0*Q);

			// --- impulse invariant
			double b_1 = 0.0;
			double b_2 = exp(-2.0*q*theta_c);
			if (q <= 1.0)
			{
				b_1 = -2.0*exp(-q*theta_c)*cos(pow((1.0 - q*q), 0.5)*theta_c);
			}
			else
			{
				b_1 = -2.0*exp(-q*theta_c)*cosh(pow((q*q - 1.0), 0.5)*theta_c);
			}

			// --- TIGHT FIT --- //
			double B0 = (1.0 + b_1 + b_2)*(1.0 + b_1 + b_2);
			double B1 = (1.0 - b_1 + b_2)*(1.0 - b_1 + b_2);
			double B2 = -4.0*b_2;

			double phi_0 = 1.0 - sin(theta_c / 2.0)*sin(theta_c / 2.0);
			double phi_1 = sin(theta_c / 2.0)*sin(theta_c / 2.0);
			double phi_2 = 4.0*phi_0*phi_1;

			double R1 = B0*phi_0 + B1*phi_1 + B2*phi_2;
			double R2 = -B0 + B1 + 4.0*(phi_0 - phi_1)*B2;

			double A2 = (R1 - R2*phi_1) / (4.0*phi_1*phi_1);
			double A1 = R2 + 4.0*(phi_1 - phi_0)*A2;

			double a_1 = -0.5*(pow(A1, 0.5));
			double a_0 = 0.5*(pow((A2 + (a_1*a_1)), 0.5) - a_1);
			double a_2 = -a_0 - a_1;

			coeffArray[a0] = a_0;
			coeffArray[a1] = a_1;
			coeffArray[a2] = a_2;
			coeffArray[b1] = b_1;
			coeffArray[b2] = b_2;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		// --- kMatchBP2B = LOOSE fit BPF vicanek algo
		else if (algorithm == filterAlgorithm::kMatchBP2B)
		{
			// http://vicanek.de/articles/BiquadFits.pdf
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double q = 1.0 / (2.0*Q);

			// --- impulse invariant
			double b_1 = 0.0;
			double b_2 = exp(-2.0*q*theta_c);
			if (q <= 1.0)
			{
				b_1 = -2.0*exp(-q*theta_c)*cos(pow((1.0 - q*q), 0.5)*theta_c);
			}
			else
			{
				b_1 = -2.0*exp(-q*theta_c)*cosh(pow((q*q - 1.0), 0.5)*theta_c);
			}

			// --- LOOSE FIT --- //
			double f0 = theta_c / M_PI; // note f0 = fraction of pi, so that f0 = 1.0 = pi = Nyquist

			double r0 = (1.0 + b_1 + b_2) / (M_PI*f0*Q);
			double denom = (1.0 - f0*f0)*(1.0 - f0*f0) + (f0*f0) / (Q*Q);
			denom = pow(denom, 0.5);

			double r1 = ((1.0 - b_1 + b_2)*(f0 / Q)) / (denom);

			double a_1 = -r1 / 2.0;
			double a_0 = (r0 - a_1) / 2.0;
			double a_2 = -a_0 - a_1;

			coeffArray[a0] = a_0;
			coeffArray[a1] = a_1;
			coeffArray[a2] = a_2;
			coeffArray[b1] = b_1;
			coeffArray[b2] = b_2;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}*/
		else if (algorithm == filterAlgorithm::kLPF1)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 gamma = cos(theta_c) / (1.0 + sin(theta_c));

			// --- update coeffs
			coeffArray[a0] = (1.0 - gamma) / 2.0;
			coeffArray[a1] = (1.0 - gamma) / 2.0;
			coeffArray[a2] = 0.0;
			coeffArray[b1] = -gamma;
			coeffArray[b2] = 0.0;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kHPF1)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 gamma = cos(theta_c) / (1.0 + sin(theta_c));

			// --- update coeffs
			coeffArray[a0] = (1.0 + gamma) / 2.0;
			coeffArray[a1] = -(1.0 + gamma) / 2.0;
			coeffArray[a2] = 0.0;
			coeffArray[b1] = -gamma;
			coeffArray[b2] = 0.0;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kLPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 d = 1.0 / Q;
			rack::simd::float_4 betaNumerator = 1.0 - ((d / 2.0)*(sin(theta_c)));
			rack::simd::float_4 betaDenominator = 1.0 + ((d / 2.0)*(sin(theta_c)));

			rack::simd::float_4 beta = 0.5*(betaNumerator / betaDenominator);
			rack::simd::float_4 gamma = (0.5 + beta)*(cos(theta_c));
			rack::simd::float_4 alpha = (0.5 + beta - gamma) / 2.0;

			// --- update coeffs
			coeffArray[a0] = alpha;
			coeffArray[a1] = 2.0*alpha;
			coeffArray[a2] = alpha;
			coeffArray[b1] = -2.0*gamma;
			coeffArray[b2] = 2.0*beta;
		
			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kHPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 d = 1.0 / Q;

			rack::simd::float_4 betaNumerator = 1.0 - ((d / 2.0)*(sin(theta_c)));
			rack::simd::float_4 betaDenominator = 1.0 + ((d / 2.0)*(sin(theta_c)));

			rack::simd::float_4 beta = 0.5*(betaNumerator / betaDenominator);
			rack::simd::float_4 gamma = (0.5 + beta)*(cos(theta_c));
			rack::simd::float_4 alpha = (0.5 + beta + gamma) / 2.0;

			// --- update coeffs
			coeffArray[a0] = alpha;
			coeffArray[a1] = -2.0*alpha;
			coeffArray[a2] = alpha;
			coeffArray[b1] = -2.0*gamma;
			coeffArray[b2] = 2.0*beta;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kBPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 K = tan(M_PI*fc / sampleRate);
			rack::simd::float_4
			delta = K*K*Q + K + Q;

			// --- update coeffs
			coeffArray[a0] = K / delta;;
			coeffArray[a1] = 0.0;
			coeffArray[a2] = -K / delta;
			coeffArray[b1] = 2.0*Q*(K*K - 1) / delta;
			coeffArray[b2] = (K*K*Q - K + Q) / delta;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kBSF2)
		{
			// --- see book for formulae
			rack::simd::float_4 K = tan(M_PI*fc / sampleRate);
			rack::simd::float_4 delta = K*K*Q + K + Q;

			// --- update coeffs
			coeffArray[a0] = Q*(1 + K*K) / delta;
			coeffArray[a1] = 2.0*Q*(K*K - 1) / delta;
			coeffArray[a2] = Q*(1 + K*K) / delta;
			coeffArray[b1] = 2.0*Q*(K*K - 1) / delta;
			coeffArray[b2] = (K*K*Q - K + Q) / delta;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kButterLPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = M_PI*fc / sampleRate;
			rack::simd::float_4 C = 1.0 / tan(theta_c);

			// --- update coeffs
			coeffArray[a0] = 1.0 / (1.0 + kSqrtTwo*C + C*C);
			coeffArray[a1] = 2.0*coeffArray[a0];
			coeffArray[a2] = coeffArray[a0];
			coeffArray[b1] = 2.0*coeffArray[a0] * (1.0 - C*C);
			coeffArray[b2] = coeffArray[a0] * (1.0 - kSqrtTwo*C + C*C);

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kButterHPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = M_PI*fc / sampleRate;
			rack::simd::float_4 C = tan(theta_c);

			// --- update coeffs
			coeffArray[a0] = 1.0 / (1.0 + kSqrtTwo*C + C*C);
			coeffArray[a1] = -2.0*coeffArray[a0];
			coeffArray[a2] = coeffArray[a0];
			coeffArray[b1] = 2.0*coeffArray[a0] * (C*C - 1.0);
			coeffArray[b2] = coeffArray[a0] * (1.0 - kSqrtTwo*C + C*C);

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kButterBPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 BW = fc / Q;
			rack::simd::float_4 delta_c = M_PI*BW / sampleRate;
			delta_c = rack::simd::ifelse(delta_c >= 0.95*M_PI / 2.0,0.95*M_PI / 2.0,delta_c);

			rack::simd::float_4 C = 1.0 / tan(delta_c);
			rack::simd::float_4 D = 2.0*cos(theta_c);

			// --- update coeffs
			coeffArray[a0] = 1.0 / (1.0 + C);
			coeffArray[a1] = 0.0;
			coeffArray[a2] = -coeffArray[a0];
			coeffArray[b1] = -coeffArray[a0] * (C*D);
			coeffArray[b2] = coeffArray[a0] * (C - 1.0);

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kButterBSF2)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 BW = fc / Q;
			rack::simd::float_4 delta_c = M_PI*BW / sampleRate;
			delta_c = rack::simd::ifelse(delta_c >= 0.95*M_PI / 2.0,0.95*M_PI / 2.0,delta_c);

			rack::simd::float_4 C = tan(delta_c);
			rack::simd::float_4 D = 2.0*cos(theta_c);

			// --- update coeffs
			coeffArray[a0] = 1.0 / (1.0 + C);
			coeffArray[a1] = -coeffArray[a0] * D;
			coeffArray[a2] = coeffArray[a0];
			coeffArray[b1] = -coeffArray[a0] * D;
			coeffArray[b2] = coeffArray[a0] * (1.0 - C);

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kMMALPF2 || algorithm == filterAlgorithm::kMMALPF2B)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 resonance_dB = 0;

			rack::simd::float_4 peak = rack::simd::ifelse(Q > 0.707, Q*Q / rack::simd::sqrt(Q*Q - 0.25),0);
			resonance_dB = rack::simd::ifelse(Q > 0.707,20.0*log10(peak),0);

			// --- intermediate vars
			rack::simd::float_4 resonance = (cos(theta_c) + (sin(theta_c) * sqrt(pow(10.0, (resonance_dB / 10.0)) - 1))) / ((pow(10.0, (resonance_dB / 20.0)) * sin(theta_c)) + 1);
			rack::simd::float_4 g = pow(10.0, (-resonance_dB / 40.0));

			// --- kMMALPF2B disables the GR with increase in Q
			if (algorithm == filterAlgorithm::kMMALPF2B)
				g = 1.0;

			rack::simd::float_4 filter_b1 = (-2.0) * resonance * cos(theta_c);
			rack::simd::float_4 filter_b2 = resonance * resonance;
			rack::simd::float_4 filter_a0 = g * (1 + filter_b1 + filter_b2);

			// --- update coeffs
			coeffArray[a0] = filter_a0;
			coeffArray[a1] = 0.0;
			coeffArray[a2] = 0.0;
			coeffArray[b1] = filter_b1;
			coeffArray[b2] = filter_b2;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kLowShelf)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 mu = pow(10.0, boostCut_dB / 20.0);

			rack::simd::float_4 beta = 4.0 / (1.0 + mu);
			rack::simd::float_4 delta = beta*tan(theta_c / 2.0);
			rack::simd::float_4 gamma = (1.0 - delta) / (1.0 + delta);

			// --- update coeffs
			coeffArray[a0] = (1.0 - gamma) / 2.0;
			coeffArray[a1] = (1.0 - gamma) / 2.0;
			coeffArray[a2] = 0.0;
			coeffArray[b1] = -gamma;
			coeffArray[b2] = 0.0;

			coeffArray[c0] = mu - 1.0;
			coeffArray[d0] = 1.0;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kHiShelf)
		{
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 mu = pow(10.0, boostCut_dB / 20.0);

			rack::simd::float_4 beta = (1.0 + mu) / 4.0;
			rack::simd::float_4 delta = beta*tan(theta_c / 2.0);
			rack::simd::float_4 gamma = (1.0 - delta) / (1.0 + delta);

			coeffArray[a0] = (1.0 + gamma) / 2.0;
			coeffArray[a1] = -coeffArray[a0];
			coeffArray[a2] = 0.0;
			coeffArray[b1] = -gamma;
			coeffArray[b2] = 0.0;

			coeffArray[c0] = mu - 1.0;
			coeffArray[d0] = 1.0;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kCQParaEQ)
		{
			// --- see book for formulae
			rack::simd::float_4 K = tan(M_PI*fc / sampleRate);
			rack::simd::float_4 Vo = pow(10.0, boostCut_dB / 20.0);
			rack::simd::float_4 bBoost = rack::simd::ifelse(boostCut_dB >= 0,1.f,0.f);

			rack::simd::float_4 d0 = 1.0 + (1.0 / Q)*K + K*K;
			rack::simd::float_4 e0 = 1.0 + (1.0 / (Vo*Q))*K + K*K;
			rack::simd::float_4 alpha = 1.0 + (Vo / Q)*K + K*K;
			rack::simd::float_4 beta = 2.0*(K*K - 1.0);
			rack::simd::float_4 gamma = 1.0 - (Vo / Q)*K + K*K;
			rack::simd::float_4 delta = 1.0 - (1.0 / Q)*K + K*K;
			rack::simd::float_4 eta = 1.0 - (1.0 / (Vo*Q))*K + K*K;

			// --- update coeffs
			coeffArray[a0] = rack::simd::ifelse(bBoost==1.f,alpha / d0, d0 / e0);
			coeffArray[a1] = rack::simd::ifelse(bBoost==1.f,beta / d0 , beta / e0);
			coeffArray[a2] = rack::simd::ifelse(bBoost==1.f,gamma / d0 , delta / e0);
			coeffArray[b1] = rack::simd::ifelse(bBoost==1.f,beta / d0 , beta / e0);
			coeffArray[b2] = rack::simd::ifelse(bBoost==1.f,delta / d0 , eta / e0);

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kNCQParaEQ)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 mu = pow(10.0, boostCut_dB / 20.0);

			// --- clamp to 0.95 pi/2 (you can experiment with this)
			rack::simd::float_4 tanArg = theta_c / (2.0 * Q);
			tanArg = rack::simd::ifelse(tanArg >= 0.95*M_PI / 2.0,0.95*M_PI / 2.0,tanArg);

			// --- intermediate variables (you can condense this if you wish)
			rack::simd::float_4 zeta = 4.0 / (1.0 + mu);
			rack::simd::float_4 betaNumerator = 1.0 - zeta*tan(tanArg);
			rack::simd::float_4 betaDenominator = 1.0 + zeta*tan(tanArg);

			rack::simd::float_4 beta = 0.5*(betaNumerator / betaDenominator);
			rack::simd::float_4 gamma = (0.5 + beta)*(cos(theta_c));
			rack::simd::float_4 alpha = (0.5 - beta);

			// --- update coeffs
			coeffArray[a0] = alpha;
			coeffArray[a1] = 0.0;
			coeffArray[a2] = -alpha;
			coeffArray[b1] = -2.0*gamma;
			coeffArray[b2] = 2.0*beta;

			coeffArray[c0] = mu - 1.0;
			coeffArray[d0] = 1.0;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kLWRLPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 omega_c = M_PI*fc;
			rack::simd::float_4 theta_c = M_PI*fc / sampleRate;

			rack::simd::float_4 k = omega_c / tan(theta_c);
			rack::simd::float_4 denominator = k*k + omega_c*omega_c + 2.0*k*omega_c;
			rack::simd::float_4 b1_Num = -2.0*k*k + 2.0*omega_c*omega_c;
			rack::simd::float_4 b2_Num = -2.0*k*omega_c + k*k + omega_c*omega_c;

			// --- update coeffs
			coeffArray[a0] = omega_c*omega_c / denominator;
			coeffArray[a1] = 2.0*omega_c*omega_c / denominator;
			coeffArray[a2] = coeffArray[a0];
			coeffArray[b1] = b1_Num / denominator;
			coeffArray[b2] = b2_Num / denominator;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kLWRHPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 omega_c = M_PI*fc;
			rack::simd::float_4 theta_c = M_PI*fc / sampleRate;

			rack::simd::float_4 k = omega_c / tan(theta_c);
			rack::simd::float_4 denominator = k*k + omega_c*omega_c + 2.0*k*omega_c;
			rack::simd::float_4 b1_Num = -2.0*k*k + 2.0*omega_c*omega_c;
			rack::simd::float_4 b2_Num = -2.0*k*omega_c + k*k + omega_c*omega_c;

			// --- update coeffs
			coeffArray[a0] = k*k / denominator;
			coeffArray[a1] = -2.0*k*k / denominator;
			coeffArray[a2] = coeffArray[a0];
			coeffArray[b1] = b1_Num / denominator;
			coeffArray[b2] = b2_Num / denominator;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kAPF1)
		{
			// --- see book for formulae
			rack::simd::float_4 alphaNumerator = tan((M_PI*fc) / sampleRate) - 1.0;
			rack::simd::float_4 alphaDenominator = tan((M_PI*fc) / sampleRate) + 1.0;
			rack::simd::float_4 alpha = alphaNumerator / alphaDenominator;

			// --- update coeffs
			coeffArray[a0] = alpha;
			coeffArray[a1] = 1.0;
			coeffArray[a2] = 0.0;
			coeffArray[b1] = alpha;
			coeffArray[b2] = 0.0;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kAPF2)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 BW = fc / Q;
			rack::simd::float_4 argTan = M_PI*BW / sampleRate;
			argTan = rack::simd::ifelse(argTan >= 0.95*M_PI / 2.0, 0.95*M_PI / 2.0,argTan);

			rack::simd::float_4 alphaNumerator = tan(argTan) - 1.0;
			rack::simd::float_4 alphaDenominator = tan(argTan) + 1.0;
			rack::simd::float_4 alpha = alphaNumerator / alphaDenominator;
			rack::simd::float_4 beta = -cos(theta_c);

			// --- update coeffs
			coeffArray[a0] = -alpha;
			coeffArray[a1] = beta*(1.0 - alpha);
			coeffArray[a2] = 1.0;
			coeffArray[b1] = beta*(1.0 - alpha);
			coeffArray[b2] = -alpha;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kResonA)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 BW = fc / Q;
			rack::simd::float_4 filter_b2 = exp(-2.0*M_PI*(BW / sampleRate));
			rack::simd::float_4 filter_b1 = ((-4.0*filter_b2) / (1.0 + filter_b2))*cos(theta_c);
			rack::simd::float_4 filter_a0 = (1.0 - filter_b2)*sqrt((1.0 - (filter_b1*filter_b1) / (4.0 * filter_b2)));

			// --- update coeffs
			coeffArray[a0] = filter_a0;
			coeffArray[a1] = 0.0;
			coeffArray[a2] = 0.0;
			coeffArray[b1] = filter_b1;
			coeffArray[b2] = filter_b2;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kResonB)
		{
			// --- see book for formulae
			rack::simd::float_4 theta_c = 2.0*M_PI*fc / sampleRate;
			rack::simd::float_4 BW = fc / Q;
			rack::simd::float_4 filter_b2 = exp(-2.0*M_PI*(BW / sampleRate));
			rack::simd::float_4 filter_b1 = ((-4.0*filter_b2) / (1.0 + filter_b2))*cos(theta_c);
			rack::simd::float_4 filter_a0 = 1.0 - sqrt(filter_b2); // (1.0 - filter_b2)*pow((1.0 - (filter_b1*filter_b1) / (4.0 * filter_b2)), 0.5);

			// --- update coeffs
			coeffArray[a0] = filter_a0;
			coeffArray[a1] = 0.0;
			coeffArray[a2] = -filter_a0;
			coeffArray[b1] = filter_b1;
			coeffArray[b2] = filter_b2;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}

		// --- we did n't update :(
		return false;
	}
	
	
	BiquadParameters bqp;

};