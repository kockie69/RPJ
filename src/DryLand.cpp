#include "RPJ.hpp"
#include "DryLand.hpp"


DryLand::DryLand() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");

	LPFaudioFilter.reset(14100);
	HPFaudioFilter.reset(14100);
	LPFafp.algorithm=filterAlgorithm::kLPF1;
	HPFafp.algorithm=filterAlgorithm::kHPF1;
}

void DryLand::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected()) {
		LPFaudioFilter.setSampleRate(args.sampleRate);
		HPFaudioFilter.setSampleRate(args.sampleRate);

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = abs(inputs[INPUT_CVFC].getVoltage() / 10.0f);
 	
 		LPFafp.fc = HPFafp.fc = params[PARAM_FC].getValue() * cvfc;
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

struct DryLandModuleWidget : ModuleWidget {
	DryLandModuleWidget(DryLand* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DryLand.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			RPJTitle * title = new RPJTitle(box.size.x,MODULE_WIDTH);
			title->setText("DRY LAND");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 19),10,MODULE_WIDTH);
			tl->setText("1st order LPF/HPF");
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

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, DryLand::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, DryLand::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, DryLand::OUTPUT_HPFMAIN));

		addParam(createParam<RoundBlackKnob>(Vec(8, 60), module, DryLand::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 62), module, DryLand::INPUT_CVFC));

		addParam(createParam<RoundBlackKnob>(Vec(8, 185), module, DryLand::PARAM_WET));
		addParam(createParam<RoundBlackKnob>(Vec(55, 185), module, DryLand::PARAM_DRY));
	}

};

Model * modelDryLand = createModel<DryLand, DryLandModuleWidget>("DryLand");