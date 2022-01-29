#include "rack.hpp"
#include "AudioFilter.hpp"

using namespace rack;

const int MODULE_WIDTH=6;
const char *JSON_FILTER_ALGORITHM_KEY="Algorithm";
		
struct Estonia : Module {

	enum ParamIds {
		PARAM_UP,
		PARAM_DOWN,
		PARAM_FC,
		PARAM_CVFC,
		PARAM_CVB,
		PARAM_BOOSTCUT_DB,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
		INPUT_CVB,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_MAIN,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		Estonia();
		void processChannel(int, Input&, Output&);
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		void onSampleRateChange() override;
		AudioFilter<rack::simd::float_4> audioFilter[4];
		void process(const ProcessArgs &) override;
		dsp::SchmittTrigger upTrigger,downTrigger;
		AudioFilterParameters afp;
		std::string strAlgorithm;
};

struct EstoniaFilterNameDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	Estonia* module;
	const int fh = 12; // font height


	EstoniaFilterNameDisplay(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(0xff, 0xff, 0xff, 0xff);
		//font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	EstoniaFilterNameDisplay(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
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
				std::snprintf(tbuf, sizeof(tbuf), "%s", "carry on");
			else
			 	std::snprintf(tbuf, sizeof(tbuf), "%s", &module->strAlgorithm[0]);
		
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

