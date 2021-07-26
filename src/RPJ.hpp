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
extern Model *modelGaza;
extern Model *modelCircularRide;

struct RPJTextLabel : TransparentWidget {

	RPJTextLabel(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		setColor(0x00, 0x00, 0x00, 0xFF);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
		setText(" ");
	}

	RPJTextLabel(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		box.pos = pos;
		box.size.y = fh;
		setColor(r, g, b, a);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
		setText(" ");
	}

	RPJTextLabel(Vec, int, int);

	RPJTextLabel(Vec, unsigned char, unsigned char, unsigned char, unsigned char);

	void setText(const char * txt) {
		strncpy(text, txt, sizeof(text));
		box.size.x = strlen(text) * 8;
	}

	void drawBG(const DrawArgs &args) {
		Vec c = Vec(box.size.x/2, box.size.y);
		const int whalf = box.size.x/2;

		// Draw rectangle
		nvgFillColor(args.vg, nvgRGBA(0xF0, 0xF0, 0xF0, 0xFF));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, c.x -whalf, c.y +2);
			nvgLineTo(args.vg, c.x +whalf, c.y +2);
			nvgLineTo(args.vg, c.x +whalf, c.y+fh+2);
			nvgLineTo(args.vg, c.x -whalf, c.y+fh+2);
			nvgLineTo(args.vg, c.x -whalf, c.y +2);
			nvgClosePath(args.vg);
		}
		nvgFill(args.vg);
	}

	void drawTxt(const DrawArgs &args, const char * txt) {

		Vec c = Vec(box.size.x/2, box.size.y);

		nvgFontSize(args.vg, fh);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));

	void draw(const DrawArgs &) override;

	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	char text[128];
	int fh=14;
	int moduleWidth;
};

struct RPJTitle: TransparentWidget {
	
	RPJTitle(float , int );

	void setColor(unsigned char , unsigned char , unsigned char , unsigned char );

	void setText(const char * );

	void draw(const DrawArgs &) override;

	void drawTxt(const DrawArgs &, const char * );

	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	//char text[128];
	char text[128];
	int fh = 25;
	float parentW = 0;
	int moduleWidth;
};

