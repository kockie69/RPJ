#include "RPJ.hpp"
Plugin *pluginInstance;
void init(Plugin *p) {
    pluginInstance = p;
    p->addModel(modelLFO);
    p->addModel(modelLavender);
    p->addModel(modelEaster);
    p->addModel(modelDryLand);
    p->addModel(modelTheWeb);
    p->addModel(modelGazpacho);    
    p->addModel(modelEstonia);
    p->addModel(modelBrave);  
    p->addModel(modelEssence);
    p->addModel(modelLadyNina);
    p->addModel(modelSugarMice);
    p->addModel(modelMontreal);
    p->addModel(modelBlindCurve);
	p->addModel(modelGaza);
	p->addModel(modelCircularRide);
}

RPJTextLabel::RPJTextLabel(Vec pos) {
	box.pos = pos;
	box.size.y = fh;
	setColor(0x00, 0x00, 0x00, 0xFF);
	font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	setText(" ");
}

RPJTextLabel::RPJTextLabel(Vec pos, int h) {
	box.pos = pos;
    fh = h;
	box.size.y = fh;
	setColor(0x00, 0x00, 0x00, 0xFF);
	font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	setText(" ");
}

RPJTextLabel::RPJTextLabel(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	box.pos = pos;
	box.size.y = fh;
	setColor(r, g, b, a);
	font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	setText(" ");
}

void RPJTextLabel::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	txtCol.r = r;
	txtCol.g = g;
	txtCol.b = b;
	txtCol.a = a;
}

void RPJTextLabel::setText(const char * txt) {
	strncpy(text, txt, sizeof(text)-1);
	box.size.x = strlen(text) * 8;
}

void RPJTextLabel::drawBG(const DrawArgs &args) {
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

void RPJTextLabel::drawTxt(const DrawArgs &args, const char * txt) {

	Vec c = Vec(box.size.x/2, box.size.y);

	nvgFontSize(args.vg, fh);
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, -2);
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));

	nvgText(args.vg, c.x, c.y+fh, txt, NULL);
}

void RPJTextLabel::draw(const DrawArgs &args) {
	TransparentWidget::draw(args);
	drawBG(args);
	drawTxt(args, text);
}


