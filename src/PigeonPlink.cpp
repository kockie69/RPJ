
#include <sstream>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include "RPJ.hpp"
#include "../composites/WidgetComposite.h"

#include "PigeonPlink.hpp"
#include "ctrl/SqWidgets.h"
#include "ctrl/RPJKnobs.hpp"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/PopupMenuParamWidget.h"
#include "ctrl/RPJButtons.hpp"

using Comp = WVCO<WidgetComposite>;
using Input = ::rack::engine::Input;

class RatioParamQuantity : public ParamQuantity {
public:
    RatioParamQuantity(const ParamQuantity& other, const std::vector<std::string>& str) : strings(str) {
        ParamQuantity* base = this;
        *base = other;
    }

    std::string getDisplayValueString() override {
        float rDiv  = 10.f / (strings.size()-1);
        float KV = getValue();
        int ratioIndex = round(KV / rDiv);
        std::string str;

        if (strings.size()==0)
            str = std::to_string(KV*3.2f);
        else {
            str = strings[ratioIndex];
        }
        return str; 
    }

private:
    std::vector<std::string> strings;
};


class DiscreteParamQuantity : public ParamQuantity {
public:
    DiscreteParamQuantity(const ParamQuantity& other, const std::vector<std::string>& str) : strings(str) {
        ParamQuantity* base = this;
        *base = other;
    }

    std::string getDisplayValueString() override {
        const unsigned int index = (unsigned int)(std::round(getValue()));
        if (index < strings.size()) {
            return strings[index];
        } else {
            assert(false);
            return "error";
        }
        
    }
private:
    std::vector<std::string> strings;
};

inline void substituteDiscreteParamQuantity(const std::vector<std::string>& strings, Module& module, unsigned int paramNumber) {
    auto orig = module.paramQuantities[paramNumber];

     auto p = new DiscreteParamQuantity(*orig, strings);
    
    delete orig;
    module.paramQuantities[paramNumber] = p;
}

inline void substituteRatioParamQuantity(const std::vector<std::string>& strings, Module& module, unsigned int paramNumber) {
    auto orig = module.paramQuantities[paramNumber];

     auto p = new RatioParamQuantity(*orig, strings);
    
    delete orig;
    module.paramQuantities[paramNumber] = p;
}

/**
 * This guy knows how to go out an patch in other instances
 */
class WvcoPatcher
{
public:
    static void go(ModuleWidget* hostWidget, Module* hostModule)
    {
        Module* otherModule = getLeftMatchingModule(hostModule); 
        ModuleWidget* otherModuleWidget = getWidgetForModule(otherModule);
    
        patchVOct(otherModuleWidget, hostWidget);
        patchGate(otherModuleWidget, hostWidget);
        patchModulator(otherModuleWidget, hostWidget);
    }

    static bool shouldShowMenu(Module* hostModule) 
    {
        return bool( getLeftMatchingModule(hostModule));
    }
private:

    static bool isPortPatched(PortWidget* portWidget) {
        auto cables = APP->scene->rack->getCablesOnPort(portWidget);
        return !cables.empty();
    }

    static void patchModulator(ModuleWidget* otherModuleWidget, ModuleWidget* myModuleWidget) {
        auto myFMPort = getInput(myModuleWidget, Comp::LINEAR_FM_INPUT);
        if (isPortPatched(myFMPort)) {
            WARN("my FM input already connected\n");
            return;
        }
        auto otherOutput = getOutput(otherModuleWidget, Comp::MAIN_OUTPUT);

        assert(myFMPort->type == Port::INPUT);
        assert(otherOutput->type == Port::OUTPUT);

         //   (output, input)
        patchBetweenPorts(myModuleWidget, otherModuleWidget, otherOutput, myFMPort);
    }

    static void patchVOct(ModuleWidget* otherModuleWidget, ModuleWidget* myModuleWidget) {
        auto myVOctPort = getInput(myModuleWidget, Comp::VOCT_INPUT);

        if (!isPortPatched(myVOctPort)) {
            WARN("my V/Oct not connected\n");
            return;
        }
        auto otherVOctPort = getInput(otherModuleWidget, Comp::VOCT_INPUT);

         if (isPortPatched(otherVOctPort)) {
        WARN("othersub V/Oct port already patched");
            return;
        }
        PortWidget* source = getOutputThatConnectsToThisInput(myVOctPort);

        patchBetweenPorts(myModuleWidget, otherModuleWidget, source, otherVOctPort);
    }

