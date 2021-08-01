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
	audioFilter.reset(44100);
	afp.algorithm = filterAlgorithm::kResonA;
}

void Easter::process(const ProcessArgs &args) {

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		afp.algorithm = filterAlgorithm::kResonB;	
	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f)))
		afp.algorithm = filterAlgorithm::kResonA;
	afp.strAlgorithm = audioFilter.filterAlgorithmTxt[static_cast<int>(afp.algorithm)];
	audioFilter.setParameters(afp);

	if (outputs[OUTPUT_MAIN].isConnected() && inputs[INPUT_MAIN].isConnected()) {
		audioFilter.setSampleRate(args.sampleRate);
	
		float cvfc = 1.f;
		if (inputs[INPUT_CVFC].isConnected())
			cvfc = abs(inputs[INPUT_CVFC].getVoltage() /10.0);
	
		float cvq = 1.f;
		if (inputs[INPUT_CVQ].isConnected())
			cvq = abs(inputs[INPUT_CVQ].getVoltage() / 10.0);
 	
		afp.fc = params[PARAM_FC].getValue() * cvfc;
		afp.Q = params[PARAM_Q].getValue() * cvq;
		afp.dry = params[PARAM_DRY].getValue();
		afp.wet = params[PARAM_WET].getValue();

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

struct EasterModuleWidget : ModuleWidget {
	EasterModuleWidget(Easter* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Easter.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			RPJTitle * title = new RPJTitle(box.size.x,MODULE_WIDTH);
			title->setText("EASTER");
			addChild(title);
		} 
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 19),10,MODULE_WIDTH);
			tl->setText("Resonance Filter");
			addChild(tl);
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
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 110));
			tl->setText("RESONANCE");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(58, 190));
			tl->setText("DRY");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(5, 190));
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
		addInput(createInput<PJ301MPort>(Vec(55, 82), module, Easter::INPUT_CVFC));
		addParam(createParam<RoundBlackKnob>(Vec(8, 140), module, Easter::PARAM_Q));
		addInput(createInput<PJ301MPort>(Vec(55, 142), module, Easter::INPUT_CVQ));	
		addParam(createParam<RoundBlackKnob>(Vec(8, 225), module, Easter::PARAM_WET));
		addParam(createParam<RoundBlackKnob>(Vec(55, 225), module, Easter::PARAM_DRY));
	}

};

json_t *Easter::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_RESONATOR_TYPE_KEY, json_integer(static_cast<int>(afp.algorithm)));
	return rootJ;
}

void Easter::dataFromJson(json_t *rootJ) {
	json_t *nAlgorithmJ = json_object_get(rootJ, JSON_RESONATOR_TYPE_KEY);
	if (nAlgorithmJ) {
		afp.algorithm=static_cast<filterAlgorithm>(json_integer_value(nAlgorithmJ));
	}
}

Model * modelEaster = createModel<Easter, EasterModuleWidget>("Easter");