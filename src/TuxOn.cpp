#include "RPJ.hpp"
#define DR_WAV_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#include "ctrl/RPJButtons.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "ctrl/RPJLights.hpp"
#include "TuxOn.hpp"

TuxOn::TuxOn() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configInput(INPUT_PANCV, "Pan CV");
	configInput(INPUT_STARTSTOP, "Start/Stop Trigger");
	configOutput(OUTPUT_LEFT, "Left Audio");
	configOutput(OUTPUT_RIGHT, "Right Audio");
	configParam(PARAM_START, 0.f, 1.f, 0.f);
	configButton(PARAM_START, "Start Button");
	configParam(PARAM_PAUSE, 0.f, 1.f, 0.f);
	configButton(PARAM_PAUSE, "Pause Button");
	configParam(PARAM_STOP, 0.f, 1.f, 0.f);
	configButton(PARAM_STOP, "Stop Button");
	configParam(PARAM_FWD, 0.f, 1.f, 0.f);
	configButton(PARAM_FWD, "Fast Forward Button");
	configParam(PARAM_RWD, 0.f, 1.f, 0.f);
	configButton(PARAM_RWD, "Fast Backward Button");
	configParam(PARAM_EJECT, 0.f, 1.f, 0.f);
	configButton(PARAM_EJECT, "Eject Button");
	configParam(PARAM_ZOOMIN, 0.f, 1.f, 0.f);
	configButton(PARAM_ZOOMIN, "Zoom in");
	configParam(PARAM_ZOOMOUT, 0.f, 1.f, 0.f);
	configButton(PARAM_ZOOMOUT, "Zoom out");
	float maxTGFader = std::pow(2.0f, 1.0f / 3.0f);
	configParam(PARAM_DB, 0.0f, maxTGFader, 1.0f, "Volume level", " dB", -10, 60.0f);

	configParam(PARAM_PANNING, -1.f, 1.f, 0.f, "Panning");
	configParam(PARAM_STARTPOS, 0.f, 1024.f, 0.f, "Start position");
	configParam(PARAM_ENDPOS, 0.f, 1024.f, 1024.f, "End position");
	configParam(PARAM_SPEED, -0.1f, 0.1f, 0.f, "Speed");
	adp.panningType=CONSTPOWER;
	adp.dB=1;
	vuMeters.reset(lights);
	vuColorThemeLocal=0;
	fileName="";
	fileDesc="No WAV, FLAC or MP3 file loaded.";
	buttonToDisplay=BLACK;
	playBufferCopy.resize(2);
	playBufferCopy[0].resize(0);
	playBufferCopy[1].resize(0);
	zoom=0;
	adp.playMode=REPEAT;
}

json_t *TuxOn::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_FILE_NAME, json_string(fileName.c_str()));
	json_object_set_new(rootJ, JSON_ZOOM_LEVEL, json_integer(zoom));
	json_object_set_new(rootJ, JSON_PLAY, json_boolean(buttonToDisplay==START));
	json_object_set_new(rootJ, JSON_PAUSE, json_boolean(buttonToDisplay==PAUSE));
	json_object_set_new(rootJ, JSON_STOP, json_boolean(buttonToDisplay==STOP));
	json_object_set_new(rootJ, JSON_SAMPLE_POS, json_real(audio.samplePos));
	json_object_set_new(rootJ, JSON_BEGIN_POS, json_real(audio.begin));
	json_object_set_new(rootJ, JSON_END_POS, json_real(audio.end));
	json_t *zoomP = json_array();
	if (zoomParameters.size()>0) {
		for (int i=0;i<(int)zoomParameters.size();i++) {
			json_t *zoomObject = json_object();
			json_object_set(zoomObject, JSON_ZOOM_BEGIN, json_real(zoomParameters[i].begin));
			json_object_set(zoomObject, JSON_ZOOM_END, json_real(zoomParameters[i].end));
			json_object_set(zoomObject, JSON_ZOOM_TOTALPCM, json_integer(zoomParameters[i].totalPCMFrameCount));
			json_array_append(zoomP, zoomObject);
		}
		json_object_set(rootJ, JSON_ZOOM_PARAMS, zoomP);
	}
	json_object_set_new(rootJ, JSON_PLAY_MODE, json_integer(static_cast<int>(adp.playMode)));
	return rootJ;
}

