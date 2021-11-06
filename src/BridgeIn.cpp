#include "RPJ.hpp"
#include "BridgeIn.hpp"


BridgeIn::BridgeIn() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void BridgeIn::processChannel(Input& inA, Input& inB, Output& out) {
	// Get input
	int channels = std::max(inA.getChannels(), inB.getChannels());
	out.setChannels(channels);
	simd::float_4 v1[4],v2[4];
	for (int c = 0; c < channels; c += 4) {
		v1[c / 4] = simd::float_4::load(inA.getVoltages(c));
		v2[c / 4] = simd::float_4::load(inB.getVoltages(c));

		inB.isConnected() ? v2[c / 4].store(out.getVoltages(c)) : v1[c / 4].store(out.getVoltages(c));
	}
}

void BridgeIn::process(const ProcessArgs &args) {
	id = 1;

	//for (int i=0; i<3;i++)
		//if (outputs[POLYOUTPUT_1+i].isConnected() && (inputs[POLYINPUT_A+(2*i)].isConnected() || inputs[POLYINPUT_B+(2*i)].isConnected())) 
		//	processChannel(inputs[POLYINPUT_A+(2*i)],inputs[POLYINPUT_B+(2*i)],outputs[POLYOUTPUT_1+i]);
}

struct BridgeInModuleWidget : ModuleWidget {
	BridgeInModuleWidget(BridgeIn* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BridgeIn.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);			

		addInput(createInput<PJ301MPort>(Vec(10, 55), module, BridgeIn::POLYINPUT_A));
		addInput(createInput<PJ301MPort>(Vec(10, 90), module, BridgeIn::POLYINPUT_B));
		addInput(createInput<PJ301MPort>(Vec(10, 125), module, BridgeIn::POLYINPUT_C));
		addInput(createInput<PJ301MPort>(Vec(10, 160), module, BridgeIn::POLYINPUT_D));
		addInput(createInput<PJ301MPort>(Vec(10, 195), module, BridgeIn::POLYINPUT_E));
		addInput(createInput<PJ301MPort>(Vec(10, 230), module, BridgeIn::POLYINPUT_F));
		addInput(createInput<PJ301MPort>(Vec(10, 265), module, BridgeIn::POLYINPUT_G));
		addInput(createInput<PJ301MPort>(Vec(10, 300), module, BridgeIn::POLYINPUT_H));
	}
};

json_t *BridgeIn::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_IN_ID, json_integer(id));
	return rootJ;
}

void BridgeIn::dataFromJson(json_t *rootJ) {
	json_t *nIdJ = json_object_get(rootJ, JSON_IN_ID);
	if (nIdJ) {
		id = (json_integer_value(nIdJ));
	}
}

Model * modelBridgeIn = createModel<BridgeIn, BridgeInModuleWidget>("BridgeIn");