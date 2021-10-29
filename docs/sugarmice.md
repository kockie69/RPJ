# Sugar Mice / 3rd Order Butterworth LPF <a name="sugarmice"></a>
![sugarmice image](./sugarmice.png)

Sugar Mice is a Wave digital filter (WDF). WDFs are designed to simulate analog electronic circuits. The idea behind WDFs is to simulate the signal flow through analog circuit components: Resistors (R),  Capacitors (C), and Inductors (L).

Sugar Mice is a 3rd Order Butterworth low pass ladder filter. 


## Controls
The **Cutoff** knob controls the cutoff frequency of the filter.

From the menu the **Enable warping** option can be selected to fix the frequency-warping problem where the analog frequencies do not exactly  map to the digital frequencies. The WDF component equations do not take the frequency warping into account and so they do not attempt to correct for this error.

## Credits
The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.