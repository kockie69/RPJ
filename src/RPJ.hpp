#include "rack.hpp"

using namespace rack;

extern Plugin *pluginInstance;
extern Model *modelLFO;
extern Model *modelLavender;
extern Model *modelEaster;
extern Model *modelDryLand;
extern Model *modelTheWeb;
extern Model *modelGazpacho;
extern Model *modelEstonia;
extern Model *modelBrave;
extern Model *modelEssence;
extern Model *modelLadyNina;
extern Model *modelSugarMice;
extern Model *modelMontreal;
extern Model *modelBlindCurve;

struct RPJTextLabel : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	char text[128];
	int fh = 14;

	RPJTextLabel(Vec);

	RPJTextLabel(Vec, int);

	RPJTextLabel(Vec, unsigned char, unsigned char, unsigned char, unsigned char);

	void setColor(unsigned char, unsigned char, unsigned char, unsigned char);

	void setText(const char *);

	void drawBG(const DrawArgs &);

	void drawTxt(const DrawArgs &, const char *);

	void draw(const DrawArgs &) override;
};

struct ATitle: TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	char text[128];
	int fh = 25;
	float parentW = 0;

	ATitle(float pW) {
		parentW = pW;
		box.pos = Vec(1 , 3);
		box.size.y = fh;
		setColor(0x00, 0x00, 0x00, 0xFF);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Acme-Regular.ttf"));
		setText(" ");
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	void setText(const char * txt) {
		strncpy(text, txt, sizeof(text));
		box.size.x = strlen(text) * 10;
	}

	void draw(const DrawArgs &args) override {
		TransparentWidget::draw(args);
		drawTxt(args, text);
	}

	void drawTxt(const DrawArgs &args, const char * txt) {
		float bounds[4];
		Vec c = Vec(box.pos.x, box.pos.y);

		nvgFontSize(args.vg, fh);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgTextAlign(args.vg, NVG_ALIGN_LEFT);

		// CHECK WHETHER TEXT FITS IN THE MODULE
		nvgTextBounds(args.vg, c.x, c.y, txt, NULL, bounds);
		float xmax = bounds[2];
		if (xmax > parentW) {
			float ratio = parentW / xmax;
			fh = (int)floor(ratio * fh); // reduce fontsize to fit the parent width
		} else {
			c.x += (parentW - xmax)/2; // center text
		}

		nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));
		nvgText(args.vg, c.x, c.y+fh, txt, NULL);
	}

};

