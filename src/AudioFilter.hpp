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

struct AudioFilter {
public:
	AudioFilter() {}		/* C-TOR */
	~AudioFilter() {}		/* D-TOR */

	// --- IAudioSignalProcessor
	/** --- set sample rate, then update coeffs */
	bool reset(double );

	/** return false: this object only processes samples */
	bool canProcessAudioFrame();

	/** process input x(n) through the filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual rack::simd::float_4 processAudioSample(rack::simd::float_4);

	/** --- sample rate change necessarily requires recalculation */
	void setSampleRate(double);

	/** --- get parameters */
	AudioFilterParameters getParameters();

	/** --- set parameters */
	void setParameters(const AudioFilterParameters&);

	/** --- helper for Harma filters (phaser) */
	double getG_value();

	/** --- helper for Harma filters (phaser) */
	double getS_value();

protected:
	// --- our calculator
	Biquad biquad; ///< the biquad object

	// --- array to hold coeffs (we need them too)
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; ///< our local copy of biquad coeffs

	// --- object parameters
	AudioFilterParameters audioFilterParameters; ///< parameters
	double sampleRate = 44100.0; ///< current sample rate

	/** --- function to recalculate coefficients due to a change in filter parameters */
	bool calculateFilterCoeffs();
	BiquadParameters bqp;
};
