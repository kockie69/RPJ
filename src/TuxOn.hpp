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

	void onAction(const event::Action& e) override {
		static const char SMF_FILTERS[] = "Standard WAV file (.wav):wav";
		osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
		
		char * PathC = osdialog_file(OSDIALOG_OPEN, "", "", filters);
		module->fileName = PathC;
	}
};