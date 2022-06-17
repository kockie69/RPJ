#include "ZVA.hpp"

/**
@sgn
\ingroup FX-Functions

@brief calculates sgn( ) of input
\param xn - the input value
\return -1 if xn is negative or +1 if xn is 0 or greater
*/
inline double sgn(double xn)
{
	return (xn > 0) - (xn < 0);
}







/**
\struct ZVAFilterParameters
\ingroup FX-Objects
\brief
Custom parameter structure for the ZVAFilter object.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

ZVAFilterParameters::ZVAFilterParameters() {}
/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
/*ZVAFilterParameters::ZVAFilterParameters& operator=(const ZVAFilterParameters& params)	// need this override for collections to work
{
	if (this == &params)
		return *this;

	filterAlgorithm = params.filterAlgorithm;
	fc = params.fc;
	Q = params.Q;
	filterOutputGain_dB = params.filterOutputGain_dB;
	enableGainComp = params.enableGainComp;
	matchAnalogNyquistLPF = params.matchAnalogNyquistLPF;
	selfOscillate = params.selfOscillate;
	enableNLP = params.enableNLP;
	return *this;
}*/

