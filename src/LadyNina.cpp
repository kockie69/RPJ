#include "RPJ.hpp"
#include "LadyNina.hpp"
#include "ctrl/RPJKnobs.hpp"


LadyNina::LadyNina() {
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, -1.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_CVB, -1.f, 1.0f, 0.0f, "Drive CV", "%", 0, 100);
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "Dry", "%", 0.f, 100.f);
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "Wet", "%", 0.f, 100.f);
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "Boost/Cut","dB");
	configInput(INPUT_CVFC, "Cutoff CV");
	configInput(INPUT_CVQ, "Quality CV");
	configInput(INPUT_CVBCDB,"Boost/Cut CV");
	configInput(INPUT_MAIN,"Main");
	configOutput(OUTPUT_LPFMAIN, "Low Pass Filter");
	configOutput(OUTPUT_HPFMAIN, "High Pass Filter");
	configOutput(OUTPUT_BPFMAIN, "Band Pass Filter");
	configOutput(OUTPUT_BSFMAIN, "Band Stop Filter");
	for (int i=0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
	LPFafp.filterAlgorithm=vaFilterAlgorithm::kSVF_LP;
	HPFafp.filterAlgorithm=vaFilterAlgorithm::kSVF_HP;
	BPFafp.filterAlgorithm=vaFilterAlgorithm::kSVF_BP;
	BSFafp.filterAlgorithm=vaFilterAlgorithm::kSVF_BS;
	nlp=gain=osc=match=false;
}

