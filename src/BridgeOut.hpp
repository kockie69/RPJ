#include "rack.hpp"

using namespace rack;

const int MODULE_WIDTH=3;
const char* JSON_OUT_ID = "Bridge ID";
const char* JSON_OUT_CONNECTED = "Connected";

struct BridgeOut : Module {

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
		void findSource();
		void process(const ProcessArgs &) override;
		void processChannel(Output&);
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		rack::engine::Module* bridgeSource;
		int id;
		bool connected;
		rack::app::ModuleWidget* modwid;
		int64_t sourceId;
};

struct BridgeOutDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	BridgeOut* module;
	const int fh = 12; // font height


	BridgeOutDisplay(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		box.size.x = 4;
		setColor(0xff, 0xff, 0xff, 0xff);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	BridgeOutDisplay(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
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
			std::snprintf(tbuf, sizeof(tbuf), "%d", *(&module->id));	
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
