#include "RPJ.hpp"
#include "BlindCurve.hpp"

BlindCurve::BlindCurve() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(PARAM_ATK, 1.f, 250.0f,20.f, "Attack");
    configParam(PARAM_REL, 1.f, 2000.f,500.f, "Release");
	configParam<DetectModeQuantity>(PARAM_MODE, 0.f, 2.f, 0.f, "Detect Mode");
	configParam(PARAM_CLAMP, 0.f, 1.f,0.f, "Clamp");
	configParam(PARAM_DB, 0.f, 1.f,0.f, "Db");

	audioDetector.reset(14400);
}

void BlindCurve::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {

		adp.attackTime_mSec = params[PARAM_ATK].getValue();
		adp.releaseTime_mSec = params[PARAM_REL].getValue();
		adp.detectMode = static_cast<int>(params[PARAM_MODE].getValue());
		adp.clampToUnityMax = static_cast<int>(params[PARAM_CLAMP].getValue());
		adp.detect_dB = static_cast<int>(params[PARAM_DB].getValue());
		audioDetector.setParameters(adp);
		//float out = envelopeFollower.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		float out = audioDetector.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		outputs[OUTPUT_MAIN].setVoltage(out);
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
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 150));
			tl->setText("DETECT MODE");
			addChild(tl);
		}
		{
			RPJSmallTextLabel * tl = new RPJSmallTextLabel(Vec(8, 200));
			tl->setText("PEAK");
			addChild(tl);
		}	
		{
			RPJSmallTextLabel * tl = new RPJSmallTextLabel(Vec(39, 169));
			tl->setText("MS");
			addChild(tl);
		}	
		{
			RPJSmallTextLabel * tl = new RPJSmallTextLabel(Vec(55, 200));
			tl->setText("RMS");
			addChild(tl);
		}				
		{
			RPJSmallTextLabel * tl = new RPJSmallTextLabel(Vec(11, 230));
			tl->setText("YES");
			addChild(tl);
		}
				{
			RPJSmallTextLabel * tl = new RPJSmallTextLabel(Vec(15, 270));
			tl->setText("NO");
			addChild(tl);
		}
		{
			RPJSmallTextLabel * tl = new RPJSmallTextLabel(Vec(61, 230));
			tl->setText("LOG");
			addChild(tl);
		}
		{
			RPJSmallTextLabel * tl = new RPJSmallTextLabel(Vec(61, 270));
			tl->setText("LIN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(13, 290));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 290));
			tl->setText("OUT");
			addChild(tl);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 320), module, BlindCurve::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, BlindCurve::OUTPUT_MAIN));
		
		addParam(createParam<RoundBlackKnob>(Vec(8, 60), module, BlindCurve::PARAM_ATK));
       	addParam(createParam<RoundBlackKnob>(Vec(55, 110), module, BlindCurve::PARAM_REL)); 
		{
			auto w = createParam<RoundBlackKnob>(Vec(31,190), module, BlindCurve::PARAM_MODE);
			auto k = dynamic_cast<SvgKnob*>(w);
			k->snap = true;
			k->minAngle = -0.75*M_PI;
			k->maxAngle = 0.75*M_PI;
			addParam(w);
		}
		addParam(createParam<Toggle2P>(Vec(5, 250), module, BlindCurve::PARAM_CLAMP)); 
		addParam(createParam<Toggle2P>(Vec(55, 250), module, BlindCurve::PARAM_DB)); 
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

Toggle2P::Toggle2P() {
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SW_Toggle_0.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SW_Toggle_2.svg")));

	// no shadow for switches
	shadow->opacity = 0.0f;

	neg = pos = 0;
}
	
// handle the manually entered values
void Toggle2P::onChange(const event::Change &e) {
		
	SvgSwitch::onChange(e);
		
	if (paramQuantity->getValue() > 0.5f)
		paramQuantity->setValue(1.0f);
	else
		paramQuantity->setValue(0.0f);
}

		
// override the base randomizer as it sets switches to invalid values.
void Toggle2P::randomize() {
	SvgSwitch::randomize();

	if (paramQuantity->getValue() > 0.5f)
		paramQuantity->setValue(1.0f);
	else
		paramQuantity->setValue(0.0f);
}	

Model * modelBlindCurve = createModel<BlindCurve, BlindCurveModuleWidget>("BlindCurve");