    static void patchGate(ModuleWidget* otherModuleWidget, ModuleWidget* myModuleWidget) {
        auto myGatePort = getInput(myModuleWidget, Comp::GATE_INPUT);

        if (!isPortPatched(myGatePort)) {
            WARN("my gate not connected\n");
            return;
        }
        auto otherGatePort = getInput(otherModuleWidget, Comp::GATE_INPUT);

         if (isPortPatched(otherGatePort)) {
            WARN("other gate port already patched");
            return;
        }
        PortWidget* source = getOutputThatConnectsToThisInput(myGatePort);
        patchBetweenPorts(myModuleWidget, otherModuleWidget, source, otherGatePort);
    }

    static Module* getLeftMatchingModule(Module* myModule) {
        Module* left = nullptr;
        auto leftExpander = myModule->leftExpander;
        if (leftExpander.module) {
            auto leftModule = leftExpander.module;
            if (leftModule->model == myModule->model) {
                // There is a copy of me to my left
                left = leftModule;
            }
        }
        return left;
    }

    static PortWidget* getOutputThatConnectsToThisInput(PortWidget* thisInput) {
        assert(thisInput->type == Port::INPUT);
        auto cables = APP->scene->rack->getCablesOnPort(thisInput);
        assert(cables.size() == 1);
        auto cable = cables.begin();
        CableWidget* cw = *cable;
        PortWidget* ret =  cw->outputPort;
        assert(ret->type == Port::OUTPUT);
        return ret;
    }

    static ModuleWidget* getWidgetForModule(Module* module) {
        auto rack = APP->scene->rack;
        for (Widget* w2 : rack->getModuleContainer()->children) {
            ModuleWidget* modwid = dynamic_cast<ModuleWidget *>(w2);
            if (modwid) {
                if (modwid->module == module) {
                    return modwid;
                }
            }
        }
        return nullptr;
    }

    static void patchBetweenPorts(ModuleWidget* host,ModuleWidget* neighbour, PortWidget* output, PortWidget* input) {
        if (isPortPatched(input)) {
            WARN("can't patch to input that is already patched");
            return;
        }
        rack::app::CableWidget* cw = new rack::app::CableWidget;
	    cw->inputPort = input;
        cw->outputPort = output;
	    cw->color = APP->scene->rack->getNextCableColor();
        cw->updateCable();
	    if (cw->isComplete())
            APP->scene->rack->addCable(cw);
    }

    static PortWidget* getInput(ModuleWidget* moduleWidget, int portId) {
        for (PortWidget* input : moduleWidget->getInputs()) {
            if (input->portId == portId) {
                return input;
            }
        }
        return nullptr;
    }

    static PortWidget* getOutput(ModuleWidget* moduleWidget, int portId) {
        for (PortWidget* output : moduleWidget->getOutputs()) {
            if (output->portId == portId) {
                return output;
            }
        }
        return nullptr;
    }
};

enum steppings {Off, quart, full};

/**
 */
struct PigeonPlinkModule : Module
{
public:
    int oldStepping;
    steppings stepping;
    std::vector<std::string> waveFormTxt = { "sine", "fold", "T/S"};
    std::vector<std::string> steppingTxt = { "None", "Legacy", "Legacy+Sub", "Octaves", "Digitone", "DX11", "DX7"};

    PigeonPlinkModule();
    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;
    json_t *dataToJson() override;
	void dataFromJson(json_t *) override;
    void onAdd(const AddEvent&) override;
    std::shared_ptr<Comp> wvco;
  
private:
    bool haveCheckedFormat = false;
    void checkForFormatUpgrade();
    void stampPatchAs2();

};

