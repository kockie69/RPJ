#include "RPJ.hpp"
#include "Gaza.hpp"

Gaza::Gaza() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 10000.f, 1000.f, "fc"," Hz");
    configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
    configParam(PARAM_ATK, 1.f, 250.0f,20.f, "Attack"," mSec");
    configParam(PARAM_REL, 1.f, 2000.f,500.f, "Release"," mSec");
    configParam(PARAM_THRES, -20.f, 0.f,-6.f, "Threshold");
    configParam(PARAM_SENS, 0.5f, 5.f,1.f, "Sensitivity");
	envelopeFollower.reset(14400);
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

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("GAZA");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(-22, 19),10);
			tl->setText("Envelope Follower");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 30));
			tl->setText("CUTOFF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(25, 75));
			tl->setText("RESONANCE");
			addChild(tl);
		}	
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 115));
			tl->setText("ATTACK");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(35, 155));
			tl->setText("RELEASE");
			addChild(tl);
		}
        		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 190));
			tl->setText("THRESH");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 235));
			tl->setText("SENS");
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

		addInput(createInput<PJ301MPort>(Vec(10, 290), module, Gaza::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Gaza::OUTPUT_MAIN));
		addParam(createParam<RoundBlackKnob>(Vec(8, 60), module, Gaza::PARAM_FC));
       	addParam(createParam<RoundBlackKnob>(Vec(55, 110), module, Gaza::PARAM_Q)); 
		addParam(createParam<RoundBlackKnob>(Vec(8, 145), module, Gaza::PARAM_ATK));
       	addParam(createParam<RoundBlackKnob>(Vec(55, 187), module, Gaza::PARAM_REL)); 
        addParam(createParam<RoundBlackKnob>(Vec(8, 220), module, Gaza::PARAM_THRES));
       	addParam(createParam<RoundBlackKnob>(Vec(55, 265), module, Gaza::PARAM_SENS)); 
	}

};

Model * modelGaza = createModel<Gaza, GazaModuleWidget>("Gaza");