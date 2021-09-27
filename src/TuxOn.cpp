#include "RPJ.hpp"
#define DR_WAV_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#include "TuxOn.hpp"

template <typename TLightBase>
LEDLightSliderFixed<TLightBase>::LEDLightSliderFixed() {
	this->setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LEDSliderHandle.svg")));
}

MmSmallFader::MmSmallFader() {
	// no adjustment needed in this code, simply adjust the background svg's width to match the width of the handle by temporarily making it visible in the code below, and tweaking the svg's width as needed (when scaling not 100% between inkscape and Rack)
	setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fader-channel-bg.svg")));
	setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fader-channel.svg")));
	setupSlider();
}

TuxOn::TuxOn() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_START, 0.f, 1.f, 0.f);
	configParam(PARAM_PAUSE, 0.f, 1.f, 0.f);
	configParam(PARAM_STOP, 0.f, 1.f, 0.f);
	configParam(PARAM_FWD, 0.f, 1.f, 1.f);
	configParam(PARAM_RWD, 0.f, 1.f, 1.f);
	configParam(PARAM_EJECT, 0.f, 1.f, 1.f);
	float maxTGFader = std::pow(2.0f, 1.0f / 3.0f);
	configParam(PARAM_DB, 0.0f, maxTGFader, 1.0f, "", " dB", -10, 60.0f);

	configParam(PARAM_PANNING, -1.f, 1.f, 0.f);
	configParam(PARAM_STARTPOS, 0.f, 1024.f, 0.f);
	configParam(PARAM_ENDPOS, 0.f, 1024.f, 1024.f);
	configParam(PARAM_SPEED, -0.1f, 0.1f, 0.f);
	adp.panningType=CONSTPOWER;
	adp.dB=1;
	adp.repeat=true;
	vuMeters.reset();
	vuColorThemeLocal=0;
	fileName=NULL;
	fileDesc="        --- NO SONG SELECTED ---";
	buttonToDisplay=BLACK;
	playBufferCopy.resize(2);
	playBufferCopy[0].resize(0);
	playBufferCopy[1].resize(0);
	zoom=0;
}

void TuxOn::setDisplay() {
	display->width = WIDTH;
	if (zoomParameters.size()) {
		display->setDisplayPos(audio.samplePos,zoomParameters[zoom].begin,zoomParameters[zoom].totalPCMFrameCount);
		display->setBegin(beginRatio/1024);
		display->setEnd(endRatio/1024);
	}
}

float TuxOn::getBegin() {
	if (zoomParameters.size()) 
		return zoomParameters[zoom].begin + zoomParameters[zoom].totalPCMFrameCount * beginRatio/1024;
	else return 0;
}

float TuxOn::getEnd() {
	if (zoomParameters.size())	
		return zoomParameters[zoom].begin + zoomParameters[zoom].totalPCMFrameCount * endRatio/1024;
	else return 0;
}

float TuxOn::stepSize() {
	if (zoomParameters.size())
		//return abs((zoomParameters[zoom].begin-zoomParameters[zoom].end)/pow(2,zoom));
		return 1;
	else return 0;
}

void TuxOn::selectAndLoadFile(void) {	
	static const char SMF_FILTERS[] = "Standard WAV file (.wav):wav;Standard FLAC file (.flac):flac;Standard MP3 file (.mp3):mp3";
	osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
	
	char * PathC = osdialog_file(OSDIALOG_OPEN, "", "", filters);
	if (PathC!=NULL) {
		fileName = PathC;
		if (audio.loadSample(PathC))
		{
			zoomParameters.push_back(zoomParameter());
			zoomParameters[0].totalPCMFrameCount=audio.totalPCMFrameCount;
			zoomParameters[0].begin=0;
			zoomParameters[0].end=audio.totalPCMFrameCount;
			display->setDisplayBuff(zoomParameters[0].begin,zoomParameters[0].end,audio.playBuffer);
			display->fileDesc = system::getFilename(PathC)+ "\n";
			display->fileDesc += std::to_string(audio.sampleRate)+ " Hz" + "\n";
			display->fileDesc += std::to_string(audio.channels)+ " channel(s)" + "\n";
		}
	}
}