PigeonPlinkModule::PigeonPlinkModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    configBypass(Comp::LINEAR_FM_INPUT, Comp::MAIN_OUTPUT);
    configInput(Comp::GATE_INPUT,"Gate");
    configInput(Comp::LINEAR_FM_DEPTH_INPUT,"Linear Frequency Modulation Depth");
    configInput(Comp::FEEDBACK_INPUT,"Feedback");
    configInput(Comp::SHAPE_INPUT,"Shape");
    configInput(Comp::VOCT_INPUT,"1V/oct");
    configInput(Comp::FM_INPUT,"Frequency modulation");
    configInput(Comp::LINEAR_FM_INPUT,"Linear Frequency Modulation");
    configInput(Comp::SYNC_INPUT,"Sync");
    configInput(Comp::RATIO_INPUT,"Ratio Modulation");
    configOutput(Comp::MAIN_OUTPUT,"Audio");
    configParam(Comp::VCA_PARAM, .0f, 1.0f, 0.f, "VCA");
    configParam(Comp::FREQUENCY_MULTIPLIER_PARAM,0.f, 10.f, 1.f/3.2f, "Frequency Ratio");
    configParam(Comp::FINE_TUNE_PARAM,-12.0f, 12.0f, 0, "Fine tune");
    configParam(Comp::FM_DEPTH_PARAM,.0f, 100.0f, 0, "Frequency modulation");
    configParam(Comp::LINEAR_FM_DEPTH_PARAM,0, 100, 0, "Through-zero FM Depth");
    configParam(Comp::LINEAR_FM_PARAM,0, 100, 0, "Linear FM Modulation");
    configParam(Comp::WAVESHAPE_GAIN_PARAM,0, 100, 0, "Shape modulation");
    configParam(Comp::WAVE_SHAPE_PARAM,0, 2, 0, "Wave shape");
    configParam(Comp::FEEDBACK_PARAM,0, 100, 0, "FM Feedback depth");
    configParam(Comp::OUTPUT_LEVEL_PARAM,0, 100, 100, "output Level");
    configParam(Comp::LINEXP_PARAM,0, 1, 1, "Linear or Logarithmic");
    configParam(Comp::POSINV_PARAM,0, 1, 1, "Positive or Inverted");
    configParam(Comp::PATCH_VERSION_PARAM,0, 10, 0, "patch version");
    configButton(Comp::PARAM_STEPPING_DOWN, "Previous Stepping type");
    configButton(Comp::PARAM_STEPPING_UP, "Next Stepping type");
    configButton(Comp::PARAM_WAVEFORM_DOWN, "Previous Waveform type");
    configButton(Comp::PARAM_WAVEFORM_UP,"Next Waveform type");

    oldStepping = -1;
    wvco = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 
    onSampleRateChange();
    wvco->init();

    substituteDiscreteParamQuantity(Comp::getWaveformNames(), *this, Comp::WAVE_SHAPE_PARAM);
}

void PigeonPlinkModule::stampPatchAs2()
{
    // WARN("marking patch as 2.0\n");
    APP->engine->setParamValue(this, Comp::PATCH_VERSION_PARAM, 1); 
}

void PigeonPlinkModule::checkForFormatUpgrade()
{
    const bool needsUpdate = APP->engine->getParamValue(this, Comp::PATCH_VERSION_PARAM) < .5;
    if (!needsUpdate) {
        // WARN("checkForFormatUpgrade not needed");
        return;
    }

    const float waveformShapeF = APP->engine->getParamValue(this, Comp::WAVE_SHAPE_PARAM);
    const WVCODsp::WaveForm wf = WVCODsp::WaveForm( int( std::round(waveformShapeF))); 
    if (wf != WVCODsp::WaveForm::Fold) {
        // WARN("checkForFormatUpgrade not folder wf=%d, folder=%d\n", wf, WVCODsp::WaveForm::Fold);
        stampPatchAs2();
        return;
    }

    const float shapeGain = APP->engine->getParamValue(this, Comp::WAVESHAPE_GAIN_PARAM);
    const float newShapeGain = wvco->convertOldShapeGain(shapeGain);
    // WARN("gain was %f, now %f", shapeGain, newShapeGain);

    APP->engine->setParamValue(this, Comp::WAVESHAPE_GAIN_PARAM, newShapeGain);
    stampPatchAs2();
    // WARN("finished update");
}

void PigeonPlinkModule::onSampleRateChange()
{
}

void PigeonPlinkModule::step()
{
    if (!haveCheckedFormat) {
        // WARN("checking on first call to step\n");
        checkForFormatUpgrade();
        haveCheckedFormat = true;
    }
    if (wvco->steppingFromUI != oldStepping){
        substituteRatioParamQuantity(Comp::getRatioNames(wvco->R[wvco->steppingFromUI]), *this, Comp::FREQUENCY_MULTIPLIER_PARAM);
        oldStepping = wvco->steppingFromUI;
    }

    wvco->step();
}