void LadyNina::onSampleRateChange() {
	for (int i=0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void LadyNina::processChannel(int c,Input& in, Output& lpfout, Output& hpfout, Output& bpfout, Output& bsfout) {
		
	simd::float_4 v = in.getPolyVoltageSimd<simd::float_4>(c);
	
	LPFaudioFilter[c/4].setParameters(LPFafp);
	lpfout.setVoltageSimd(simd::clamp(LPFaudioFilter[c/4].processAudioSample(v),-5.f,5.f),c);


	HPFaudioFilter[c/4].setParameters(HPFafp);
	hpfout.setVoltageSimd(simd::clamp(HPFaudioFilter[c/4].processAudioSample(v),-5.f,5.f),c);


	BPFaudioFilter[c/4].setParameters(BPFafp);
	bpfout.setVoltageSimd(simd::clamp(BPFaudioFilter[c/4].processAudioSample(v),-5.f,5.f),c);

	BSFaudioFilter[c/4].setParameters(BSFafp);
	bsfout.setVoltageSimd(simd::clamp(BSFaudioFilter[c/4].processAudioSample(v),-5.f,5.f),c);
}

void LadyNina::process(const ProcessArgs &args) {

	if ((outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected() ||
		outputs[OUTPUT_BPFMAIN].isConnected() || outputs[OUTPUT_BSFMAIN].isConnected()) &&
		inputs[INPUT_MAIN].isConnected()) {

		int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);

		outputs[OUTPUT_LPFMAIN].setChannels(channels);
		outputs[OUTPUT_HPFMAIN].setChannels(channels);
		outputs[OUTPUT_BPFMAIN].setChannels(channels);
		outputs[OUTPUT_BSFMAIN].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {

			rack::simd::float_4 cvq = 0.f;

			if (inputs[INPUT_CVQ].isConnected())
				cvq = inputs[INPUT_CVQ].getPolyVoltageSimd<rack::simd::float_4>(c) / 10.0;
 	
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
	 		
			rack::simd::float_4 cvbcdb = inputs[INPUT_CVBCDB].getPolyVoltageSimd<rack::simd::float_4>(c);
 			LPFafp.Q = HPFafp.Q = BPFafp.Q = BSFafp.Q = clamp((params[PARAM_CVQ].getValue() * cvq * 20.f) + params[PARAM_Q].getValue(),0.707f, 20.0f);
			
			LPFafp.filterOutputGain_dB = HPFafp.filterOutputGain_dB = BPFafp.filterOutputGain_dB = BSFafp.filterOutputGain_dB = clamp(params[PARAM_BOOSTCUT_DB].getValue() + cvbcdb*20,-20.f,20.f);
			LPFafp.enableGainComp = HPFafp.enableGainComp = BPFafp.enableGainComp = BSFafp.enableGainComp = gain;
			LPFafp.enableNLP = HPFafp.enableNLP = BPFafp.enableNLP = BSFafp.enableNLP = nlp;
			LPFafp.selfOscillate = HPFafp.selfOscillate = BPFafp.selfOscillate = BSFafp.selfOscillate = osc;
			LPFafp.matchAnalogNyquistLPF = match;

			processChannel(c, inputs[INPUT_MAIN], outputs[OUTPUT_LPFMAIN],outputs[OUTPUT_HPFMAIN],outputs[OUTPUT_BPFMAIN],outputs[OUTPUT_BSFMAIN]);
		}
	}
}

struct LadyNinaModuleWidget : ModuleWidget {
	LadyNinaModuleWidget(LadyNina* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LadyNina.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		// First do the knobs
		const float knobX1 = 15;
		const float knobX2 = 87;

		const float knobY1 = 47;
		const float knobY2 = 50;
		const float knobY3 = 122;
		const float knobY4 = 125;
		const float knobY5 = 197;
		const float knobY6 = 200;

		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY2), module, LadyNina::PARAM_FC));
		addParam(createParam<RPJKnob>(Vec(knobX2, knobY1), module, LadyNina::PARAM_CVFC));
		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY4), module, LadyNina::PARAM_Q));
		addParam(createParam<RPJKnob>(Vec(knobX2, knobY3), module, LadyNina::PARAM_CVQ));
		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY6), module, LadyNina::PARAM_BOOSTCUT_DB));
		addParam(createParam<RPJKnob>(Vec(knobX2, knobY5), module, LadyNina::PARAM_CVB));

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

		addInput(createInput<PJ301MPort>(Vec(jackX5, jackY1), module, LadyNina::INPUT_CVFC));
		addInput(createInput<PJ301MPort>(Vec(jackX5, jackY2), module, LadyNina::INPUT_CVQ));
		addInput(createInput<PJ301MPort>(Vec(jackX5, jackY3), module, LadyNina::INPUT_CVBCDB));		
		addInput(createInput<PJ301MPort>(Vec(jackX3, jackY4), module, LadyNina::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX1, jackY5), module, LadyNina::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX2, jackY5), module, LadyNina::OUTPUT_HPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX4, jackY5), module, LadyNina::OUTPUT_BPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX5, jackY5), module, LadyNina::OUTPUT_BSFMAIN));
	}

	void appendContextMenu(Menu *menu) override {
		LadyNina *module = dynamic_cast<LadyNina*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(rack::createBoolPtrMenuItem("Enable Gain Compensation", "", &module->gain));
		menu->addChild(rack::createBoolPtrMenuItem("Enable NLP", "", &module->nlp));
		menu->addChild(rack::createBoolPtrMenuItem("Enable Self Oscilation", "", &module->osc));
		menu->addChild(rack::createBoolPtrMenuItem("Match gain at Nyquist for LPF", "", &module->match));		
	}
};

json_t *LadyNina::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_GAIN_KEY, json_boolean(gain));
	json_object_set_new(rootJ, JSON_NLP_KEY, json_boolean(nlp));
	json_object_set_new(rootJ, JSON_OSCILLATOR_KEY, json_boolean(osc));
	json_object_set_new(rootJ, JSON_MATCH_KEY, json_boolean(match));	
	return rootJ;
}

void LadyNina::dataFromJson(json_t *rootJ) {
	json_t *nGainJ = json_object_get(rootJ, JSON_GAIN_KEY);
	if (nGainJ) {
		gain = json_boolean_value(nGainJ);
	}

	json_t *nNlpJ = json_object_get(rootJ, JSON_NLP_KEY);
	if (nNlpJ) {
		nlp = json_boolean_value(nNlpJ);
	}

	json_t *nTOscillatorJ = json_object_get(rootJ, JSON_OSCILLATOR_KEY);
	if (nTOscillatorJ) {
		osc = json_boolean_value(nTOscillatorJ);
	}

	json_t *nMatchJ = json_object_get(rootJ, JSON_MATCH_KEY);
	if (nMatchJ) {
		match = json_boolean_value(nMatchJ);
	}
}

Model * modelLadyNina = createModel<LadyNina, LadyNinaModuleWidget>("LadyNina");