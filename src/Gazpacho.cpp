#include "RPJ.hpp"
#include "Gazpacho.hpp"
#include "ctrl/RPJKnobs.hpp"


Gazpacho::Gazpacho() {
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
	LPFaudioFilter.reset(44100);
	HPFaudioFilter.reset(44100);
	LPFafp.algorithm=filterAlgorithm::kLWRLPF2;
	HPFafp.algorithm=filterAlgorithm::kLWRHPF2;
}

void Gazpacho::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected()) {
		LPFaudioFilter.setSampleRate(args.sampleRate);
		HPFaudioFilter.setSampleRate(args.sampleRate);

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = abs(inputs[INPUT_CVFC].getVoltage() / 10.0);
 	
 		LPFafp.fc = HPFafp.fc = params[PARAM_FC].getValue() * cvfc;
		LPFafp.dry = HPFafp.dry = params[PARAM_DRY].getValue();
		LPFafp.wet = HPFafp.wet = params[PARAM_WET].getValue();

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

struct GazpachoModuleWidget : ModuleWidget {
	GazpachoModuleWidget(Gazpacho* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Gazpacho.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		addInput(createInput<PJ301MPort>(Vec(10, 245), module, Gazpacho::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 245), module, Gazpacho::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 305), module, Gazpacho::OUTPUT_HPFMAIN));

		addParam(createParam<RPJKnob>(Vec(8, 60), module, Gazpacho::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 62), module, Gazpacho::INPUT_CVFC));

		addParam(createParam<RPJKnob>(Vec(8, 155), module, Gazpacho::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(55, 155), module, Gazpacho::PARAM_DRY));
	}

};

Model * modelGazpacho = createModel<Gazpacho, GazpachoModuleWidget>("Gazpacho");