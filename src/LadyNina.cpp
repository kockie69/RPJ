#include "RPJ.hpp"
#include "LadyNina.hpp"
#include "ctrl/RPJKnobs.hpp"


LadyNina::LadyNina() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB,  -20.f, 20.f, 0.f, "Boost/Cut"," dB");
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
	LPFafp.filterAlgorithm=vaFilterAlgorithm::kSVF_LP;
	HPFafp.filterAlgorithm=vaFilterAlgorithm::kSVF_HP;
	BPFafp.filterAlgorithm=vaFilterAlgorithm::kSVF_BP;
	BSFafp.filterAlgorithm=vaFilterAlgorithm::kSVF_BS;
	nlp=gain=osc=match=false;
}

void LadyNina::onSampleRateChange() {
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
}

void LadyNina::process(const ProcessArgs &args) {

	if ((outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected() ||
		outputs[OUTPUT_BPFMAIN].isConnected() || outputs[OUTPUT_BSFMAIN].isConnected()) &&
		inputs[INPUT_MAIN].isConnected()) {

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = inputs[INPUT_CVFC].getVoltage();
	
		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = inputs[INPUT_CVQ].getVoltage();
 	
	 	float cvbcdb = 1.f;
		if (inputs[INPUT_CVBCDB].isConnected())
			cvq = inputs[INPUT_CVBCDB].getVoltage();

 		LPFafp.fc = HPFafp.fc = BPFafp.fc = BSFafp.fc = params[PARAM_FC].getValue() * rescale(cvfc,-10,10,0,1);
		LPFafp.Q = HPFafp.Q = BPFafp.Q = BSFafp.Q = params[PARAM_Q].getValue() * rescale(cvq,-10,10,0,1);
		LPFafp.filterOutputGain_dB = HPFafp.filterOutputGain_dB = BPFafp.filterOutputGain_dB = BSFafp.filterOutputGain_dB = params[PARAM_BOOSTCUT_DB].getValue() * rescale(cvbcdb,-10,10,0,1);
		LPFafp.enableGainComp = HPFafp.enableGainComp = BPFafp.enableGainComp = BSFafp.enableGainComp = gain;
		LPFafp.enableNLP = HPFafp.enableNLP = BPFafp.enableNLP = BSFafp.enableNLP = nlp;
		LPFafp.selfOscillate = HPFafp.selfOscillate = BPFafp.selfOscillate = BSFafp.selfOscillate = osc;
		LPFafp.matchAnalogNyquistLPF = match;

		if (outputs[OUTPUT_LPFMAIN].isConnected()) {
			LPFaudioFilter.setParameters(LPFafp);
			float LPFOut = LPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());	
			outputs[OUTPUT_LPFMAIN].setVoltage(LPFOut);
		}
		if (outputs[OUTPUT_HPFMAIN].isConnected()) {
			HPFaudioFilter.setParameters(HPFafp);
			float HPFOut = HPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
			outputs[OUTPUT_HPFMAIN].setVoltage(HPFOut);
		}
		if (outputs[OUTPUT_BPFMAIN].isConnected()) {
			BPFaudioFilter.setParameters(BPFafp);
			float BPFOut = BPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
			outputs[OUTPUT_BPFMAIN].setVoltage(BPFOut);
		}
		if (outputs[OUTPUT_BSFMAIN].isConnected()) {
			BSFaudioFilter.setParameters(BSFafp);
			float BSFOut = BSFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
			outputs[OUTPUT_BSFMAIN].setVoltage(BSFOut);
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