#include "RPJ.hpp"
#include "SugarMice.hpp"


SugarMice::SugarMice() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void SugarMice::process(const ProcessArgs &args) {

}

struct SugarMiceModuleWidget : ModuleWidget {
	SugarMiceModuleWidget(SugarMice* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SugarMice.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("SUGARMICE");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(13, 270));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 250));
			tl->setText("OUT");
			addChild(tl);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, SugarMice::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, SugarMice::OUTPUT_MAIN));
	}

};

Model * modelSugarMice = createModel<SugarMice, SugarMiceModuleWidget>("SugarMice");