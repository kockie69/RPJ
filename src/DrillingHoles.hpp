#include "rack.hpp"

using namespace rack;

const int MODULE_WIDTH=6;

struct DrillingHoles : Module {

	enum ParamIds {
		NUM_PARAMS,
	};

	enum InputIds {
		POLYINPUT_A,
		POLYINPUT_B,
		POLYINPUT_C,
		POLYINPUT_D,
		POLYINPUT_E,
		POLYINPUT_F,
		NUM_INPUTS,
	};

	enum OutputIds {
		POLYOUTPUT_1,
		POLYOUTPUT_2,
		POLYOUTPUT_3,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};
		DrillingHoles();
		void process(const ProcessArgs &) override;
		void processChannel(Input&, Input&, Output&);
};

