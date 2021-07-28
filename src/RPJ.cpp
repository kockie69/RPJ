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
	p->addModel(modelDrillingHoles);
}

RPJTextLabel::RPJTextLabel(Vec pos) {
	box.pos = pos;
	box.size.y = fh;
	moduleWidth = 0;
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

RPJTextLabel::RPJTextLabel(Vec pos, int h, int mw) {
	box.pos = pos;
    fh = h;
	box.size.y = fh;
	moduleWidth = mw;
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
	text[127]= '\0';
	if (moduleWidth)
		box.size.x = moduleWidth * RACK_GRID_WIDTH;
	else
		box.size.x = 7 * strlen(text);
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

RPJTitle::RPJTitle(float pW, int mW) {
	parentW = pW;
	box.pos = Vec(1 , 3);
	box.size.y = fh;
	moduleWidth = mW;
	setColor(0x00, 0x00, 0x00, 0xFF);
	font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Acme-Regular.ttf"));
	setText(" ");
}

void RPJTitle::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {		
	txtCol.r = r;		
	txtCol.g = g;
	txtCol.b = b;
	txtCol.a = a;
}

void RPJTitle::setText(const char * txt) {
	strncpy(text, txt, sizeof(text)-1);
	text[127]= '\0';
	box.size.x = strlen(text) * moduleWidth;
}

void RPJTitle::draw(const DrawArgs &args) {
	TransparentWidget::draw(args);
	drawTxt(args, text);
}

void RPJTitle::drawTxt(const DrawArgs &args, const char * txt) {
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
