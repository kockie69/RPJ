#include "rack.hpp"
#include "dep/dr_wav.h"
#include "dep/dr_flac.h"
#include "dep/dr_mp3.h"

using namespace std;

enum PanningType {SIMPLEPAN, CONSTPOWER };

struct PanPos {
	double left;
	double right;
};

struct AudioParameters {
    AudioParameters();
    float dB;
	PanningType panningType;
    float panningValue;  
    float rackSampleRate;
    bool stop;
    bool repeat;
    float speed;
    float begin, end;
};

struct Audio {
    Audio();
    ~Audio();
    bool loadSample(char *);
    bool pause,loading,fileLoaded,play,stop;
    float * pSampleData;
    unsigned int channels;
    unsigned int sampleRate; 
	drwav_uint64 totalPCMFrameCount;
    drmp3_config mp3config;
    vector<vector<float>> playBuffer;
    float peak;
    void setParameters(const AudioParameters& );
    void ejectSong();
    PanPos panning(PanningType, double);
    void processAudioSample();
    void start(void);
    void rewind(float);
    void forward(float);
    bool withinBoundery(void);
    void setPause(bool);
    void setPlay(bool);
    bool getPlay(void);
    void setStop(bool);
    void setRepeat(bool);
    float scaleFac;
	float samplePos;
    PanningType panningType;
    char * fileName;
    float left,right;
    float dB;
    float speed;
    float panningValue; 
    float rackSampleRate;
    float begin, end;
    bool up,repeat; 
};
