#include "Audio.hpp"

AudioParameters::AudioParameters() {

}

Audio::Audio() {
	playBuffer.resize(2);
	playBuffer[0].resize(0);
	playBuffer[1].resize(0);
	samplePos=0;
	peak=0;
	panningType=CONSTPOWER;
}

void Audio::setParameters(const AudioParameters& params) {
    dB = params.dB;
	panningType = params.panningType;
    panningValue = params.panningValue;  
    rackSampleRate = params.rackSampleRate;
	stop = params.stop;
}

void Audio::loadSample(char *path) {
	loading = true;
	float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(path, &channels, &sampleRate, &totalPCMFrameCount, NULL);
    if (pSampleData != NULL) {
		playBuffer[0].clear();
		playBuffer[1].clear();
		for (unsigned int i=0; i < (totalPCMFrameCount*channels); i = i + channels) {
			playBuffer[0].push_back(pSampleData[i]);
			if (channels == 2)
				playBuffer[1].push_back((float)pSampleData[i+1]);
			if ((abs(pSampleData[i])>peak) || abs((pSampleData[i+1])>peak))
				peak = max(abs(pSampleData[i]),abs(pSampleData[i+1]));	
		}
        fileLoaded=true;
		play=true;
	}
	else 
		fileLoaded = false;
	loading = false;
}

void Audio::processAudioSample() {
    if (peak>0) {
	    scaleFac = (float)(pow(10.0,dB/peak)*20); 
    }
	
	if (stop)
		play=false;
		
    if ((!loading) && (play) && ((floor(samplePos) < totalPCMFrameCount) && (floor(samplePos) >= 0))) {	
		if (channels == 1) {
			left = panning(panningType, panningValue).left * scaleFac * (playBuffer[0][floor(samplePos)]);
			right = panning(panningType, panningValue).right * scaleFac * (playBuffer[0][floor(samplePos)]);
		}
		else if (channels ==2) {
			left = scaleFac * panning(panningType, panningValue).left * (playBuffer[0][floor(samplePos)]);
			right = scaleFac * panning(panningType, panningValue).right * (playBuffer[1][floor(samplePos)]);
       	}
		samplePos=samplePos+sampleRate/rackSampleRate;
	}
	if (floor(samplePos) >= totalPCMFrameCount) {
		samplePos = 0;
		play=false;
	}
}

PanPos Audio::panning(PanningType panType, double position) {
	PanPos pos;

	switch (panType) {
		case 0: {
			position *= 0.5;
			pos.left = position - 0.5;
			pos.right = position + 0.5;
			break;
		}
		case 1: {
			const double piovr2 = 4.0 * atan(1.0) * 0.5;
			const double root2ovr2 = sqrt(2.0) * 0.5;
			double thispos = position * piovr2;
			double angle = thispos * 0.5;
			pos.left = root2ovr2 * (cos(angle) - sin(angle));
			pos.right = root2ovr2 * (cos(angle) + sin(angle));
			break;
		}
 		default:
			break;
	}
	return pos;
}
