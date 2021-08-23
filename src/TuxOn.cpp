#include "RPJ.hpp"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "TuxOn.hpp"

TuxOn::TuxOn() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_START, 0.f, 1.f, 0.f);
	configParam(PARAM_STOP, 0.f, 1.f, 0.f);
	configParam(PARAM_DB, 0.f, -60.f, -3.f);
	configParam(PARAM_PANTYPE, 0.f, 1.f, 0.f);
	configParam(PARAM_PANNING, -1.f, 1.f, 0.f);
	adp.panningType=CONSTPOWER;
	adp.dB=-3;
	adp.stop=false;
}

void TuxOn::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LEFT].isConnected() || outputs[OUTPUT_RIGHT].isConnected()) {
		
		adp.dB = params[PARAM_DB].getValue()/20.0;
		adp.panningType = static_cast<PanningType>(params[PARAM_PANTYPE].getValue());
		adp.panningValue = params[PARAM_PANNING].getValue();
		adp.rackSampleRate = args.sampleRate;
		adp.stop = false;

		if (startTrigger.process(rescale(params[PARAM_START].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
			
			if (fileName != NULL) {
				audio.loadSample(fileName);
			}
		}

		if (stopTrigger.process(rescale(params[PARAM_STOP].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
			adp.stop=true;
		}

		audio.setParameters(adp);
		audio.processAudioSample();
		outputs[OUTPUT_LEFT].setVoltage(5 * audio.left);
		outputs[OUTPUT_RIGHT].setVoltage(5 * audio.right);
	}
}

struct TuxOnModuleWidget : ModuleWidget {
	TuxOnModuleWidget(TuxOn* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TuxOn.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			RPJTitle * title = new RPJTitle(box.size.x,MODULE_WIDTH);
			title->setText("TuxOn");
			addChild(title);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(13, 270));
			tl->setText("LEFT");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 290));
			tl->setText("RIGHT");
			addChild(tl);
		}


		addOutput(createOutput<PJ301MPort>(Vec(10, 300), module, TuxOn::OUTPUT_LEFT));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, TuxOn::OUTPUT_RIGHT));
		addParam(createParam<LEDButton>(Vec(8, 60), module, TuxOn::PARAM_START));
		addParam(createParam<LEDButton>(Vec(8, 110), module, TuxOn::PARAM_STOP));
		addParam(createParam<RoundBlackKnob>(Vec(8, 170), module, TuxOn::PARAM_DB));
		addParam(createParam<CKSS>(Vec(31, 170), module, TuxOn::PARAM_PANTYPE));
		addParam(createParam<RoundBlackKnob>(Vec(55, 170), module, TuxOn::PARAM_PANNING));
	}

	void appendContextMenu(Menu *menu) override {
		TuxOn *module = dynamic_cast<TuxOn*>(this->module);

		menu->addChild(new MenuEntry);
		nSelectFileMenuItem *nSelectFileItem = new nSelectFileMenuItem();
		nSelectFileItem->text = "Select Audio File";
		nSelectFileItem->module = module;
		if (module->fileName!=NULL)
			nSelectFileItem->rightText = module->fileName;
		else 
			nSelectFileItem->rightText = "";
		menu->addChild(nSelectFileItem);
	}
};

Model * modelTuxOn = createModel<TuxOn, TuxOnModuleWidget>("TuxOn");