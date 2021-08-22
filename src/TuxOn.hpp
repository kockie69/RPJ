#include "rack.hpp"
#include <thread>
#include <osdialog.h>
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

using namespace rack;
using namespace std;

const int MODULE_WIDTH=6;

enum PanningType {SIMPLEPAN, CONSTPOWER };

struct PanPos {
	double left;
	double right;
};

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
		void loadSample(std::string);
		void process(const ProcessArgs &) override;
		PanPos panning(PanningType, double);

		char * fileName = NULL;
		unsigned int channels;
		unsigned int sampleRate;
		drwav_uint64 totalPCMFrameCount;
		dsp::SchmittTrigger startTrigger,stopTrigger;
		bool start=false,stop=false,loading=false,fileLoaded=false,play=false;
		vector<vector<float>> playBuffer;
		float * pSampleData;
		float samplePos;
		float peak;
		float scaleFac;
		PanningType panningType;
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