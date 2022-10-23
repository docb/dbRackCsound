# dbRackCsound

![](images/csound1.png?raw=true)

A csound module for VCVRack v2.

**Csound**: This module is designed for generating sound. It provides a V/Oct input and a
Gate input (polyphonic). A Gate will activate the instrument 1 with the fractional instrument
number 1 + channel/100 so that 16 polyphonic voices can be played. The channel number is passed 
in the p4 parameter of the instrument

Note that the outputs are summed up in this variant, 
so OutL and OutR are monophonic. For polyphonic outputs **Csound16** can be used - which needs significant more CPU.

There are fixed defined csound channel names for communication between VCVRack and Csound:
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

There are further examples provided int the factory presets.

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

Here a small demo:
