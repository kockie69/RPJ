//***********************************************************************************************
//Mixer module for VCV Rack by Steve Baker and Marc Boulé 
//
//Based on code from the Fundamental plugin by Andrew Belt 
//See ./LICENSE.md for all licenses
//***********************************************************************************************


#include "VuMeters.hpp"


// VuMeter signal processing code for peak/rms
// ----------------------------------------------------------------------------

// all in header file



// VuMeter displays
// ----------------------------------------------------------------------------

// VuMeterBase

void VuMeterBase::prepareYellowAndRedThresholds(float yellowMinDb, float redMinDb) {
	float maxLin = std::pow(faderMaxLinearGain, 1.0f / faderScalingExponent);
	float yellowLin = std::pow(std::pow(10.0f, yellowMinDb / 20.0f), 1.0f / faderScalingExponent);
	yellowThreshold = barY * (yellowLin / maxLin);
	float redLin = std::pow(std::pow(10.0f, redMinDb / 20.0f), 1.0f / faderScalingExponent);
	redThreshold = barY * (redLin / maxLin);
}

void VuMeterBase::processPeakHold() {// use APP->window->getLastFrameRate()
	holdTimeRemainBeforeReset -= 1.0f / APP->window->getLastFrameDuration();
	if ( holdTimeRemainBeforeReset < 0.0f ) {
		holdTimeRemainBeforeReset = 2.0f;// in seconds
		peakHold[0] = 0.0f;
		peakHold[1] = 0.0f;
	}		
	for (int i = 0; i < 2; i++) {
		if (VuMeterAllDual::getPeak(srcLevels, i) > peakHold[i]) {
			peakHold[i] = VuMeterAllDual::getPeak(srcLevels, i);
		}
	}
}

void VuMeterBase::draw(const DrawArgs &args) {

	processPeakHold();
	
	setColor();

	// PEAK
	drawVu(args, VuMeterAllDual::getPeak(srcLevels, 0), 0, 0);
	drawVu(args, VuMeterAllDual::getPeak(srcLevels, 1), barX + gapX, 0);

	// RMS
	drawVu(args, VuMeterAllDual::getRms(srcLevels, 0), 0, 1);
	drawVu(args, VuMeterAllDual::getRms(srcLevels, 1), barX + gapX, 1);
	
	// PEAK_HOLD
	drawPeakHold(args, peakHold[0], 0);
	drawPeakHold(args, peakHold[1], barX + gapX);	

	
	Widget::draw(args);
}

// Track-like

void VuMeterBase::drawVu(const DrawArgs &args, float vuValue, float posX, int colorIndex) {
	if (vuValue >= epsilon) {

		float vuHeight = vuValue / (faderMaxLinearGain * zeroDbVoltage);
		vuHeight = std::pow(vuHeight, 1.0f / faderScalingExponent);
		vuHeight = std::min(vuHeight, 1.0f);// normalized is now clamped
		vuHeight *= barY;

		NVGcolor colTop = VU_THEMES_TOP[colorTheme][1];
		NVGcolor colBot = VU_THEMES_BOT[colorTheme][1];
		NVGpaint gradGreen = nvgLinearGradient(args.vg, 0, barY - redThreshold, 0, barY, colTop, colBot);
		
		if (vuHeight >= redThreshold) {
			// Yellow-Red gradient
			NVGcolor colTopRed = VU_RED[colorIndex];
			NVGcolor colTopYel = VU_YELLOW[colorIndex];
			NVGpaint gradTop = nvgLinearGradient(args.vg, 0, 0, 0, barY - redThreshold - sepYtrack, colTopRed, colTopYel);
			nvgBeginPath(args.vg);
			nvgRect(args.vg, posX, barY - vuHeight - sepYtrack, barX, vuHeight - redThreshold);
			nvgFillPaint(args.vg, gradTop);
			nvgFill(args.vg);
			// Green
			nvgBeginPath(args.vg);
			nvgRect(args.vg, posX, barY - redThreshold, barX, redThreshold);
			nvgFillPaint(args.vg, gradGreen);
			nvgFill(args.vg);			
		}
		else {
			// Green
			nvgBeginPath(args.vg);
			nvgRect(args.vg, posX, barY - vuHeight, barX, vuHeight);
			nvgFillPaint(args.vg, gradGreen);
			nvgFill(args.vg);
		}

	}
}

void VuMeterBase::drawPeakHold(const DrawArgs &args, float holdValue, float posX) {
	if (holdValue >= epsilon) {
		float vuHeight = holdValue / (faderMaxLinearGain * zeroDbVoltage);
		vuHeight = std::pow(vuHeight, 1.0f / faderScalingExponent);
		vuHeight = std::min(vuHeight, 1.0f);// normalized is now clamped
		vuHeight *= barY;
		
		if (vuHeight >= redThreshold) {
			// Yellow-Red gradient
			NVGcolor colTopRed = VU_RED[1];
			NVGcolor colTopYel = VU_YELLOW[1];
			NVGpaint gradTop = nvgLinearGradient(args.vg, 0, 0, 0, barY - redThreshold - sepYtrack, colTopRed, colTopYel);
			nvgBeginPath(args.vg);
			nvgRect(args.vg, posX, barY - vuHeight - sepYtrack - peakHoldThick, barX, peakHoldThick);
			nvgFillPaint(args.vg, gradTop);
			nvgFill(args.vg);	
		}
		else {
			// Green
			NVGcolor colTop = VU_THEMES_TOP[colorTheme][1];
			NVGcolor colBot = VU_THEMES_BOT[colorTheme][1];
			NVGpaint gradGreen = nvgLinearGradient(args.vg, 0, barY - redThreshold, 0, barY, colTop, colBot);
			nvgBeginPath(args.vg);
			nvgRect(args.vg, posX, barY - vuHeight, barX, peakHoldThick);
			nvgFillPaint(args.vg, gradGreen);
			nvgFill(args.vg);
		}
	}		
}

