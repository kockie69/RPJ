#include "BiQuad.hpp"

bool Biquad::checkFloatUnderflow(rack::simd::float_4& value)
{
	bool retValue = false;
	/*if (value > 0.0 && value < kSmallestPositiveFloatValue)
	{
		value = 0;
		retValue = true;
	}
	else if (value < 0.0 && value > kSmallestNegativeFloatValue)
	{
		value = 0;
		retValue = true;
	}*/
	for (int i=0;i<4;i++) {
		if (value[i] > 0.0 && value[i] < kSmallestPositiveFloatValue) {
			value[i]=0;
			retValue = true;
		}
		else if (value[i] < 0.0 && value[i] < kSmallestNegativeFloatValue)
		{
			value[i]=0;
			retValue = true;
		}
	}
	return retValue;
}


bool Biquad::reset(double _sampleRate) {
		memset(&stateArray[0], 0, sizeof(double)*numStates);
		return true;  // handled = true
}

bool Biquad::canProcessAudioFrame() { 
	return false; 
}

BiquadParameters Biquad::getParameters() { 
    return parameters ; 
}

void Biquad::setParameters(const BiquadParameters& _parameters) { 
    parameters = _parameters; 
}

double* Biquad::getCoefficients() {
	// --- read/write access to the array (not used)
	return &coeffArray[0];
}

rack::simd::float_4* Biquad::getStateArray() {
	// --- read/write access to the array (used only in direct form oscillator)
	return &stateArray[0];
}

void Biquad::setCoefficients(double* coeffs){
// --- fast block memory copy:
	memcpy(&coeffArray[0], &coeffs[0], sizeof(double)*numCoeffs);
}

double Biquad::getG_value() { 
    return coeffArray[a0]; 
}

rack::simd::float_4 Biquad::processAudioSample(rack::simd::float_4 xn) {
	if (parameters.biquadCalcType  == biquadAlgorithm::kDirect) {
		// --- 1)  form output y(n) = a0*x(n) + a1*x(n-1) + a2*x(n-2) - b1*y(n-1) - b2*y(n-2)
		rack::simd::float_4 yn = coeffArray[a0] * xn + 
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
		rack::simd::float_4 wn = xn - coeffArray[b1] * stateArray[x_z1] - coeffArray[b2] * stateArray[x_z2];

		// --- y(n):
		rack::simd::float_4 yn = coeffArray[a0] * wn + coeffArray[a1] * stateArray[x_z1] + coeffArray[a2] * stateArray[x_z2];

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
		rack::simd::float_4 wn = xn + stateArray[y_z1];

		// --- y(n) = a0*w(n) + stateArray[x_z1]
		rack::simd::float_4 yn = coeffArray[a0] * wn + stateArray[x_z1];

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
		rack::simd::float_4 yn = coeffArray[a0] * xn + stateArray[x_z1];

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
