#include "rack.hpp"
#include <thread>
#include <osdialog.h>
#include "Audio.hpp"
#include "ctrl/RPJVumeters.hpp"
#include "Display.hpp"

using namespace rack;

const char *JSON_FILE_NAME="Filename";
const char *JSON_ZOOM_LEVEL="Zoomlevel";
const char *JSON_PLAY="Play";
const char *JSON_STOP="Stop";
const char *JSON_PAUSE="Pause";
const char *JSON_SAMPLE_POS="Samplepos";
const char *JSON_BEGIN_POS="Beginpos";
const char *JSON_END_POS="Endpos";
const char *JSON_ZOOM_PARAMS="Zoomparams";
const char *JSON_ZOOM_TOTALPCM="TotalPCM";
const char *JSON_ZOOM_BEGIN="Begin";
const char *JSON_ZOOM_END="End";
const char *JSON_PLAY_MODE="Playmode";

const int MODULE_WIDTH=16;
const uint16_t ecoMode = 0xFFFF;// all 1's means yes, 0 means no
const int WIDTH=215;

union PackedBytes4 {
	int32_t cc1;
	int8_t cc4[4];
};

enum ButtonSvgs { EMPTY, PAUSE, REWIND, FORWARD, EJECT, STOP, START, BLACK, NUM_BUTTONSVGS};

struct zoomParameter {
	float begin;
	float end;
	int totalPCMFrameCount;
	float zoomBeginDelta;
	float zoomEndDelta;
};


struct TuxOn : Module {

	enum ParamIds {
		PARAM_PAUSE,
		PARAM_START,
		PARAM_STOP,
		PARAM_FWD,
		PARAM_RWD,
		PARAM_EJECT,
		PARAM_DB,
		PARAM_PANNING,
		PARAM_STARTPOS,
		PARAM_ENDPOS,
		PARAM_SPEED,
		PARAM_ZOOMIN,
		PARAM_ZOOMOUT,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_PANCV,
		INPUT_STARTSTOP,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_LEFT,
		OUTPUT_RIGHT,
		NUM_OUTPUTS,
	};

	enum LightIds {
		LIGHT_LEFT_LOW1,
		LIGHT_LEFT_LOW2,
		LIGHT_LEFT_MED1,
		LIGHT_LEFT_HIGH1,
		LIGHT_RIGHT_LOW1,
		LIGHT_RIGHT_LOW2,
		LIGHT_RIGHT_MED1,
		LIGHT_RIGHT_HIGH1,
		NUM_LIGHTS
	};

	TuxOn();
	void process(const ProcessArgs &) override;
	void setPlayBufferCopy(void);
	void selectAndLoadFile(void);
	json_t *dataToJson(void) override;
	void dataFromJson(json_t *) override;
	float getBegin(void);
	float getEnd(void);
	float stepSize(void);
	void setDisplay(bool);
	std::string fileName;
	dsp::BooleanTrigger startTrigger,pauseTrigger,stopTrigger,ejectTrigger,zoominTrigger,zoomoutTrigger;
	dsp::SchmittTrigger cvPlayStopTrigger;
	AudioParameters adp;
	Audio audio;
	std::string fileDesc;
	vector<zoomParameter> zoomParameters;
	VuMeterAllDual vuMeters;

	int clipping; // 0 is soft, 1 is hard (must be single ls bit)
	int8_t vuColorThemeLocal;
	float fadeGainScaled;
	PackedBytes4 colorAndCloak;// see enum called ccIds for fields
	float values[2];
	int svgIndex;
	int zoom;
	vector<vector<float>> playBufferCopy;
	ButtonSvgs buttonToDisplay;
	float beginRatio, endRatio;
	Display *display = new Display(WIDTH);
	float prevBeginRatio;
	float prevEndRatio;
};

struct MmSlider : SvgSlider {
	void setupSlider() {
		maxHandlePos = Vec(0, 0);
		minHandlePos = Vec(0, background->box.size.y - 0.01f);// 0.01f is epsilon so handle doesn't disappear at bottom
		float offsetY = handle->box.size.y / 2.0f;
		background->box.pos.y = offsetY;
		box.size.y = background->box.size.y + offsetY * 2.0f;
		background->visible = false;
	}
};

struct MmSmallFader : MmSlider {
	MmSmallFader();
};

struct nSelectFileMenuItem : ui::MenuItem {
	TuxOn *module;
	void onAction(const event::Action&) override; 
};

struct ButtonSVG : TransparentWidget {
	ButtonSVG();
	void addFrame(std::shared_ptr<Svg>);
	void drawLayer(const DrawArgs &, int) override;
	TuxOn *module;
	widget::FramebufferWidget *fb;
	widget::SvgWidget *sw;
    std::vector<std::shared_ptr<Svg>> frames;
};



struct TuxOnModuleWidget : ModuleWidget {
	TuxOnModuleWidget(TuxOn*);
	void appendContextMenu(Menu *) override;
	ButtonSVG *button;
	MenuLabel *PantypeLabel;
};

