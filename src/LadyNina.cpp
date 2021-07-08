#include "RPJ.hpp"
#include "LadyNina.hpp"


bool Biquad::reset(double _sampleRate) {
		memset(&stateArray[0], 0, sizeof(double)*numStates);
		return true;  // handled = true
}

bool Biquad::canProcessAudioFrame() { 
	return false; 
}

inline double getMagResponse(double theta, double a0, double a1, double a2, double b1, double b2)
{
	double magSqr = 0.0;
	double num = a1*a1 + (a0 - a2)*(a0 - a2) + 2.0*a1*(a0 + a2)*cos(theta) + 4.0*a0*a2*cos(theta)*cos(theta);
	double denom = b1*b1 + (1.0 - b2)*(1.0 - b2) + 2.0*b1*(1.0 + b2)*cos(theta) + 4.0*b2*cos(theta)*cos(theta);

	magSqr = num / denom;
	if (magSqr < 0.0)
		magSqr = 0.0;

	double mag = pow(magSqr, 0.5);

	return mag;
}

inline bool checkFloatUnderflow(double& value)
{
	bool retValue = false;
	if (value > 0.0 && value < kSmallestPositiveFloatValue)
	{
		value = 0;
		retValue = true;
	}
	else if (value < 0.0 && value > kSmallestNegativeFloatValue)
	{
		value = 0;
		retValue = true;
	}
	return retValue;
}

double Biquad::processAudioSample(double xn) {
	if (parameters.biquadCalcType  == biquadAlgorithm::kDirect) {
		// --- 1)  form output y(n) = a0*x(n) + a1*x(n-1) + a2*x(n-2) - b1*y(n-1) - b2*y(n-2)
		double yn = coeffArray[a0] * xn + 
					coeffArray[a1] * stateArray[x_z1] +
					coeffArray[a2] * stateArray[x_z2] -
					coeffArray[b1] * stateArray[y_z1] -
					coeffArray[b2] * stateArray[y_z2];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[x_z2] = stateArray[x_z1];
		stateArray[x_z1] = xn;

		stateArray[y_z2] = stateArray[y_z1];
		stateArray[y_z1] = yn;

		return yn;
	}
	else if (parameters.biquadCalcType == biquadAlgorithm::kCanonical)
	{
		// --- 1)  form output y(n) = a0*w(n) + m_f_a1*stateArray[x_z1] + m_f_a2*stateArray[x_z2][x_z2];
		//
		// --- w(n) = x(n) - b1*stateArray[x_z1] - b2*stateArray[x_z2]
		double wn = xn - coeffArray[b1] * stateArray[x_z1] - coeffArray[b2] * stateArray[x_z2];

		// --- y(n):
		double yn = coeffArray[a0] * wn + coeffArray[a1] * stateArray[x_z1] + coeffArray[a2] * stateArray[x_z2];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[x_z2] = stateArray[x_z1];
		stateArray[x_z1] = wn;

		// --- return value
		return yn;
	}
	else if (parameters.biquadCalcType == biquadAlgorithm::kTransposeDirect)
	{
		// --- 1)  form output y(n) = a0*w(n) + stateArray[x_z1]
		//
		// --- w(n) = x(n) + stateArray[y_z1]
		double wn = xn + stateArray[y_z1];

		// --- y(n) = a0*w(n) + stateArray[x_z1]
		double yn = coeffArray[a0] * wn + stateArray[x_z1];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[y_z1] = stateArray[y_z2] - coeffArray[b1] * wn;
		stateArray[y_z2] = -coeffArray[b2] * wn;

		stateArray[x_z1] = stateArray[x_z2] + coeffArray[a1] * wn;
		stateArray[x_z2] = coeffArray[a2] * wn;

		// --- return value
		return yn;
	}
	else if (parameters.biquadCalcType == biquadAlgorithm::kTransposeCanonical)
	{
		// --- 1)  form output y(n) = a0*x(n) + stateArray[x_z1]
		double yn = coeffArray[a0] * xn + stateArray[x_z1];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- shuffle/update
		stateArray[x_z1] = coeffArray[a1]*xn - coeffArray[b1]*yn + stateArray[x_z2];
		stateArray[x_z2] = coeffArray[a2]*xn - coeffArray[b2]*yn;

		// --- return value
		return yn;
	}
	return xn; // didn't process anything :(
}

