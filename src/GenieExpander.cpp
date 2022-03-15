#include "RPJ.hpp"
#include "ctrl/RPJPorts.hpp"
#include "ctrl/RPJKnobs.hpp"
#include <random>
#include "GenieExpander.hpp"


GenieExpander::GenieExpander() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_HISTORY, 1.f, 100.f,10.f, "Wasp length");
	historyTimer=0;
	maxHistory=10;
}

void GenieExpander::doHistory(float dt) {
	historyTimer-=dt;
	if (historyTimer<=0) {
		for (int n=0;n<MAXPENDULUMS;n++) {
			//oldEdges[n].push_back({edges[n][1].first,edges[n][1].second});
			oldEdges[n][0].push_back({edges[n][0].first,edges[n][0].second});
			std::rotate(oldEdges[n][0].rbegin(), oldEdges[n][0].rbegin() + 1, oldEdges[n][0].rend());
			oldEdges[n][1].push_back({edges[n][1].first,edges[n][1].second});
			std::rotate(oldEdges[n][1].rbegin(), oldEdges[n][1].rbegin() + 1, oldEdges[n][1].rend());
			oldEdges[n][2].push_back({edges[n][2].first,edges[n][2].second});
			std::rotate(oldEdges[n][2].rbegin(), oldEdges[n][2].rbegin() + 1, oldEdges[n][2].rend());
			oldEdges[n][3].push_back({edges[n][3].first,edges[n][3].second});
			std::rotate(oldEdges[n][3].rbegin(), oldEdges[n][3].rbegin() + 1, oldEdges[n][3].rend());
			if ((int)oldEdges[n][0].size()>maxHistory) {
				oldEdges[n][0].erase(oldEdges[n][0].end());
				oldEdges[n][1].erase(oldEdges[n][1].end());
				oldEdges[n][2].erase(oldEdges[n][2].end());
				oldEdges[n][3].erase(oldEdges[n][3].end());
			}
			historyTimer = 1000*dt;
		}
	}
};

void GenieExpander::process(const ProcessArgs &args) {
	
	maxHistory = params[PARAM_HISTORY].getValue();

	doHistory(args.sampleTime);
	
	parentConnected = leftExpander.module && leftExpander.module->model == modelGenie;
	if (parentConnected) {
    	xpanderPairs* rdMsg = (xpanderPairs*)leftExpander.module->rightExpander.consumerMessage;
		for (int n=0;n < MAXPENDULUMS;n++)
			for (int i=0; i < EDGES;i++)
				edges[n][i] = rdMsg->edges[n][i];
		mass = rdMsg->mass;
		nrOfPendulums = rdMsg->nrOfPendulums;
	}
	else {
		nrOfPendulums=1;
		edges[0][0].first = inputs[INPUT_1_X].getVoltage()*10;
		edges[0][0].second = inputs[INPUT_1_Y].getVoltage()*10;
		edges[0][1].first = inputs[INPUT_2_X].getVoltage()*20;
		edges[0][1].second = inputs[INPUT_2_Y].getVoltage()*20;
		edges[0][2].first = inputs[INPUT_3_X].getVoltage()*30;
		edges[0][2].second = inputs[INPUT_3_Y].getVoltage()*30;
		edges[0][3].first = inputs[INPUT_4_X].getVoltage()*40;
		edges[0][3].second = inputs[INPUT_4_Y].getVoltage()*40;
		mass = 10;
	}
}

void GenieDisplay::drawMass(NVGcontext *vg,NVGcolor massColor,float xpos,float ypos) {
		nvgFillColor(vg, massColor);
		nvgStrokeColor(vg, massColor);
		nvgStrokeWidth(vg, 2);
		nvgBeginPath(vg);
		nvgCircle(vg, xpos, ypos, module->mass);
		nvgFill(vg);
		nvgStroke(vg);
		nvgClosePath(vg);
}

void GenieDisplay::drawSwarm(int pendulum,int edge,NVGcontext *vg,NVGcolor massColor,float xpos,float ypos) {
	for (int i=1;i<module->maxHistory;i++) {
		if (i < (int)module->oldEdges[pendulum][edge].size()) {
			// The 50 in the next line can be a parameter
			NVGcolor massColorFaded = nvgTransRGBA(massColor, (module->maxHistory-i)*50/module->maxHistory);
			nvgFillColor(vg, massColorFaded);
			nvgStrokeColor(vg, massColorFaded);
			nvgStrokeWidth(vg, 2);
			nvgBeginPath(vg);
			nvgCircle(vg, (module->oldEdges[pendulum][edge][i].first+xpos), (module->oldEdges[pendulum][edge][i].second+ypos), module->mass);				
			nvgFill(vg);
			nvgStroke(vg);
			nvgClosePath(vg);
		}
	}
}

