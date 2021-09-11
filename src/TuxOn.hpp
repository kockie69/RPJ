#include "rack.hpp"
#include <thread>
#include <osdialog.h>
#include "Audio.hpp"
#include "VuMeters.hpp"

using namespace rack;

const int MODULE_WIDTH=15;
const uint16_t ecoMode = 0xFFFF;// all 1's means yes, 0 means no

union PackedBytes4 {
	int32_t cc1;
	int8_t cc4[4];
};

enum ccIds {
	cloakedMode, // turn off track VUs only, keep master VUs (also called "Cloaked mode"), this has only two values, 0x0 and 0xFF so that it can be used in bit mask operations
	vuColorGlobal, // 0 is green, 1 is aqua, 2 is cyan, 3 is blue, 4 is purple, 5 is individual colors for each track/group/master (every user of vuColor must first test for != 5 before using as index into color table, or else array overflow)
	dispColorGlobal, // 0 is yellow, 1 is light-gray, 2 is green, 3 is aqua, 4 is cyan, 5 is blue, 6 is purple, 7 is per track
	detailsShow // bit 0 is knob param arc, bit 1 is knob cv arc, bit 2 is fader cv pointer
};

template <typename TLightBase = RedLight>
struct LEDLightSliderFixed : LEDLightSlider<TLightBase> {
	LEDLightSliderFixed() {
		this->setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LEDSliderHandle.svg")));
	}
};

struct TuxOn : Module {

	enum ParamIds {
		PARAM_PAUSE,
		PARAM_START,
		PARAM_STOP,
		PARAM_FWD,
		PARAM_BWD,
		PARAM_EJECT,
		PARAM_DB,
		PARAM_PANNING,
		PARAM_STARTPOS,
		PARAM_ENDPOS,
		PARAM_SPEED,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_PANCV,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_LEFT,
		OUTPUT_RIGHT,
		NUM_OUTPUTS,
	};

	enum LightIds {
		ENUMS(VU_LIGHTS, 2 * 5),
		RATE_LIGHT,
		START_LIGHT,
		PAUSE_LIGHT,
		NUM_LIGHTS
	};


	TuxOn();
	void process(const ProcessArgs &) override;
	char * fileName = NULL;
	dsp::BooleanTrigger startTrigger,pauseTrigger,stopTrigger,ejectTrigger;
	AudioParameters adp;
	Audio audio;
	std::string fileDesc;
	vector<double> displayBuff;
	VuMeterAllDual vuMeters;

	int clipping; // 0 is soft, 1 is hard (must be single ls bit)
	int8_t vuColorThemeLocal;
	float fadeGainScaled;
	PackedBytes4 colorAndCloak;// see enum called ccIds for fields
	float values[2];
	int svgIndex;
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
	MmSmallFader() {
		// no adjustment needed in this code, simply adjust the background svg's width to match the width of the handle by temporarily making it visible in the code below, and tweaking the svg's width as needed (when scaling not 100% between inkscape and Rack)
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fader-channel-bg.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fader-channel.svg")));
		setupSlider();
	}
};

struct TuxOnDisplay : TransparentWidget {
	TuxOnDisplay(); 
	void draw(const DrawArgs &args) override; 
	TuxOn *module;
	int frame;
	shared_ptr<Font> font;
};

struct nSelectFileMenuItem : ui::MenuItem {
	TuxOn *module;
	void onAction(const event::Action&) override; 
};

struct nSelectRepeatMenuItem : ui::MenuItem {
	TuxOn *module;
	bool Repeat;
	void onAction(const event::Action&) override; 
};

struct nSelectPantypeMenuItem : ui::MenuItem {
	TuxOn *module;
	PanningType Pantype;
	void onAction(const event::Action&) override; 
};

struct TuxOnModuleWidget : ModuleWidget {
	TuxOnModuleWidget(TuxOn*);
	void appendContextMenu(Menu *) override;
};