json_t *PigeonPlinkModule::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, "JSON_WAVE", json_integer(static_cast<int>(wvco->wfFromUI)));
    json_object_set_new(rootJ, "JSON_STEPPING", json_integer(static_cast<int>(wvco->steppingFromUI)));
	return rootJ;
}

void PigeonPlinkModule::dataFromJson(json_t *rootJ) {
	json_t *nWaveJ = json_object_get(rootJ, "JSON_WAVE");
    json_t *nSteppingJ = json_object_get(rootJ, "JSON_STEPPING");
	if (nWaveJ) 
		wvco->wfFromUI = static_cast<int>(json_integer_value(nWaveJ));
    if (nSteppingJ)
        wvco->steppingFromUI = static_cast<int>(json_integer_value(nSteppingJ));
}

void PigeonPlinkModule::onAdd(const rack::engine::Module::AddEvent& e) {
	std::string configPath = asset::user("RPJ.json");
	FILE* file = std::fopen(configPath.c_str(), "r");

	//	throw Exception("Could not open autosave patch %s", configPath.c_str());
    if (file) {
        INFO("Loading config file %s", configPath.c_str());
	    json_error_t error;
	    json_t* rootJ = json_loadf(file, 0, &error);
        
	    if (!rootJ) {
	        throw Exception("Failed to load config. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
            fclose(file);
        }
        steppingTxt = {};
        wvco->R = {};
        json_t *nSteppingJ = json_object_get(rootJ, "steppings");
	    if (nSteppingJ) {
            size_t nrSteppings = json_array_size(nSteppingJ);
            for (int i=0;i<(int)nrSteppings;i++){
                json_t* stepping = json_array_get(nSteppingJ,i);
                if (stepping) {
                    json_t *nNameJ = json_object_get(stepping, "name");
                    if (nNameJ) 
                        steppingTxt.push_back(json_string_value(nNameJ));
                    json_t *nValuesJ = json_object_get(stepping, "values");
                    if (nValuesJ) {
                        size_t nrValues = json_array_size(nValuesJ);
                        std::vector<float> tmp_vec;
                        for (int j=0;j<(int)nrValues;j++){
                            json_t* nValueJ = json_array_get(nValuesJ,j);
                            tmp_vec.push_back(static_cast<float>(json_number_value(nValueJ)));
                        }
                        wvco->R.push_back(tmp_vec);
                    }
                }
            }
            fclose(file);
            json_decref(nSteppingJ);
        }
    }
    else
        INFO("Config file %s does not exist, using default settings", configPath.c_str());
    Module::onAdd(e);
}

////////////////////
// module widget
////////////////////

struct PigeonPlinkWidget : ModuleWidget
{
    PigeonPlinkWidget(PigeonPlinkModule *);
    void appendContextMenu(Menu *menu) override;

 #ifdef _TEXT
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
 #endif

    void addDisplays(PigeonPlinkModule *module, std::shared_ptr<IComposite> icomp);
    void addKnobs(PigeonPlinkModule *module, std::shared_ptr<IComposite> icomp);
    void addJacks(PigeonPlinkModule *module, std::shared_ptr<IComposite> icomp);
    void addButtons(PigeonPlinkModule *module, std::shared_ptr<IComposite> icomp);

    PigeonPlinkModule* module = nullptr;
};

void PigeonPlinkWidget::appendContextMenu(Menu *menu)
{
    MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);
    
    if (WvcoPatcher::shouldShowMenu(module)) {
        auto item = new SqMenuItem( []() { return false; }, [this](){
        assert(module);
        WvcoPatcher::go(this, module);
        });

        item->text = "Hookup Modulator";
        menu->addChild(item);
    }
}

const float dispX1 = 138;
const float dispY1 = 40;
const float dispDeltaY = 48;
const float dispY2 = dispY1 + dispDeltaY;

