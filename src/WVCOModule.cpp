
#include <sstream>
#include "RPJ.hpp"
#include "../composites/WidgetComposite.h"

#include "../composites/WVCO.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/RPJKnobs.hpp"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/PopupMenuParamWidget.h"
#include "ctrl/RPJButtons.hpp"

using Comp = WVCO<WidgetComposite>;
using Input = ::rack::engine::Input;

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

inline void subsituteDiscreteParamQuantity(const std::vector<std::string>& strings, Module& module, unsigned int paramNumber) {
    auto orig = module.paramQuantities[paramNumber];

     auto p = new DiscreteParamQuantity(*orig, strings);
    
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

        // RPJ: assert(myFMPort->type == PortWidget::INPUT);
        // RPJ: assert(otherOutput->type == PortWidget::OUTPUT);

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
        // RPJ: assert(thisInput->type == PortWidget::INPUT);
        auto cables = APP->scene->rack->getCablesOnPort(thisInput);
        assert(cables.size() == 1);
        auto cable = cables.begin();
        CableWidget* cw = *cable;
        PortWidget* ret =  cw->outputPort;
        // RPJ: assert(ret->type == PortWidget::OUTPUT);
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
        rack::engine::Cable* cable = new rack::engine::Cable;
        cable->inputModule=host->getModule();
        cable->outputModule=neighbour->getModule();
        cable->id=-1;      
        cable->outputId=output->portId;
        cable->inputId=input->portId;
        APP->engine->addCable(cable);
        rack::app::CableWidget* cw = new rack::app::CableWidget;
	    cw->setCable(cable);
	    cw->color = APP->scene->rack->getNextCableColor();
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
struct WVCOModule : Module
{
public:
    steppings stepping;
    std::string waveFormTxt[3] = { "sine", "fold", "T/S"};
    std::string steppingTxt[6] = { "Legacy", "Legacy + SubOctaves", "Octaves", "Digitone Operator", "Yamaha TX81Z", "Yamaha DX7"};

    WVCOModule();
    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;
    std::shared_ptr<Comp> wvco;
  
private:
    bool haveCheckedFormat = false;
    void checkForFormatUpgrade();
    void stampPatchAs2();

};

WVCOModule::WVCOModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    configInput(Comp::GATE_INPUT,"Gate");
    configInput(Comp::LINEAR_FM_DEPTH_INPUT,"Linear Frequency Modulation Depth");
    configInput(Comp::FEEDBACK_INPUT,"Feedback");
    configInput(Comp::SHAPE_INPUT,"Shape");
    configInput(Comp::VOCT_INPUT,"1V/oct");
    configInput(Comp::FM_INPUT,"Frequency modulation");
    configInput(Comp::LINEAR_FM_INPUT,"Linear Frequency Modulation");
    configInput(Comp::SYNC_INPUT,"Sync");
    configOutput(Comp::MAIN_OUTPUT,"Audio");

    wvco = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    wvco->init();

    subsituteDiscreteParamQuantity(Comp::getWaveformNames(), *this, Comp::WAVE_SHAPE_PARAM);
}

void WVCOModule::stampPatchAs2()
{
    // WARN("marking patch as 2.0\n");
    APP->engine->setParamValue(this, Comp::PATCH_VERSION_PARAM, 1); 
}

void WVCOModule::checkForFormatUpgrade()
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

void WVCOModule::onSampleRateChange()
{
}

void WVCOModule::step()
{
    if (!haveCheckedFormat) {
        // WARN("checking on first call to step\n");
        checkForFormatUpgrade();
        haveCheckedFormat = true;
    }

    wvco->step();
}

////////////////////
// module widget
////////////////////

struct WVCOWidget : ModuleWidget
{
    WVCOWidget(WVCOModule *);
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

    void addDisplays(WVCOModule *module, std::shared_ptr<IComposite> icomp);
    void addKnobs(WVCOModule *module, std::shared_ptr<IComposite> icomp);
    void addJacks(WVCOModule *module, std::shared_ptr<IComposite> icomp);
    void addButtons(WVCOModule *module, std::shared_ptr<IComposite> icomp);

    WVCOModule* module = nullptr;
};

void WVCOWidget::appendContextMenu(Menu *menu)
{
    MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);
    
#if 0
    MenuLabel *spacerLabel2 = new MenuLabel();
    menu->addChild(spacerLabel2);
    SqMenuItem_BooleanParam2 * item = new SqMenuItem_BooleanParam2(module, Comp::SNAP_PARAM);
    item->text = "Envelope \"Snap\"";
    menu->addChild(item);
    item = new SqMenuItem_BooleanParam2(module, Comp::SNAP2_PARAM);
    item->text = "Extra Envelope \"Snap\"";
    menu->addChild(item);
#endif

    menu->addChild(createIndexPtrSubmenuItem("Stepping", {"Off", "Quart", "Full"}, &module->stepping));

    {
        if (WvcoPatcher::shouldShowMenu(module)) {
            auto item = new SqMenuItem( []() { return false; }, [this](){
                assert(module);
                WvcoPatcher::go(this, module);
            });

            item->text = "Hookup Modulator";
            menu->addChild(item);
        }
    }
}

const float dispX1 = 150;
const float dispY1 = 40;
const float dispDeltaY = 48;
const float dispY2 = dispY1 + dispDeltaY;

struct RPJDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	WVCOModule* module;
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
	WVCOModule* module;
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

