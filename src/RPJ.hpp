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
extern Model *modelDrillingHoles;
extern Model *modelTuxOn;

struct RPJTextLabel : TransparentWidget {

	RPJTextLabel(Vec);

	RPJTextLabel(Vec, int);

	RPJTextLabel(Vec, int, int);

	RPJTextLabel(Vec, unsigned char, unsigned char, unsigned char, unsigned char);

	void setColor(unsigned char, unsigned char, unsigned char, unsigned char);

	void setText(const char *);

	void drawBG(const DrawArgs &);

	void drawTxt(const DrawArgs &, const char *);

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


struct buttonPlusSmall : SvgSwitch  {
	buttonPlusSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonPlus_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonPlus_1.svg")));
	}

	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};

struct buttonMinSmall : SvgSwitch  {
	buttonMinSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonMin_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonMin_1.svg")));
	}
	
	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};


