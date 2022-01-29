#include "RPJ.hpp"
#include "Lavender.hpp"
#include "ctrl/RPJKnobs.hpp"


Lavender::Lavender() {
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, -1.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_CVDRIVE, -1.f, 1.0f, 0.0f, "Drive CV", "%", 0, 100);
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "Dry", "%", 0.f, 100.f);
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "Wet", "%", 0.f, 100.f);
	configParam(PARAM_DRIVE, 0.f, 1.0f, 0.3f, "Drive", "%", 0, 100, 100);
	configBypass(INPUT_MAIN, OUTPUT_LPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_HPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_BPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_BSFMAIN);
	configInput(INPUT_MAIN,"Main Audio");
	configInput(INPUT_CVFC, "Frequency Cutoff CV");
	configInput(INPUT_CVQ, "Resonance CV");
	configInput(INPUT_CVDRIVE, "Drive CV");
	configOutput(OUTPUT_LPFMAIN,"Low Pass Filter");
	configOutput(OUTPUT_HPFMAIN,"High Pass Filter");
	configOutput(OUTPUT_BPFMAIN,"Band Pass Filter");
	configOutput(OUTPUT_BSFMAIN,"Band Stop Filter");

	for (int i = 0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
		BPFaudioFilter[i].reset(APP->engine->getSampleRate());
		BSFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
	LPFafp.algorithm=filterAlgorithm::kLPF2;
	HPFafp.algorithm=filterAlgorithm::kHPF2;
	BPFafp.algorithm=filterAlgorithm::kBPF2;
	BSFafp.algorithm=filterAlgorithm::kBSF2;
	bqa=biquadAlgorithm::kDirect;
}

void Lavender::onSampleRateChange() {
	for (int i = 0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Lavender::processChannel(int c,Input& in, Output& lpfOut, Output& hpfOut, Output& bpfOut, Output& bsfOut) {

	simd::float_4 v = in.getPolyVoltageSimd<simd::float_4>(c);
	if (lpfOut.isConnected()) {
		LPFaudioFilter[c/4].setParameters(LPFafp);
		lpfOut.setVoltageSimd(simd::clamp(LPFaudioFilter[c/4].processAudioSample(v)*LPFafp.drive,-5.f,5.f),c);
	}
	if (hpfOut.isConnected()) {
		HPFaudioFilter[c/4].setParameters(HPFafp);
		hpfOut.setVoltageSimd(simd::clamp(HPFaudioFilter[c/4].processAudioSample(v)*HPFafp.drive,-5.f,5.f),c);
	}
	if (bpfOut.isConnected()) {
		BPFaudioFilter[c/4].setParameters(BPFafp);
		bpfOut.setVoltageSimd(simd::clamp(BPFaudioFilter[c/4].processAudioSample(v)*BPFafp.drive,-5.f,5.f),c);
	}
	if (bsfOut.isConnected()) {
		BSFaudioFilter[c/4].setParameters(BSFafp);
		bsfOut.setVoltageSimd(simd::clamp(BSFaudioFilter[c/4].processAudioSample(v)*BSFafp.drive,-5.f,5.f),c);
	}
}

void Lavender::process(const ProcessArgs &args) {
	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected() || outputs[OUTPUT_BPFMAIN].isConnected() || outputs[OUTPUT_BSFMAIN].isConnected()) {

		int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);

		outputs[OUTPUT_LPFMAIN].setChannels(channels);
		outputs[OUTPUT_HPFMAIN].setChannels(channels);
		outputs[OUTPUT_BPFMAIN].setChannels(channels);
		outputs[OUTPUT_BSFMAIN].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {

			double cvdrive = 0.f;
			double cvq = 0.f;

			if (inputs[INPUT_CVQ].isConnected())
				cvq = inputs[INPUT_CVQ].getVoltage() / 10.0;
 	
			float freqParam = params[PARAM_FC].getValue();
			// Rescale for backward compatibility
			freqParam = freqParam * 10.f - 5.f;
			float freqCvParam = params[PARAM_CVFC].getValue();
			// Get pitch
			simd::float_4 pitch = freqParam + inputs[INPUT_CVFC].getPolyVoltageSimd<simd::float_4>(c) * freqCvParam;
			// Set cutoff
			simd::float_4 cutoff = dsp::FREQ_C4 * simd::pow(2.f, pitch);

			cutoff = clamp(cutoff, 20.f, args.sampleRate * 0.46f);
			LPFafp.fc = HPFafp.fc = BPFafp.fc = BSFafp.fc = cutoff.v[0];
			
			LPFafp.Q = HPFafp.Q = BPFafp.Q = BSFafp.Q = clamp((params[PARAM_CVQ].getValue() * cvq * 20.f) + params[PARAM_Q].getValue(),0.707f, 20.0f);
			LPFafp.dry = HPFafp.dry = BPFafp.dry = BSFafp.dry = params[PARAM_DRY].getValue();
			LPFafp.wet = HPFafp.wet = BPFafp.wet = BSFafp.wet = params[PARAM_WET].getValue();
			LPFafp.bqa = HPFafp.bqa = BPFafp.bqa = BSFafp.bqa = bqa;

			if (inputs[INPUT_CVDRIVE].isConnected())
				cvdrive = inputs[INPUT_CVDRIVE].getVoltage() / 10.0;
			LPFafp.drive = HPFafp.drive = BPFafp.drive = BSFafp.drive = clamp((params[PARAM_CVDRIVE].getValue() * cvdrive) + params[PARAM_DRIVE].getValue(),0.f,1.f);

			processChannel(c, inputs[INPUT_MAIN],outputs[OUTPUT_LPFMAIN],outputs[OUTPUT_HPFMAIN],outputs[OUTPUT_BPFMAIN],outputs[OUTPUT_BSFMAIN]);
		}
	}
}