void TuxOn::dataFromJson(json_t *rootJ) {
	json_t *nfileNameJ = json_object_get(rootJ, JSON_FILE_NAME);
	json_t *nzoomJ = json_object_get(rootJ, JSON_ZOOM_LEVEL);
	json_t *nplayJ = json_object_get(rootJ, JSON_PLAY);
	json_t *npauseJ = json_object_get(rootJ, JSON_PAUSE);
	json_t *nstopJ = json_object_get(rootJ, JSON_STOP);
	json_t *nsamplePosJ = json_object_get(rootJ, JSON_SAMPLE_POS);
	json_t *nbeginPosJ = json_object_get(rootJ, JSON_BEGIN_POS);
	json_t *nendPosJ = json_object_get(rootJ, JSON_END_POS);
	json_t *nparamsJ = json_object_get(rootJ, JSON_ZOOM_PARAMS);
	json_t *nplaymodeJ = json_object_get(rootJ, JSON_PLAY_MODE);

	if (nfileNameJ) {
		fileName=(char *)json_string_value(nfileNameJ);
		if (fileName!="")
			selectAndLoadFile();
	}
	if (nzoomJ) {
		zoom=json_integer_value(nzoomJ);
	}
	if (nplayJ) {
		if (json_boolean_value(nplayJ)) {
			audio.start(); 
			buttonToDisplay = START;
		}
	}
	if (npauseJ) {
		if (json_boolean_value(npauseJ)) {
			audio.pause=true;
			buttonToDisplay = PAUSE;
		}
	}
	if (nstopJ) {
		if (json_boolean_value(nstopJ)) {
			audio.stop=true;
			buttonToDisplay = STOP;
		}
	}
	if (nsamplePosJ) {
		audio.samplePos=json_real_value(nsamplePosJ);
	}
	if (nbeginPosJ) {
		audio.begin=json_real_value(nbeginPosJ);
	}
	if (nendPosJ) {
		audio.end=json_real_value(nendPosJ);
	}
	if (nparamsJ) {
		if (json_is_array(nparamsJ)) {
			json_t *zoomObject = json_object();
			size_t i;
			zoomParameters.clear();			
			json_array_foreach(nparamsJ, i, zoomObject) {
				const char *key;
				json_t *value;
				zoomParameters.push_back(zoomParameter());
				json_object_foreach(zoomObject, key, value) {
					if (!strcmp(key, JSON_ZOOM_BEGIN)) 
						zoomParameters[(int)i].begin=json_real_value(value);
					if (!strcmp(key, JSON_ZOOM_END)) 	
						zoomParameters[(int)i].end=json_real_value(value);
					if (!strcmp(key, JSON_ZOOM_TOTALPCM)) 
						zoomParameters[(int)i].totalPCMFrameCount=json_integer_value(value);
				}
			}
		}
	}
	if (zoom >0) {
		audio.begin = zoomParameters[zoom].begin;
		audio.end = zoomParameters[zoom].end;
		audio.totalPCMFrameCount = zoomParameters[zoom].totalPCMFrameCount;
	}
	if (nplaymodeJ) {
		adp.playMode=static_cast<PlayMode>(json_integer_value(nplaymodeJ));
	}
}

