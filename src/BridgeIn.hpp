#include "rack.hpp"

using namespace rack;

const int MODULE_WIDTH=3;
const char* JSON_IN_ID = "Bridge ID";

struct BridgeIn : Module {

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
		POLYINPUT_G,
		POLYINPUT_H,
		NUM_INPUTS,
	};

	enum OutputIds {
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};
		BridgeIn();
		void process(const ProcessArgs &) override;
		void processChannel(Input&, Input&, Output&);
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		int id;
};

