#include "RPJ.hpp"
#include "Lavender.hpp"
#include "ctrl/RPJKnobs.hpp"


Lavender::Lavender() {
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
	configBypass(INPUT_MAIN, OUTPUT_BPFMAIN);
	configBypass(INPUT_MAIN, OUTPUT_BSFMAIN);
	for (int i = 0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
		BPFaudioFilter[i].reset(APP->engine->getSampleRate());
		BSFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
	LPFafp.algorithm=filterAlgorithm::kLPF2;
	HPFafp.algorithm=filterAlgorithm::kHPF2;
	BPFafp.algorithm=filterAlgorithm::kBPF2;
	BSFafp.algorithm=filterAlgorithm::kBSF2;
	bqa=biquadAlgorithm::kDirect;
}

void Lavender::onSampleRateChange() {
	for (int i = 0;i<4;i++) {
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
		LPFaudioFilter[i].reset(APP->engine->getSampleRate());
		HPFaudioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Lavender::processChannel(Input& in, Output& lpfOut, Output& hpfOut, Output& bpfOut, Output& bsfOut) {
		
	// Get input
	int channels = std::max(in.getChannels(), 1);
	simd::float_4 v[4];
	for (int c = 0; c < channels; c += 4) {
		v[c/4] = simd::float_4::load(in.getVoltages(c));
	}
	
	simd::float_4 output;
	lpfOut.setChannels(channels);
	hpfOut.setChannels(channels);
	bpfOut.setChannels(channels);
	bsfOut.setChannels(channels);

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
		if (bpfOut.isConnected()) {
			BPFaudioFilter[c/4].setParameters(BPFafp);
			output = BPFaudioFilter[c/4].processAudioSample(v[c/4]);
			output.store(bpfOut.getVoltages(c));
		}
		if (bsfOut.isConnected()) {
			BSFaudioFilter[c/4].setParameters(BSFafp);
			output = BSFaudioFilter[c/4].processAudioSample(v[c/4]);
			output.store(bsfOut.getVoltages(c));
		}
	}
}

void Lavender::process(const ProcessArgs &args) {
	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected() || outputs[OUTPUT_BPFMAIN].isConnected() || outputs[OUTPUT_BSFMAIN].isConnected()) {

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = abs(inputs[INPUT_CVFC].getVoltage() / 10.0);
	
		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = abs(inputs[INPUT_CVQ].getVoltage() / 10.0);
 	
 		LPFafp.fc = HPFafp.fc = BPFafp.fc = BSFafp.fc = pow(2048,params[PARAM_FC].getValue()) * 10 * cvfc;
		LPFafp.Q = HPFafp.Q = BPFafp.Q = BSFafp.Q = params[PARAM_Q].getValue() * cvq;
		LPFafp.dry = HPFafp.dry = BPFafp.dry = BSFafp.dry = params[PARAM_DRY].getValue();
		LPFafp.wet = HPFafp.wet = BPFafp.wet = BSFafp.wet = params[PARAM_WET].getValue();
		LPFafp.bqa = HPFafp.bqa = BPFafp.bqa = BSFafp.bqa = bqa;

		processChannel(inputs[INPUT_MAIN],outputs[OUTPUT_LPFMAIN],outputs[OUTPUT_HPFMAIN],outputs[OUTPUT_BPFMAIN],outputs[OUTPUT_BSFMAIN]);
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