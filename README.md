# Introduction

This is a side-project from 2018 that I built as a wedding present for my friend
in 2018. It was inspired by a festival tent that he, his fiance, and I discovered
one night called the "Sound Puddle," designed by an artist based in Longmont, CO.
The Sound Puddle is microcontroller-based contraption that maps sound frequencies
onto light frequencies, and sends blips of colors dripping down an LED string.

You can view the functioning of the assembled emulation on
[YouTube](youtu.be/m5dzctOYbeE)

In the video I tested on an Arduino Uno, but switched to a Teensy++ 2.0 board
when soldering. The Teensy board is smaller, and fit better in the enclosure.

# Components

* [Teensy++ 2.0](https://www.pjrc.com/store/teensypp.html)
* [Electret microphone](https://www.adafruit.com/product/1713)
* [LPD8806 LED strip](https://www.adafruit.com/product/306)

# Conceptual Overview

The electret microphone is wired up to pin A0 on Teensy. The A0 pin is an
analog-to-digital-conversion (ADC) pin, which can be configured with an interrupt
to take a voltage reading every XX number of CPU clock cycles. This
functionality is configured using the ADCSRA instruction register. Some simple
math allows you to calculate the audio sampling rate based on the pre-scalar
value, which is the ratio between the CPU clock rate and the ADC clock rate.

Once the ADC is configured and enabled, the ISR interrupt (native ADC interrupt)
is triggered once per ADC clock cycle. When this happens, a new 8-bit audio
sample (i.e. a reading of the voltage from the electret microphone) is stored
in the ADCH buffer.

The main loop of the Teensy gathers audio samples from the ADCH buffer when
they become available, and stores them in a buffer to save them for further
processing. The buffer, when full, is essentially a short audio clip.

Now that we have an audio clip, we are half-way to our goal of translating
audio frequencies into light frequencies. This transformation is performed using
a discrete fast Fourier transform (DFFT). The DFFT algorithm takes as input the
audio clip segment (time domain), and returns the magnitude of the sine-wave
components of the waveform. You can read about DFFT
[here](https://en.wikipedia.org/wiki/Discrete_Fourier_transform).

The magnitudes of the frequencies in the audible range (roughly 40Hz to 20kHz)
are divided into low-mid-high buckets, and mapped to the red, green, and blue
LEDs in the LPD8806 strip:
* Low -> Red
* Mid -> Green
* High -> Blue

The main loop runs quickly, and every time the DFFT is run on an audio segment
(64 samples), one LED in the strip is turned on with the color frequencies
mapped to the audio frequencies of that short clip. That color cell is then
propagated one cell down the string on the next loop iteration, and the next
audio clip gets mapped onto the first LED in the string. In this way, the colors
"drip" down the LED strip over time, allowing you to see the propagation of changes
in the frequency content of the audio sound (as color) over time.

# License

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.