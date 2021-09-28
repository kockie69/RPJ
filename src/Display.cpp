#include "Display.hpp"

Display::Display() {
		frame = 0;
		font = NULL;
		fontPath="";
		displayBuff.resize(1);	
}

void Display::setDisplayFont(Plugin *pluginInstance,std::string font) {
	fontPath = asset::plugin(pluginInstance, font);
}

void Display::setDisplayPos(float samplePos,float zoomBegin,float FrameCount) {
	displayPos = floor (((samplePos - zoomBegin) / FrameCount)* width);
}
void Display::setDisplayWidth(int width) {
	this->width = width;
}


void Display::setBegin(float beginRatio) {
	displayBegin = floor(beginRatio * width);
}

void Display::setEnd(float endRatio) {
	displayEnd = endRatio * width;
}

void Display::setDisplayBuff(float begin, float end, vector<vector<float>> playBuffer) {
	if (abs(((end-begin)/width))>=1) {
		vector<double>().swap(displayBuff);

		for (int i=floor(begin); i < floor(end); i = i + floor((end-begin)/width)) {
			float q = playBuffer[0][i];
			displayBuff.push_back(playBuffer[0][i]);
		}
	}
}

void Display::draw(const DrawArgs &args) {
	nvgGlobalTint(args.vg, color::WHITE);
	if (fontPath!="") {
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
		nvgFontSize(args.vg, 12);
		if (font)
			nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));	
		nvgTextBox(args.vg, 5, 5,215, fileDesc.c_str(), NULL);
	}
		
	// Draw ref line
	nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
	{
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0, 125);
		nvgLineTo(args.vg, width, 125);
		nvgClosePath(args.vg);
	}
	nvgStroke(args.vg);
		
	if (displayBuff.size()) {
		// Draw play line
		nvgStrokeColor(args.vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
        nvgStrokeWidth(args.vg, 0.8);
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, displayPos , 84);
			nvgLineTo(args.vg, displayPos , 164);

			nvgClosePath(args.vg);
		}
		nvgStroke(args.vg);
            
        // Draw start line
		nvgStrokeColor(args.vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
        nvgStrokeWidth(args.vg, 1.5);
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, displayBegin , 84);
			nvgLineTo(args.vg, displayBegin , 164);
			nvgClosePath(args.vg);
		}
		nvgStroke(args.vg);
            
		// Draw end line
		nvgStrokeColor(args.vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
        nvgStrokeWidth(args.vg, 1.5);
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, displayEnd , 84);
			nvgLineTo(args.vg, displayEnd , 164);
			nvgClosePath(args.vg);
		}
		nvgStroke(args.vg);
		
		// Draw waveform
		nvgStrokeColor(args.vg, nvgRGBA(0xe1, 0x02, 0x78, 0xc0));
		nvgSave(args.vg);
		rack::Rect b = Rect(Vec(0, 84), Vec(215, 80));
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgBeginPath(args.vg);
		for (unsigned int i = 0; i < displayBuff.size(); i++) {
			float x, y;
			x = (float)i / (displayBuff.size() - 1);
			y = displayBuff[i] + 0.5;
			Vec p;
			p.x = b.pos.x + b.size.x * x;
			p.y = b.pos.y + b.size.y * (1.0 - y);
			if (i == 0)
				nvgMoveTo(args.vg, p.x, p.y);
			else
				nvgLineTo(args.vg, p.x, p.y);
		}
		nvgLineCap(args.vg, NVG_ROUND);
		nvgMiterLimit(args.vg, 2.0);
		nvgStrokeWidth(args.vg, 1.5);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgStroke(args.vg);			
		nvgResetScissor(args.vg);
		nvgRestore(args.vg);	
	}

}
