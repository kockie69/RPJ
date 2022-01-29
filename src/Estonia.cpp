#include "RPJ.hpp"
#include "Estonia.hpp"
#include "ctrl/RPJButtons.hpp"
#include "ctrl/RPJKnobs.hpp"


std::string EstoniaAlgorithmTxt[static_cast<int>(filterAlgorithm::numFilterAlgorithms)] = { "LPF1", "HPF1", "LPF2", "HPF2", "BPF2", "BSF2", 
		"ButterLPF2", "ButterHPF2", "ButterBPF2", "ButterBSF2", "MMALPF2", "MMALPF2B", "LowShelf",
		"HiShelf", "NCQParaEQ", "CQParaEQ", "LWRLPF2", "LWRHPF2", "APF1", "APF2", "ResonA", "ResonB",
		"ImpInvLP1", "ImpInvLP2" };

Estonia::Estonia() {
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "Boost/Cut","dB");
	configParam(PARAM_CVB, -1.f, 1.0f, 0.0f, "CV Boost/Cut", "%", 0.f, 100.f);
	configButton(PARAM_UP, "Next Algorithm");
	configButton(PARAM_DOWN, "Previous Algorithm");
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	configInput(INPUT_MAIN,"Main");
	configInput(INPUT_CVFC, "Frequency Cutoff CV");
	configInput(INPUT_CVB,"Boost/Cut CV");
	configOutput(OUTPUT_MAIN,"Filter");
	afp.algorithm = filterAlgorithm::kLowShelf;
	strAlgorithm = "LowShelf";
	for (int i = 0;i<4;i++) {
		audioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Estonia::onSampleRateChange() {
	for (int i = 0;i<4;i++) {
		audioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Estonia::processChannel(int c, Input& in, Output& out) {
		
	simd::float_4 v = in.getPolyVoltageSimd<simd::float_4>(c);
	audioFilter[c/4].setParameters(afp);
	out.setVoltageSimd(simd::clamp(audioFilter[c/4].processAudioSample(v),-5.f,5.f),c);
}


void Estonia::process(const ProcessArgs &args) {

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f)) || downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f))) { 
		if (afp.algorithm != filterAlgorithm::kHiShelf)
			afp.algorithm = filterAlgorithm::kHiShelf;
		else
			afp.algorithm = filterAlgorithm::kLowShelf;	
	}
	strAlgorithm = EstoniaAlgorithmTxt[static_cast<int>(afp.algorithm)];

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

			if (inputs[INPUT_CVB].isConnected())
				cvb = inputs[INPUT_CVB].getVoltage() / 10.0;	
 
			afp.boostCut_dB = clamp((params[PARAM_BOOSTCUT_DB].getValue() * cvb * 20.f) + params[PARAM_CVB].getValue(),-20.f,20.f);
	
			processChannel(c,inputs[INPUT_MAIN],outputs[OUTPUT_MAIN]);
		}
	}
}

struct EstoniaModuleWidget : ModuleWidget {
	EstoniaModuleWidget(Estonia* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Estonia.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		// First the small selection buttons
		const float buttonX1 = 2;
		const float buttonX2 = 74;

		const float buttonY1 = 215;

		addParam(createParam<ButtonMinBig>(Vec(buttonX1,buttonY1),module, Estonia::PARAM_DOWN));
		addParam(createParam<ButtonPlusBig>(Vec(buttonX2,buttonY1),module, Estonia::PARAM_UP));

		// And the display that shows the selection
		{
			EstoniaFilterNameDisplay * fnd = new EstoniaFilterNameDisplay(Vec(39,202));
			fnd->module = module;
			addChild(fnd);
		}



		// Next do the knobs
		const float knobX1 = 3;
		const float knobX2 = 60;

		const float knobY1 = 47;
		const float knobY2 = 50;
		const float knobY3 = 122;
		const float knobY4 = 125;

		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY2), module, Estonia::PARAM_FC));
		addParam(createParam<RPJKnob>(Vec(knobX2, knobY1), module, Estonia::PARAM_CVFC));
		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY4), module, Estonia::PARAM_BOOSTCUT_DB));
		addParam(createParam<RPJKnob>(Vec(knobX2, knobY3), module, Estonia::PARAM_CVB));

		// Next do the Jacks
		const float jackX1 = 33.5f;
		const float jackX2 = 62;

		const float jackY1 = 78;
		const float jackY2 = 153;
		const float jackY3 = 278;
		const float jackY4 = 325;

		addInput(createInput<PJ301MPort>(Vec(jackX1, jackY3), module, Estonia::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX1, jackY4), module, Estonia::OUTPUT_MAIN));
		addInput(createInput<PJ301MPort>(Vec(jackX2, jackY1), module, Estonia::INPUT_CVFC));
		addInput(createInput<PJ301MPort>(Vec(jackX2, jackY2), module, Estonia::INPUT_CVB));	
	}
};

json_t *Estonia::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_FILTER_ALGORITHM_KEY, json_integer(static_cast<int>(afp.algorithm)));
	return rootJ;
}

void Estonia::dataFromJson(json_t *rootJ) {
	json_t *nAlgorithmJ = json_object_get(rootJ, JSON_FILTER_ALGORITHM_KEY);
	if (nAlgorithmJ) {
		afp.algorithm=static_cast<filterAlgorithm>(json_integer_value(nAlgorithmJ));
	}
}

Model * modelEstonia = createModel<Estonia, EstoniaModuleWidget>("Estonia");