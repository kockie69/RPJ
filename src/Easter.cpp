#include "RPJ.hpp"
#include "Easter.hpp"

Easter::Easter() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_FC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_CVFC, 0.f, 1.0f, 0.0f, "CV FC");
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, 0.f, 1.0f, 0.0f, "CV Q");
	configParam(PARAM_DRY, 0.f, 1.0f, 0.0f, "DRY");
	configParam(PARAM_WET, 0.f, 1.0f, 1.0f, "WET");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
	afp.algorithm = filterAlgorithm::kResonA;
}

void Easter::process(const ProcessArgs &args) {

	audioFilter.setSampleRate(args.sampleRate);
	
	float cvfc = 1.f;
	if (inputs[INPUT_CVFC].isConnected())
		cvfc = inputs[INPUT_CVFC].getVoltage();
	
	float cvq = 1.f;
	if (inputs[INPUT_CVQ].isConnected())
		cvq = inputs[INPUT_CVQ].getVoltage();
 	
	afp.fc = params[PARAM_FC].getValue() * rescale(cvfc,-10,10,0,1);
	afp.Q = params[PARAM_Q].getValue() * rescale(cvq,-10,10,0,1);
	afp.dry = params[PARAM_DRY].getValue();
	afp.wet = params[PARAM_WET].getValue();
	
	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		afp.algorithm = filterAlgorithm::kResonB;	
	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
	afp.algorithm = filterAlgorithm::kResonA;
	}

	afp.strAlgorithm = audioFilter.filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
	audioFilter.setParameters(afp);

	float out = audioFilter.processAudioSample(inputs[INPUT_MAIN].getVoltage());

	outputs[OUTPUT_MAIN].setVoltage(out);
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

struct EasterModuleWidget : ModuleWidget {
	EasterModuleWidget(Easter* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/myVCF.svg")));

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("EASTER");
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

		addInput(createInput<PJ301MPort>(Vec(10, 300), module, Easter::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Easter::OUTPUT_MAIN));
		
		addParam(createParam<buttonMinSmall>(Vec(5,45),module, Easter::PARAM_DOWN));
		addParam(createParam<buttonPlusSmall>(Vec(76,45),module, Easter::PARAM_UP));
		addParam(createParam<RoundBlackKnob>(Vec(8, 80), module, Easter::PARAM_FC));
		addInput(createInput<PJ301MPort>(Vec(43, 70), module, Easter::INPUT_CVFC));
		addParam(createParam<RoundBlackKnob>(Vec(55, 120), module, Easter::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(25, 130), module, Easter::INPUT_CVQ));	
		addParam(createParam<RoundBlackKnob>(Vec(8, 245), module, Easter::PARAM_WET));
		addParam(createParam<RoundBlackKnob>(Vec(55, 205), module, Easter::PARAM_DRY));
	}

};

Model * modelEaster = createModel<Easter, EasterModuleWidget>("Easter");