void TuxOn::setDisplay() {
	display->width = WIDTH;
	display->fileDesc=fileDesc;
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
	
	if (fileName=="") {
		char* pathC  = osdialog_file(OSDIALOG_OPEN, "", "", filters);
		if (!pathC) {
        	// Fail silently
    		return;
		}
		else fileName.assign(pathC);
    }
	if (fileName!="") {
		if (audio.loadSample(fileName))
		{
			zoomParameters.push_back(zoomParameter());
			zoomParameters[0].totalPCMFrameCount=audio.totalPCMFrameCount;
			zoomParameters[0].begin=0;
			zoomParameters[0].end=audio.totalPCMFrameCount;
			display->setDisplayBuff(zoomParameters[0].begin,zoomParameters[0].end,audio.playBuffer);
			fileDesc = system::getFilename(fileName)+ "\n";
			fileDesc += std::to_string(audio.sampleRate)+ " Hz" + "\n";
			fileDesc += std::to_string(audio.channels)+ " channel(s)" + "\n";
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

	if (startTrigger.process((bool)params[PARAM_START].getValue()) || (cvPlayStopTrigger.process(rescale(inputs[INPUT_STARTSTOP].getVoltage(), 0.1f, 2.f, 0.f, 1.f)) && inputs[INPUT_STARTSTOP].isConnected())) {
		if (audio.fileLoaded) {
			audio.start();
			buttonToDisplay=START;	
		}
	}

	if (pauseTrigger.process((bool)params[PARAM_PAUSE].getValue()) || (cvPlayStopTrigger.process(rescale(inputs[INPUT_STARTSTOP].getVoltage(), 2.f, 0.1f, 0.f, 1.f)) && inputs[INPUT_STARTSTOP].isConnected())) {
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

		std::string oldFileName = fileName;
		std::string oldFileDesc = fileDesc;

		fileDesc="        --- EJECTING SONG ---";
		audio.ejectSong();
		fileName = "";
		fileDesc="No WAV, FLAC or MP3 file loaded.";
		setDisplay();
		selectAndLoadFile();
		if (fileName == "") {
			fileName = oldFileName;
			fileDesc = oldFileDesc;
			audio.fileLoaded=true;
			audio.setPlay(true);
			setDisplay();
		}
		buttonToDisplay=BLACK;
	}

	if (zoominTrigger.process((bool)params[PARAM_ZOOMIN].getValue())) {

		if (endRatio < beginRatio) 
			std::swap(beginRatio,endRatio);

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

	vuMeters.process(lights,audio.left,audio.right);
	float level=abs(audio.left)*10;
	lights[LightIds::LIGHT_LEFT_HIGH1].value = (level >= 7) ? .8f : .1f;
    lights[LightIds::LIGHT_LEFT_MED1].value = (level >= 3.5) ? .8f : .1f;
   	lights[LightIds::LIGHT_LEFT_LOW2].value = (level >= 1.75) ? .8f : .1f;
    lights[LightIds::LIGHT_LEFT_LOW1].value = (level >= .87) ? .8f : .1f;
	level=abs(audio.right)*10;
	lights[LightIds::LIGHT_RIGHT_HIGH1].value = (level >= 7) ? .8f : .1f;
    lights[LightIds::LIGHT_RIGHT_MED1].value = (level >= 3.5) ? .8f : .1f;
   	lights[LightIds::LIGHT_RIGHT_LOW2].value = (level >= 1.75) ? .8f : .1f;
    lights[LightIds::LIGHT_RIGHT_LOW1].value = (level >= .87) ? .8f : .1f;
}

void nSelectFileMenuItem::onAction(const event::Action& e) {
	module->selectAndLoadFile();
}

ButtonSVG::ButtonSVG() {
	fb = new widget::FramebufferWidget;
	addChild(fb);
	sw = new widget::SvgWidget;
	fb->addChild(sw);

	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/buttons/Black_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/buttons/Pause_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/buttons/Rwd_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/buttons/Fwd_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/buttons/Ejct_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/buttons/Stop_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/buttons/Start_On.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/buttons/Black_On.svg")));
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

void ButtonSVG::drawLayer(const DrawArgs &args, int layer) {
	if (layer == 1) {
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
	TransparentWidget::drawLayer(args,layer);
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
		//module->display = new Display();
		module->display->box.pos = Vec(5, 40);
		module->display->box.size = Vec(WIDTH, 250);
		module->display->setDisplayFont(pluginInstance,asset::system("res/fonts/ShareTechMono-Regular.ttf"));
		addChild(module->display);
	}
	else {
		// Fill the display for the module browser
		Display *display = new Display(WIDTH);
		display->box.pos = Vec(5, 40);
		display->box.size = Vec(WIDTH, 250);
		display->setDisplayFont(pluginInstance,asset::system("res/fonts/ShareTechMono-Regular.ttf"));
		display->fileDesc="And on Sunday your mates were toasting you with champagne down the pub";

		// Draw waveform
		/*for (int i=0; i < WIDTH-1; i++ ) {
			display->displayBuff[i]=sin(i);
		}*/
		addChild(display);	
	}
	{
		button = new ButtonSVG();
		button->box.pos = Vec(192, 35);
		button->module = module;
		addChild(button);
	}

	addParam(createParam<StartButton>(Vec(17, 89), module, TuxOn::PARAM_START));
	addParam(createParam<PauseButton>(Vec(52, 93), module, TuxOn::PARAM_PAUSE));
	addParam(createParam<RwdButton>(Vec(82, 89), module, TuxOn::PARAM_RWD));
	addParam(createParam<FwdButton>(Vec(117, 89), module, TuxOn::PARAM_FWD));
	addParam(createParam<EjectButton>(Vec(157, 91), module, TuxOn::PARAM_EJECT));
	addParam(createParam<StopButton>(Vec(192, 95), module, TuxOn::PARAM_STOP));

	addParam(createParam<RPJKnob>(Vec(12, 233), module, TuxOn::PARAM_STARTPOS));
	addParam(createParam<RPJKnob>(Vec(71, 233), module, TuxOn::PARAM_ENDPOS));
	addParam(createParam<RPJKnob>(Vec(131, 233), module, TuxOn::PARAM_PANNING));
	
	addParam(createParam<RedButton>(Vec(70,286),module, TuxOn::PARAM_ZOOMIN));
	addParam(createParam<RedButton>(Vec(70,320),module, TuxOn::PARAM_ZOOMOUT));

	addParam(createParam<RPJKnob>(Vec(131, 285), module, TuxOn::PARAM_DB));
	addParam(createParam<RPJKnob>(Vec(12, 285), module, TuxOn::PARAM_SPEED));

	addInput(createInput<PJ301MPort>(Vec(192, 237), module, TuxOn::INPUT_PANCV));
	addInput(createInput<PJ301MPort>(Vec(12, 339), module, TuxOn::INPUT_STARTSTOP));
	addOutput(createOutput<PJ301MPort>(Vec(192, 287), module, TuxOn::OUTPUT_LEFT));
	addOutput(createOutput<PJ301MPort>(Vec(192, 339), module, TuxOn::OUTPUT_RIGHT));
	
	for (int i=0;i<2;i++) {
		for (int j=0;j<4;j++) {
        	switch (j) {
            	case 0:
            	case 1:
                	addChild(createLightCentered<SmallLight<GreenLight>>(
                    	Vec(130+(j*10), 320+(i*10)),
                    	module,
                    	TuxOn::LIGHT_LEFT_LOW1+j+(4*i)));
                	break;
            	case 2:
                	addChild(createLightCentered<SmallLight<YellowLight>>(
                    	Vec(130+(j*10), 320+(i*10)),
                    	module,
                    	TuxOn::LIGHT_LEFT_LOW1+j+(4*i)));
                	break;
            	case 3:
                	addChild(createLightCentered<SmallLight<RedLight>>(
                    	Vec(130+(j*10), 320+(i*10)),
                    	module,
                    	TuxOn::LIGHT_LEFT_LOW1 + j+(4*i)));
                	break;
        	}
    	}
	}
}

void TuxOnModuleWidget::appendContextMenu(Menu *menu) {
	TuxOn * module = dynamic_cast<TuxOn*>(this->module);

	menu->addChild(new MenuSeparator());

	menu->addChild(createIndexPtrSubmenuItem("Panningtype", {"Simple", "Constant Power"}, &module->adp.panningType));

	menu->addChild(new MenuSeparator());

	menu->addChild(createIndexPtrSubmenuItem("PlayMode", {"Single", "Repeat", "PingPong"}, &module->adp.playMode));

	menu->addChild(new MenuSeparator());

	menu->addChild(createMenuItem("Select Audio File", module->fileName, [=]() {module->selectAndLoadFile();}));
}

Model * modelTuxOn = createModel<TuxOn, TuxOnModuleWidget>("TuxOn");