#include "RPJ.hpp"
#include "ctrl/knob/RPJKnob.h"
#include "BlindCurve.hpp"

BlindCurve::BlindCurve() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	configInput(INPUT_MAIN, "Audio");
	configOutput(OUTPUT_MAIN, "Audio");
    configParam(PARAM_ATK, 1.f, 250.0f,20.f, "Attack"," mSec");
    configParam(PARAM_REL, 1.f, 2000.f,500.f, "Release"," mSec");
	configParam<DetectModeQuantity>(PARAM_MODE, 0.f, 2.f, 0.f, "Detect Mode");

	audioDetector.reset(14400);
}

void BlindCurve::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {

		adp.attackTime_mSec = params[PARAM_ATK].getValue();
		adp.releaseTime_mSec = params[PARAM_REL].getValue();
		adp.detectMode = static_cast<int>(params[PARAM_MODE].getValue());
		adp.clampToUnityMax = true;
		adp.detect_dB = false;
		audioDetector.setParameters(adp);
		float out = audioDetector.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		outputs[OUTPUT_MAIN].setVoltage(out * 5);
	}
}

struct BlindCurveModuleWidget : ModuleWidget {
	BlindCurveModuleWidget(BlindCurve* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BlindCurve.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	
		addInput(createInput<PJ301MPort>(Vec(33, 255), module, BlindCurve::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(33, 310), module, BlindCurve::OUTPUT_MAIN));
		
		addParam(createParam<RPJKnob>(Vec(31, 52), module, BlindCurve::PARAM_ATK));
       	addParam(createParam<RPJKnob>(Vec(31, 115), module, BlindCurve::PARAM_REL)); 
		{
			auto w = createParam<RPJKnobSnap>(Vec(32,185), module, BlindCurve::PARAM_MODE);
			auto k = dynamic_cast<SvgKnob*>(w);
			k->minAngle = -0.75*M_PI;
			k->maxAngle = 0.75*M_PI;
			addParam(w);
		}
	}

};

std::string DetectModeQuantity::getDisplayValueString() {
	if ( module == NULL) 
		v = "";
	else switch (static_cast<int>(module->params[BlindCurve::PARAM_MODE].getValue())) {
		case 0: 
			v = "Peak";
			break;
		case 1: 
			v = "Mean-Square";
			break;
		case 2: 
			v = "Root-Mean-Square";
			break;
		default:
			break;
	}
	return v;
}

Model * modelBlindCurve = createModel<BlindCurve, BlindCurveModuleWidget>("BlindCurve");