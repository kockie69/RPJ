#include "RPJ.hpp"
#include "Estonia.hpp"
#include "ctrl/RPJButtons.hpp"
#include "ctrl/RPJKnobs.hpp"


std::string EstoniaAlgorithmTxt[static_cast<int>(filterAlgorithm::numFilterAlgorithms)] = { "LPF1", "HPF1", "LPF2", "HPF2", "BPF2", "BSF2", 
		"ButterLPF2", "ButterHPF2", "ButterBPF2", "ButterBSF2", "MMALPF2", "MMALPF2B", "LowShelf",
		"HiShelf", "NCQParaEQ", "CQParaEQ", "LWRLPF2", "LWRHPF2", "APF1", "APF2", "ResonA", "ResonB",
		"MatchLP2A", "MatchLP2B", "MatchBP2A", "MatchBP2B", "ImpInvLP1", "ImpInvLP2" };

Estonia::Estonia() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "dB","Boost/Cut");
	configParam(PARAM_CVB, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
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

void Estonia::processChannel(Input& in, Output& out) {
		
	// Get input
	int channels = std::max(in.getChannels(), 1);
	simd::float_4 v[4];
	simd::float_4 output;
	out.setChannels(channels);

	for (int c = 0; c < channels; c += 4) {
		v[c/4] = simd::float_4::load(in.getVoltages(c));
		if (out.isConnected()) {
			audioFilter[c/4].setParameters(afp);
			output = audioFilter[c/4].processAudioSample(v[c/4]);
			output.store(out.getVoltages(c));
		}
	}
}


void Estonia::process(const ProcessArgs &args) {

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		afp.algorithm = filterAlgorithm::kHiShelf;	
	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		afp.algorithm = filterAlgorithm::kLowShelf;
	strAlgorithm = EstoniaAlgorithmTxt[static_cast<int>(afp.algorithm)];

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
	
		float cvfc = inputs[INPUT_CVFC].isConnected() ? abs(inputs[INPUT_CVFC].getVoltage() / 10) : 1.f;
		float cvb = inputs[INPUT_CVB].isConnected() ? abs(inputs[INPUT_CVB].getVoltage() / 10) : 1.f;
 	
		afp.fc = pow(2048,params[PARAM_FC].getValue()) * 10 * cvfc;
		afp.boostCut_dB = params[PARAM_BOOSTCUT_DB].getValue() * cvb;
	
		processChannel(inputs[INPUT_MAIN],outputs[OUTPUT_MAIN]);
	}
}

struct EstoniaModuleWidget : ModuleWidget {
	EstoniaModuleWidget(Estonia* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Estonia.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			EstoniaFilterNameDisplay * fnd = new EstoniaFilterNameDisplay(Vec(39,30));
			fnd->module = module;
			addChild(fnd);
		}

		addInput(createInput<PJ301MPort>(Vec(33, 258), module, Estonia::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(33, 315), module, Estonia::OUTPUT_MAIN));
		
		addParam(createParam<buttonMinSmall>(Vec(5,45),module, Estonia::PARAM_DOWN));
		addParam(createParam<buttonPlusSmall>(Vec(76,45),module, Estonia::PARAM_UP));
		addParam(createParam<RPJKnob>(Vec(8, 100), module, Estonia::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 102), module, Estonia::INPUT_CVFC));
		addParam(createParam<RPJKnob>(Vec(8, 173), module, Estonia::PARAM_BOOSTCUT_DB));
		addInput(createInput<PJ301MPort>(Vec(55, 175), module, Estonia::INPUT_CVB));	
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