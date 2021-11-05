#include "AudioDelay.hpp"



	AudioDelayParameters::AudioDelayParameters() {
		algorithm = delayAlgorithm::kNormal; ///< delay algorithm

		wetLevel_dB = -3.0;	///< wet output level in dB
		dryLevel_dB = -3.0;	///< dry output level in dB
		feedback_Pct = 0.0;	///< feedback as a % value

		updateType = delayUpdateType::kLeftAndRight;///< update algorithm
		leftDelay_mSec = 0.0;	///< left delay time
		centreDelay_mSec = 0.0;	///< left delay time
		rightDelay_mSec = 0.0;	///< right delay time
		delayRatio_Pct = 100.0;	///< dela ratio: right length = (delayRatio)*(left length)

		lpfFc = 100.0;
		hpfFc = 100.0;
		useLPF = true;
		useHPF = true;
	}





