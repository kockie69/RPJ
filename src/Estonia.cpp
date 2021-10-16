#include "RPJ.hpp"
#include "Estonia.hpp"
#include "ctrl/button/SmallButton.hpp"
#include "ctrl/knob/RPJKnob.hpp"

Estonia::Estonia() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "dB","Boost/Cut");
	configParam(PARAM_CVB, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	afp.algorithm = filterAlgorithm::kLowShelf;
}

void Estonia::process(const ProcessArgs &args) {

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		afp.algorithm = filterAlgorithm::kHiShelf;	
	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		afp.algorithm = filterAlgorithm::kLowShelf;
	afp.strAlgorithm = audioFilter.filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
	audioFilter.setParameters(afp);

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
		audioFilter.setSampleRate(args.sampleRate);
	
		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = inputs[INPUT_CVFC].getVoltage();
	
		float cvb = 1.f;
		if (inputs[INPUT_CVB].isConnected())
			cvb = inputs[INPUT_CVB].getVoltage();
 	
		afp.fc = params[PARAM_FC].getValue() * rescale(cvfc,-10,10,0,1);
		afp.boostCut_dB = params[PARAM_BOOSTCUT_DB].getValue() * rescale(cvb,-10,10,0,1);
	
		afp.strAlgorithm = audioFilter.filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
		audioFilter.setParameters(afp);
		
		float out = audioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		outputs[OUTPUT_MAIN].setVoltage(out);
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
			FilterNameDisplay * fnd = new FilterNameDisplay(Vec(39,30));
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