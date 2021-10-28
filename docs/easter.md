# Easter / Resonance filter <a name="easter"></a>
![easter image](./easter.png)

Easter is a resonance filter module that delivers 2 different resonators. A resonator is a Band pass filter (BPF) that can be made to have a very narrow peak. 

One of the resonators delivered is a standard type (ResonA) and the other is a Smith-Angell resonator (ResonB). 

The basic resonator (ResonA) implements a single conjugate pair of poles in a 2nd order feedback topology. The drawback to this design is that the frequency response is symmetrical only at one frequency,  π/2, when the low frequency and high frequency magnitude vectors are symmetrical. At all other  frequencies, the response is shifted asymmetrically.

The improved design (ResonB) , known as the Smith-Angell resonator, reduces this asymmetry problem.

You can choose the filters by clicking the small + and - buttons at each side of the display.

## Controls
The **Cutoff** knob controls the center frequency of the filters.

The **Resonance** knob of controls the 3 dB bandwidth.

With the **Wet** knob you control the amount of processed sound/signal that will be mixed at the output.

The **Dry** knob controls the amount of unprocessed sounds that comes from the input straight into the mixed audio output.

From the menu the Biquadric **structure** can be selected that will be used for the filter arithmetic. There are 4 options to select from: Direct, Canonical, TransposeDirect and TransposeCanonical.

The direct forms (direct form and transposed direct form) are the simplest to implement but are also the  most susceptible to errors. The transposed canonical form is generally regarded as the best structure 
for floating point implementations due to the way the multiple summers split the work and add similarly sized values. There are also pros and cons with using the various structures with modulation  of the filter frequency, especially when the modulation value changes rapidly. 
 
## Credits
The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.