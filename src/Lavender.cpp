#include "RPJ.hpp"
#include "Lavender.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "ctrl/RPJPorts.hpp"


Lavender::Lavender() {
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, -1.f, 1.0f, 0.0f, "Quality CV", "%", 0.f, 100.f);
	configParam(PARAM_CVDRIVE, -1.f, 1.0f, 0.0f, "Drive CV", "%", 0, 100);
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "Dry", "%", 0.f, 100.f);
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "Wet", "%", 0.f, 100.f);
	configParam(PARAM_DRIVE, 0.f, 1.0f, 0.3f, "Drive", "%", 0, 100, 100);
	configBypass(INPUT_MAIN, OUTPUT_LPF);
	configBypass(INPUT_MAIN, OUTPUT_HPF);
	configBypass(INPUT_MAIN, OUTPUT_BPF);
	configBypass(INPUT_MAIN, OUTPUT_BSF);
	configInput(INPUT_MAIN,"Main Audio");
	configInput(INPUT_CVFC, "Frequency Cutoff CV");
	configInput(INPUT_CVQ, "Resonance CV");
	configInput(INPUT_CVDRIVE, "Drive CV");
	configOutput(OUTPUT_LPF,"Low Pass Filter");
	configOutput(OUTPUT_HPF,"High Pass Filter");
	configOutput(OUTPUT_BPF,"Band Pass Filter");
	configOutput(OUTPUT_BSF,"Band Stop Filter");

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
	bqaUI=biquadAlgorithm::kDirect;
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
	if (outputs[OUTPUT_LPF].isConnected() || outputs[OUTPUT_HPF].isConnected() || outputs[OUTPUT_BPF].isConnected() || outputs[OUTPUT_BSF].isConnected()) {

		int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);

		outputs[OUTPUT_LPF].setChannels(channels);
		outputs[OUTPUT_HPF].setChannels(channels);
		outputs[OUTPUT_BPF].setChannels(channels);
		outputs[OUTPUT_BSF].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {

			rack::simd::float_4 cvdrive = 0.f;
			rack::simd::float_4 cvq = 0.f;

			if (inputs[INPUT_CVQ].isConnected())
				cvq = inputs[INPUT_CVQ].getPolyVoltageSimd<rack::simd::float_4>(c) / 10.0f;
 	
			float freqParam = params[PARAM_FC].getValue();
			// Rescale for backward compatibility
			freqParam = freqParam * 10.f - 5.f;
			float freqCvParam = params[PARAM_CVFC].getValue();
			// Get pitch
			simd::float_4 pitch = freqParam + inputs[INPUT_CVFC].getPolyVoltageSimd<simd::float_4>(c) * freqCvParam;
			// Set cutoff
			simd::float_4 cutoff = dsp::FREQ_C4 * simd::pow(2.f, pitch);

			cutoff = clamp(cutoff, 20.f, args.sampleRate * 0.46f);
			LPFafp.fc = HPFafp.fc = BPFafp.fc = BSFafp.fc = cutoff;
			
			LPFafp.Q = HPFafp.Q = BPFafp.Q = BSFafp.Q = clamp((params[PARAM_CVQ].getValue() * cvq * 20.f) + params[PARAM_Q].getValue(),0.707f, 20.0f);
			LPFafp.dry = HPFafp.dry = BPFafp.dry = BSFafp.dry = params[PARAM_DRY].getValue();
			LPFafp.wet = HPFafp.wet = BPFafp.wet = BSFafp.wet = params[PARAM_WET].getValue();
			LPFafp.bqa = HPFafp.bqa = BPFafp.bqa = BSFafp.bqa = bqaUI;

			if (inputs[INPUT_CVDRIVE].isConnected())
				cvdrive = inputs[INPUT_CVDRIVE].getPolyVoltageSimd<rack::simd::float_4>(c) / 10.0f;
			LPFafp.drive = HPFafp.drive = BPFafp.drive = BSFafp.drive = clamp((params[PARAM_CVDRIVE].getValue() * cvdrive) + params[PARAM_DRIVE].getValue(),0.f,1.f);

			processChannel(c, inputs[INPUT_MAIN],outputs[OUTPUT_LPF],outputs[OUTPUT_HPF],outputs[OUTPUT_BPF],outputs[OUTPUT_BSF]);
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

		const float knobX1 = 8.5;
		const float knobX2 = 11.4;
		const float knobX3 = 36;
		const float knobX4 = 47;
		const float knobX5 = 48;
		const float knobX6 = 85.2;
		const float knobX7 = 88.2;

		const float knobY1 = 27.5;
		const float knobY2 = 98;
		const float knobY3 = 150;
		const float knobY4 = 205;
		const float knobY5 = 269;

		addParam(createParam<RPJKnobBig>(Vec(knobX3, knobY1), module, Lavender::PARAM_FC));
		addParam(createParam<RPJKnobSmall>(Vec(knobX2, knobY4), module, Lavender::PARAM_CVFC));
		addParam(createParam<RPJKnob>(Vec(knobX4, knobY2), module, Lavender::PARAM_Q));
		addParam(createParam<RPJKnobSmall>(Vec(knobX7, knobY4), module, Lavender::PARAM_CVQ));
		addParam(createParam<RPJKnob>(Vec(knobX4, knobY3), module, Lavender::PARAM_DRIVE));
		addParam(createParam<RPJKnobSmall>(Vec(knobX5, knobY4), module, Lavender::PARAM_CVDRIVE));
		addParam(createParam<RPJKnob>(Vec(knobX1, knobY5), module, Lavender::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(knobX6, knobY5), module, Lavender::PARAM_DRY));

		// Next do the Jacks
		const float jackX1 = 5.5;
		const float jackX2 = 10;		
		const float jackX3 = 34;
		const float jackX4 = 49;
		const float jackX5 = 63;
		const float jackX6 = 87;
		const float jackX7 = 92;

		const float jackY1 = 233;
		const float jackY2 = 272;
		const float jackY3 = 311;

		addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY1), module, Lavender::INPUT_CVFC));
		addInput(createInput<RPJPJ301MPort>(Vec(jackX6, jackY1), module, Lavender::INPUT_CVQ));
		addInput(createInput<RPJPJ301MPort>(Vec(jackX4, jackY1), module, Lavender::INPUT_CVDRIVE));		
		addInput(createInput<RPJPJ301MPort>(Vec(jackX4, jackY2), module, Lavender::INPUT_MAIN));
		addOutput(createOutput<RPJPJ301MPort>(Vec(jackX1, jackY3), module, Lavender::OUTPUT_LPF));
		addOutput(createOutput<RPJPJ301MPort>(Vec(jackX3, jackY3), module, Lavender::OUTPUT_HPF));
		addOutput(createOutput<RPJPJ301MPort>(Vec(jackX5, jackY3), module, Lavender::OUTPUT_BPF));
		addOutput(createOutput<RPJPJ301MPort>(Vec(jackX7, jackY3), module, Lavender::OUTPUT_BSF));
	}

	void appendContextMenu(Menu *menu) override {
		Lavender * module = dynamic_cast<Lavender*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqaUI));

	}
};

json_t *Lavender::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqaUI)));
	return rootJ;
}

void Lavender::dataFromJson(json_t *rootJ) {
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nBiquadAlgorithmJ) {
		bqaUI = static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelLavender = createModel<Lavender, LavenderModuleWidget>("Lavender");