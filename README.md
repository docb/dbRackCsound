# dbRackCsound

![](images/csound1.png?raw=true)

Some Csound modules for VCVRack v2.

**Csound**: This module is designed for generating sound. It provides a V/Oct input and a
Gate input (polyphonic). A Gate will activate the instrument 1 with the fractional instrument
number 1 + channel/100 so that 16 polyphonic voices can be played. The channel number is passed 
in the p4 parameter of the instrument

Note that the outputs are summed up in this variant, 
so OutL and OutR are monophonic. For polyphonic outputs **Csound16** can be used - which needs significant more CPU.

There are fixed defined Csound channel names for communication between VCVRack and Csound:
- FREQ[1-16] 16 special control rate channels providing the frequency computed from the V/Oct input where C4 = 0V
- IN[1-4]C[1-16]  4x16 input control rate channels providing the values of the 4 inputs IN[1-4]
- P[1-4] 4 control rate channels providing the input of the 4 knobs 

So the smallest polyphonic sound generator look like this:

```csound-orc
instr 1
kfreq chnget sprintf("FREQ%d",p4)
ao vco2 0.7,kfreq
outs ao,ao
endin
```
See also further examples provided in the factory presets.

**CsoundFX**: This module is designed for processing sound. 
It provides two monophonic audio inputs (for polyphonic use Csound16FX),
which can be accessed via the inch opcode and two monophonic outputs.

Here an example: 

```csound-orc
instr 1
  ainL inch 1
  kcutoff chnget "P1"
  kres chnget "P2"
  kcutoff scale2 kcutoff,7,14,-10,10
  kfreq pow 2,kcutoff
  kres scale2 kres,0,1,-10,10
  aresL moogvcf2 ainL, kfreq, kres
  aresL balance aresL,ainL
  outs aresL,aresL
endin
```

There are further examples provided int the factory presets.

In CsoundFX the instrument 1 must be turned on with the On Knob. 
Currently the instrument 1 is turned off after (re-)compiling and must be turned on manually.
(Using the turnon opcode can cause starting intrument 1 multiple times when recompiling which is not intended.
There should be a solution in future versions).


Here a small demo. For the sequencer (top left) there is a special built in opcode *smt* available which provides a Schmitt Trigger for processing the clock signal. The bottom line shows some "manual" written effects - chorus, delay, reverb.


https://user-images.githubusercontent.com/1134412/197388214-8e12ffdf-dba5-4715-940a-09e9f547de96.mp4


**LICENCE**
Csound is licensed under GLPL