void TuxOn::process(const ProcessArgs &args) {
			
	adp.dB = params[PARAM_DB].getValue();
	adp.panningValue = params[PARAM_PANNING].getValue() + inputs[INPUT_PANCV].getVoltage()/5;
	adp.rackSampleRate = args.sampleRate;
	audio.setPause(false);
	audio.setStop(false);
	adp.speed = params[PARAM_SPEED].getValue();
	beginRatio = params[PARAM_STARTPOS].getValue();
	endRatio = params[PARAM_ENDPOS].getValue();
	adp.begin = getBegin();
	adp.end = getEnd();


	if (params[PARAM_FWD].getValue())
		audio.forward(stepSize());
	if (params[PARAM_RWD].getValue())
		audio.rewind(stepSize());

	if (startTrigger.process((bool)params[PARAM_START].getValue())) {
		if (audio.fileLoaded) {
			audio.start();
			buttonToDisplay=START;	
		}
	}

	if (pauseTrigger.process((bool)params[PARAM_PAUSE].getValue())) {
		if (audio.getPlay()) {
			audio.setPause(true);
			buttonToDisplay=PAUSE;
		}
	}

	if (stopTrigger.process((bool)params[PARAM_STOP].getValue())) {
		if (audio.getPlay()) { 
			audio.setStop(true);
			buttonToDisplay=STOP;
		}
	}

	if (ejectTrigger.process((bool)params[PARAM_EJECT].getValue())) {
		buttonToDisplay=EJECT;

		fileDesc="        --- EJECTING SONG ---";
		audio.ejectSong();
		fileName = NULL;
			fileDesc="        --- NO SONG SELECTED ---";

		selectAndLoadFile();
		buttonToDisplay=BLACK;
	}

	if (zoominTrigger.process((bool)params[PARAM_ZOOMIN].getValue())) {

		zoom++;
		zoomParameters.push_back(zoomParameter());
		zoomParameters[zoom].begin=zoomParameters[zoom-1].begin+zoomParameters[zoom-1].totalPCMFrameCount*beginRatio/1024;
		zoomParameters[zoom].end=zoomParameters[zoom-1].begin+zoomParameters[zoom-1].totalPCMFrameCount*endRatio/1024;
		zoomParameters[zoom].totalPCMFrameCount=abs(zoomParameters[zoom].end-zoomParameters[zoom].begin);
		if (audio.fileLoaded)
			display->setDisplayBuff(zoomParameters[zoom].begin,zoomParameters[zoom].end,audio.playBuffer);
	}

	if (zoomoutTrigger.process((bool)params[PARAM_ZOOMOUT].getValue())) {

		zoom--;
		if (zoom==-1)
			zoom=0;
		else {
			zoomParameters.pop_back();
			if (audio.fileLoaded) 
				display->setDisplayBuff(zoomParameters[zoom].begin,zoomParameters[zoom].end,audio.playBuffer);
		}
	}

	audio.setParameters(adp);
	audio.processAudioSample();
	setDisplay();
	
	outputs[OUTPUT_LEFT].setVoltage(10.f * audio.left);
	outputs[OUTPUT_RIGHT].setVoltage(10.f * audio.right);

	float sampleTimeEco = args.sampleTime * (1 + (ecoMode & 0x3));
	vuMeters.process(sampleTimeEco, &audio.left);
}

void nSelectFileMenuItem::onAction(const event::Action& e) {
	module->selectAndLoadFile();
}

void nSelectRepeatMenuItem::onAction(const event::Action& e) {

	module->adp.repeat = Repeat;
}

void nSelectPantypeMenuItem::onAction(const event::Action& e) {

	module->adp.panningType = Pantype;
}

struct StartButton : SvgSwitch  {
	StartButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Start_Off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Start_On.svg")));
	}
};

struct StopButton : SvgSwitch  {

	StopButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Stop_Off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Stop_On.svg")));
	}
};

struct PauseButton : SvgSwitch  {
	PauseButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Pause_Off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Pause_On.svg")));
	}
};


struct FwdButton : SvgSwitch  {
	FwdButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Fwd_Off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Fwd_On.svg")));
	}
};

struct RwdButton : SvgSwitch  {
	RwdButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Rwd_Off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Rwd_On.svg")));
	}
};

struct EjectButton : SvgSwitch  {
	EjectButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Ejct_Off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Ejct_On.svg")));
	}
};


ButtonSVG::ButtonSVG() {
	fb = new widget::FramebufferWidget;
	addChild(fb);
	sw = new widget::SvgWidget;
	fb->addChild(sw);

	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Buttons/Black_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Buttons/Pause_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Buttons/Rwd_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Buttons/Fwd_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Buttons/Ejct_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Buttons/Stop_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Buttons/Start_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Buttons/Black_On.svg")));
}

void ButtonSVG::addFrame(std::shared_ptr<Svg> svg) {
	frames.push_back(svg);
	// If this is our first frame, automatically set SVG and size
	if (!sw->svg) {
		sw->setSvg(svg);
		box.size = sw->box.size;
		fb->box.size = sw->box.size;
	}
}

void ButtonSVG::draw(const DrawArgs &args) {
	if (module) {
		// Bit weird check, shouldn't that be module->start ?
		if (!(module->buttonToDisplay == START && !module->audio.fileLoaded)) {
			sw->setSvg(frames[static_cast<int>(module->buttonToDisplay)]);
			fb->dirty = true;
		}
	}
	else {
	}

	if (sw->svg && sw->svg->handle) {
		svgDraw(args.vg, sw->svg->handle);
	}
}


