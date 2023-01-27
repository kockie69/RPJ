#pragma once

#include "rack.hpp"
#include "settings.hpp"
#include "../../composites/WidgetComposite.h"
#include "SqHelper.h"
#include "SqUI.h"
#include <functional>

/**
 * Like RoganSLBlue40, but smaller.
 */

struct RoganSLBlue30 : Rogan
{
    RoganSLBlue30()
    {
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
		setSvg(SqHelper::loadSvg("res/RoganSLBlue30.svg"));
		bg->setSvg(SqHelper::loadSvg("res/RoganSLBlue30-bg.svg"));
		fg->setSvg(SqHelper::loadSvg("res/RoganSLBlue30-fg.svg"));
    }
};

struct RoganSLBlue40 : Rogan
{
    RoganSLBlue40()
    {
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
		setSvg(SqHelper::loadSvg("res/RoganSLBlue40.svg"));
		bg->setSvg(SqHelper::loadSvg("res/RoganSLBlue40-bg.svg"));
		fg->setSvg(SqHelper::loadSvg("res/RoganSLBlue40-fg.svg"));
    }
};

struct Blue30SnapKnob : RoganSLBlue30
{
    Blue30SnapKnob()
    {
        snap = true;
        smooth = false;
    }
};

struct Blue40SnapKnob : RoganSLBlue40
{
    Blue40SnapKnob()
    {
        snap = true;
        smooth = false;
    }
};

struct NKKSmall : SvgSwitch
{
    NKKSmall()
    {
        addFrame(SqHelper::loadSvg("res/NKKSmall_0.svg"));
        addFrame(SqHelper::loadSvg("res/NKKSmall_1.svg"));
        addFrame(SqHelper::loadSvg("res/NKKSmall_2.svg"));
    }
};

struct BlueToggle : public SvgSwitch
{
    BlueToggle()
    {
        addFrame(SqHelper::loadSvg("res/BluePush_1.svg"));
        addFrame(SqHelper::loadSvg("res/BluePush_0.svg"));
    }
};

struct SqTrimpot24 : Trimpot {
	SqTrimpot24() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
        //setSvg(SqHelper::loadSvg("res/trimpot-24.svg"));
	}
};


class SqPortBase : public app::SvgPort {
private:
//    ui::Tooltip* tooltip = NULL;
    std::string toolTipString;
};

class SqOutputJack : public SqPortBase {
public:
	SqOutputJack() {
        setSvg(SqHelper::loadSvg("res/jack-24-medium.svg"));
	}

};

class SqInputJack : public SqPortBase {
public:
	SqInputJack() {
        setSvg(SqHelper::loadSvg("res/jack-24-light.svg"));
	}
};

/**
 * A very basic momentary push button.
 */
struct SQPush : SvgButton
{
    CircularShadow* shadowToDelete = nullptr;

    SQPush()
    {
        // we don't want the shadow to show, so remove it from the tree
        shadowToDelete = this->shadow;
        this->fb->removeChild(shadowToDelete);

        // But the widget still references it so we just need to 
        // delete it later
        addFrame(SqHelper::loadSvg("res/BluePush_0.svg"));
        addFrame(SqHelper::loadSvg("res/BluePush_1.svg"));
    }

    ~SQPush()
    {
        if (shadowToDelete) {
            delete shadowToDelete;
        }
    }


    SQPush(const char* upSVG, const char* dnSVG)
    {
        shadowToDelete = this->shadow;
        this->fb->removeChild(shadowToDelete);

        addFrame(SqHelper::loadSvg(upSVG));
        addFrame(SqHelper::loadSvg(dnSVG));
    }

    void center(Vec& pos)
    {
        this->box.pos = pos.minus(this->box.size.div(2));
    }

    void onButton(const event::Button &e) override
    {
        //only pick the mouse events we care about.
        // TODO: should our buttons be on release, like normal buttons?
        // Probably should use drag end
        if ((e.button != GLFW_MOUSE_BUTTON_LEFT) ||
            e.action != GLFW_RELEASE) {
                return;
            }

        if (clickHandler) {
            clickHandler();
        }
        sq::consumeEvent(&e, this);
    }

    /**
     * User of button passes in a callback lamba here
     */
    void onClick(std::function<void(void)> callback)
    {
        clickHandler = callback;
    }

    std::function<void(void)> clickHandler;
};

/**************************************************************************
 **
 ** SQPanelItem
 ** a menu item made for expaning panels.
 **
 ************************************************************************/

using SQStatusCallback = std::function<bool()>;
using SQActionCAllback = std::function<void()>;

struct SQPanelItem : MenuItem
{

    SQPanelItem(SQStatusCallback, SQActionCAllback);

    void onAction(const event::Action &e) override
    {
        actionCallback();
    }

    void step() override
    {
        const bool b = statusCallback();
        rightText = CHECKMARK(b);
    }

    SQStatusCallback statusCallback;
    SQActionCAllback actionCallback;
};

inline SQPanelItem::SQPanelItem(SQStatusCallback s, SQActionCAllback a) :
    statusCallback(s),
    actionCallback(a)
{
    text = "Expanded Panel";
}

/**
 * A Widget that holds values and will serialize,
 * But doesn't interacts with mouse.
 */
class NullWidget : public ParamWidget
{
public:

    NullWidget() : ParamWidget()
    {
        // Make sure we have no dimensions to fool 
        // hit testing in VCV.
        box.pos.x = 0;
        box.pos.y = 0;
        box.size.x = 0;
        box.size.y = 0;
    }

    // TODO: port this
#if 0
    // Swallow mouse commands
    void onMouseDown(EventMouseDown &e) override
    {
        e.consumed = false;
    }
#endif

};


/**
 * From VCV Mutes, just a light that is a specific size
 */
template <typename BASE>
struct MuteLight : BASE {
	MuteLight() {
	  this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

struct SquinkyLight : GrayModuleLightWidget {
	SquinkyLight() {
		addBaseColor(SqHelper::COLOR_SQUINKY);
	}
};

/**
 * our enlarged version of the stock bezel
 */
struct LEDBezelLG : app::SvgSwitch {
	LEDBezelLG() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LEDBezelLG.svg")));
	}
};

