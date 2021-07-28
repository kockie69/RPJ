#include "rack.hpp"

using namespace rack;

const int MODULE_WIDTH=6;

struct DrillingHoles : Module {

	enum ParamIds {
		PARAM_FC,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_A,
		INPUT_B,
		INPUT_C,
		INPUT_D,
		INPUT_E,
		INPUT_F,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};
		DrillingHoles();
		void process(const ProcessArgs &) override;
};

