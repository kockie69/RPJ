#include "rack.hpp"
#include "EnvelopeFollower.hpp"

using namespace rack;
		
struct BlindCurve : Module {

	enum ParamIds {
		PARAM_FC,
		PARAM_CVFC,
		PARAM_Q,
		PARAM_CVQ,
		PARAM_BOOSTCUT_DB,
		PARAM_CVB,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
		INPUT_CVQ,
		INPUT_CVB,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_MAIN,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		BlindCurve();
		EnvelopeFollower envelopeFollower;
		void process(const ProcessArgs &) override;
		EnvelopeFollowerParameters efp;
};