struct LavenderModuleWidget : ModuleWidget {
	LavenderModuleWidget(Lavender* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Lavender.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		// First do the knobs
		const float knobX1 = 6;
		const float knobX2 = 15;
		const float knobX3 = 87;

		const float knobY1 = 47;
		const float knobY2 = 50;
		const float knobY3 = 122;
		const float knobY4 = 125;
		const float knobY5 = 197;
		const float knobY6 = 200;
		const float knobY7 = 275;

		addParam(createParam<RPJKnobBig>(Vec(knobX2, knobY2), module, Lavender::PARAM_FC));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY1), module, Lavender::PARAM_CVFC));
		addParam(createParam<RPJKnobBig>(Vec(knobX2, knobY4), module, Lavender::PARAM_Q));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY3), module, Lavender::PARAM_CVQ));
		addParam(createParam<RPJKnobBig>(Vec(knobX2, knobY6), module, Lavender::PARAM_DRIVE));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY5), module, Lavender::PARAM_CVDRIVE));
		addParam(createParam<RPJKnob>(Vec(knobX1, knobY7), module, Lavender::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY7), module, Lavender::PARAM_DRY));

		// Next do the Jacks
		const float jackX1 = 8;
		const float jackX2 = 35;
		const float jackX3 = 49;
		const float jackX4 = 62;
		const float jackX5 = 89;

		const float jackY1 = 78;
		const float jackY2 = 153;
		const float jackY3 = 228;
		const float jackY4 = 278;
		const float jackY5 = 325;

		addInput(createInput<PJ301MPort>(Vec(jackX5, jackY1), module, Lavender::INPUT_CVFC));
		addInput(createInput<PJ301MPort>(Vec(jackX5, jackY2), module, Lavender::INPUT_CVQ));
		addInput(createInput<PJ301MPort>(Vec(jackX5, jackY3), module, Lavender::INPUT_CVDRIVE));		
		addInput(createInput<PJ301MPort>(Vec(jackX3, jackY4), module, Lavender::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX1, jackY5), module, Lavender::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX2, jackY5), module, Lavender::OUTPUT_HPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX4, jackY5), module, Lavender::OUTPUT_BPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX5, jackY5), module, Lavender::OUTPUT_BSFMAIN));
	}

	void appendContextMenu(Menu *menu) override {
		Lavender * module = dynamic_cast<Lavender*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqa));

	}
};

json_t *Lavender::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqa)));
	return rootJ;
}

void Lavender::dataFromJson(json_t *rootJ) {
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nBiquadAlgorithmJ) {
		bqa = static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelLavender = createModel<Lavender, LavenderModuleWidget>("Lavender");