bool AudioFilter::calculateFilterCoeffs()
{
	// --- clear coeff array
	memset(&coeffArray[0], 0, sizeof(double)*numCoeffs);

	// --- set default pass-through
	coeffArray[a0] = 1.0;
	coeffArray[c0] = 1.0;
	coeffArray[d0] = 0.0;

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
		double T = 1.0 / sampleRate;
		double omega = 2.0*M_PI*fc;
		double eT = exp(-T*omega);

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
	else if (algorithm == filterAlgorithm::kLPF1P)
	{
		// --- see book for formulae
		double theta_c = 2.0*M_PI*fc / sampleRate;
		double gamma = 2.0 - cos(theta_c);

		double filter_b1 = pow((gamma*gamma - 1.0), 0.5) - gamma;
		double filter_a0 = 1.0 + filter_b1;

		// --- update coeffs
		coeffArray[a0] = filter_a0;
		coeffArray[a1] = 0.0;
		coeffArray[a2] = 0.0;
		coeffArray[b1] = filter_b1;
		coeffArray[b2] = 0.0;

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

		double mag = getMagResponse(theta_c, coeffArray[a0], coeffArray[a1], coeffArray[a2], coeffArray[b1], coeffArray[b2]);
		
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
		double filter_a0 = (1.0 - filter_b2)*pow((1.0 - (filter_b1*filter_b1) / (4.0 * filter_b2)), 0.5);

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

double AudioFilter::processAudioSample(double xn)
{
	// --- let biquad do the grunt-work
	//
	// return (dry) + (processed): x(n)*d0 + y(n)*c0
	return coeffArray[d0] * xn + coeffArray[c0] * biquad.processAudioSample(xn);
}

LadyNina::LadyNina() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "dB","Boost/Cut");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, -1.5f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
}

void LadyNina::process(const ProcessArgs &args) {

	audioFilter.setSampleRate(args.sampleRate);
 	afp.fc = params[PARAM_FC].getValue();
	afp.Q = params[PARAM_Q].getValue();
	afp.boostCut_dB = params[PARAM_BOOSTCUT_DB].getValue();

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
		if ((static_cast<int>(afp.algorithm)+1) == static_cast<int>(filterAlgorithm::numFilterAlgorithms))
			afp.algorithm = static_cast<filterAlgorithm>(static_cast<int>(afp.algorithm));
		else
			afp.algorithm = static_cast<filterAlgorithm>(static_cast<int>(afp.algorithm)+1);
	}
	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
		if ((static_cast<int>(afp.algorithm)-1) < 0)
			afp.algorithm = static_cast<filterAlgorithm>(static_cast<int>(afp.algorithm));
		else 
			afp.algorithm = static_cast<filterAlgorithm>(static_cast<int>(afp.algorithm)-1);
	}

	afp.strAlgorithm = filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
	audioFilter.setParameters(afp);
	float out = audioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());

	outputs[OUTPUT_MAIN].setVoltage(out);
}

struct buttonPlusSmall : SvgSwitch  {
	buttonPlusSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonPlus_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonPlus_1.svg")));
	}
};

struct buttonMinSmall : SvgSwitch  {
	buttonMinSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonMin_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonMin_1.svg")));
	}
};

struct LadyNinaModuleWidget : ModuleWidget {
	LadyNinaModuleWidget(LadyNina* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/myVCF.svg")));

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("LadyNina");
			addChild(title);
		}
		{
			AFilterNameDisplay * fnd = new AFilterNameDisplay(Vec(39,30));
			fnd->module = module;
			addChild(fnd);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(1, 50));
			title->setText("CUTOFF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(22, 90));
			title->setText("RESONANCE");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(1, 140));
			title->setText("BOOST/CUT");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(58, 170));
			title->setText("DRY");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(5, 212));
			title->setText("WET");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(13, 270));
			title->setText("IN");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 290));
			title->setText("OUT");
			addChild(title);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, LadyNina::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, LadyNina::OUTPUT_MAIN));

		addParam(createParam<buttonMinSmall>(Vec(5,45),module, LadyNina::PARAM_DOWN));
		addParam(createParam<buttonPlusSmall>(Vec(76,45),module, LadyNina::PARAM_UP));
		addParam(createParam<RoundBlackKnob>(Vec(8, 80), module, LadyNina::PARAM_FC));
		addParam(createParam<RoundBlackKnob>(Vec(55, 120), module, LadyNina::PARAM_Q));
		addParam(createParam<RoundBlackKnob>(Vec(8, 172), module, LadyNina::PARAM_BOOSTCUT_DB));	
		addParam(createParam<RoundBlackKnob>(Vec(8, 245), module, LadyNina::PARAM_WET));
		addParam(createParam<RoundBlackKnob>(Vec(55, 205), module, LadyNina::PARAM_DRY));
	}

};

Model * modelLadyNina = createModel<LadyNina, LadyNinaModuleWidget>("LadyNina");