#include "RPJ.hpp"
#include "Lavender.hpp"
#include "ctrl/RPJKnobs.hpp"


Lavender::Lavender() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	configBypass(INPUT_MAIN, OUTPUT_LPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_HPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_BPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_BSFMAIN);
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
	BPFaudioFilter.reset(APP->engine->getSampleRate());
	BSFaudioFilter.reset(APP->engine->getSampleRate());
	LPFafp.algorithm=filterAlgorithm::kLPF2;
	HPFafp.algorithm=filterAlgorithm::kHPF2;
	BPFafp.algorithm=filterAlgorithm::kBPF2;
	BSFafp.algorithm=filterAlgorithm::kBSF2;
	bqa=biquadAlgorithm::kDirect;
}

void Lavender::onSampleRateChange() {
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
	LPFaudioFilter.reset(APP->engine->getSampleRate());
	HPFaudioFilter.reset(APP->engine->getSampleRate());
}

void Lavender::process(const ProcessArgs &args) {
	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected() || outputs[OUTPUT_BPFMAIN].isConnected() || outputs[OUTPUT_BSFMAIN].isConnected()) {

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = abs(inputs[INPUT_CVFC].getVoltage() / 10.0);
	
		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = abs(inputs[INPUT_CVQ].getVoltage() / 10.0);
 	
 		LPFafp.fc = HPFafp.fc = BPFafp.fc = BSFafp.fc = params[PARAM_FC].getValue() * cvfc;
		LPFafp.Q = HPFafp.Q = BPFafp.Q = BSFafp.Q = params[PARAM_Q].getValue() * cvq;
		LPFafp.dry = HPFafp.dry = BPFafp.dry = BSFafp.dry = params[PARAM_DRY].getValue();
		LPFafp.wet = HPFafp.wet = BPFafp.wet = BSFafp.wet = params[PARAM_WET].getValue();
		LPFafp.bqa = HPFafp.bqa = BPFafp.bqa = BSFafp.bqa = bqa;

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

struct LavenderModuleWidget : ModuleWidget {
	LavenderModuleWidget(Lavender* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Lavender.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		addInput(createInput<PJ301MPort>(Vec(10, 240), module, Lavender::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 240), module, Lavender::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, Lavender::OUTPUT_HPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(10, 320), module, Lavender::OUTPUT_BPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Lavender::OUTPUT_BSFMAIN));


		addParam(createParam<RPJKnob>(Vec(8, 60), module, Lavender::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 62), module, Lavender::INPUT_CVFC));
		addParam(createParam<RPJKnob>(Vec(8, 115), module, Lavender::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(55, 117), module, Lavender::INPUT_CVQ));
		addParam(createParam<RPJKnob>(Vec(8, 175), module, Lavender::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(55, 175), module, Lavender::PARAM_DRY));
	}

	void appendContextMenu(Menu *menu) override {
		Lavender * module = dynamic_cast<Lavender*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqa));

	}
};

json_t *Lavender::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqa)));
	return rootJ;
}

void Lavender::dataFromJson(json_t *rootJ) {
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nBiquadAlgorithmJ) {
		bqa = static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelLavender = createModel<Lavender, LavenderModuleWidget>("Lavender");