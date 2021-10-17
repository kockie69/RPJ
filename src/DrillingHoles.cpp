#include "RPJ.hpp"
#include "DrillingHoles.hpp"


DrillingHoles::DrillingHoles() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void DrillingHoles::process(const ProcessArgs &args) {

	for (int i=0; i<3;i++)
		if (outputs[OUTPUT_1+i].isConnected() && (inputs[INPUT_A+(2*i)].isConnected() || inputs[INPUT_B+(2*i)].isConnected())) 
			inputs[INPUT_B+(2*i)].isConnected() ? outputs[OUTPUT_1+i].setVoltage(inputs[INPUT_B+(2*i)].getVoltage()) : outputs[OUTPUT_1+i].setVoltage(inputs[INPUT_A+(2*i)].getVoltage());
}

struct DrillingHolesModuleWidget : ModuleWidget {
	DrillingHolesModuleWidget(DrillingHoles* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DrillingHoles.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);			

		addInput(createInput<PJ301MPort>(Vec(10, 55), module, DrillingHoles::INPUT_A));
		addInput(createInput<PJ301MPort>(Vec(10, 95), module, DrillingHoles::INPUT_B));
		addOutput(createOutput<PJ301MPort>(Vec(55, 95), module, DrillingHoles::OUTPUT_1));

		addInput(createInput<PJ301MPort>(Vec(10, 155), module, DrillingHoles::INPUT_C));
		addInput(createInput<PJ301MPort>(Vec(10, 195), module, DrillingHoles::INPUT_D));
		addOutput(createOutput<PJ301MPort>(Vec(55, 195), module, DrillingHoles::OUTPUT_2));

		addInput(createInput<PJ301MPort>(Vec(10, 255), module, DrillingHoles::INPUT_E));
		addInput(createInput<PJ301MPort>(Vec(10, 295), module, DrillingHoles::INPUT_F));
		addOutput(createOutput<PJ301MPort>(Vec(55, 295), module, DrillingHoles::OUTPUT_3));
	}
};

Model * modelDrillingHoles = createModel<DrillingHoles, DrillingHolesModuleWidget>("DrillingHoles");