#include "RPJ.hpp"
#include "Gaza.hpp"
#include "ctrl/RPJKnobs.hpp"

Gaza::Gaza() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 10000.f, 1000.f, "fc"," Hz");
    configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
    configParam(PARAM_ATK, 1.f, 250.0f,20.f, "Attack"," mSec");
    configParam(PARAM_REL, 1.f, 2000.f,500.f, "Release"," mSec");
    configParam(PARAM_THRES, -20.f, 0.f,-6.f, "Threshold");
    configParam(PARAM_SENS, 0.5f, 5.f,1.f, "Sensitivity");
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	envelopeFollower.reset(APP->engine->getSampleRate());
}

void Gaza::onSampleRateChange() {
	envelopeFollower.reset(APP->engine->getSampleRate());
}

void Gaza::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {

		efp.attackTime_mSec = params[PARAM_ATK].getValue();
		efp.releaseTime_mSec = params[PARAM_REL].getValue();
		efp.fc = params[PARAM_FC].getValue();
        efp.Q = params[PARAM_Q].getValue();
        efp.sensitivity = params[PARAM_SENS].getValue();
        efp.threshold_dB = params[PARAM_THRES].getValue();
		envelopeFollower.setParameters(efp);
		float out = envelopeFollower.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		
        outputs[OUTPUT_MAIN].setVoltage(out);
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