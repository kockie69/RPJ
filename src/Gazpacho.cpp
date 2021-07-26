#include "RPJ.hpp"
#include "Gazpacho.hpp"


Gazpacho::Gazpacho() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	LPFafp.algorithm=filterAlgorithm::kLWRLPF2;
	HPFafp.algorithm=filterAlgorithm::kLWRHPF2;
}

void Gazpacho::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected()) {
		LPFaudioFilter.setSampleRate(args.sampleRate);
		HPFaudioFilter.setSampleRate(args.sampleRate);

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = inputs[INPUT_CVFC].getVoltage();
	
		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = inputs[INPUT_CVQ].getVoltage();
 	
 		LPFafp.fc = HPFafp.fc = params[PARAM_FC].getValue() * rescale(cvfc,-10,10,0,1);
		LPFafp.Q = HPFafp.Q = params[PARAM_Q].getValue() * rescale(cvq,-10,10,0,1);
		LPFafp.dry = HPFafp.dry = params[PARAM_DRY].getValue();
		LPFafp.wet = HPFafp.wet = params[PARAM_WET].getValue();

		if (outputs[OUTPUT_LPFMAIN].isConnected()) {
			LPFaudioFilter.setParameters(LPFafp);
			float LPFOut = LPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
			outputs[OUTPUT_LPFMAIN].setVoltage(LPFOut);
		}
		if (outputs[OUTPUT_HPFMAIN].isConnected()) {
			HPFaudioFilter.setParameters(HPFafp);
			float HPFOut = HPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
			outputs[OUTPUT_HPFMAIN].setVoltage(HPFOut);
		}
	}
}

struct GazpachoModuleWidget : ModuleWidget {
	GazpachoModuleWidget(Gazpacho* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Gazpacho.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			RPJTitle * title = new RPJTitle(box.size.x,MODULE_WIDTH);
			title->setText("GAZPACHO");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 20),10,MODULE_WIDTH);
			tl->setText("Linkwitz-Riley");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 30));
			tl->setText("CUTOFF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(58, 155));
			tl->setText("DRY");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(5, 155));
			tl->setText("WET");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(13, 270));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 250));
			tl->setText("LPF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 290));
			tl->setText("HPF");
			addChild(tl);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, Gazpacho::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, Gazpacho::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Gazpacho::OUTPUT_HPFMAIN));

		addParam(createParam<RoundBlackKnob>(Vec(8, 60), module, Gazpacho::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 62), module, Gazpacho::INPUT_CVFC));

		addParam(createParam<RoundBlackKnob>(Vec(8, 185), module, Gazpacho::PARAM_WET));
		addParam(createParam<RoundBlackKnob>(Vec(55, 185), module, Gazpacho::PARAM_DRY));
	}

};

Model * modelGazpacho = createModel<Gazpacho, GazpachoModuleWidget>("Gazpacho");