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
	adp.pause=false;
	adp.stop=false;
	adp.repeat=true;
	vuMeters.reset();
	vuColorThemeLocal=0;
	fileName=NULL;
	fileDesc="        --- NO SONG SELECTED ---";
	svgIndex=7;
	playBufferCopy.resize(2);
	playBufferCopy[0].resize(0);
	playBufferCopy[1].resize(0);
	zoom=0;
}

void TuxOn::setPlayBufferCopy(void) {
	playBufferCopy[0].clear();
	playBufferCopy[1].clear();
	for (unsigned int i=0; i < audio.totalPCMFrameCount; i++) {
		playBufferCopy[0].push_back(audio.playBuffer[0][i]);
		if (audio.channels == 2)
			playBufferCopy[1].push_back(audio.playBuffer[1][i]);
	}
}

void TuxOn::setDisplay(float begin, float end, unsigned int zoom) {
	vector<double>().swap(displayBuff);
	for (int i=begin; i < floor(end); i = i + floor(end/130/zoom)) {
		displayBuff.push_back(audio.playBuffer[0][i]);
	}	
}

void TuxOn::process(const ProcessArgs &args) {
			
	adp.dB = params[PARAM_DB].getValue();
	adp.panningValue = params[PARAM_PANNING].getValue() + inputs[INPUT_PANCV].getVoltage()/5;
	adp.rackSampleRate = args.sampleRate;
	adp.pause = false;
	adp.stop = false;
	adp.speed = params[PARAM_SPEED].getValue();
	adp.startRatio = params[PARAM_STARTPOS].getValue();
	adp.endRatio = params[PARAM_ENDPOS].getValue();
	
	if (params[PARAM_FWD].getValue())
		audio.forward();
	if (params[PARAM_RWD].getValue())
		audio.rewind();

	if (startTrigger.process((bool)params[PARAM_START].getValue())) {

		if (audio.fileLoaded) {
			audio.start();
			svgIndex=6;	
		}
	}

	if (pauseTrigger.process((bool)params[PARAM_PAUSE].getValue())) {
		if (audio.play) {
			adp.pause=true;
			svgIndex=1;
		}
	}

	if (stopTrigger.process((bool)params[PARAM_STOP].getValue())) {
		if (audio.play) { 
			adp.stop=true;
			svgIndex=5;
		}
	}

	if (ejectTrigger.process((bool)params[PARAM_EJECT].getValue())) {
		svgIndex=4;

		fileDesc="        --- EJECTING SONG ---";
		audio.ejectSong();
		fileName = NULL;
			fileDesc="        --- NO SONG SELECTED ---";

		static const char SMF_FILTERS[] = "Standard WAV file (.wav):wav;Standard FLAC file (.flac):flac;Standard MP3 file (.mp3):mp3";
		osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
	
		char * PathC = osdialog_file(OSDIALOG_OPEN, "", "", filters);
		if (PathC!=NULL) {
			fileName = PathC;
			if (audio.loadSample(PathC))
			{
				setPlayBufferCopy();
				setDisplay(0,audio.totalPCMFrameCount,pow(2,zoom));
				fileDesc = rack::string::filename(PathC)+ "\n";
				fileDesc += std::to_string(audio.sampleRate)+ " Hz" + "\n";
				fileDesc += std::to_string(audio.channels)+ " channel(s)" + "\n";
			}
		}
		svgIndex=7;
	}

	if (zoominTrigger.process((bool)params[PARAM_ZOOMIN].getValue())) {

		zoom++;
		if (audio.fileLoaded) {
			setDisplay(audio.totalPCMFrameCount*audio.beginRatio/1024,audio.totalPCMFrameCount*audio.endRatio/1024,zoom);
		}
	}

	if (zoomoutTrigger.process((bool)params[PARAM_ZOOMOUT].getValue())) {

//		audio.totalPCMFrameCount *= 2;
//		displayBuff.clear();
//		for (int i=audio.beginRatio*audio.totalPCMFrameCount/1024; i < floor(audio.endRatio*audio.totalPCMFrameCount/1024); i = i + floor(audio.totalPCMFrameCount/(130*zoom))) {
//			displayBuff.push_back(audio.playBuffer[0][i]);
//		}
	}

	audio.setParameters(adp);
	audio.processAudioSample();

	outputs[OUTPUT_LEFT].setVoltage(10.f * audio.left);
	outputs[OUTPUT_RIGHT].setVoltage(10.f * audio.right);

	float sampleTimeEco = args.sampleTime * (1 + (ecoMode & 0x3));
	vuMeters.process(sampleTimeEco, &audio.left);
}

TuxOnDisplay::TuxOnDisplay() {
	font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	frame = 0;
}
	
