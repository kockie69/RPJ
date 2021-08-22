#include "RPJ.hpp"
#include "TuxOn.hpp"


TuxOn::TuxOn() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_START, 0.f, 1.f, 0.f);
	configParam(PARAM_STOP, 0.f, 1.f, 0.f);
	configParam(PARAM_DB, 0.f, -60.f, -3.f);
	configParam(PARAM_PANTYPE, 0.f, 1.f, 0.f);
	configParam(PARAM_PANNING, -1.f, 1.f, 0.f);
	playBuffer.resize(2);
	playBuffer[0].resize(0);
	playBuffer[1].resize(0);
	samplePos=0;
	peak=0;
	panningType=CONSTPOWER;
}

void TuxOn::loadSample(std::string path) {
	loading = true;
	float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(fileName, &channels, &sampleRate, &totalPCMFrameCount, NULL);
    if (pSampleData != NULL) {
		playBuffer[0].clear();
		playBuffer[1].clear();
		for (unsigned int i=0; i < (totalPCMFrameCount*channels); i = i + channels) {
			playBuffer[0].push_back(pSampleData[i]);
			if (channels == 2)
				playBuffer[1].push_back((float)pSampleData[i+1]);
			if ((abs(pSampleData[i])>peak) || abs((pSampleData[i+1])>peak))
				peak = max(abs(pSampleData[i]),abs(pSampleData[i+1]));	
		}
		play=true;
	}
	else 
		fileLoaded = false;
	loading = false;
}

PanPos TuxOn::panning(PanningType panType, double position) {
	PanPos pos;

	switch (panType) {
		case 0: {
			position *= 0.5;
			pos.left = position - 0.5;
			pos.right = position + 0.5;
			break;
		}
		case 1: {
			const double piovr2 = 4.0 * atan(1.0) * 0.5;
			const double root2ovr2 = sqrt(2.0) * 0.5;
			double thispos = position * piovr2;
			double angle = thispos * 0.5;
			pos.left = root2ovr2 * (cos(angle) - sin(angle));
			pos.right = root2ovr2 * (cos(angle) + sin(angle));
			break;
		}
 		default:
			break;
	}
	return pos;
}

void TuxOn::process(const ProcessArgs &args) {

	if (outputs[OUTPUT_LEFT].isConnected() || outputs[OUTPUT_RIGHT].isConnected()) {
		
		if (peak>0)
			scaleFac = (float)(pow(10.0,(params[PARAM_DB].getValue()/20.0)) / peak);

		if (startTrigger.process(rescale(params[PARAM_START].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
			if (fileName != NULL) {
				if (!fileLoaded) {
					loadSample(fileName);
				}				
				else {
					play=true;
				}
			}
		}
		if (stopTrigger.process(rescale(params[PARAM_STOP].getValue(), 1.f, 0.1f, 0.f, 1.f))) {
			play=false;
		}

		if ((!loading) && (play) && ((floor(samplePos) < totalPCMFrameCount) && (floor(samplePos) >= 0))) {
			
			panningType = static_cast<PanningType>(params[PARAM_PANTYPE].getValue());

			if (channels == 1) {
				outputs[OUTPUT_LEFT].setVoltage(5 * panning(panningType, params[PARAM_PANNING].getValue()).left * scaleFac * (playBuffer[0][floor(samplePos)]));
				outputs[OUTPUT_RIGHT].setVoltage(5 * panning(panningType, params[PARAM_PANNING].getValue()).right * scaleFac * (playBuffer[0][floor(samplePos)]));
			}
			else if (channels ==2) {
				outputs[OUTPUT_LEFT].setVoltage(5 * scaleFac * panning(panningType, params[PARAM_PANNING].getValue()).left * (playBuffer[0][floor(samplePos)]));
				outputs[OUTPUT_RIGHT].setVoltage(5 * scaleFac * panning(panningType, params[PARAM_PANNING].getValue()).right * (playBuffer[1][floor(samplePos)]));
        	}
			samplePos=samplePos+sampleRate/args.sampleRate;
		}
		if (floor(samplePos) >= totalPCMFrameCount) {
			samplePos = 0;
			play=false;
		}
		//outputs[OUTPUT_LEFT].setVoltage(pSa);
		//outputs[OUTPUT_RIGHT].setVoltage(out);	
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