void WVCOWidget::addDisplays(WVCOModule *module, std::shared_ptr<IComposite> icomp) {
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
const float knobDeltaY = 70;
const float knobY2 = 145;
const float knobY3 = 245;
const float trimY = 276;
const float trimX = 52;
//const float labelAboveKnob = 20;

class SqBlueButton : public ToggleButton 
{
public:
    SqBlueButton()
    {
        addSvg("res/oval-button-up.svg");
        addSvg("res/oval-button-down.svg");
    }
};

void WVCOWidget::addKnobs(WVCOModule *module, std::shared_ptr<IComposite> icomp) {

    addParam(createParam<buttonMinSmall>(Vec(117,54),module, Comp::PARAM_WAVEFORM_DOWN));
	addParam(createParam<buttonPlusSmall>(Vec(185,54),module, Comp::PARAM_WAVEFORM_UP));
	addParam(createParam<buttonMinSmall>(Vec(117,103),module, Comp::PARAM_STEPPING_DOWN));
	addParam(createParam<buttonPlusSmall>(Vec(185,103),module, Comp::PARAM_STEPPING_UP));
    
    // first row
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX1, knobY2),
        module,
        Comp::VCA_PARAM));

    //addLabel(Vec(knobX1 - 13, knobY1 - labelAboveKnob), "Octave");

    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX2, knobY1),
        module,
        Comp::FREQUENCY_MULTIPLIER_PARAM));
        //RPJ: Ratio in KitchenSink
    //addLabel(Vec(knobX2 - 6, knobY1 - labelAboveKnob), "Ratio");

    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX4, knobY2),
        module,
        Comp::FINE_TUNE_PARAM));
    //addLabel(Vec(knobX3 - 4, knobY1 - labelAboveKnob), "Fine");

    addParam(SqHelper::createParam<Toggle2P>(
        icomp,
        Vec(knobX2, knobY2), 
        module, 
        Comp::LINEXP_PARAM));

    addParam(SqHelper::createParam<Toggle2P>(
        icomp,
        Vec(knobX3, knobY2), 
        module, 
        Comp::POSINV_PARAM));

    // second row
    // 1 level
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX3, knobY3),
        module,
        Comp::OUTPUT_LEVEL_PARAM));
    //addLabel(Vec(knobX1 - 8, knobY2 - labelAboveKnob), "Level");

    // 2 fm-0
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX1, knobY3),
        module,
        Comp::LINEAR_FM_DEPTH_PARAM));
    //addLabel(Vec(knobX2 - 10, knobY2 - labelAboveKnob), "Depth");
  
  // 3 Fdbk
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX3, knobY3),
        module,
        Comp::FEEDBACK_PARAM));
    //addLabel(Vec(knobX3 - 6, knobY2 - labelAboveKnob), "Fbck");

    // 4 SHAPE
    addParam(SqHelper::createParam<RPJKnob>(
        icomp,
        Vec(knobX4, knobY3),
        module,
        Comp::WAVESHAPE_GAIN_PARAM));
   // addLabel(Vec(knobX4 - 10, knobY2 - labelAboveKnob), "Shape");


    // third row
}

const float switchRow = 164;
const float buttonXShift = 3;

const float jacksX1 = 11;
const float jacksDeltaX = 53;
const float jacksX2 = jacksX1 + jacksDeltaX;
const float jacksX3 = jacksX2 + jacksDeltaX;
const float jacksX4 = jacksX3 + jacksDeltaX;

const float jacksY1 = 120;
const float jacksY2 = 174;
const float jacksY3 = 276;
const float jacksY4 = 327;

void WVCOWidget::addJacks(WVCOModule *module, std::shared_ptr<IComposite> icomp) {

    //-------------------------------- first row ----------------------------------
    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY2),
        module,
        Comp::GATE_INPUT));
    ///addLabel(Vec(jacksX1 - 8, jacksY1 - labelAboveKnob), "Gate");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY3),
        module,
        Comp::LINEAR_FM_DEPTH_INPUT));
    //addLabel(Vec(jacksX3 - 12, jacksY1 - labelAboveKnob), "Depth");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX3, jacksY3),
        module,
        Comp::FEEDBACK_INPUT));
    //addLabel(Vec(jacksX4 - 8, jacksY1 - labelAboveKnob), "Fbck");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX4, jacksY3),
        module,
        Comp::SHAPE_INPUT));
    //addLabel(Vec(jacksX5 - 12, jacksY1 - labelAboveKnob), "Shape");

    //----------------------------- second row -----------------------
    addInput(createInput<PJ301MPort>(
        Vec(jacksX2, jacksY4),
        module,
        Comp::VOCT_INPUT));
    //addLabel(Vec(jacksX1 - 11, jacksY2 - labelAboveKnob), "V/Oct");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY4),
        module,
        Comp::FM_INPUT));
    //addLabel(Vec(jacksX2 - 3, jacksY2 - labelAboveKnob), "FM");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX2, jacksY3),
        module,
        Comp::LINEAR_FM_INPUT));
    //addLabel(Vec(jacksX3 - 6, jacksY2 - labelAboveKnob), "LFM");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX4, jacksY4),
        module,
        Comp::SYNC_INPUT));
    //addLabel(Vec(jacksX4 - 9, jacksY2 - labelAboveKnob), "Sync");

    addOutput(createOutput<PJ301MPort>(
        Vec(jacksX4, jacksY4),
        module,
        Comp::MAIN_OUTPUT));
    //addLabel(Vec(jacksX5 - 7, jacksY2 - labelAboveKnob), "Out");
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

WVCOWidget::WVCOWidget(WVCOModule *mod) : module(mod)
{
    setModule(module);
  //  box.size = Vec(14 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/kitchen-sink-panel.svg");

    //addLabel(Vec(60, 14), "Kitchen Sink");

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

Model *modelWVCOModule = createModel<WVCOModule, WVCOWidget>("rpj-wvco");

