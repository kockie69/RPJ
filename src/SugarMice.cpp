#include "RPJ.hpp"
#include "SugarMice.hpp"
#include "ctrl/RPJKnobs.hpp"


SugarMice::SugarMice() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	sampleRate=APP->engine->getSampleRate();
	fc=0;
	warp=false;
}

void SugarMice::onSampleRateChange() {
	wdfButterLPF3.reset(APP->engine->getSampleRate());
}

void SugarMice::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
		if (params[PARAM_FC].getValue() != fc || wdfButterLPF3.getUsePostWarping() != warp) {
			wdfButterLPF3.setFilterFc(params[PARAM_FC].getValue());
			fc = params[PARAM_FC].getValue();
			warp = wdfButterLPF3.getUsePostWarping();
		}

		float out = wdfButterLPF3.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		outputs[OUTPUT_MAIN].setVoltage(out);
	}
}


/* Context Menu Item for changing the warping settings */
void nWarpSelectionMenuItem::onAction(const event::Action &e) {
	module->wdfButterLPF3.setUsePostWarping(Warp);
}



struct SugarMiceModuleWidget : ModuleWidget {
	SugarMiceModuleWidget(SugarMice* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SugarMice.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	
		addInput(createInput<PJ301MPort>(Vec(11, 250), module, SugarMice::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(11, 320), module, SugarMice::OUTPUT_MAIN));
		addParam(createParam<RPJKnob>(Vec(8, 60), module, SugarMice::PARAM_FC));
	}

	void appendContextMenu(Menu *menu) override {
		SugarMice *module = dynamic_cast<SugarMice*>(this->module);

		menu->addChild(new MenuEntry);


		nWarpSelectionMenuItem *nWarpItem = new nWarpSelectionMenuItem();
		nWarpItem->text = "Enable Warping";
		nWarpItem->module = module;
		nWarpItem->Warp = !module->wdfButterLPF3.getUsePostWarping();
		nWarpItem->rightText = CHECKMARK(!nWarpItem->Warp);
		menu->addChild(nWarpItem);
	}
};

Model * modelSugarMice = createModel<SugarMice, SugarMiceModuleWidget>("SugarMice");