#include "BiQuad.hpp"

const double kSqrtTwo = pow(2.0, 0.5);

enum class filterAlgorithm {
	kLPF1, kHPF1, kLPF2, kHPF2, kBPF2, kBSF2, kButterLPF2, kButterHPF2, kButterBPF2,
	kButterBSF2, kMMALPF2, kMMALPF2B, kLowShelf, kHiShelf, kNCQParaEQ, kCQParaEQ, kLWRLPF2, kLWRHPF2,
	kAPF1, kAPF2, kResonA, kResonB, kMatchLP2A, kMatchLP2B, kMatchBP2A, kMatchBP2B,
	kImpInvLP1, kImpInvLP2, numFilterAlgorithms
}; 


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
	filterAlgorithm algorithm = filterAlgorithm::kMatchLP2A; ///< filter algorithm

	double fc = 100.0; ///< filter cutoff or center frequency (Hz)
	double Q = 0.707; ///< filter Q
	double boostCut_dB = 0.0; ///< filter gain; note not used in all types
	double wet = 0;
	double dry = 1;
	biquadAlgorithm bqa;
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
		if (audioFilterParameters.algorithm != parameters.algorithm ||
			audioFilterParameters.boostCut_dB != parameters.boostCut_dB ||
			audioFilterParameters.fc != parameters.fc ||
			audioFilterParameters.Q != parameters.Q ||
			audioFilterParameters.dry != parameters.dry ||
			audioFilterParameters.wet != parameters.wet ||
			audioFilterParameters.bqa != parameters.bqa)
		{
			// --- save new params
			audioFilterParameters = parameters;
			bqp.biquadCalcType = audioFilterParameters.bqa; 
			biquad.setParameters(bqp);
		}
		else
			return;

		// --- don't allow 0 or (-) values for Q
		if (audioFilterParameters.Q <= 0)
			audioFilterParameters.Q = 0.707;

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
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; ///< our local copy of biquad coeffs

	// --- object parameters
	AudioFilterParameters audioFilterParameters; ///< parameters
	double sampleRate = 44100.0; ///< current sample rate

	/** --- function to recalculate coefficients due to a change in filter parameters */
	bool calculateFilterCoeffs()
	{
		// --- clear coeff array
		memset(&coeffArray[0], 0, sizeof(double)*numCoeffs);

		// --- set default pass-through
		coeffArray[a0] = 1.0;

		coeffArray[c0] = audioFilterParameters.wet;
		coeffArray[d0] = audioFilterParameters.dry;

		// --- grab these variables, to make calculations look more like the book
		filterAlgorithm algorithm = audioFilterParameters.algorithm;
		double fc = audioFilterParameters.fc;
		double Q = audioFilterParameters.Q;
		double boostCut_dB = audioFilterParameters.boostCut_dB;

		// --- decode filter type and calculate accordingly
		// --- impulse invariabt LPF, matches closely with one-pole version,
		//     but diverges at VHF
		if (algorithm == filterAlgorithm::kImpInvLP1)
		{
			double Tee = 1.0 / sampleRate;
			double omega = 2.0*M_PI*fc;
			double eT = exp(-Tee*omega);

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
			double alpha = 2.0*M_PI*fc / sampleRate;
			double p_Re = -alpha / (2.0*Q);
			double zeta = 1.0 / (2.0 * Q);
			double p_Im = alpha*pow((1.0 - (zeta*zeta)), 0.5);
			double c_Re = 0.0;
			double c_Im = alpha / (2.0*pow((1.0 - (zeta*zeta)), 0.5));

			double eP_re = exp(p_Re);
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
		else if (algorithm == filterAlgorithm::kMatchLP2A)
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
		}
		else if (algorithm == filterAlgorithm::kLPF1)
		{
			// --- see book for formulae
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double gamma = cos(theta_c) / (1.0 + sin(theta_c));

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double gamma = cos(theta_c) / (1.0 + sin(theta_c));

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double d = 1.0 / Q;
			double betaNumerator = 1.0 - ((d / 2.0)*(sin(theta_c)));
			double betaDenominator = 1.0 + ((d / 2.0)*(sin(theta_c)));

			double beta = 0.5*(betaNumerator / betaDenominator);
			double gamma = (0.5 + beta)*(cos(theta_c));
			double alpha = (0.5 + beta - gamma) / 2.0;

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double d = 1.0 / Q;

			double betaNumerator = 1.0 - ((d / 2.0)*(sin(theta_c)));
			double betaDenominator = 1.0 + ((d / 2.0)*(sin(theta_c)));

			double beta = 0.5*(betaNumerator / betaDenominator);
			double gamma = (0.5 + beta)*(cos(theta_c));
			double alpha = (0.5 + beta + gamma) / 2.0;

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
			double K = tan(M_PI*fc / sampleRate);
			double delta = K*K*Q + K + Q;

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
			double K = tan(M_PI*fc / sampleRate);
			double delta = K*K*Q + K + Q;

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
			double theta_c = M_PI*fc / sampleRate;
			double C = 1.0 / tan(theta_c);

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
			double theta_c = M_PI*fc / sampleRate;
			double C = tan(theta_c);

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double BW = fc / Q;
			double delta_c = M_PI*BW / sampleRate;
			if (delta_c >= 0.95*M_PI / 2.0) delta_c = 0.95*M_PI / 2.0;

			double C = 1.0 / tan(delta_c);
			double D = 2.0*cos(theta_c);

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double BW = fc / Q;
			double delta_c = M_PI*BW / sampleRate;
			if (delta_c >= 0.95*M_PI / 2.0) delta_c = 0.95*M_PI / 2.0;

			double C = tan(delta_c);
			double D = 2.0*cos(theta_c);

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double resonance_dB = 0;

			if (Q > 0.707)
			{
				double peak = Q*Q / pow(Q*Q - 0.25, 0.5);
				resonance_dB = 20.0*log10(peak);
			}

			// --- intermediate vars
			double resonance = (cos(theta_c) + (sin(theta_c) * sqrt(pow(10.0, (resonance_dB / 10.0)) - 1))) / ((pow(10.0, (resonance_dB / 20.0)) * sin(theta_c)) + 1);
			double g = pow(10.0, (-resonance_dB / 40.0));

			// --- kMMALPF2B disables the GR with increase in Q
			if (algorithm == filterAlgorithm::kMMALPF2B)
				g = 1.0;

			double filter_b1 = (-2.0) * resonance * cos(theta_c);
			double filter_b2 = resonance * resonance;
			double filter_a0 = g * (1 + filter_b1 + filter_b2);

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double mu = pow(10.0, boostCut_dB / 20.0);

			double beta = 4.0 / (1.0 + mu);
			double delta = beta*tan(theta_c / 2.0);
			double gamma = (1.0 - delta) / (1.0 + delta);

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double mu = pow(10.0, boostCut_dB / 20.0);

			double beta = (1.0 + mu) / 4.0;
			double delta = beta*tan(theta_c / 2.0);
			double gamma = (1.0 - delta) / (1.0 + delta);

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
			double K = tan(M_PI*fc / sampleRate);
			double Vo = pow(10.0, boostCut_dB / 20.0);
			bool bBoost = boostCut_dB >= 0 ? true : false;

			double d0 = 1.0 + (1.0 / Q)*K + K*K;
			double e0 = 1.0 + (1.0 / (Vo*Q))*K + K*K;
			double alpha = 1.0 + (Vo / Q)*K + K*K;
			double beta = 2.0*(K*K - 1.0);
			double gamma = 1.0 - (Vo / Q)*K + K*K;
			double delta = 1.0 - (1.0 / Q)*K + K*K;
			double eta = 1.0 - (1.0 / (Vo*Q))*K + K*K;

			// --- update coeffs
			coeffArray[a0] = bBoost ? alpha / d0 : d0 / e0;
			coeffArray[a1] = bBoost ? beta / d0 : beta / e0;
			coeffArray[a2] = bBoost ? gamma / d0 : delta / e0;
			coeffArray[b1] = bBoost ? beta / d0 : beta / e0;
			coeffArray[b2] = bBoost ? delta / d0 : eta / e0;

			// --- update on calculator
			biquad.setCoefficients(coeffArray);

			// --- we updated
			return true;
		}
		else if (algorithm == filterAlgorithm::kNCQParaEQ)
		{
			// --- see book for formulae
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double mu = pow(10.0, boostCut_dB / 20.0);

			// --- clamp to 0.95 pi/2 (you can experiment with this)
			double tanArg = theta_c / (2.0 * Q);
			if (tanArg >= 0.95*M_PI / 2.0) tanArg = 0.95*M_PI / 2.0;

			// --- intermediate variables (you can condense this if you wish)
			double zeta = 4.0 / (1.0 + mu);
			double betaNumerator = 1.0 - zeta*tan(tanArg);
			double betaDenominator = 1.0 + zeta*tan(tanArg);

			double beta = 0.5*(betaNumerator / betaDenominator);
			double gamma = (0.5 + beta)*(cos(theta_c));
			double alpha = (0.5 - beta);

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
			double omega_c = M_PI*fc;
			double theta_c = M_PI*fc / sampleRate;

			double k = omega_c / tan(theta_c);
			double denominator = k*k + omega_c*omega_c + 2.0*k*omega_c;
			double b1_Num = -2.0*k*k + 2.0*omega_c*omega_c;
			double b2_Num = -2.0*k*omega_c + k*k + omega_c*omega_c;

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
			double omega_c = M_PI*fc;
			double theta_c = M_PI*fc / sampleRate;

			double k = omega_c / tan(theta_c);
			double denominator = k*k + omega_c*omega_c + 2.0*k*omega_c;
			double b1_Num = -2.0*k*k + 2.0*omega_c*omega_c;
			double b2_Num = -2.0*k*omega_c + k*k + omega_c*omega_c;

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
			double alphaNumerator = tan((M_PI*fc) / sampleRate) - 1.0;
			double alphaDenominator = tan((M_PI*fc) / sampleRate) + 1.0;
			double alpha = alphaNumerator / alphaDenominator;

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double BW = fc / Q;
			double argTan = M_PI*BW / sampleRate;
			if (argTan >= 0.95*M_PI / 2.0) argTan = 0.95*M_PI / 2.0;

			double alphaNumerator = tan(argTan) - 1.0;
			double alphaDenominator = tan(argTan) + 1.0;
			double alpha = alphaNumerator / alphaDenominator;
			double beta = -cos(theta_c);

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double BW = fc / Q;
			double filter_b2 = exp(-2.0*M_PI*(BW / sampleRate));
			double filter_b1 = ((-4.0*filter_b2) / (1.0 + filter_b2))*cos(theta_c);
			double filter_a0 = (1.0 - filter_b2)*sqrt((1.0 - (filter_b1*filter_b1) / (4.0 * filter_b2)));

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
			double theta_c = 2.0*M_PI*fc / sampleRate;
			double BW = fc / Q;
			double filter_b2 = exp(-2.0*M_PI*(BW / sampleRate));
			double filter_b1 = ((-4.0*filter_b2) / (1.0 + filter_b2))*cos(theta_c);
			double filter_a0 = 1.0 - pow(filter_b2, 0.5); // (1.0 - filter_b2)*pow((1.0 - (filter_b1*filter_b1) / (4.0 * filter_b2)), 0.5);

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