#include "RPJ.hpp"
#include "Easter.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "ctrl/RPJButtons.hpp"

Easter::Easter() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	audioFilter.reset(APP->engine->getSampleRate());
	afp.algorithm = filterAlgorithm::kResonA;
	bqa=biquadAlgorithm::kDirect;
}

void Easter::onSampleRateChange() {
	audioFilter.reset(APP->engine->getSampleRate());
}

void Easter::process(const ProcessArgs &args) {

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		afp.algorithm = filterAlgorithm::kResonB;	
	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f)))
		afp.algorithm = filterAlgorithm::kResonA;
	afp.strAlgorithm = audioFilter.filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
	audioFilter.setParameters(afp);

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
		audioFilter.setSampleRate(args.sampleRate);
	
		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = abs(inputs[INPUT_CVFC].getVoltage() /10.0);
	
		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = abs(inputs[INPUT_CVQ].getVoltage() / 10.0);
 	
		afp.fc = pow(2048,params[PARAM_FC].getValue()) * 10 * cvfc;
		afp.Q = params[PARAM_Q].getValue() * cvq;
		afp.dry = params[PARAM_DRY].getValue();
		afp.wet = params[PARAM_WET].getValue();
		afp.bqa = bqa;

		afp.strAlgorithm = audioFilter.filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
		audioFilter.setParameters(afp);
		
		float out = audioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		outputs[OUTPUT_MAIN].setVoltage(out);
	}
}

struct EasterModuleWidget : ModuleWidget {
	EasterModuleWidget(Easter* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Easter.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			FilterNameDisplay * fnd = new FilterNameDisplay(Vec(39,30));
			fnd->module = module;
			addChild(fnd);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 320), module, Easter::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Easter::OUTPUT_MAIN));
		
		addParam(createParam<buttonMinSmall>(Vec(5,45),module, Easter::PARAM_DOWN));
		addParam(createParam<buttonPlusSmall>(Vec(76,45),module, Easter::PARAM_UP));
		addParam(createParam<RPJKnob>(Vec(8, 105), module, Easter::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 107), module, Easter::INPUT_CVFC));
		addParam(createParam<RPJKnob>(Vec(8, 175), module, Easter::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(55, 177), module, Easter::INPUT_CVQ));	
		addParam(createParam<RPJKnob>(Vec(8, 250), module, Easter::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(55, 250), module, Easter::PARAM_DRY));
	}

	void appendContextMenu(Menu *menu) override {
		Easter * module = dynamic_cast<Easter*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqa));

	}
};

json_t *Easter::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_RESONATOR_TYPE_KEY, json_integer(static_cast<int>(afp.algorithm)));
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqa)));
	return rootJ;
}

void Easter::dataFromJson(json_t *rootJ) {
	json_t *nAlgorithmJ = json_object_get(rootJ, JSON_RESONATOR_TYPE_KEY);
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nAlgorithmJ) {
		afp.algorithm=static_cast<filterAlgorithm>(json_integer_value(nAlgorithmJ));
	}
	if (nBiquadAlgorithmJ) {
		bqa=static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelEaster = createModel<Easter, EasterModuleWidget>("Easter");