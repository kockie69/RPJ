#include "RPJ.hpp"
#include "Gaza.hpp"
#include "ctrl/RPJKnobs.hpp"

Gaza::Gaza() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
    configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
    configParam(PARAM_ATK, 1.f, 250.0f,20.f, "Attack"," mSec");
    configParam(PARAM_REL, 1.f, 2000.f,500.f, "Release"," mSec");
    configParam(PARAM_THRES, -20.f, 0.f,-6.f, "Threshold");
    configParam(PARAM_SENS, 0.5f, 5.f,1.f, "Sensitivity");
	configInput(INPUT_MAIN,"Main");
	configOutput(OUTPUT_MAIN,"Main");
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	for (int i=0;i<4;i++) {
		envelopeFollower[i].reset(APP->engine->getSampleRate());
	}
}

void Gaza::onSampleRateChange() {
	for (int i=0;i<4;i++) {
		envelopeFollower[i].reset(APP->engine->getSampleRate());
	}
}

void Gaza::processChannel(Input& in, Output& out) {
		
	// Get input
	int channels = std::max(in.getChannels(), 1);
	simd::float_4 v[4];
	simd::float_4 output;
	out.setChannels(channels);

	for (int c = 0; c < channels; c += 4) {
		v[c/4] = simd::float_4::load(in.getVoltages(c));
		if (out.isConnected()) {
			envelopeFollower[c/4].setParameters(efp);
			output = envelopeFollower[c/4].processAudioSample(v[c/4]);
			output.store(out.getVoltages(c));
		}
	}
}

void Gaza::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {

		efp.attackTime_mSec = params[PARAM_ATK].getValue();
		efp.releaseTime_mSec = params[PARAM_REL].getValue();
		efp.fc = pow(2048,params[PARAM_FC].getValue()) * 10;
        efp.Q = params[PARAM_Q].getValue();
        efp.sensitivity = params[PARAM_SENS].getValue();
        efp.threshold_dB = params[PARAM_THRES].getValue();
		
		processChannel(inputs[INPUT_MAIN], outputs[OUTPUT_MAIN]);

	}
}

struct GazaModuleWidget : ModuleWidget {
	GazaModuleWidget(Gaza* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Gaza.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		addInput(createInput<PJ301MPort>(Vec(10, 320), module, Gaza::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Gaza::OUTPUT_MAIN));
		addParam(createParam<RPJKnob>(Vec(30, 55), module, Gaza::PARAM_FC));
       	addParam(createParam<RPJKnob>(Vec(30, 105), module, Gaza::PARAM_Q)); 
		addParam(createParam<RPJKnob>(Vec(8, 160), module, Gaza::PARAM_ATK));
       	addParam(createParam<RPJKnob>(Vec(55, 160), module, Gaza::PARAM_REL)); 
        addParam(createParam<RPJKnob>(Vec(30, 213), module, Gaza::PARAM_THRES));
       	addParam(createParam<RPJKnob>(Vec(30, 262), module, Gaza::PARAM_SENS)); 
	}

};

Model * modelGaza = createModel<Gaza, GazaModuleWidget>("Gaza");