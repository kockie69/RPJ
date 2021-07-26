#include "RPJ.hpp"
#include "Brave.hpp"

Brave::Brave() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_BOOSTCUT_DB, -20.f, 20.f, 0.f, "dB","Boost/Cut");
	configParam(PARAM_CVB, 0.f, 1.0f, 0.0f, "CV Q");
	afp.algorithm = filterAlgorithm::kNCQParaEQ;
}

void Brave::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_MAIN].isConnected()) {
		audioFilter.setSampleRate(args.sampleRate);
	
		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = inputs[INPUT_CVFC].getVoltage();

		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = inputs[INPUT_CVQ].getVoltage();
	
		float cvb = 1.f;
		if (inputs[INPUT_CVB].isConnected())
			cvb = inputs[INPUT_CVB].getVoltage();
 	
		afp.fc = params[PARAM_FC].getValue() * rescale(cvfc,-10,10,0,1);
		afp.Q = params[PARAM_Q].getValue() * rescale(cvq,-10,10,0,1);
		afp.boostCut_dB = params[PARAM_BOOSTCUT_DB].getValue() * rescale(cvb,-10,10,0,1);

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

struct BraveModuleWidget : ModuleWidget {
	BraveModuleWidget(Brave* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Brave.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			RPJTitle * title = new RPJTitle(box.size.x,MODULE_WIDTH);
			title->setText("BRAVE");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 19),10,MODULE_WIDTH);
			tl->setText("2nd Order Filter");
			addChild(tl);
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

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, Brave::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Brave::OUTPUT_MAIN));
		
		addParam(createParam<RoundBlackKnob>(Vec(8, 80), module, Brave::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(55, 82), module, Brave::INPUT_CVFC));
		addParam(createParam<RoundBlackKnob>(Vec(8, 140), module, Brave::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(55, 142), module, Brave::INPUT_CVQ));
		addParam(createParam<RoundBlackKnob>(Vec(8, 200), module, Brave::PARAM_BOOSTCUT_DB));
		addInput(createInput<PJ301MPort>(Vec(55, 202), module, Brave::INPUT_CVB));	
	}

};

Model * modelBrave = createModel<Brave, BraveModuleWidget>("Brave");