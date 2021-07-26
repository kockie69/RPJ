#include "RPJ.hpp"
#include "Montreal.hpp"


Montreal::Montreal() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	sampleRate=0;
	wdfp.fc=0;
}

void Montreal::process(const ProcessArgs &args) {

	if ((outputs[OUTPUT_LPF].isConnected() || 
		outputs[OUTPUT_HPF].isConnected() || outputs[OUTPUT_BPF].isConnected() || 
		outputs[OUTPUT_BSF].isConnected()) && inputs[INPUT_MAIN].isConnected()) {


		if (args.sampleRate!=sampleRate) {
				sampleRate = args.sampleRate;
				wdfIdealRLCLPF.reset(sampleRate);
				wdfIdealRLCHPF.reset(sampleRate);
				wdfIdealRLCBPF.reset(sampleRate);
				wdfIdealRLCBSF.reset(sampleRate);
		}
		
		if (params[PARAM_FC].getValue() != wdfp.fc || params[PARAM_Q].getValue() !=  wdfp.Q)  {
			
			wdfp.fc=params[PARAM_FC].getValue();
			wdfp.Q=params[PARAM_Q].getValue();
			wdfIdealRLCLPF.setParameters(wdfp);
			wdfIdealRLCHPF.setParameters(wdfp);
			wdfIdealRLCBPF.setParameters(wdfp);
			wdfIdealRLCBSF.setParameters(wdfp);
		}

		if (outputs[OUTPUT_LPF].isConnected())
			outputs[OUTPUT_LPF].setVoltage(wdfIdealRLCLPF.processAudioSample(inputs[INPUT_MAIN].getVoltage()));
		if (outputs[OUTPUT_HPF].isConnected())
			outputs[OUTPUT_HPF].setVoltage(wdfIdealRLCHPF.processAudioSample(inputs[INPUT_MAIN].getVoltage()));
		if (outputs[OUTPUT_BPF].isConnected())
			outputs[OUTPUT_BPF].setVoltage(wdfIdealRLCBPF.processAudioSample(inputs[INPUT_MAIN].getVoltage()));
		if (outputs[OUTPUT_BSF].isConnected())
			outputs[OUTPUT_BSF].setVoltage(wdfIdealRLCBSF.processAudioSample(inputs[INPUT_MAIN].getVoltage()));
	}
}

struct MontrealModuleWidget : ModuleWidget {
	MontrealModuleWidget(Montreal* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Montreal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			RPJTitle * title = new RPJTitle(box.size.x,MODULE_WIDTH);
			title->setText("MONTREAL");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 30));
			tl->setText("CUTOFF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 110));
			tl->setText("RESONANCE");
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

		addInput(createInput<PJ301MPort>(Vec(10, 240), module, Montreal::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 240), module, Montreal::OUTPUT_LPF));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, Montreal::OUTPUT_HPF));
		addOutput(createOutput<PJ301MPort>(Vec(10, 320), module, Montreal::OUTPUT_BPF));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Montreal::OUTPUT_BSF));

		addParam(createParam<RoundBlackKnob>(Vec(8, 60), module, Montreal::PARAM_FC));
		addParam(createParam<RoundBlackKnob>(Vec(8, 140), module, Montreal::PARAM_Q));
	}

};

Model * modelMontreal = createModel<Montreal, MontrealModuleWidget>("Montreal");