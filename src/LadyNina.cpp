#include "RPJ.hpp"
#include "LadyNina.hpp"
#include "ctrl/RPJKnobs.hpp"


LadyNina::LadyNina() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB,  -20.f, 20.f, 0.f, "Boost/Cut"," dB");
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

void LadyNina::processChannel(Input& in, Output& lpfout, Output& hpfout, Output& bpfout, Output& bsfout) {
		
	// Get input
	int channels = std::max(in.getChannels(), 1);
	simd::float_4 v[4];
	simd::float_4 output;
	lpfout.setChannels(channels);
	hpfout.setChannels(channels);
	bpfout.setChannels(channels);
	bsfout.setChannels(channels);

	for (int c = 0; c < channels; c += 4) {
		v[c/4] = simd::float_4::load(in.getVoltages(c));
		if (lpfout.isConnected()) {
			LPFaudioFilter[c/4].setParameters(LPFafp);
			output = LPFaudioFilter[c/4].processAudioSample(v[c/4]);
			output.store(lpfout.getVoltages(c));
		}
		if (hpfout.isConnected()) {
			HPFaudioFilter[c/4].setParameters(HPFafp);
			output = HPFaudioFilter[c/4].processAudioSample(v[c/4]);
			output.store(hpfout.getVoltages(c));
		}
		if (bpfout.isConnected()) {
			BPFaudioFilter[c/4].setParameters(BPFafp);
			output = BPFaudioFilter[c/4].processAudioSample(v[c/4]);
			output.store(bpfout.getVoltages(c));
		}
		if (bsfout.isConnected()) {
			BSFaudioFilter[c/4].setParameters(BSFafp);
			output = BSFaudioFilter[c/4].processAudioSample(v[c/4]);
			output.store(bsfout.getVoltages(c));
		}
	}
}

void LadyNina::process(const ProcessArgs &args) {

	if ((outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected() ||
		outputs[OUTPUT_BPFMAIN].isConnected() || outputs[OUTPUT_BSFMAIN].isConnected()) &&
		inputs[INPUT_MAIN].isConnected()) {

		float cvfc = inputs[INPUT_CVFC].getVoltage()/10.f;
		float cvq = inputs[INPUT_CVQ].getVoltage() * 2.f;

	 	float cvbcdb = inputs[INPUT_CVBCDB].getVoltage() * 2.f;

 		LPFafp.fc = HPFafp.fc = BPFafp.fc = BSFafp.fc = pow(2048,clamp(params[PARAM_FC].getValue()+cvfc,0.0909f,1.f)) * 10;
		LPFafp.Q = HPFafp.Q = BPFafp.Q = BSFafp.Q = clamp(params[PARAM_Q].getValue()+cvq,0.707f, 20.0f);
		LPFafp.filterOutputGain_dB = HPFafp.filterOutputGain_dB = BPFafp.filterOutputGain_dB = BSFafp.filterOutputGain_dB = clamp(params[PARAM_BOOSTCUT_DB].getValue() + cvbcdb,-20.f,20.f);
		LPFafp.enableGainComp = HPFafp.enableGainComp = BPFafp.enableGainComp = BSFafp.enableGainComp = gain;
		LPFafp.enableNLP = HPFafp.enableNLP = BPFafp.enableNLP = BSFafp.enableNLP = nlp;
		LPFafp.selfOscillate = HPFafp.selfOscillate = BPFafp.selfOscillate = BSFafp.selfOscillate = osc;
		LPFafp.matchAnalogNyquistLPF = match;

		processChannel(inputs[INPUT_MAIN], outputs[OUTPUT_LPFMAIN],outputs[OUTPUT_HPFMAIN],outputs[OUTPUT_BPFMAIN],outputs[OUTPUT_BSFMAIN]);

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

		addInput(createInput<PJ301MPort>(Vec(10, 240), module, LadyNina::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 240), module, LadyNina::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, LadyNina::OUTPUT_HPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(10, 320), module, LadyNina::OUTPUT_BPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, LadyNina::OUTPUT_BSFMAIN));

		addParam(createParam<RPJKnob>(Vec(8, 60), module, LadyNina::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 62), module, LadyNina::INPUT_CVFC));
		addParam(createParam<RPJKnob>(Vec(8, 115), module, LadyNina::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(55, 117), module, LadyNina::INPUT_CVQ));
		addParam(createParam<RPJKnob>(Vec(8, 170), module, LadyNina::PARAM_BOOSTCUT_DB));
		addInput(createInput<PJ301MPort>(Vec(55, 172), module, LadyNina::INPUT_CVBCDB));
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