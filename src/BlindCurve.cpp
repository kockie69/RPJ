#include "RPJ.hpp"
#include "BlindCurve.hpp"

BlindCurve::BlindCurve() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "dB","Boost/Cut");
	configParam(PARAM_CVB, 0.f, 1.0f, 0.0f, "CV Q");
}

void BlindCurve::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected()) {

		efp.fc = params[PARAM_FC].getValue();
		efp.Q = params[PARAM_Q].getValue();
		
        envelopeFollower.setParameters(efp);

		float out = envelopeFollower.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		outputs[OUTPUT_MAIN].setVoltage(out);
	}
}

struct buttonPlusSmall : SvgSwitch  {
	buttonPlusSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonPlus_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonPlus_1.svg")));
	}
};

struct buttonMinSmall : SvgSwitch  {
	buttonMinSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonMin_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonMin_1.svg")));
	}
};

struct BlindCurveModuleWidget : ModuleWidget {
	BlindCurveModuleWidget(BlindCurve* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BlindCurve.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("BlindCurve");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 50));
			tl->setText("CUTOFF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 110));
			tl->setText("RESONANCE");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 170));
			tl->setText("BOOST/CUT");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(13, 270));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 290));
			tl->setText("OUT");
			addChild(tl);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, BlindCurve::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, BlindCurve::OUTPUT_MAIN));
		
		addParam(createParam<RoundBlackKnob>(Vec(8, 80), module, BlindCurve::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 82), module, BlindCurve::INPUT_CVFC));
		addParam(createParam<RoundBlackKnob>(Vec(8, 140), module, BlindCurve::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(55, 142), module, BlindCurve::INPUT_CVQ));
		addParam(createParam<RoundBlackKnob>(Vec(8, 200), module, BlindCurve::PARAM_BOOSTCUT_DB));
		addInput(createInput<PJ301MPort>(Vec(55, 202), module, BlindCurve::INPUT_CVB));	
	}

};

Model * modelBlindCurve = createModel<BlindCurve, BlindCurveModuleWidget>("BlindCurve");