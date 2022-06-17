#include "RPJ.hpp"
#include "Gazpacho.hpp"
#include "ctrl/RPJKnobs.hpp"


Gazpacho::Gazpacho() {
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configParam(PARAM_CVDRIVE, -1.f, 1.0f, 0.0f, "Drive CV", "%", 0, 100);
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "Dry", "%", 0.f, 100.f);
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "Wet", "%", 0.f, 100.f);
	configParam(PARAM_DRIVE, 0.f, 1.0f, 0.3f, "Drive", "%", 0, 100, 100);
	configInput(INPUT_CVFC, "Frequency Cutoff CV");
	configInput(INPUT_CVDRIVE, "Drive CV");
	configInput(INPUT_MAIN,"Main");
	configOutput(OUTPUT_HPFMAIN,"High Pass Filter");
	configOutput(OUTPUT_LPFMAIN,"Low Pass Filter");
	configBypass(INPUT_MAIN, OUTPUT_LPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_HPFMAIN);
	for (int i = 0; i < 4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
	LPFafp.algorithm=filterAlgorithm::kLWRLPF2;
	HPFafp.algorithm=filterAlgorithm::kLWRHPF2;
	bqaUI=biquadAlgorithm::kDirect;
}

void Gazpacho::onSampleRateChange() {
	for (int i = 0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Gazpacho::processChannel(int c,Input& in, Output& lpfOut, Output& hpfOut) {
		
	simd::float_4 v = in.getPolyVoltageSimd<simd::float_4>(c);
	if (lpfOut.isConnected()) {
		LPFaudioFilter[c/4].setParameters(LPFafp);
		lpfOut.setVoltageSimd(simd::clamp(LPFaudioFilter[c/4].processAudioSample(v)*LPFafp.drive,-5.f,5.f),c);;	
	}
	if (hpfOut.isConnected()) {
		HPFaudioFilter[c/4].setParameters(HPFafp);
		hpfOut.setVoltageSimd(simd::clamp(HPFaudioFilter[c/4].processAudioSample(v)*HPFafp.drive,-5.f,5.f),c);;	
	}
}

void Gazpacho::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected()) {

		// Get input
		int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);

		outputs[OUTPUT_LPFMAIN].setChannels(channels);
		outputs[OUTPUT_HPFMAIN].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {

			rack::simd::float_4 cvdrive = 0.f;
			
			float freqParam = params[PARAM_FC].getValue();
			// Rescale for backward compatibility
			freqParam = freqParam * 10.f - 5.f;
			float freqCvParam = params[PARAM_CVFC].getValue();
			// Get pitch
			simd::float_4 pitch = freqParam + inputs[INPUT_CVFC].getPolyVoltageSimd<simd::float_4>(c) * freqCvParam;
			// Set cutoff
			simd::float_4 cutoff = dsp::FREQ_C4 * simd::pow(2.f, pitch);

			cutoff = clamp(cutoff, 20.f, args.sampleRate * 0.46f);
 			LPFafp.fc = HPFafp.fc = cutoff;			
			LPFafp.dry = HPFafp.dry = params[PARAM_DRY].getValue();
			LPFafp.wet = HPFafp.wet = params[PARAM_WET].getValue();
			LPFafp.bqa = HPFafp.bqa = bqaUI;
		
		
			if (inputs[INPUT_CVDRIVE].isConnected())
				cvdrive = inputs[INPUT_CVDRIVE].getPolyVoltageSimd<rack::simd::float_4>(c) / 10.0;
			LPFafp.drive = HPFafp.drive = clamp((params[PARAM_CVDRIVE].getValue() * cvdrive) + params[PARAM_DRIVE].getValue(),0.f,1.f);
			
			processChannel(c,inputs[INPUT_MAIN],outputs[OUTPUT_LPFMAIN],outputs[OUTPUT_HPFMAIN]);

		}
	}
}

struct GazpachoModuleWidget : ModuleWidget {
	GazpachoModuleWidget(Gazpacho* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Gazpacho.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		// First do the knobs
		const float knobX1 = 3;
		const float knobX2 = 3;
		const float knobX3 = 60;
		const float knobX4 = 60;

		const float knobY1 = 47;
		const float knobY2 = 50;
		const float knobY3 = 122;
		const float knobY4 = 125;
		const float knobY5 = 275;

		addParam(createParam<RPJKnobBig>(Vec(knobX2, knobY2), module, Gazpacho::PARAM_FC));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY1), module, Gazpacho::PARAM_CVFC));
		addParam(createParam<RPJKnobBig>(Vec(knobX2, knobY4), module, Gazpacho::PARAM_DRIVE));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY3), module, Gazpacho::PARAM_CVDRIVE));
		addParam(createParam<RPJKnob>(Vec(knobX1, knobY5), module, Gazpacho::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(knobX4, knobY5), module, Gazpacho::PARAM_DRY));

		// Next do the Jacks
		const float jackX1 = 5;
		const float jackX2 = 33.5f;
		const float jackX3 = 62;

		const float jackY1 = 78;
		const float jackY2 = 153;
		const float jackY3 = 278;
		const float jackY4 = 325;

		addInput(createInput<PJ301MPort>(Vec(jackX2, jackY3), module, Gazpacho::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX1, jackY4), module, Gazpacho::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX3, jackY4), module, Gazpacho::OUTPUT_HPFMAIN));
		addInput(createInput<PJ301MPort>(Vec(jackX3, jackY1), module, Gazpacho::INPUT_CVFC));
		addInput(createInput<PJ301MPort>(Vec(jackX3, jackY2), module, Gazpacho::INPUT_CVDRIVE));
	}

	
	void appendContextMenu(Menu *menu) override {
		Gazpacho * module = dynamic_cast<Gazpacho*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqaUI));

	}
};

json_t *Gazpacho::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqaUI)));
	return rootJ;
}

void Gazpacho::dataFromJson(json_t *rootJ) {
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nBiquadAlgorithmJ) {
		bqaUI = static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelGazpacho = createModel<Gazpacho, GazpachoModuleWidget>("Gazpacho");