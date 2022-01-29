#include "RPJ.hpp"
#include "Brave.hpp"
#include "ctrl/RPJKnobs.hpp"

Brave::Brave() {
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "Boost/Cut","dB");
	configParam(PARAM_CVQ, -1.f, 1.0f, 0.0f, "CV Resonance");
	configParam(PARAM_CVB, -1.f, 1.0f, 0.0f, "CV Boost/Cut");
	configInput(INPUT_CVB,"Boost/Cut CV");
	configInput(INPUT_CVQ,"Resonance CV");
	configInput(INPUT_CVFC,"Frequency Cutoff CV");
	configInput(INPUT_MAIN,"Main");
	configOutput(OUTPUT_MAIN,"Filter");
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	afp.algorithm = filterAlgorithm::kNCQParaEQ;
	for (int i=0;i<4;i++) {
		audioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Brave::onSampleRateChange() {
	for (int i=0;i<4;i++) {
		audioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Brave::processChannel(int c, Input& in, Output& out) {
		
	simd::float_4 v = in.getPolyVoltageSimd<simd::float_4>(c);

	audioFilter[c/4].setParameters(afp);
	out.setVoltageSimd(simd::clamp(audioFilter[c/4].processAudioSample(v),-5.f,5.f),c);
}

void Brave::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected()) {

		int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);

		outputs[OUTPUT_MAIN].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
	
			float freqParam = params[PARAM_FC].getValue();
			// Rescale for backward compatibility
			freqParam = freqParam * 10.f - 5.f;
			float freqCvParam = params[PARAM_CVFC].getValue();
			// Get pitch
			simd::float_4 pitch = freqParam + inputs[INPUT_CVFC].getPolyVoltageSimd<simd::float_4>(c) * freqCvParam;
			// Set cutoff
			simd::float_4 cutoff = dsp::FREQ_C4 * simd::pow(2.f, pitch);

			cutoff = clamp(cutoff, 20.f, args.sampleRate * 0.46f);
 			afp.fc = cutoff.v[0];

			double cvb = 1.f;
			double cvq = 1.f;

			if (inputs[INPUT_CVQ].isConnected())
				cvq = inputs[INPUT_CVQ].getVoltage() / 10.0;

			if (inputs[INPUT_CVB].isConnected())
				cvb = inputs[INPUT_CVB].getVoltage() / 10.0;	
 
			afp.Q = clamp((params[PARAM_CVQ].getValue() * cvq * 20.f) + params[PARAM_Q].getValue(),0.707f, 20.0f);
			afp.boostCut_dB = clamp((params[PARAM_BOOSTCUT_DB].getValue() * cvb * 20.f) + params[PARAM_CVB].getValue(),-20.f,20.f);
			afp.bqa = bqa;
		
			processChannel(c,inputs[INPUT_MAIN],outputs[OUTPUT_MAIN]);
		}
	}
}

struct BraveModuleWidget : ModuleWidget {
	BraveModuleWidget(Brave* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Brave.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		// First do the knobs
		const float knobX1 = 3;
		const float knobX2 = 60;


		const float knobY1 = 47;
		const float knobY2 = 50;
		const float knobY3 = 122;
		const float knobY4 = 125;
		const float knobY5 = 197;
		const float knobY6 = 200;


		addParam(createParam<RPJKnob>(Vec(knobX2, knobY1), module, Brave::PARAM_CVFC));	
		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY2), module, Brave::PARAM_FC));
		addParam(createParam<RPJKnob>(Vec(knobX2, knobY3), module, Brave::PARAM_CVQ));
		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY4), module, Brave::PARAM_Q));
		addParam(createParam<RPJKnob>(Vec(knobX2, knobY5), module, Brave::PARAM_CVB));
		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY6), module, Brave::PARAM_BOOSTCUT_DB));

		// Next do the Jacks
		const float jackX1 = 33.5f;
		const float jackX2 = 62;

		const float jackY1 = 78;
		const float jackY2 = 153;
		const float jackY3 = 228;
		const float jackY4 = 278;
		const float jackY5 = 325;

		addInput(createInput<PJ301MPort>(Vec(jackX2, jackY3), module, Brave::INPUT_CVB));
		addInput(createInput<PJ301MPort>(Vec(jackX1, jackY4), module, Brave::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX1, jackY5), module, Brave::OUTPUT_MAIN));	
		addInput(createInput<PJ301MPort>(Vec(jackX2, jackY1), module, Brave::INPUT_CVFC));
		addInput(createInput<PJ301MPort>(Vec(jackX2, jackY2), module, Brave::INPUT_CVQ));
	}

	void appendContextMenu(Menu *menu) override {
		Brave * module = dynamic_cast<Brave*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqa));

	}
};

json_t *Brave::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqa)));
	return rootJ;
}

void Brave::dataFromJson(json_t *rootJ) {
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nBiquadAlgorithmJ) {
		bqa = static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelBrave = createModel<Brave, BraveModuleWidget>("Brave");