struct RPJDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	PigeonPlinkModule* module;
	const int fh = 12; // font height

	RPJDisplay(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(0xff, 0xff, 0xff, 0xff);
		//font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	RPJDisplay(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(r, g, b, a);
		//font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (layer == 1) {
			char tbuf[11];
			if (module == NULL)
				std::snprintf(tbuf, sizeof(tbuf), "%s", "fly away");
			else 
				std::snprintf(tbuf, sizeof(tbuf), "%s", &module->waveFormTxt[module->wvco->wfFromUI][0]);
		
			TransparentWidget::draw(args);
			drawBackground(args);
			drawValue(args, tbuf);
		}
		TransparentWidget::drawLayer(args,layer);
	}

	void drawBackground(const DrawArgs &args) {
		Vec c = Vec(box.size.x/2, box.size.y);
		int whalf = 2.25*box.size.x;
		int hfh = floor(fh / 2);

		// Draw rounded rectangle
		nvgFillColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, c.x -whalf, c.y +2);
			nvgLineTo(args.vg, c.x +whalf, c.y +2);
			nvgQuadTo(args.vg, c.x +whalf +5, c.y +2+hfh, c.x +whalf, c.y+fh+2);
			nvgLineTo(args.vg, c.x -whalf, c.y+fh+2);
			nvgQuadTo(args.vg, c.x -whalf -5, c.y +2+hfh, c.x -whalf, c.y +2);
			nvgClosePath(args.vg);
		}
		nvgFill(args.vg);
		nvgStrokeColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0x0F));
		nvgStrokeWidth(args.vg, 1.f);
		nvgStroke(args.vg);
	}
    
	void drawValue(const DrawArgs &args, const char * txt) {
		Vec c = Vec(box.size.x/2, box.size.y);
		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));

		nvgFontSize(args.vg, fh);
		if (font)
			nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));
		nvgText(args.vg, c.x, c.y+fh-1, txt, NULL);
	}
};


struct RPJSteppingDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	PigeonPlinkModule* module;
	const int fh = 12; // font height

	RPJSteppingDisplay(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(0xff, 0xff, 0xff, 0xff);
		//font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	RPJSteppingDisplay(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(r, g, b, a);
		//font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (layer == 1) {
			char tbuf[11];
			if (module == NULL)
				std::snprintf(tbuf, sizeof(tbuf), "%s", "fly away");
			else 
				std::snprintf(tbuf, sizeof(tbuf), "%s", &module->steppingTxt[module->wvco->steppingFromUI][0]);
		
			TransparentWidget::draw(args);
			drawBackground(args);
			drawValue(args, tbuf);
		}
		TransparentWidget::drawLayer(args,layer);
	}

	void drawBackground(const DrawArgs &args) {
		Vec c = Vec(box.size.x/2, box.size.y);
		int whalf = 2.25*box.size.x;
		int hfh = floor(fh / 2);

		// Draw rounded rectangle
		nvgFillColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, c.x -whalf, c.y +2);
			nvgLineTo(args.vg, c.x +whalf, c.y +2);
			nvgQuadTo(args.vg, c.x +whalf +5, c.y +2+hfh, c.x +whalf, c.y+fh+2);
			nvgLineTo(args.vg, c.x -whalf, c.y+fh+2);
			nvgQuadTo(args.vg, c.x -whalf -5, c.y +2+hfh, c.x -whalf, c.y +2);
			nvgClosePath(args.vg);
		}
		nvgFill(args.vg);
		nvgStrokeColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0x0F));
		nvgStrokeWidth(args.vg, 1.f);
		nvgStroke(args.vg);
	}

	void drawValue(const DrawArgs &args, const char * txt) {
		Vec c = Vec(box.size.x/2, box.size.y);
		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));

		nvgFontSize(args.vg, fh);
		if (font)
			nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));
		nvgText(args.vg, c.x, c.y+fh-1, txt, NULL);
	}
};

void PigeonPlinkWidget::addDisplays(PigeonPlinkModule *module, std::shared_ptr<IComposite> icomp) {
	RPJDisplay * wfd = new RPJDisplay(Vec(dispX1,dispY1));
    wfd->module = module;
	addChild(wfd);
    
    RPJSteppingDisplay * std = new RPJSteppingDisplay(Vec(dispX1,dispY2));
	std->module = module;
    addChild(std);
    
}

//const float knobLeftEdge = 24;
//const float knobDeltaX = 46;
const float knobX1 = 9;
const float knobDeltaX = 53;
const float knobX2 = knobX1 + knobDeltaX;
const float knobX3 = knobX2 + knobDeltaX;
const float knobX4 = knobX3 + knobDeltaX;

