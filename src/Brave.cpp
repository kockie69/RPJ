#include "RPJ.hpp"
#include "Brave.hpp"
#include "ctrl/RPJKnobs.hpp"

Brave::Brave() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "dB","Boost/Cut");
	configParam(PARAM_CVB, 0.f, 1.0f, 0.0f, "CV Q");
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	afp.algorithm = filterAlgorithm::kNCQParaEQ;
	audioFilter.reset(APP->engine->getSampleRate());
}

void Brave::onSampleRateChange() {
	audioFilter.reset(APP->engine->getSampleRate());
}

void Brave::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected()) {
		audioFilter.setSampleRate(args.sampleRate);
	
		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = inputs[INPUT_CVFC].getVoltage();

		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = inputs[INPUT_CVQ].getVoltage();
	
		float cvb = 1.f;
		if (inputs[INPUT_CVB].isConnected())
			cvb = inputs[INPUT_CVB].getVoltage();
 	
		afp.fc = params[PARAM_FC].getValue() * rescale(cvfc,-10,10,0,1);
		afp.Q = params[PARAM_Q].getValue() * rescale(cvq,-10,10,0,1);
		afp.boostCut_dB = params[PARAM_BOOSTCUT_DB].getValue() * rescale(cvb,-10,10,0,1);

		afp.strAlgorithm = audioFilter.filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
		afp.bqa = bqa;

		audioFilter.setParameters(afp);

		float out = audioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		outputs[OUTPUT_MAIN].setVoltage(out);
	}
}

struct BraveModuleWidget : ModuleWidget {
	BraveModuleWidget(Brave* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Brave.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		addInput(createInput<PJ301MPort>(Vec(33, 258), module, Brave::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(33, 315), module, Brave::OUTPUT_MAIN));
		
		addParam(createParam<RPJKnob>(Vec(8, 80), module, Brave::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 82), module, Brave::INPUT_CVFC));
		addParam(createParam<RPJKnob>(Vec(8, 140), module, Brave::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(55, 142), module, Brave::INPUT_CVQ));
		addParam(createParam<RPJKnob>(Vec(8, 200), module, Brave::PARAM_BOOSTCUT_DB));
		addInput(createInput<PJ301MPort>(Vec(55, 202), module, Brave::INPUT_CVB));	
	}

		void appendContextMenu(Menu *menu) override {
		Brave * module = dynamic_cast<Brave*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqa));

	}
};

json_t *Brave::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqa)));
	return rootJ;
}

void Brave::dataFromJson(json_t *rootJ) {
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nBiquadAlgorithmJ) {
		bqa = static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelBrave = createModel<Brave, BraveModuleWidget>("Brave");