TuxOnModuleWidget::TuxOnModuleWidget(TuxOn* module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TuxOn.svg")));

	addChild(createWidget<ScrewSilver>(Vec(0, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15,0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(0, 365)));

	box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	if (module) {
		module->display = new Display();
		module->display->box.pos = Vec(5, 40);
		module->display->box.size = Vec(WIDTH, 250);
		module->display->setDisplayFont(pluginInstance,"res/DejaVuSansMono.ttf");
		addChild(module->display);
	}

	{
		button = new ButtonSVG();
		button->box.pos = Vec(185, 35);
		button->module = module;
		addChild(button);
	}

	addParam(createParam<StartButton>(Vec(10, 85), module, TuxOn::PARAM_START));
	addParam(createParam<PauseButton>(Vec(45, 85), module, TuxOn::PARAM_PAUSE));
	addParam(createParam<RwdButton>(Vec(80, 85), module, TuxOn::PARAM_RWD));
	addParam(createParam<FwdButton>(Vec(115, 85), module, TuxOn::PARAM_FWD));
	addParam(createParam<EjectButton>(Vec(150, 85), module, TuxOn::PARAM_EJECT));
	addParam(createParam<StopButton>(Vec(185, 85), module, TuxOn::PARAM_STOP));

	addParam(createParam<RoundBlackKnob>(Vec(10, 233), module, TuxOn::PARAM_STARTPOS));
	addParam(createParam<RoundBlackKnob>(Vec(55, 233), module, TuxOn::PARAM_ENDPOS));

	addParam(createParam<buttonMinSmall>(Vec(110,233),module, TuxOn::PARAM_ZOOMOUT));
	addParam(createParam<buttonPlusSmall>(Vec(135,233),module, TuxOn::PARAM_ZOOMIN));
			
	// Fader
	MmSmallFader *newFader;
	addParam(newFader = createParamCentered<MmSmallFader>(mm2px(Vec(71,99.55)), module, TuxOn::PARAM_DB));
	if (module) {
		// VU meters
		VuMeterTrack *newVU = createWidgetCentered<VuMeterTrack>(mm2px(Vec(67.25, 99.65)));
		newVU->srcLevels = module->vuMeters.vuValues;
		newVU->colorThemeLocal = &(module->vuColorThemeLocal);
		addChild(newVU);
	}
	
	addParam(createParam<RoundBlackKnob>(Vec(55, 285), module, TuxOn::PARAM_PANNING));

	addParam(createParam<RoundBlackKnob>(Vec(10, 285), module, TuxOn::PARAM_SPEED));

	addInput(createInput<PJ301MPort>(Vec(57, 328), module, TuxOn::INPUT_PANCV));
	addOutput(createOutput<PJ301MPort>(Vec(105, 285), module, TuxOn::OUTPUT_LEFT));
	addOutput(createOutput<PJ301MPort>(Vec(105, 328), module, TuxOn::OUTPUT_RIGHT));
}

void TuxOnModuleWidget::appendContextMenu(Menu *menu) {
	TuxOn * module = dynamic_cast<TuxOn*>(this->module);

	menu->addChild(new MenuSeparator());

	PantypeLabel = new MenuLabel();
	PantypeLabel->text = "Panning Type";
	menu->addChild(PantypeLabel);

	nSelectPantypeMenuItem *nSelectSimplepanItem = new nSelectPantypeMenuItem();
	nSelectSimplepanItem->text = "Simple Panning";
	nSelectSimplepanItem->module = module;
	nSelectSimplepanItem->Pantype = PanningType::SIMPLEPAN;
	nSelectSimplepanItem->rightText = CHECKMARK(module->adp.panningType == nSelectSimplepanItem->Pantype);
	menu->addChild(nSelectSimplepanItem);

	nSelectPantypeMenuItem *nSelectConstpowerItem = new nSelectPantypeMenuItem();
	nSelectConstpowerItem->text = "Constant Power";
	nSelectConstpowerItem->module = module;
	nSelectConstpowerItem->Pantype = PanningType::CONSTPOWER;
	nSelectConstpowerItem->rightText = CHECKMARK(module->adp.panningType == nSelectConstpowerItem->Pantype);
	menu->addChild(nSelectConstpowerItem);

	menu->addChild(new MenuSeparator());

	MenuLabel *RepeatLabel = new MenuLabel();
	RepeatLabel->text = "Repeat";
	menu->addChild(RepeatLabel);

	nSelectRepeatMenuItem *nSelectRepeatItem = new nSelectRepeatMenuItem();
	nSelectRepeatItem->text = "Repeat fragment";
	nSelectRepeatItem->module = module;
	nSelectRepeatItem->Repeat = !module->adp.repeat;
	nSelectRepeatItem->rightText = CHECKMARK(!nSelectRepeatItem->Repeat);
	menu->addChild(nSelectRepeatItem);

	menu->addChild(new MenuSeparator());

	MenuLabel *AudioFileLabel = new MenuLabel();
	AudioFileLabel->text = "Audio File";
	menu->addChild(AudioFileLabel);

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