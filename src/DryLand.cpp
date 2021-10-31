#include "RPJ.hpp"
#include "DryLand.hpp"
#include "ctrl/RPJKnobs.hpp"


DryLand::DryLand() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
	LPFafp.algorithm=filterAlgorithm::kLPF1;
	HPFafp.algorithm=filterAlgorithm::kHPF1;
	bqa=biquadAlgorithm::kDirect;
}

void DryLand::onSampleRateChange() {
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
}

void DryLand::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected()) {

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = abs(inputs[INPUT_CVFC].getVoltage() / 10.0f);
 	
 		LPFafp.fc = HPFafp.fc = pow(2048,params[PARAM_FC].getValue()) * 10 * cvfc;
		LPFafp.dry = HPFafp.dry = params[PARAM_DRY].getValue();
		LPFafp.wet = HPFafp.wet = params[PARAM_WET].getValue();
		LPFafp.bqa = HPFafp.bqa = bqa;

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
	}
}

struct DryLandModuleWidget : ModuleWidget {
	DryLandModuleWidget(DryLand* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DryLand.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		addInput(createInput<PJ301MPort>(Vec(10, 220), module, DryLand::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 220), module, DryLand::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, DryLand::OUTPUT_HPFMAIN));

		addParam(createParam<RPJKnob>(Vec(8, 60), module, DryLand::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 62), module, DryLand::INPUT_CVFC));

		addParam(createParam<RPJKnob>(Vec(8, 145), module, DryLand::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(55, 145), module, DryLand::PARAM_DRY));
	}


	void appendContextMenu(Menu *menu) override {
		DryLand * module = dynamic_cast<DryLand*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqa));

	}
};

json_t *DryLand::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqa)));
	return rootJ;
}

void DryLand::dataFromJson(json_t *rootJ) {
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nBiquadAlgorithmJ) {
		bqa = static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelDryLand = createModel<DryLand, DryLandModuleWidget>("DryLand");