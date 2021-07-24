#include "RPJ.hpp"
#include "BlindCurve.hpp"

BlindCurve::BlindCurve() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

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

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("BLINDCURVE");
			addChild(title);
		}
				{
			RPJTextLabel * tl = new RPJTextLabel(Vec(-10, 17),10);
			tl->setText("Audio Detector");
			addChild(tl);
		}	
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 30));
			tl->setText("ATTACK");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(35, 80));
			tl->setText("RELEASE");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 170));
			tl->setText("DETECT MODE");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(8, 220),10);
			tl->setText("PEAK");
			addChild(tl);
		}	
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(39, 189),10);
			tl->setText("MS");
			addChild(tl);
		}	
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 220),10);
			tl->setText("RMS");
			addChild(tl);
		}				
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(13, 260));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 290));
			tl->setText("OUT");
			addChild(tl);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 290), module, BlindCurve::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, BlindCurve::OUTPUT_MAIN));
		
		addParam(createParam<RoundBlackKnob>(Vec(8, 60), module, BlindCurve::PARAM_ATK));
       	addParam(createParam<RoundBlackKnob>(Vec(55, 110), module, BlindCurve::PARAM_REL)); 
		{
			auto w = createParam<RoundBlackKnob>(Vec(31,210), module, BlindCurve::PARAM_MODE);
			auto k = dynamic_cast<SvgKnob*>(w);
			k->snap = true;
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