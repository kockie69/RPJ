#include "RPJ.hpp"
#include "Brave.hpp"
#include "ctrl/RPJKnobs.hpp"

Brave::Brave() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "dB","Boost/Cut");
	configParam(PARAM_CVB, 0.f, 1.0f, 0.0f, "CV Q");
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	afp.algorithm = filterAlgorithm::kNCQParaEQ;
	for (int i=0;i<4;i++) {
		audioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Brave::onSampleRateChange() {
	for (int i=0;i<4;i++) {
		audioFilter[i].reset(APP->engine->getSampleRate());
	}
}

void Brave::processChannel(Input& in, Output& out) {
		
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

void Brave::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected()) {
	
		float cvfc = inputs[INPUT_CVFC].isConnected() ? inputs[INPUT_CVFC].getVoltage() : 1.f;
		float cvq = inputs[INPUT_CVQ].isConnected() ? inputs[INPUT_CVQ].getVoltage() : 1.f;
		float cvb = inputs[INPUT_CVB].isConnected() ? inputs[INPUT_CVB].isConnected() : 1.f;
 	
		afp.fc = pow(2048,params[PARAM_FC].getValue()) * 10 * cvfc;
		afp.Q = params[PARAM_Q].getValue() * cvq;
		afp.boostCut_dB = params[PARAM_BOOSTCUT_DB].getValue() *cvb;

		processChannel(inputs[INPUT_MAIN],outputs[OUTPUT_MAIN]);
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