void GenieDisplay::drawLayer(const DrawArgs &args,int layer) {

	if (module == NULL) return;
	if (layer == 1) {
		// The clipbox is whole module, so we fix so it is only display screen
		Rect clipBox = args.clipBox;
		clipBox.pos.x = args.clipBox.pos.x + 75;
		nvgScissor(args.vg, RECT_ARGS(clipBox));
		
		// Draw first Red Mass
		NVGcolor massColor0 = nvgRGB(0xAE, 0x1C, 0x28);
		drawMass(args.vg,massColor0,xpos, ypos);

		for (int n=0;n<module->nrOfPendulums;n++) {
			if (!(((module->inputs[GenieExpander::INPUT_1_X].isConnected()) && (module->inputs[GenieExpander::INPUT_1_Y].isConnected())) || module->parentConnected))
			return;
			// draw Second White Mass
			NVGcolor massColor1 = nvgRGB(0xFF, 0xFF, 0xFF);
			drawMass(args.vg,massColor1,module->edges[n][0].first+xpos, module->edges[n][0].second+ypos);
			drawSwarm(n,0,args.vg,massColor1,xpos, ypos);

			// Draw line between masses
			NVGcolor lineColor = nvgRGB(0xFF, 0x7F, 0x00);
			nvgFillColor(args.vg, lineColor);
			nvgStrokeColor(args.vg, lineColor);
			nvgStrokeWidth(args.vg, 2);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg,xpos,ypos);
			nvgLineTo(args.vg,module->edges[n][0].first+xpos, module->edges[n][0].second+ypos);
			nvgStroke(args.vg);
			nvgClosePath(args.vg);
			if (!(((module->inputs[GenieExpander::INPUT_2_X].isConnected()) && (module->inputs[GenieExpander::INPUT_2_Y].isConnected())) || module->parentConnected))
				return;
			// draw third Blue Mass
			NVGcolor massColor2 = nvgRGB(0x21, 0x46, 0x8B);
			drawMass(args.vg,massColor2,module->edges[n][1].first+xpos, module->edges[n][1].second+ypos);
			drawSwarm(n,1,args.vg,massColor2,xpos,ypos);

			// Draw line between masses
			nvgFillColor(args.vg, lineColor);
			nvgStrokeColor(args.vg, lineColor);
			nvgStrokeWidth(args.vg, 2);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg,module->edges[n][0].first+xpos, module->edges[n][0].second+ypos);
			nvgLineTo(args.vg,module->edges[n][1].first+xpos, module->edges[n][1].second+ypos);
			nvgStroke(args.vg);
			if (module->inputs[GenieExpander::INPUT_3_X].isConnected() && module->inputs[GenieExpander::INPUT_3_Y].isConnected() && !module->parentConnected) {
				// Draw fourth Mass
				NVGcolor massColor3 = nvgRGB(0xFF, 0x7F, 0x00);
				drawMass(args.vg,massColor3,module->edges[n][2].first+xpos, module->edges[n][2].second+ypos);
				drawSwarm(n,2,args.vg,massColor3,xpos, ypos);

				// Draw line between masses
				nvgFillColor(args.vg, lineColor);
				nvgStrokeColor(args.vg, lineColor);
				nvgStrokeWidth(args.vg, 2);
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg,module->edges[n][1].first+xpos, module->edges[n][1].second+ypos);
				nvgLineTo(args.vg,module->edges[n][2].first+xpos, module->edges[n][2].second+ypos);
				nvgStroke(args.vg);
				// Draw fifth Mass
				drawMass(args.vg,massColor0,module->edges[n][3].first+xpos, module->edges[n][3].second+ypos);
				drawSwarm(n,3,args.vg,massColor0,xpos, ypos);

				// Draw line between masses
				nvgFillColor(args.vg, lineColor);
				nvgStrokeColor(args.vg, lineColor);
				nvgStrokeWidth(args.vg, 2);
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg,module->edges[n][2].first+xpos, module->edges[n][2].second+ypos);
				nvgLineTo(args.vg,module->edges[n][3].first+xpos, module->edges[n][3].second+ypos);
				nvgStroke(args.vg);
				nvgClosePath(args.vg);
			}
		}
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

		// Next do the Jacks
	const float jackX1 = 8.5;
	const float jackX2 = 45;		

	const float jackY1 = 49;
	const float jackY2 = 94;
	const float jackY3 = 143;
	const float jackY4 = 189;


	addInput(createInput<RPJPJ301MPort>(Vec(jackX1, jackY1), module, GenieExpander::INPUT_1_X));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY1), module, GenieExpander::INPUT_1_Y));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX1, jackY2), module, GenieExpander::INPUT_2_X));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY2), module, GenieExpander::INPUT_2_Y));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX1, jackY3), module, GenieExpander::INPUT_3_X));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY3), module, GenieExpander::INPUT_3_Y));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX1, jackY4), module, GenieExpander::INPUT_4_X));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY4), module, GenieExpander::INPUT_4_Y));

		// Then do the knobs
	const float knobX1 = 6;

	const float knobY1 = 31.5;
	const float knobY2 = 70;

	addParam(createParam<RPJKnob>(Vec(knobX1, knobY1), module, GenieExpander::PARAM_HISTORY));
}

Model * modelGenieExpander = createModel<GenieExpander, GenieExpanderModuleWidget>("GenieExpander");