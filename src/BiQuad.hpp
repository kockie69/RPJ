#include "rack.hpp"

const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
const double kSmallestNegativeFloatValue = -1.175494351e-38;         /* min negative value */


enum biquadAlgorithm { kDirect, kCanonical, kTransposeDirect, kTransposeCanonical }; //  4 types of biquad calculations, constants (k)

enum filterCoeff { a0, a1, a2, b1, b2, c0, d0, numCoeffs };

enum {x_z1, x_z2, y_z1, y_z2, numStates };

struct BiquadParameters
{
	BiquadParameters () {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	/*BiquadParameters& operator=(const BiquadParameters& params)
	{
		if (this == &params)
			return *this;

		biquadCalcType = params.biquadCalcType;
		return *this;
	}*/

	biquadAlgorithm biquadCalcType = biquadAlgorithm::kDirect; ///< biquad structure to use
};

struct Biquad {
	public:
	Biquad() {}		/* C-TOR */
	~Biquad() {}	/* D-TOR */

    bool checkFloatUnderflow(double&);

	// --- IAudioSignalProcessor FUNCTIONS --- //
	//
	/** reset: clear out the state array (flush delays); can safely ignore sampleRate argument - we don't need/use it */
	bool reset(double);

	/** return false: this object only processes samples */
	bool canProcessAudioFrame();

	/** process input x(n) through biquad to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn);

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return BiquadParameters custom data structure
	*/
	BiquadParameters getParameters();

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param BiquadParameters custom data structure
	*/
	void setParameters(const BiquadParameters&); 

	// --- MUTATORS & ACCESSORS --- //
	/** set the coefficient array NOTE: passing by pointer to array; allows us to use "array notation" with pointers i.e. [ ] */
	void setCoefficients(double*);


	/** get the coefficient array for read/write access to the array (not used in current objects) */
	double* getCoefficients();

	/** get the state array for read/write access to the array (used only in direct form oscillator) */
	double* getStateArray();

	/** get the structure G (gain) value for Harma filters; see 2nd Ed FX book */
	double getG_value();

	/** get the structure S (storage) value for Harma filters; see 2nd Ed FX book */
	double getS_value();// { return storageComponent; }

protected:
	/** array of coefficients */
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	/** array of state (z^-1) registers */
	double stateArray[numStates] = { 0.0, 0.0, 0.0, 0.0 };

	/** type of calculation (algorithm  structure) */
	BiquadParameters parameters;

	/** for Harma loop resolution */
	double storageComponent = 0.0;
};

