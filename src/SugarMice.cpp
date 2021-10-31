#include "RPJ.hpp"
#include "SugarMice.hpp"
#include "ctrl/RPJKnobs.hpp"


SugarMice::SugarMice() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
	sampleRate=APP->engine->getSampleRate();
	fc = 1000;
	warp=false;
	for (int i = 0;i<4;i++)
		wdfButterLPF3[i].reset(APP->engine->getSampleRate());
}

void SugarMice::onSampleRateChange() {
	for (int i = 0;i<4;i++)
		wdfButterLPF3[i].reset(APP->engine->getSampleRate());
}

void SugarMice::processChannel(Input& in, Output& out) {
		
		// Get input
		int channels = std::max(in.getChannels(), 1);
		simd::float_4 v[4];
		for (int c = 0; c < channels; c += 4) {
			v[c/4] = simd::float_4::load(in.getVoltages(c));
		}
		
		out.setChannels(channels);
		simd::float_4 output;
		for (int c = 0; c < channels; c += 4) {
			output = wdfButterLPF3[c/4].processAudioSample(v[c/4]);
			output.store(out.getVoltages(c));
		}
}

void SugarMice::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
		if (params[PARAM_FC].getValue() != fc || wdfButterLPF3[0].getUsePostWarping() != warp) {
			for (int i = 0;i < 4;i++) { 
				wdfButterLPF3[i].setFilterFc(pow(2048,params[PARAM_FC].getValue()) * 10);
				fc = params[PARAM_FC].getValue();
				wdfButterLPF3[i].setUsePostWarping(warp);
			}
		}
		processChannel(inputs[INPUT_MAIN],outputs[OUTPUT_MAIN]);
	}
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

		menu->addChild(rack::createBoolPtrMenuItem("Enable Warping", "", &module->warp));
	}
};

Model * modelSugarMice = createModel<SugarMice, SugarMiceModuleWidget>("SugarMice");