# Lavender / Second order LPF/HPF/BPF/BSF filter <a name="lavender"></a>
![lavender image](./lavender.png)

Lavender is a second order Low-pass, High-pass, Band-pass and Band-stop filter. The DC gain is normalized to 0.0 dB resulting in peak gains that are greater than unity. A Q of 20  produces a peak gain of ~25 dB or a raw gain of ~18; at this gain, a signal can easily be clipped. 

## Controls
**Cutoff** knob controls the corner frequency of the filters.

For the LPF and HPF the **Resonance** knob is the quality factor controlling the resonant peaking. For the BPF and the BSF it is the quality factor controlling width of peak or notch = 1/Bandwidth.

With the **Wet** knob you control the amount of processed sound/signal that will be mixed at the output.

The **Dry** knob controls the amount of unprocessed sounds that comes from the input straight into the mixed audio output.

From the menu the Biquadric **structure** can be selected that will be used for the filter arithmetic. There are 4 options to select from: Direct, Canonical, TransposeDirect and TransposeCanonical.

The direct forms (direct form and transposed direct form) are the simplest to implement but are also the  most susceptible to errors. The transposed canonical form is generally regarded as the best structure 
for floating point implementations due to the way the multiple summers split the work and add similarly sized values. There are also pros and cons with using the various structures with modulation  of the filter frequency, especially when the modulation value changes rapidly. 

## Credits
The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.