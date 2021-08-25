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
	scaleFac = 1;
	repeat = true;
}

void Audio::setParameters(const AudioParameters& params) {
    dB = params.dB;
	panningType = params.panningType;
    panningValue = params.panningValue;  
    rackSampleRate = params.rackSampleRate;
	stop = params.stop;
	if (!play)
		play = params.play;
	repeat = params.repeat;
	beginRatio = params.startRatio;
	endRatio = params.endRatio;
}

bool Audio::loadSample(char *path) {
	loading = true;
	float* pSampleData = NULL;

	if (rack::string::filenameExtension(path)=="flac")
		pSampleData = drflac_open_file_and_read_pcm_frames_f32(path, &channels, &sampleRate, &totalPCMFrameCount, NULL);
	else if (rack::string::filenameExtension(path)=="wav")
		pSampleData = drwav_open_file_and_read_pcm_frames_f32(path, &channels, &sampleRate, &totalPCMFrameCount, NULL);
    else if (rack::string::filenameExtension(path)=="mp3")
	{
		pSampleData = drmp3_open_file_and_read_pcm_frames_f32(path, &mp3config, &totalPCMFrameCount, NULL);
		channels = mp3config.channels;
		sampleRate = mp3config.sampleRate;
	}
	
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
		if (samplePos==0)
			samplePos = (totalPCMFrameCount*beginRatio/1024);
	}
	else 
		fileLoaded = false;
	loading = false;
	return fileLoaded;
}

void Audio::processAudioSample() {
	
	if (stop)
		play=false;

	// Determine if we are going up or down
	//float beginPos = totalPCMFrameCount*(beginRatio/1024);
	//float endPos = totalPCMFrameCount*(endRatio/1024);
	up = (endRatio >= beginRatio);
	
	// First check if samplePos is still within begin and end bounderies, otherwise return to start
    if ((!loading) && (play) && (((up) && (floor(samplePos) <= (totalPCMFrameCount * endRatio/1024))) || ((!up) && (floor(samplePos) >= totalPCMFrameCount * endRatio/1024)))) {
  
		if (peak>0) {
	    	scaleFac = (float)(pow(10.0,dB/peak)*20); 
    	}

		// if we are going up and beginPos has moved forward, change samplePos
		if (up && samplePos < totalPCMFrameCount * beginRatio/1024)
			samplePos = totalPCMFrameCount * beginRatio/1024;

		// if we are going down and endPos has moved backwards, change samplePos
		if (!up && samplePos > totalPCMFrameCount * beginRatio/1024)
			samplePos = totalPCMFrameCount * beginRatio/1024;

		if (channels == 1) {
			left = panning(panningType, panningValue).left * scaleFac * (playBuffer[0][floor(samplePos)]);
			right = panning(panningType, panningValue).right * scaleFac * (playBuffer[0][floor(samplePos)]);
		}
		else if (channels ==2) {
			left = scaleFac * panning(panningType, panningValue).left * (playBuffer[0][floor(samplePos)]);
			right = scaleFac * panning(panningType, panningValue).right * (playBuffer[1][floor(samplePos)]);
       	}
		if (up)
			samplePos=samplePos+(sampleRate/rackSampleRate);
		else
			samplePos=samplePos-(sampleRate/rackSampleRate);
	}
	else {
		if (up && play) {
			samplePos = totalPCMFrameCount * (beginRatio/1024);
		}
		if (!up && play) {
			samplePos = totalPCMFrameCount * (beginRatio/1024);
		}

		// remove next line if repeat added to parameters
		if (!repeat)
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
