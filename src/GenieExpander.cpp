#include "RPJ.hpp"
#include "ctrl/RPJButtons.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "ctrl/RPJLights.hpp"
#include <random>
#include "GenieExpander.hpp"


GenieExpander::GenieExpander() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void GenieExpander::process(const ProcessArgs &args) {
	bool parentConnected = leftExpander.module && leftExpander.module->model == modelGenie;
	if (parentConnected) {
    	xpanderPairs* rdMsg = (xpanderPairs*)leftExpander.module->rightExpander.consumerMessage;
		first_edge = rdMsg->edges[0];
		second_edge = rdMsg->edges[1];
		mass = rdMsg->mass;
	}
}

void GenieDisplay::drawLayer(const DrawArgs &args,int layer) {

	if (module == NULL) return;
	if (layer == 1) {
		// Draw first Red Mass
		NVGcolor massColor0 = nvgRGB(0xAE, 0x1C, 0x28);
		nvgFillColor(args.vg, massColor0);
		nvgStrokeColor(args.vg, massColor0);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, xpos, ypos, module->mass);	
		nvgFill(args.vg);
		nvgStroke(args.vg);
		// draw Second Mass
		NVGcolor massColor1 = nvgRGB(0xFF, 0xFF, 0xFF);
		nvgFillColor(args.vg, massColor1);
		nvgStrokeColor(args.vg, massColor1);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, (module->first_edge.first+xpos), (module->first_edge.second+ypos), module->mass);	
		
		nvgFill(args.vg);
		nvgStroke(args.vg);
		// Draw line between masses
		NVGcolor lineColor = nvgRGB(0xFF, 0x7F, 0x00);
		nvgFillColor(args.vg, lineColor);
		nvgStrokeColor(args.vg, lineColor);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg,xpos,ypos);
		nvgLineTo(args.vg,module->first_edge.first+xpos, module->first_edge.second+ypos);

		nvgStroke(args.vg);
		nvgClosePath(args.vg);	
		// draw third Mass
		NVGcolor massColor2 = nvgRGB(0x21, 0x46, 0x8B);
		nvgFillColor(args.vg, massColor2);
		nvgStrokeColor(args.vg, massColor2);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, (module->second_edge.first+xpos), (module->second_edge.second+ypos), module->mass);

		nvgFill(args.vg);
		nvgStroke(args.vg);
		// Draw line between masses
		nvgFillColor(args.vg, lineColor);
		nvgStrokeColor(args.vg, lineColor);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg,module->first_edge.first+xpos, module->first_edge.second+ypos);
		nvgLineTo(args.vg,module->second_edge.first+xpos, module->second_edge.second+ypos);

		nvgStroke(args.vg);
		nvgClosePath(args.vg);
	}
	TransparentWidget::drawLayer(args,layer);
}


GenieExpanderModuleWidget::GenieExpanderModuleWidget(GenieExpander* module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/GenieExpander.svg")));

	addChild(createWidget<ScrewSilver>(Vec(0, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15,0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(0, 365)));

	// Fill the display for the module browser
	GenieDisplay *display = new GenieDisplay();
	display->box.pos = Vec(15, 30);
	display->box.size = Vec(WIDTH, HEIGHT);
	display->module = module;
	addChild(display);	
}

Model * modelGenieExpander = createModel<GenieExpander, GenieExpanderModuleWidget>("GenieExpander");