#include "rack.hpp"
#include "Display.hpp"

using namespace rack;

const int WIDTH=400;
const int HEIGHT=370;

struct Root;
struct GenieExpander;

struct XpanderPairs {
	std::pair<float, float> edges[4][2];
    double weight;
	int nrOfPendulums;
};

struct Joint: Widget {
	    private:
		NVGcolor jointColor;
        float thick;
		Vec positionBegin,positionEnd;
		GenieExpander* module;
	public:
		Joint(GenieExpander *);
    	void drawLayer(const DrawArgs &args,int) override;
		void setBegin(Vec);
		void setEnd(Vec);
        int elapsed=0;
};

struct GenieExpander : Module {

	enum ParamIds {
		PARAM_HISTORY,
		PARAM_HISTORYTIMER,
		PARAM_PEND_1_X,
		PARAM_PEND_1_Y,
		PARAM_PEND_2_X,
		PARAM_PEND_2_Y,
		PARAM_PEND_3_X,
		PARAM_PEND_3_Y,
		PARAM_PEND_4_X,
		PARAM_PEND_4_Y,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_1_X,
		INPUT_1_Y,
		INPUT_2_X,
		INPUT_2_Y,
		INPUT_3_X,
		INPUT_3_Y,
		INPUT_4_X,
		INPUT_4_Y,
		NUM_INPUTS,
	};

	enum OutputIds {

		NUM_OUTPUTS,
	};

	enum LightIds {
		NUM_LIGHTS
	};

	public:
		GenieExpander();
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		void process(const ProcessArgs &) override;
        int getPendulums();
		int nrOfPendulums;
		int nrOfNodes;
		std::pair<float, float> edges[4][2];
		NVGcolor nodeColors[5];
		float weight;
		bool parentConnected;
		dsp::ClockDivider historyTimer;
        XpanderPairs* rdMsg;
        bool drawLines;
        int maxHistory;
        float swarmThickness;
		Vec XY[4];
		Vec prevXY[4];
		float _sampleR,_sampleG,_sampleB;
		float colors[4][3];
		float jointColor[3];
	private:
};

struct GenieDisplay : OpaqueWidget {
	GenieDisplay();
	void process();
	void drawLayer(const DrawArgs &args,int) override;
	void onDragHover(const DragHoverEvent&) override;
	void step() override;
	void root2Mass(Root*);
	float xpos, ypos;
    GenieExpander* module;
	Root* roots[4];
};

struct GenieExpanderModuleWidget : ModuleWidget {
	GenieExpanderModuleWidget(GenieExpander*);
	void appendContextMenu(Menu *) override;
	void onDragHover(const DragHoverEvent &) override;
	GenieDisplay *display;
};

struct Mass : Widget {
    private:
		NVGcolor massColor;
	public:
		Mass(GenieExpander*,int,int);
    	void drawLayer(const DrawArgs &args,int) override;
		//void onDragHover(const DragHoverEvent &) override;
		void onDragEnd(const DragEndEvent &) override;
    	void setColor(NVGcolor massColor);
		void step() override;
        int history;
        int elapsed;
		int node; //Is this really needed?
		float weight;
};

struct Root : SvgWidget {
    private:
		NVGcolor massColor;
	public:
		Root(GenieExpander*,int);
		void onDragHover(const DragHoverEvent &) override;
		void drawLayer(const DrawArgs &,int) override;
		void step() override;
    	void setColor(NVGcolor massColor);
		Vec* rootPos;
        int history;
        int elapsed;
		int node;
		float weight;
};

struct ColorQuantity : Quantity {
	GenieExpander* _module;

	float getMinValue() override { return 0.0f; }
	float getMaxValue() override { return 255.0f; }
	float getDefaultValue() override { return 100.0f; }
	float getDisplayValue() override { return roundf(getValue()); }
	void setDisplayValue(float displayValue) override { setValue(displayValue); }
	std::string getUnit() override { return ""; }
};

struct ColorQuantityLineR : ColorQuantity {
	GenieExpander* _module;
	int node;

	ColorQuantityLineR(GenieExpander* m) : _module(m) {}

	void setValue(float value) override {
		value = clamp(value, getMinValue(), getMaxValue());
		if (_module) {
			_module->jointColor[0] = value;
		}
	}

	float getValue() override {
		if (_module) {
			return _module->jointColor[0];
		}
		return getDefaultValue();
	}

	std::string getLabel() override { return "Red Color"; }
};

struct ColorQuantityLineG : ColorQuantity {
	GenieExpander* _module;
	int node;

	ColorQuantityLineG(GenieExpander* m) : _module(m) {}

	void setValue(float value) override {
		value = clamp(value, getMinValue(), getMaxValue());
		if (_module) {
			_module->jointColor[1] = value;
		}
	}

	float getValue() override {
		if (_module) {
			return _module->jointColor[1];
		}
		return getDefaultValue();
	}

