#include "rack.hpp"

using namespace rack;
const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
const double kSmallestNegativeFloatValue = -1.175494351e-38;         /* min negative value */

template <typename T>
struct Biquad {
	enum {x_z1, x_z2, y_z1, y_z2, numStates };

	enum filterCoeff { a0, a1, a2, b1, b2, c0, d0, numCoeffs };

	T processAudioSample(T);
	bool canProcessAudioFrame();
	bool checkFloatUnderflow(T&);
	bool reset(T);
	T stateArray[numStates];
	float coeffArray[numCoeffs];
	void setCoeffs();
	Module* module;
	T yn;
};

struct Filter : Module {

	enum biquadAlgorithm { kDirect, kCanonical, kTransposeDirect, kTransposeCanonical }; //  4 types of biquad calculations, constants (k)

	enum ParamIds {
		PARAM_A0,
		PARAM_A1,
		PARAM_A2,
		PARAM_B1,
		PARAM_B2,
		PARAM_C0,
		PARAM_D0,
		PARAM_ALGO,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_MAIN,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		Filter();
		Biquad<double> filter;
		void process(const ProcessArgs &) override;
};

struct knobSmall : SvgKnob  {
	knobSmall() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_16px.svg")));
		minAngle = -0.7*M_PI;
		maxAngle = 0.7*M_PI;

	}
};

struct AlgoQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;
	private:
		int value;
		std::string v;
};

