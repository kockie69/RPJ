#include "RPJ.hpp"
#include "SugarMice.hpp"


SugarMice::SugarMice() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	sampleRate=0;
	fc=0;
}

void SugarMice::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
		if (params[PARAM_FC].getValue() != fc) {
			wdfButterLPF3.setFilterFc(params[PARAM_FC].getValue());
//			wdfButterLPF3.reset(args.sampleRate);
			fc = params[PARAM_FC].getValue();
		}
		
		if (args.sampleRate!=sampleRate) {
			sampleRate = args.sampleRate;
			wdfButterLPF3.reset(sampleRate);
		}

		float out = wdfButterLPF3.processAudioSample(inputs[INPUT_MAIN].getVoltage());
		outputs[OUTPUT_MAIN].setVoltage(out);
	}
}

struct SugarMiceModuleWidget : ModuleWidget {
	SugarMiceModuleWidget(SugarMice* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SugarMice.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("SUGARMICE");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 30));
			tl->setText("CUTOFF");
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

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, SugarMice::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, SugarMice::OUTPUT_MAIN));
		addParam(createParam<RoundBlackKnob>(Vec(8, 60), module, SugarMice::PARAM_FC));
	}

};

Model * modelSugarMice = createModel<SugarMice, SugarMiceModuleWidget>("SugarMice");