#include "RPJ.hpp"
#include "Gazpacho.hpp"
#include "ctrl/RPJKnobs.hpp"


Gazpacho::Gazpacho() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	configBypass(INPUT_MAIN, OUTPUT_LPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_HPFMAIN);
	for (int i = 0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
	LPFafp.algorithm=filterAlgorithm::kLPF1;
	HPFafp.algorithm=filterAlgorithm::kHPF1;
	bqa=biquadAlgorithm::kDirect;
}

void Gazpacho::onSampleRateChange() {
	for (int i = 0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Gazpacho::processChannel(Input& in, Output& lpfOut, Output& hpfOut) {
		
	// Get input
	int channels = std::max(in.getChannels(), 1);
	simd::float_4 v[4];
	for (int c = 0; c < channels; c += 4) {
		v[c/4] = simd::float_4::load(in.getVoltages(c));
	}

	simd::float_4 output;
	lpfOut.setChannels(channels);
	hpfOut.setChannels(channels);

	for (int c = 0; c < channels; c += 4) {
		v[c/4] = simd::float_4::load(in.getVoltages(c));
		if (lpfOut.isConnected()) {
			LPFaudioFilter[c/4].setParameters(LPFafp);
			output = LPFaudioFilter[c/4].processAudioSample(v[c/4]);	
			output.store(lpfOut.getVoltages(c));
		}
		if (hpfOut.isConnected()) {
			HPFaudioFilter[c/4].setParameters(HPFafp);
			output = HPFaudioFilter[c/4].processAudioSample(v[c/4]);
			output.store(hpfOut.getVoltages(c));
		}
	}
}

void Gazpacho::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected()) {

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = abs(inputs[INPUT_CVFC].getVoltage() / 10.0);
 	
 		LPFafp.fc = HPFafp.fc = pow(2048,params[PARAM_FC].getValue()) * 10 * cvfc;
		LPFafp.dry = HPFafp.dry = params[PARAM_DRY].getValue();
		LPFafp.wet = HPFafp.wet = params[PARAM_WET].getValue();
		LPFafp.bqa = HPFafp.bqa = bqa;

		processChannel(inputs[INPUT_MAIN],outputs[OUTPUT_LPFMAIN],outputs[OUTPUT_HPFMAIN]);

	}
}

struct GazpachoModuleWidget : ModuleWidget {
	GazpachoModuleWidget(Gazpacho* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Gazpacho.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, 365)));
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

	
	void appendContextMenu(Menu *menu) override {
		Gazpacho * module = dynamic_cast<Gazpacho*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Structure", {"Direct", "Canonical", "TransposeDirect", "TransposeCanonical"}, &module->bqa));

	}
};

json_t *Gazpacho::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_BIQUAD_ALGORYTHM, json_integer(static_cast<int>(bqa)));
	return rootJ;
}

void Gazpacho::dataFromJson(json_t *rootJ) {
	json_t *nBiquadAlgorithmJ = json_object_get(rootJ, JSON_BIQUAD_ALGORYTHM);
	if (nBiquadAlgorithmJ) {
		bqa = static_cast<biquadAlgorithm>(json_integer_value(nBiquadAlgorithmJ));
	}
}

Model * modelGazpacho = createModel<Gazpacho, GazpachoModuleWidget>("Gazpacho");