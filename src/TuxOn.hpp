#include "rack.hpp"
#include <thread>
#include <osdialog.h>
#include "Audio.hpp"

using namespace rack;

const int MODULE_WIDTH=6;

struct TuxOn : Module {

	enum ParamIds {
		PARAM_STOP,
		PARAM_START,
		PARAM_DB,
		PARAM_PANTYPE,
		PARAM_PANNING,
		PARAM_STARTPOS,
		PARAM_ENDPOS,
		PARAM_REPEAT,
		PARAM_SPEED,
		NUM_PARAMS,
	};

	enum InputIds {
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_LEFT,
		OUTPUT_RIGHT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	TuxOn();
	void process(const ProcessArgs &) override;
	char * fileName = NULL;
	dsp::SchmittTrigger startTrigger,stopTrigger;
	AudioParameters adp;
	Audio audio;
};

struct nSelectFileMenuItem : ui::MenuItem {
	TuxOn *module;
	void onAction(const event::Action&) override; 
};

struct TuxOnModuleWidget : ModuleWidget {
	TuxOnModuleWidget(TuxOn*);
	void appendContextMenu(Menu *) override;
};

