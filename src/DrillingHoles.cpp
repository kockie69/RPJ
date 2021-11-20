#include "RPJ.hpp"
#include "DrillingHoles.hpp"


DrillingHoles::DrillingHoles() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void DrillingHoles::processChannel(Input& inA, Input& inB, Output& out) {
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

void DrillingHoles::process(const ProcessArgs &args) {

	for (int i=0; i<3;i++)
		if (outputs[POLYOUTPUT_1+i].isConnected() && (inputs[POLYINPUT_A+(2*i)].isConnected() || inputs[POLYINPUT_B+(2*i)].isConnected())) 
			processChannel(inputs[POLYINPUT_A+(2*i)],inputs[POLYINPUT_B+(2*i)],outputs[POLYOUTPUT_1+i]);
}

struct DrillingHolesModuleWidget : ModuleWidget {
	DrillingHolesModuleWidget(DrillingHoles* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DrillingHoles.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);			

		addInput(createInput<PJ301MPort>(Vec(10, 55), module, DrillingHoles::POLYINPUT_A));
		addInput(createInput<PJ301MPort>(Vec(10, 95), module, DrillingHoles::POLYINPUT_B));
		addOutput(createOutput<PJ301MPort>(Vec(55, 95), module, DrillingHoles::POLYOUTPUT_1));

		addInput(createInput<PJ301MPort>(Vec(10, 155), module, DrillingHoles::POLYINPUT_C));
		addInput(createInput<PJ301MPort>(Vec(10, 195), module, DrillingHoles::POLYINPUT_D));
		addOutput(createOutput<PJ301MPort>(Vec(55, 195), module, DrillingHoles::POLYOUTPUT_2));

		addInput(createInput<PJ301MPort>(Vec(10, 255), module, DrillingHoles::POLYINPUT_E));
		addInput(createInput<PJ301MPort>(Vec(10, 295), module, DrillingHoles::POLYINPUT_F));
		addOutput(createOutput<PJ301MPort>(Vec(55, 295), module, DrillingHoles::POLYOUTPUT_3));
	}
};

Model * modelDrillingHoles = createModel<DrillingHoles, DrillingHolesModuleWidget>("DrillingHoles");