#include "rack.hpp"

using namespace rack;

const int MODULE_WIDTH=3;

struct bridgeChannel {
	int channelId; // starts with 1 till nr of BridgeIns
	int64_t publisher; // module id of BridgeIn
	std::vector<int64_t> subscribers; // module ids of 0 or more subscribers
	bool isConnected;
};

struct Bridge : Module {
	const char* JSON_ID = "Bridge ID";
    static std::vector<bridgeChannel> bridgeBus;
};

std::vector<bridgeChannel> Bridge::bridgeBus;

template <typename T>
struct BridgeDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	T *module;
	const int fh = 12; // font height


	BridgeDisplay(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		box.size.x = 4;
		setColor(0xff, 0xff, 0xff, 0xff);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	BridgeDisplay(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(r, g, b, a);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (layer == 1) {
			char tbuf[3];
			if (module == NULL) return;
			std::snprintf(tbuf, sizeof(tbuf), "%d", *(&module->cid));	
			TransparentWidget::draw(args);
			drawBackground(args);
			drawValue(args, tbuf);
		}
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

		nvgFontSize(args.vg, fh);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));
		nvgText(args.vg, c.x, c.y+fh-1, txt, NULL);
	}
};

struct BridgeOut : Bridge {

	enum ParamIds {
		NUM_PARAMS,
	};

	enum InputIds {
		NUM_INPUTS,
	};

	enum OutputIds {
		POLYOUTPUT_A,
		POLYOUTPUT_B,
		POLYOUTPUT_C,
		POLYOUTPUT_D,
		POLYOUTPUT_E,
		POLYOUTPUT_F,
		POLYOUTPUT_G,
		POLYOUTPUT_H,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		ENUMS(RGB_LIGHT,3),
		NUM_LIGHTS,
	};
		BridgeOut();
		void process(const ProcessArgs &) override;
		void addSubscriber (int);
		void onRemove(const RemoveEvent &) override;
		void removeSubscriber(int64_t, int);
		void updateSubscriber(int);
		bool connected(int);
		uint64_t getPublisherId(int cid);
		rack::engine::Module* bridgeSource;
		int cid;
		json_t *rootJ2;
		rack::app::ModuleWidget* modwid;

	json_t* dataToJson() override {
		json_t *rootJ=json_object();
		json_object_set_new(rootJ, JSON_ID, json_integer(cid));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *nIdJ = json_object_get(rootJ, JSON_ID);
		if (nIdJ) {
			cid = (json_integer_value(nIdJ));
			if (cid!=0)
				BridgeOut::addSubscriber(cid);
		}
	}
};

struct BridgeIn : Bridge {

	enum ParamIds {
		NUM_PARAMS,
	};

	enum InputIds {
		POLYINPUT_A,
		POLYINPUT_B,
		POLYINPUT_C,
		POLYINPUT_D,
		POLYINPUT_E,
		POLYINPUT_F,
		POLYINPUT_G,
		POLYINPUT_H,
		NUM_INPUTS,
	};

	enum OutputIds {
		NUM_OUTPUTS,
	};

	enum LightsIds {
		ENUMS(RGB_LIGHT,3),
		NUM_LIGHTS,
	};
		
	BridgeIn();
	void onRemove(const RemoveEvent &) override;
	void removePublisher(int64_t, int);
	void process(const ProcessArgs &) override;
	void addPublisher(int);
	bool connected(int);
	bool idIsFree(int);
	void updatePublisher(int);
	int cid;
	json_t *rootJ2;
	rack::app::ModuleWidget* modwid;

	json_t* dataToJson() override {
		json_t *rootJ=json_object();
		json_object_set_new(rootJ, JSON_ID, json_integer(cid));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *nIdJ = json_object_get(rootJ, JSON_ID);
		if (nIdJ) {
			cid = (json_integer_value(nIdJ));
			if (cid!=0)
				BridgeIn::addPublisher(cid);
		}
	}
};

struct nBridgeInIdMenuItem : TextField {
	Menu* menu;
	BridgeIn *module;
	void onSelectKey(const SelectKeyEvent &) override;
};

struct nBridgeOutIdMenuItem : TextField {
	Menu* menu;
	BridgeOut *module;
	void onSelectKey(const SelectKeyEvent &) override;
};

