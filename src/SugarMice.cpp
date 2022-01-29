#include "RPJ.hpp"
#include "SugarMice.hpp"
#include "ctrl/RPJKnobs.hpp"


SugarMice::SugarMice() {
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configInput(INPUT_MAIN,"Main");
	configInput(INPUT_CVFC, "Cutoff CV");
	configOutput(OUTPUT_MAIN,"Filter");
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

void SugarMice::processChannel(int c, Input& in, Output& out) {
		
	simd::float_4 v = in.getPolyVoltageSimd<simd::float_4>(c);
	out.setVoltageSimd(wdfButterLPF3[c/4].processAudioSample(v),c);
}

void SugarMice::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
		
		int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);
		
		outputs[OUTPUT_MAIN].setChannels(channels);
		for (int c = 0; c < channels; c += 4) {

			if (params[PARAM_FC].getValue() != fc || wdfButterLPF3[0].getUsePostWarping() != warp) {
				for (int i = 0;i < 4;i++) {
					
					float freqParam = params[PARAM_FC].getValue();
					// Rescale for backward compatibility
					freqParam = freqParam * 10.f - 5.f;
					float freqCvParam = params[PARAM_CVFC].getValue();
					// Get pitch
					simd::float_4 pitch = freqParam + inputs[INPUT_CVFC].getPolyVoltageSimd<simd::float_4>(c) * freqCvParam;
					// Set cutoff
					simd::float_4 cutoff = dsp::FREQ_C4 * simd::pow(2.f, pitch);

					cutoff = clamp(cutoff, 20.f, args.sampleRate * 0.46f);
					wdfButterLPF3[i].setFilterFc(cutoff.v[0]);
					
					wdfButterLPF3[i].setUsePostWarping(warp);
				}
			}
			processChannel(c,inputs[INPUT_MAIN],outputs[OUTPUT_MAIN]);
		}
	}
}

struct SugarMiceModuleWidget : ModuleWidget {
	SugarMiceModuleWidget(SugarMice* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SugarMice.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		
		// Next do the knobs
		const float knobX1 = 3;
		const float knobX2 = 60;

		const float knobY1 = 47;
		const float knobY2 = 50;

		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY2), module, SugarMice::PARAM_FC));
		addParam(createParam<RPJKnob>(Vec(knobX2, knobY1), module, SugarMice::PARAM_CVFC));

		// Next do the Jacks
		const float jackX1 = 33.5f;
		const float jackX2 = 62;

		const float jackY1 = 78;
		const float jackY2 = 278;
		const float jackY3 = 325;

		addInput(createInput<PJ301MPort>(Vec(jackX1, jackY2), module, SugarMice::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX1, jackY3), module, SugarMice::OUTPUT_MAIN));
		addInput(createInput<PJ301MPort>(Vec(jackX2, jackY1), module, SugarMice::INPUT_CVFC));

	}

	void appendContextMenu(Menu *menu) override {
		SugarMice *module = dynamic_cast<SugarMice*>(this->module);

		menu->addChild(new MenuEntry);

		menu->addChild(rack::createBoolPtrMenuItem("Enable Warping", "", &module->warp));
	}
};

Model * modelSugarMice = createModel<SugarMice, SugarMiceModuleWidget>("SugarMice");