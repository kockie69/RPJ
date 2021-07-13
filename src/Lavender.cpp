#include "RPJ.hpp"
#include "Lavender.hpp"


Lavender::Lavender() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	LPFafp.algorithm=filterAlgorithm::kLPF2;
	HPFafp.algorithm=filterAlgorithm::kHPF2;
	BPFafp.algorithm=filterAlgorithm::kBPF2;
	BSFafp.algorithm=filterAlgorithm::kBSF2;
}

void Lavender::process(const ProcessArgs &args) {
	if (outputs[OUTPUT_LPFMAIN].isConnected() || outputs[OUTPUT_HPFMAIN].isConnected() || outputs[OUTPUT_BPFMAIN].isConnected() || outputs[OUTPUT_BSFMAIN].isConnected()) {

		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = inputs[INPUT_CVFC].getVoltage();
	
		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = inputs[INPUT_CVQ].getVoltage();
 	
 		LPFafp.fc = HPFafp.fc = BPFafp.fc = BSFafp.fc = params[PARAM_FC].getValue() * rescale(cvfc,-10,10,0,1);
		LPFafp.Q = HPFafp.Q = BPFafp.Q = BSFafp.Q = params[PARAM_Q].getValue() * rescale(cvq,-10,10,0,1);
		LPFafp.dry = HPFafp.dry = BPFafp.dry = BSFafp.dry = params[PARAM_DRY].getValue();
		LPFafp.wet = HPFafp.wet = BPFafp.wet = BSFafp.wet = params[PARAM_WET].getValue();

		if (outputs[OUTPUT_LPFMAIN].isConnected()) {
			LPFaudioFilter.setSampleRate(args.sampleRate);
			LPFaudioFilter.setParameters(LPFafp);
			float LPFOut = LPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());	
			outputs[OUTPUT_LPFMAIN].setVoltage(LPFOut);
		}
		if (outputs[OUTPUT_HPFMAIN].isConnected()) {
			HPFaudioFilter.setSampleRate(args.sampleRate);
			HPFaudioFilter.setParameters(HPFafp);
			float HPFOut = HPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
			outputs[OUTPUT_HPFMAIN].setVoltage(HPFOut);
		}
		if (outputs[OUTPUT_BPFMAIN].isConnected()) {
			BPFaudioFilter.setSampleRate(args.sampleRate);
			BPFaudioFilter.setParameters(BPFafp);
			float BPFOut = BPFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
			outputs[OUTPUT_BPFMAIN].setVoltage(BPFOut);
		}
		if (outputs[OUTPUT_BSFMAIN].isConnected()) {
			BSFaudioFilter.setSampleRate(args.sampleRate);
			BSFaudioFilter.setParameters(BSFafp);
			float BSFOut = BSFaudioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
			outputs[OUTPUT_BSFMAIN].setVoltage(BSFOut);
		}
	}
}

struct LavenderModuleWidget : ModuleWidget {
	LavenderModuleWidget(Lavender* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Lavender.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("LAVENDER");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 30));
			tl->setText("CUTOFF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 85));
			tl->setText("RESONANCE");
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
			RPJTextLabel * tl = new RPJTextLabel(Vec(13, 210));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 210));
			tl->setText("LPF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 250));
			tl->setText("HPF");
			addChild(tl);
		}
				{
			RPJTextLabel * tl = new RPJTextLabel(Vec(5, 290));
			tl->setText("BPF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 290));
			tl->setText("BSF");
			addChild(tl);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 240), module, Lavender::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 240), module, Lavender::OUTPUT_LPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, Lavender::OUTPUT_HPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(10, 320), module, Lavender::OUTPUT_BPFMAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Lavender::OUTPUT_BSFMAIN));


		addParam(createParam<RoundBlackKnob>(Vec(8, 60), module, Lavender::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 62), module, Lavender::INPUT_CVFC));
		addParam(createParam<RoundBlackKnob>(Vec(8, 115), module, Lavender::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(55, 117), module, Lavender::INPUT_CVQ));
		addParam(createParam<RoundBlackKnob>(Vec(8, 185), module, Lavender::PARAM_WET));
		addParam(createParam<RoundBlackKnob>(Vec(55, 185), module, Lavender::PARAM_DRY));
	}

};

Model * modelLavender = createModel<Lavender, LavenderModuleWidget>("Lavender");