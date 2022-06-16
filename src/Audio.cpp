#include "Audio.hpp"

using namespace rack;

AudioParameters::AudioParameters() {
}

Audio::Audio() {
	playBuffer.resize(2);
	playBuffer[0].resize(0);
	playBuffer[1].resize(0);
	samplePos=0;
	peak=0;
	panningType=CONSTPOWER;
	pause=false;
	loading=false;
	fileLoaded=false;
	play=false;
	stop=false;
	left=0;
	right=0;
	playMode=REPEAT;
	delta = 1;
}


Audio::~Audio() {
}

void Audio::setPause(bool status) {
	pause=status;
}

void Audio::setStop(bool status) {
	stop=status;
}

void Audio::setPlay(bool status) {
	play=status;
}

bool Audio::getPlay(void) {
	return play;
}

bool Audio::getPause(void) {
	return pause;
}

bool Audio::getStop(void) {
	return stop;
}

void Audio::setPlayMode(PlayMode mode) {
		playMode=mode;
}


void Audio::setParameters(const AudioParameters& params) {
    dB = params.dB;
	panningType = params.panningType;
    panningValue = params.panningValue;  
    rackSampleRate = params.rackSampleRate;
	speed = params.speed;
	begin = params.begin;
	end = params.end;
	up = ( begin <= end);
	setPlayMode(params.playMode);
}

bool Audio::loadSample(std::string path) {
	loading = true;
	pSampleData = NULL;

	if (rack::string::lowercase(system::getExtension(system::getFilename(path)))==".flac")
		pSampleData = drflac_open_file_and_read_pcm_frames_f32(path.c_str(), &channels, &sampleRate, &totalPCMFrameCount, NULL);
	else if (rack::string::lowercase(system::getExtension(system::getFilename(path)))==".wav")
		pSampleData = drwav_open_file_and_read_pcm_frames_f32(path.c_str(), &channels, &sampleRate, &totalPCMFrameCount, NULL);
    else if (rack::string::lowercase(system::getExtension(system::getFilename(path)))==".mp3")
	{
		pSampleData = drmp3_open_file_and_read_pcm_frames_f32(path.c_str(), &mp3config, &totalPCMFrameCount, NULL);
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
			if (abs(pSampleData[i])>peak || abs(pSampleData[i+1])>peak)
				peak = max(abs(pSampleData[i]),abs(pSampleData[i+1]));	
		}
        fileLoaded=true;
	}
	else 
		fileLoaded = false;
	loading = false;
	return fileLoaded;
}

void Audio::ejectSong(void) {
	fileLoaded=false;
	setPlay(false);
//	playBuffer[0].empty();
//	playBuffer[1].empty();	
}

bool Audio::withinBoundery() {
	return (((samplePos <= end) && (samplePos >= begin)) || ((samplePos <= begin) && (samplePos >= end)));
}

void Audio::rewind(float step) {
	samplePos = samplePos - step;
	if (!withinBoundery())
		samplePos = (delta==1) ? begin : end;

}

void Audio::forward(float step) {
		samplePos = samplePos + step;		
		if (!withinBoundery())
			samplePos = (delta==1) ? end : begin;
}

void Audio::start(void) {
	setPlay(true);
}

void Audio::processAudioSample() {
	
	if (fileLoaded) {

		if (pause || stop) {
			if (stop) 
				// Determine if we are going up or down
				samplePos = (delta==1) ? begin : end;
			setPlay(false);
		}

		if (play) {
			samplePos = (up) ? samplePos+((delta*(sampleRate/rackSampleRate))+speed) : samplePos+(-1*delta*(sampleRate/rackSampleRate))+speed;
		}
		// First check if samplePos is still within begin and end bounderies, otherwise return to start
    	if (play && withinBoundery()) {
			// If mono
			if (channels == 1) {
				left = panning(panningType, panningValue).left * dB * (playBuffer[0][floor(samplePos)]);
				right = panning(panningType, panningValue).right * dB * (playBuffer[0][floor(samplePos)]);
			}
			// If stereo
			else if (channels ==2) {
				left = dB * panning(panningType, panningValue).left * (playBuffer[0][floor(samplePos)]);
				right = dB * panning(panningType, panningValue).right * (playBuffer[1][floor(samplePos)]);
       		}
		}

		if (!withinBoundery()) {
			if (!(playMode==PINGPONG)) {
				samplePos = begin;
				if (!(playMode==REPEAT))
					// PlayMode == SINGLE
					setPlay(false);
			}
			// PlayMode == PINGPONG
			else {
				if (up)
					if (delta==1)
						samplePos = end;
					else 
						samplePos = begin;
				else
					if (delta==-1)
						samplePos = begin;
					else
						samplePos = end;
				delta = delta * -1;
			} 
		}
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
