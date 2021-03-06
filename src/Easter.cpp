#include "RPJ.hpp"
#include "Easter.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "ctrl/RPJButtons.hpp"

std::string EasterAlgorithmTxt[static_cast<int>(filterAlgorithm::numFilterAlgorithms)] = { "LPF1", "HPF1", "LPF2", "HPF2", "BPF2", "BSF2", 
		"ButterLPF2", "ButterHPF2", "ButterBPF2", "ButterBSF2", "MMALPF2", "MMALPF2B", "LowShelf",
		"HiShelf", "NCQParaEQ", "CQParaEQ", "LWRLPF2", "LWRHPF2", "APF1", "APF2", "ResonA", "ResonB",
		"ImpInvLP1", "ImpInvLP2" };


Easter::Easter() {

	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, -1.f, 1.0f, 0.0f, "Quality CV", "%", 0.f, 100.f);
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "Dry");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "Wet");
	configButton(PARAM_UP, "Next Algorithm");
	configButton(PARAM_DOWN, "Previous Algorithm");
	configInput(INPUT_CVFC,"Frequency Cutoff CV");
	configInput(INPUT_CVQ,"Quality CV");
	configInput(INPUT_MAIN,"Main");
	configOutput(OUTPUT_MAIN, "Filter");
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	for (int i=0;i<4;i++) {
		audioFilter[i].reset(APP->engine->getSampleRate());
	}
	afp.algorithm = filterAlgorithm::kResonA;
	strAlgorithm = "ResonA";
	bqaUI=biquadAlgorithm::kDirect;
}

void Easter::onSampleRateChange() {
	for (int i=0;i<4;i++) {
		audioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Easter::processChannel(int c,Input& in, Output& out) {
		
	audioFilter[c/4].setParameters(afp);
	out.setVoltageSimd(simd::clamp(audioFilter[c/4].processAudioSample(in.getPolyVoltage(c)),-5.f,5.f),c);
}

void Easter::process(const ProcessArgs &args) {

	int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);
	outputs[OUTPUT_MAIN].setChannels(channels);


	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f)) || downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f))) { 
		if (afp.algorithm != filterAlgorithm::kResonB)
			afp.algorithm = filterAlgorithm::kResonB;
		else
			afp.algorithm = filterAlgorithm::kResonA;	
	}

	strAlgorithm = EasterAlgorithmTxt[static_cast<int>(afp.algorithm)];

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {

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
			afp.fc = cutoff;

			rack::simd::float_4 cvq = 0.f;
			if (inputs[INPUT_CVQ].isConnected())
				cvq = inputs[INPUT_CVQ].getPolyVoltageSimd<simd::float_4>(c) / 10.0;
 		
			afp.Q = rack::simd::clamp((params[PARAM_CVQ].getValue() * cvq * 20.f) + params[PARAM_Q].getValue(),0.707f, 20.0f);
			
			afp.dry = params[PARAM_DRY].getValue();
			afp.wet = params[PARAM_WET].getValue();
			afp.bqa = bqaUI;

			processChannel(c,inputs[INPUT_MAIN],outputs[OUTPUT_MAIN]);
		}
	}
}

struct EasterModuleWidget : ModuleWidget {
	EasterModuleWidget(Easter* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Easter.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		// First the small selection buttons
		const float buttonX1 = 2;
		const float buttonX2 = 74;

		const float buttonY1 = 215;

		addParam(createParam<ButtonMinBig>(Vec(buttonX1,buttonY1),module, Easter::PARAM_DOWN));
		addParam(createParam<ButtonPlusBig>(Vec(buttonX2,buttonY1),module, Easter::PARAM_UP));

		// And the display that shows the selection
		{
			EasterFilterNameDisplay * fnd = new EasterFilterNameDisplay(Vec(39,202));
			fnd->module = module;
			addChild(fnd);
		}



		// Next do the knobs
		const float knobX1 = 3;
		const float knobX2 = 3;
		const float knobX3 = 60;
		const float knobX4 = 60;

		const float knobY1 = 47;
		const float knobY2 = 50;
		const float knobY3 = 122;
		const float knobY4 = 125;
		const float knobY5 = 275;

		addParam(createParam<RPJKnobBig>(Vec(knobX2, knobY2), module, Easter::PARAM_FC));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY1), module, Easter::PARAM_CVFC));
		addParam(createParam<RPJKnobBig>(Vec(knobX2, knobY4), module, Easter::PARAM_Q));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY3), module, Easter::PARAM_CVQ));
		addParam(createParam<RPJKnob>(Vec(knobX1, knobY5), module, Easter::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(knobX4, knobY5), module, Easter::PARAM_DRY));

		// Next do the Jacks
		const float jackX2 = 33.5f;
		const float jackX3 = 62;

		const float jackY1 = 78;
		const float jackY2 = 153;
		const float jackY3 = 278;
		const float jackY4 = 325;

		addInput(createInput<PJ301MPort>(Vec(jackX2, jackY3), module, Easter::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX2, jackY4), module, Easter::OUTPUT_MAIN));
		addInput(createInput<PJ301MPort>(Vec(jackX3, jackY1), module, Easter::INPUT_CVFC));
		addInput(createInput<PJ301MPort>(Vec(jackX3, jackY2), module, Easter::INPUT_CVQ));
	}

	void appendContextMenu(Menu *menu) override {
		Easter * module = dynamic_cast<Easter*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqaUI));

	}
};

json_t *Easter::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_RESONATOR_TYPE_KEY, json_integer(static_cast<int>(afp.algorithm)));
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqaUI)));
	return rootJ;
}

void Easter::dataFromJson(json_t *rootJ) {
	json_t *nAlgorithmJ = json_object_get(rootJ, JSON_RESONATOR_TYPE_KEY);
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nAlgorithmJ) {
		afp.algorithm=static_cast<filterAlgorithm>(json_integer_value(nAlgorithmJ));
	}
	if (nBiquadAlgorithmJ) {
		bqaUI=static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelEaster = createModel<Easter, EasterModuleWidget>("Easter");