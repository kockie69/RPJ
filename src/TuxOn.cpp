#include "RPJ.hpp"
#define DR_WAV_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#include "dr_wav.h"
#include "dr_flac.h"
#include "dr_mp3.h"
#include "TuxOn.hpp"

TuxOn::TuxOn() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_START, 0.f, 1.f, 0.f);
	configParam(PARAM_STOP, 0.f, 1.f, 0.f);
	configParam(PARAM_DB, 0.f, -60.f, -3.f);
	configParam(PARAM_PANTYPE, 0.f, 1.f, 0.f);
	configParam(PARAM_REPEAT, 0.f, 1.f, 0.f);
	configParam(PARAM_PANNING, -1.f, 1.f, 0.f);
	configParam(PARAM_STARTPOS, 0.f, 1024.f, 0.f);
	configParam(PARAM_ENDPOS, 0.f, 1024.f, 0.f);
	configParam(PARAM_SPEED, -0.1f, 0.1f, 0.f);
	adp.panningType=CONSTPOWER;
	adp.dB=-3;
	adp.stop=false;
	adp.play=false;
}

void TuxOn::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LEFT].isConnected() || outputs[OUTPUT_RIGHT].isConnected()) {
		
		adp.dB = params[PARAM_DB].getValue()/20.0;
		adp.panningType = static_cast<PanningType>(params[PARAM_PANTYPE].getValue());
		adp.repeat = static_cast<bool>(params[PARAM_REPEAT].getValue());
		adp.panningValue = params[PARAM_PANNING].getValue();
		adp.rackSampleRate = args.sampleRate;
		adp.stop = false;
		adp.play = false;
		adp.speed = params[PARAM_SPEED].getValue();
		adp.startRatio = params[PARAM_STARTPOS].getValue();
		adp.endRatio = params[PARAM_ENDPOS].getValue();

		if (startTrigger.process(rescale(params[PARAM_START].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
			
			if (fileName != NULL) {
				if (!audio.fileLoaded) {
					if (audio.loadSample(fileName)) 
						adp.play=true;
				}
				else 
					adp.play=true;
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

void nSelectFileMenuItem::onAction(const event::Action& e) {
	static const char SMF_FILTERS[] = "Standard WAV file (.wav):wav;Standard FLAC file (.flac):flac;Standard MP3 file (.mp3):mp3";
	osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
		
	char * PathC = osdialog_file(OSDIALOG_OPEN, "", "", filters);
	module->fileName = PathC;
}

TuxOnModuleWidget::TuxOnModuleWidget(TuxOn* module) {

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
	{
		RPJTextLabel * tl = new RPJTextLabel(Vec(13, 30));
		tl->setText("START");
		addChild(tl);
	}
	{
		RPJTextLabel * tl = new RPJTextLabel(Vec(55, 30));
		tl->setText("STOP");
		addChild(tl);
	}
	{
		RPJTextLabel * tl = new RPJTextLabel(Vec(55, 80));
		tl->setText("false");
		addChild(tl);
	}
	{
		RPJTextLabel * tl = new RPJTextLabel(Vec(13, 110));
		tl->setText("Db");
		addChild(tl);
	}
	{
		RPJTextLabel * tl = new RPJTextLabel(Vec(55, 110));
		tl->setText("PAN");
		addChild(tl);
	}
	{
		RPJTextLabel * tl = new RPJTextLabel(Vec(13, 180));
		tl->setText("BEGIN");
		addChild(tl);
	}
	{
		RPJTextLabel * tl = new RPJTextLabel(Vec(55, 180));
		tl->setText("END");
		addChild(tl);
	}

	addParam(createParam<LEDButton>(Vec(10, 60), module, TuxOn::PARAM_START));
	addParam(createParam<LEDButton>(Vec(55, 60), module, TuxOn::PARAM_STOP));

	addParam(createParam<CKSS>(Vec(10, 90), module, TuxOn::PARAM_PANTYPE));
	addParam(createParam<CKSS>(Vec(55, 90), module, TuxOn::PARAM_REPEAT));

	addParam(createParam<RoundBlackKnob>(Vec(10, 140), module, TuxOn::PARAM_DB));
	addParam(createParam<RoundBlackKnob>(Vec(55, 140), module, TuxOn::PARAM_PANNING));

	addParam(createParam<RoundBlackKnob>(Vec(10, 210), module, TuxOn::PARAM_STARTPOS));
	addParam(createParam<RoundBlackKnob>(Vec(55, 210), module, TuxOn::PARAM_ENDPOS));

	addParam(createParam<RoundBlackKnob>(Vec(10, 260), module, TuxOn::PARAM_SPEED));

	addOutput(createOutput<PJ301MPort>(Vec(10, 300), module, TuxOn::OUTPUT_LEFT));
	addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, TuxOn::OUTPUT_RIGHT));
}

void TuxOnModuleWidget::appendContextMenu(Menu *menu) {
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

Model * modelTuxOn = createModel<TuxOn, TuxOnModuleWidget>("TuxOn");