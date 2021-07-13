#include "RPJ.hpp"
#include "LadyNina.hpp"


LadyNina::LadyNina() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "dB","Boost/Cut");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
}

void LadyNina::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected()) {
		audioFilter.setSampleRate(args.sampleRate);

 		afp.fc = params[PARAM_FC].getValue();
		afp.Q = params[PARAM_Q].getValue();
		afp.boostCut_dB = params[PARAM_BOOSTCUT_DB].getValue();
		afp.dry = params[PARAM_DRY].getValue();
		afp.wet = params[PARAM_WET].getValue();

		if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
			if ((static_cast<int>(afp.algorithm)+1) == static_cast<int>(filterAlgorithm::numFilterAlgorithms))
				afp.algorithm = static_cast<filterAlgorithm>(static_cast<int>(afp.algorithm));
			else
				afp.algorithm = static_cast<filterAlgorithm>(static_cast<int>(afp.algorithm)+1);
		}
		if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
			if ((static_cast<int>(afp.algorithm)-1) < 0)
				afp.algorithm = static_cast<filterAlgorithm>(static_cast<int>(afp.algorithm));
			else 
				afp.algorithm = static_cast<filterAlgorithm>(static_cast<int>(afp.algorithm)-1);
		}

		afp.strAlgorithm = audioFilter.filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
		audioFilter.setParameters(afp);

		float out = audioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());
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

struct LadyNinaModuleWidget : ModuleWidget {
	LadyNinaModuleWidget(LadyNina* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/myVCF.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x -15, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("LADY NINA");
			addChild(title);
		}
		{
			FilterNameDisplay * fnd = new FilterNameDisplay(Vec(39,30));
			fnd->module = module;
			addChild(fnd);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 50));
			tl->setText("CUTOFF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(22, 90));
			tl->setText("RESONANCE");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 140));
			tl->setText("BOOST/CUT");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(58, 170));
			tl->setText("DRY");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(5, 212));
			tl->setText("WET");
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

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, LadyNina::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, LadyNina::OUTPUT_MAIN));

		addParam(createParam<buttonMinSmall>(Vec(5,45),module, LadyNina::PARAM_DOWN));
		addParam(createParam<buttonPlusSmall>(Vec(76,45),module, LadyNina::PARAM_UP));
		addParam(createParam<RoundBlackKnob>(Vec(8, 80), module, LadyNina::PARAM_FC));
		addParam(createParam<RoundBlackKnob>(Vec(55, 120), module, LadyNina::PARAM_Q));
		addParam(createParam<RoundBlackKnob>(Vec(8, 172), module, LadyNina::PARAM_BOOSTCUT_DB));	
		addParam(createParam<RoundBlackKnob>(Vec(8, 245), module, LadyNina::PARAM_WET));
		addParam(createParam<RoundBlackKnob>(Vec(55, 205), module, LadyNina::PARAM_DRY));
	}

};

Model * modelLadyNina = createModel<LadyNina, LadyNinaModuleWidget>("LadyNina");