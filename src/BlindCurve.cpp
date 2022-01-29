#include "RPJ.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "BlindCurve.hpp"

BlindCurve::BlindCurve() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configBypass(INPUT_MAIN, OUTPUT_MAIN);
	configInput(INPUT_MAIN, "Audio");
	configOutput(OUTPUT_MAIN, "Envelope");
    configParam(PARAM_ATK, 1.f, 250.0f,20.f, "Attack"," mSec");
    configParam(PARAM_REL, 1.f, 2000.f,500.f, "Release"," mSec");
	configParam<DetectModeQuantity>(PARAM_MODE, 0.f, 2.f, 0.f, "Detect Mode");
	clamping = true;
	outputType = LINEAR; 
	for (int i=0;i<4;i++) {
		audioDetector[i].reset(APP->engine->getSampleRate());
	}
}

void BlindCurve::onSampleRateChange() {
	for (int i=0;i<4;i++) {
		audioDetector[i].reset(APP->engine->getSampleRate());
	}
}

void BlindCurve::processChannel(int c, Input& in, Output& out) {
		
	simd::float_4 v = in.getPolyVoltageSimd<simd::float_4>(c);
	audioDetector[c/4].setParameters(adp);
	out.setVoltageSimd(audioDetector[c/4].processAudioSample(v),c);
}

void BlindCurve::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
			
		int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);
		outputs[OUTPUT_MAIN].setChannels(channels);
		
		for (int c = 0; c < channels; c += 4) {
			adp.attackTime_mSec = params[PARAM_ATK].getValue();
			adp.releaseTime_mSec = params[PARAM_REL].getValue();
			adp.detectMode = static_cast<int>(params[PARAM_MODE].getValue());
			adp.clampToUnityMax = clamping;
			adp.detect_dB = outputType;
		
			processChannel(c, inputs[INPUT_MAIN], outputs[OUTPUT_MAIN]);
		}
	}
}

struct BlindCurveModuleWidget : ModuleWidget {
	BlindCurveModuleWidget(BlindCurve* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BlindCurve.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	
		const float knobX1 = 21.5;
		const float knobX2 = 31.5;

		const float knobY1 = 47;
		const float knobY2 = 122;
		const float knobY3 = 210;

		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY1), module, BlindCurve::PARAM_ATK));
		addParam(createParam<RPJKnobBig>(Vec(knobX1, knobY2), module, BlindCurve::PARAM_REL));

		{
			auto w = createParam<RPJKnobSnap>(Vec(knobX2,knobY3), module, BlindCurve::PARAM_MODE);
			auto k = dynamic_cast<SvgKnob*>(w);
			k->minAngle = -0.75*M_PI;
			k->maxAngle = 0.75*M_PI;
			addParam(w);
		}

		// Next do the Jacks
		const float jackX1 = 33.5f;

		const float jackY1 = 278;
		const float jackY2 = 325;

		addInput(createInput<PJ301MPort>(Vec(jackX1, jackY1), module, BlindCurve::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(jackX1, jackY2), module, BlindCurve::OUTPUT_MAIN));
	}

	void appendContextMenu(Menu *menu) override {
		BlindCurve * module = dynamic_cast<BlindCurve*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createBoolPtrMenuItem("Clamping", "", &module->clamping));
		menu->addChild(createIndexPtrSubmenuItem("Output type", {"Linear","Logarithmic"}, &module->outputType));

	}
};

std::string DetectModeQuantity::getDisplayValueString() {
	if ( module == NULL) 
		return "";
	else switch (static_cast<int>(module->params[BlindCurve::PARAM_MODE].getValue())) {
		case 0: 
			return "Peak";
			break;
		case 1: 
			return "Mean-Square";
			break;
		case 2: 
			return "Root-Mean-Square";
			break;
		default:
			return "Peak";
	}
	return "Peak";
}

Model * modelBlindCurve = createModel<BlindCurve, BlindCurveModuleWidget>("BlindCurve");