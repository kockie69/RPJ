#include "RPJ.hpp"
#include "DrillingHoles.hpp"


DrillingHoles::DrillingHoles() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void DrillingHoles::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_1].isConnected() && (inputs[INPUT_A].isConnected() || inputs[INPUT_B].isConnected())) {
		if (inputs[INPUT_B].isConnected())
			outputs[OUTPUT_1].setVoltage(inputs[INPUT_B].getVoltage());
		else
			outputs[OUTPUT_1].setVoltage(inputs[INPUT_A].getVoltage());
	}
	if (outputs[OUTPUT_2].isConnected() && (inputs[INPUT_C].isConnected() || inputs[INPUT_D].isConnected())) {
		if (inputs[INPUT_D].isConnected())
			outputs[OUTPUT_2].setVoltage(inputs[INPUT_D].getVoltage());
		else
			outputs[OUTPUT_2].setVoltage(inputs[INPUT_C].getVoltage());
	}
	if (outputs[OUTPUT_3].isConnected() && (inputs[INPUT_E].isConnected() || inputs[INPUT_F].isConnected())) {
		if (inputs[INPUT_F].isConnected())
			outputs[OUTPUT_3].setVoltage(inputs[INPUT_F].getVoltage());
		else
			outputs[OUTPUT_3].setVoltage(inputs[INPUT_E].getVoltage());
	}
}

struct DrillingHolesModuleWidget : ModuleWidget {
	DrillingHolesModuleWidget(DrillingHoles* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DrillingHoles.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			RPJTitle * title = new RPJTitle(box.size.x,MODULE_WIDTH);
			title->setText("DRILLINGHOLES");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 19),10,MODULE_WIDTH);
			tl->setText("Normalizer");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(15, 25));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(15, 65));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(58, 65));
			tl->setText("OUT");
			addChild(tl);
		}
				{
			RPJTextLabel * tl = new RPJTextLabel(Vec(15, 125));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(15, 165));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 165));
			tl->setText("OUT");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(15, 225));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(15, 265));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 265));
			tl->setText("OUT");
			addChild(tl);
		}			

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