const float knobY1 = 55;
//const float knobDeltaY = 70;
const float knobY2 = 145;
const float knobY3 = 245;
//const float trimY = 276;
//const float trimX = 52;
//const float labelAboveKnob = 20;

void PigeonPlinkWidget::addKnobs(PigeonPlinkModule *module, std::shared_ptr<IComposite> icomp) {
  
    // The little buttons to control the displays
    addParam(createParam<ButtonMinBig>(Vec(100,52),module, Comp::PARAM_WAVEFORM_DOWN));
    addParam(createParam<ButtonPlusBig>(Vec(173,52),module, Comp::PARAM_WAVEFORM_UP));
    addParam(createParam<ButtonMinBig>(Vec(100,100),module, Comp::PARAM_STEPPING_DOWN));
    addParam(createParam<ButtonPlusBig>(Vec(173,100),module, Comp::PARAM_STEPPING_UP));

    // first row
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX1, knobY2),
        module,
        Comp::VCA_PARAM));

    addParam(SqHelper::createParam<RPJKnobBig>(
        icomp,
        Vec(knobX2-6, knobY1),
        module,
        Comp::FREQUENCY_MULTIPLIER_PARAM));

    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX4, knobY2),
        module,
        Comp::FINE_TUNE_PARAM));

    addParam(SqHelper::createParam<Toggle2P>(
        icomp,
        Vec(knobX2-3, knobY2), 
        module, 
        Comp::LINEXP_PARAM));

    addParam(SqHelper::createParam<Toggle2P>(
        icomp,
        Vec(knobX3-3, knobY2), 
        module, 
        Comp::POSINV_PARAM));

    // second row
    // 1 level
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX3, knobY3),
        module,
        Comp::OUTPUT_LEVEL_PARAM));

    // 2 fm-0
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX1, knobY3),
        module,
        Comp::LINEAR_FM_DEPTH_PARAM));
  
      addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX2, knobY3),
        module,
        Comp::FM_DEPTH_PARAM));

  // 3 Fdbk
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX3, knobY3),
        module,
        Comp::FEEDBACK_PARAM));

    // 4 SHAPE
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX4, knobY3),
        module,
        Comp::WAVESHAPE_GAIN_PARAM));

    // third row
}

//const float switchRow = 164;
//const float buttonXShift = 3;

const float jacksX1 = 11;
const float jacksDeltaX = 53;
const float jacksX2 = jacksX1 + jacksDeltaX;
const float jacksX3 = jacksX2 + jacksDeltaX;
const float jacksX4 = jacksX3 + jacksDeltaX;

const float jacksY0 = 62;
//const float jacksY1 = 120;
const float jacksY2 = 174;
const float jacksY3 = 276;
const float jacksY4 = 327;

void PigeonPlinkWidget::addJacks(PigeonPlinkModule *module, std::shared_ptr<IComposite> icomp) {

    //-------------------------------- first row ----------------------------------
    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY0),
        module,
        Comp::RATIO_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY2),
        module,
        Comp::GATE_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY3),
        module,
        Comp::LINEAR_FM_DEPTH_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jacksX3, jacksY3),
        module,
        Comp::FEEDBACK_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jacksX4, jacksY3),
        module,
        Comp::SHAPE_INPUT));

    //----------------------------- second row -----------------------
    addInput(createInput<PJ301MPort>(
        Vec(jacksX2, jacksY4),
        module,
        Comp::VOCT_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jacksX2, jacksY3),
        module,
        Comp::FM_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY4),
        module,
        Comp::LINEAR_FM_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jacksX3, jacksY4),
        module,
        Comp::SYNC_INPUT));

    addOutput(createOutput<PJ301MPort>(
        Vec(jacksX4, jacksY4),
        module,
        Comp::MAIN_OUTPUT));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

PigeonPlinkWidget::PigeonPlinkWidget(PigeonPlinkModule *mod) : module(mod)
{
    setModule(module);
    SqHelper::setPanel(this, "res/pigeonplink.svg");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addDisplays(module, icomp);
    addKnobs(module, icomp);
    addJacks(module, icomp);

}

Model *modelPigeonPlink = createModel<PigeonPlinkModule, PigeonPlinkWidget>("PigeonPlink");