void TuxOnDisplay::draw(const DrawArgs &args) {
	if (module) {
		nvgFontSize(args.vg, 12);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));	
		nvgTextBox(args.vg, 5, 5,215, module->fileDesc.c_str(), NULL);
		
		// Draw ref line
		nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, 0, 125);
			nvgLineTo(args.vg, 215, 125);
			nvgClosePath(args.vg);
		}
		nvgStroke(args.vg);
		
		if (module->audio.fileLoaded) {
			// Draw play line
			nvgStrokeColor(args.vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
            nvgStrokeWidth(args.vg, 0.8);
			{
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, floor(module->audio.samplePos * 215 / module->audio.totalPCMFrameCount) , 84);
				nvgLineTo(args.vg, floor(module->audio.samplePos * 215 / module->audio.totalPCMFrameCount) , 164);
				nvgClosePath(args.vg);
			}
			nvgStroke(args.vg);
            
            // Draw start line
			nvgStrokeColor(args.vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
            nvgStrokeWidth(args.vg, 1.5);
			{
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, floor((module->audio.beginRatio/1024) * 215) , 84);
				nvgLineTo(args.vg, floor((module->audio.beginRatio/1024) * 215) , 164);
				nvgClosePath(args.vg);
			}
			nvgStroke(args.vg);
            
			// Draw end line
			nvgStrokeColor(args.vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
            nvgStrokeWidth(args.vg, 1.5);
			{
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, floor((module->audio.endRatio/1024) * 215) , 84);
				nvgLineTo(args.vg, floor((module->audio.endRatio/1024) * 215) , 164);
				nvgClosePath(args.vg);
			}

			nvgStroke(args.vg);
			
			// Draw waveform
			nvgStrokeColor(args.vg, nvgRGBA(0xe1, 0x02, 0x78, 0xc0));
			nvgSave(args.vg);
			Rect b = Rect(Vec(0, 84), Vec(215, 80));
			nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
			nvgBeginPath(args.vg);
			for (unsigned int i = 0; i < module->displayBuff.size(); i++) {
				float x, y;
				x = (float)i / (module->displayBuff.size() - 1);
				//y = module->displayBuff[i] / 2.0 + 0.5;
				y = module->displayBuff[i] / 1.0 + 0.5;
				Vec p;
				p.x = b.pos.x + b.size.x * x;
				p.y = b.pos.y + b.size.y * (1.0 - y);
				if (i == 0)
					nvgMoveTo(args.vg, p.x, p.y);
				else
					nvgLineTo(args.vg, p.x, p.y);
			}
			nvgLineCap(args.vg, NVG_ROUND);
			nvgMiterLimit(args.vg, 2.0);
			nvgStrokeWidth(args.vg, 1.5);
			nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
			nvgStroke(args.vg);			
			nvgResetScissor(args.vg);
			nvgRestore(args.vg);	
		}
	}
}

void nSelectFileMenuItem::onAction(const event::Action& e) {
	static const char SMF_FILTERS[] = "Standard WAV file (.wav):wav;Standard FLAC file (.flac):flac;Standard MP3 file (.mp3):mp3";
	osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
	char * PathC = osdialog_file(OSDIALOG_OPEN, "", "", filters);
	
	if (PathC!=NULL) {
		module->fileName = PathC;
		if (module->audio.loadSample(PathC))
		{
			module->setPlayBufferCopy();
			module->setDisplay(0,module->audio.totalPCMFrameCount,module->zoom);
			module->fileDesc = rack::string::filename(PathC)+ "\n";
			module->fileDesc += std::to_string(module->audio.sampleRate)+ " Hz" + "\n";
			module->fileDesc += std::to_string(module->audio.channels)+ " channel(s)" + "\n";
		}
	}
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

struct ButtonSVG : TransparentWidget {
	TuxOn *module;
	widget::FramebufferWidget *fb;
	widget::SvgWidget *sw;
    std::vector<std::shared_ptr<Svg>> frames;

	ButtonSVG() {
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

	void addFrame(std::shared_ptr<Svg> svg) {
		frames.push_back(svg);
		// If this is our first frame, automatically set SVG and size
		if (!sw->svg) {
			sw->setSvg(svg);
			box.size = sw->box.size;
			fb->box.size = sw->box.size;
		}
	}

	void draw(const DrawArgs &args) override {
		if (module) {
			if (!(module->svgIndex == 6 && !module->audio.fileLoaded)) {
				sw->setSvg(frames[module->svgIndex]);
				fb->dirty = true;
			}
		}
		else {

		}

		if (sw->svg && sw->svg->handle) {
			svgDraw(args.vg, sw->svg->handle);
		}
	}
};

TuxOnModuleWidget::TuxOnModuleWidget(TuxOn* module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TuxOn.svg")));

	addChild(createWidget<ScrewSilver>(Vec(0, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15,0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(0, 365)));

	box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		TuxOnDisplay *display = new TuxOnDisplay();
		display->box.pos = Vec(5, 40);
		display->box.size = Vec(215, 250);
		display->module = module;
		addChild(display);
	}

	{
		ButtonSVG *button = new ButtonSVG();
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
	TuxOn *module = dynamic_cast<TuxOn*>(this->module);

	menu->addChild(new MenuSeparator());

	MenuLabel *PantypeLabel = new MenuLabel();
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