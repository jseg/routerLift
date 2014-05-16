routerLift
==========

Arduino code to measure the height and tilt of a router lift

*Please find a circuit description in the .ino file to compliment the diagram
*Use of a tilt sensor is optional. Comment out #define TILT to remove that feature

*Recommended Hardware:
Arudino Uno
Encoder AMT103-V KIT Digikey # 102-1308-ND with appropriate shaft adapter
Spectra Symbol Softpot for tilt sensing (linear or rotary depending on installation details)
16x2 character display
The zero button should be a piece of flatstock with a wire connected to the zero pin, while the tool should be grounded

If you are using a linear softpot, an extra function will be needed to calculate the tool angle.

This code has not yet been tested on hardware.