	std::string getLabel() override { return "Red Color"; }
};

struct ColorQuantityLineB : ColorQuantity {
	GenieExpander* _module;
	int node;

	ColorQuantityLineB(GenieExpander* m) : _module(m) {}

	void setValue(float value) override {
		value = clamp(value, getMinValue(), getMaxValue());
		if (_module) {
			_module->jointColor[2] = value;
		}
	}

	float getValue() override {
		if (_module) {
			return _module->jointColor[2];
		}
		return getDefaultValue();
	}

	std::string getLabel() override { return "Red Color"; }
};

struct ColorQuantityR : ColorQuantity {
	GenieExpander* _module;
	int node;

	ColorQuantityR(GenieExpander* m,int n) : _module(m) {node=n;}

	void setValue(float value) override {
		value = clamp(value, getMinValue(), getMaxValue());
		if (_module) {
			_module->colors[node][0] = value;
		}
	}

	float getValue() override {
		if (_module) {
			return _module->colors[node][0];
		}
		return getDefaultValue();
	}

	std::string getLabel() override { return "Red Color"; }
};

struct ColorQuantityG : ColorQuantity {
	GenieExpander* _module;
	int node;

	ColorQuantityG(GenieExpander* m,int n) : _module(m) {node=n;}

	void setValue(float value) override {
		value = clamp(value, getMinValue(), getMaxValue());
		if (_module) {
			_module->colors[node][1] = value;
		}
	}

	float getValue() override {
		if (_module) {
			return _module->colors[node][1];
		}
		return getDefaultValue();
	}

	std::string getLabel() override { return "Green Color"; }
};

struct ColorQuantityB : ColorQuantity {
	GenieExpander* _module;
	int node;

	ColorQuantityB(GenieExpander* m,int n) : _module(m) {node=n;}

	void setValue(float value) override {
		value = clamp(value, getMinValue(), getMaxValue());
		if (_module) {
			_module->colors[node][2] = value;
		}
	}

	float getValue() override {
		if (_module) {
			return _module->colors[node][2];
		}
		return getDefaultValue();
	}

	std::string getLabel() override { return "Blue Color"; }
};

struct ColorSlider : ui::Slider {
	int colorPos;
	NVGcolor sliderColor;
	int node;

	void draw(const DrawArgs &args) override {
		ui::Slider::draw(args);
		nvgBeginPath(args.vg);
		nvgRect(args.vg,box.pos.x, 0, box.pos.x+box.size.x, box.size.y);
		switch (colorPos) {
			case 1:
				sliderColor = nvgRGBA(int(quantity->getValue()),0, 0, 160);
				break;
			case 2:
				sliderColor = nvgRGBA(0,int(quantity->getValue()), 0, 160);
				break;
			case 3:
				sliderColor = nvgRGBA(0,0,int(quantity->getValue()), 160);
				break;
			default:
				return;
		}
		nvgFillColor(args.vg,sliderColor);
		nvgFill(args.vg);
		nvgClosePath(args.vg);
		//ui::Slider::draw(args);
	}

	virtual ~ColorSlider() {
		delete quantity;
	}
};

struct ColorSliderLineR : ColorSlider {
	ColorSliderLineR(GenieExpander* module) {
		quantity = new ColorQuantityLineR(module);
		box.size.x = 200.0f;
		colorPos=1;
	}
};

struct ColorSliderLineG : ColorSlider {
	ColorSliderLineG(GenieExpander* module) {
		quantity = new ColorQuantityLineG(module);
		box.size.x = 200.0f;
		colorPos=1;
	}
};

struct ColorSliderLineB : ColorSlider {
	ColorSliderLineB(GenieExpander* module) {
		quantity = new ColorQuantityLineB(module);
		box.size.x = 200.0f;
		colorPos=1;
	}
};


struct ColorSliderR : ColorSlider {
	ColorSliderR(GenieExpander* module,int n) {
		quantity = new ColorQuantityR(module,n);
		box.size.x = 200.0f;
		colorPos=1;
	}
};

struct ColorSliderG : ColorSlider {
	ColorSliderG(GenieExpander* module,int n) {
		quantity = new ColorQuantityG(module,n);
		box.size.x = 200.0f;
		colorPos=2;
	}
};

struct ColorSliderB : ColorSlider {
	ColorSliderB(GenieExpander* module,int n) {
		quantity = new ColorQuantityB(module,n);
		box.size.x = 200.0f;
		colorPos = 3;
	}
};

struct colorMenuSlider : MenuItem {
	GenieExpander* _module;
	int node;

	colorMenuSlider(GenieExpander*, const char*,int n);

	Menu* createChildMenu() override; 
};

struct colorMenuSliderLine : MenuItem {
	GenieExpander* _module;

	colorMenuSliderLine(GenieExpander*, const char*);

	Menu* createChildMenu() override; 
};
