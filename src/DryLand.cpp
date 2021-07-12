#include "RPJ.hpp"
#include "DryLand.hpp"


DryLand::DryLand() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	LPFafp.algorithm=filterAlgorithm::kLPF1;
	HPFafp.algorithm=filterAlgorithm::kHPF1;
}

void DryLand::process(const ProcessArgs &args) {

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
	
	//LPFafp.algorithm = filterAlgorithm::kResonA;

	//afp.strAlgorithm = filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
	LPFaudioFilter.setParameters(LPFafp);
	HPFaudioFilter.setParameters(HPFafp);

	float LPFOut = LPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
	float HPFOut = HPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());

	outputs[OUTPUT_LPFMAIN].setVoltage(LPFOut);
	outputs[OUTPUT_HPFMAIN].setVoltage(HPFOut);
}

struct DryLandModuleWidget : ModuleWidget {
	DryLandModuleWidget(DryLand* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DryLand.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("DRYLAND");
			addChild(title);
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