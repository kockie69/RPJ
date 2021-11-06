#include "rack.hpp"

using namespace rack;

const int MODULE_WIDTH=3;
const char* JSON_OUT_ID = "Bridge ID";

struct BridgeOut : Module {

	enum ParamIds {
		NUM_PARAMS,
	};

	enum InputIds {
		NUM_INPUTS,
	};

	enum OutputIds {
		POLYOUTPUT_A,
		POLYOUTPUT_B,
		POLYOUTPUT_C,
		POLYOUTPUT_D,
		POLYOUTPUT_E,
		POLYOUTPUT_F,
		POLYOUTPUT_G,
		POLYOUTPUT_H,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};
		BridgeOut();
		void process(const ProcessArgs &) override;
		void processChannel(Output&);
		rack::engine::Module* bridgeSource;
		int id;
};

