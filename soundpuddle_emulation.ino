/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
*/
#include <arduinoFFT.h>

#include "LPD8806.h"
#include "SPI.h"
#ifdef __AVR_ATtiny85__
 #include <avr/power.h>
#endif
#include "arduinoFFT.h"

const int NUM_LEDS = 24;
const int FFT_SAMPLE_SIZE = 64;
const int THRESHOLD = 60;
const int LED_BASELINE = 1;

LPD8806 strip = LPD8806(NUM_LEDS); // data = pin 11, clock = pin 13.
arduinoFFT FFT = arduinoFFT();
double vReal[FFT_SAMPLE_SIZE];
double vImag[FFT_SAMPLE_SIZE];
uint32_t led_array[NUM_LEDS] = {0};
int fft_pos;
int led_pos;
bool sampleready;
int r,g,b;
int magnitude;
double peak;

void setup(){
  
  //set up continuous sampling of analog pin 0 (you don't need to understand this part, just know how to use it in the loop())
  
  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1)| (1 << ADPS0); // 128 prescaler
  ADCSRA |= (1 << ADIE); // enable ADC interrupts
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  
  //if you want to add other things to setup(), do it here
  Serial.begin(9600);
  fft_pos = 0;
  led_pos = 0;
  sampleready = false;
  strip.begin();
  strip.show();
}

void loop(){
  if(sampleready) { // get new ADC sample
      vReal[fft_pos] = double(ADCH); //get new value from A0
      noInterrupts();
      sampleready = false;
      interrupts();
      fft_pos++;
  }
  if(fft_pos == FFT_SAMPLE_SIZE) { // when buffer full, do DFFT, update lights
      // reset FFT position
      fft_pos = 0;
      // zero out imaginary vector
      for(int i=0; i < FFT_SAMPLE_SIZE; i++) {
        vImag[i] = 0.0;
      }
      // perform DFFT
      FFT.Windowing(vReal, FFT_SAMPLE_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.Compute(vReal, vImag, FFT_SAMPLE_SIZE, FFT_FORWARD);
      FFT.ComplexToMagnitude(vReal, vImag, FFT_SAMPLE_SIZE);
      // convert audio signal to light signal
      r = g = b = LED_BASELINE;
      for(int i=0; i<3; i++) {
        r += vReal[2+i];
      }
      for(int i=0; i<6; i++) {
        g += vReal[5+i];
      }
      for(int i=0; i<7; i++) {
        b += vReal[11+i];
      }
      r = map(r, 0, 400, 0, 127);
      g = map(g, 0, 500, 0, 127);
      b = map(b, 0, 700, 0, 127);
      r = constrain(r, 0, 127);
      g = constrain(g, 0, 127);
      b = constrain(b, 0, 127);
      if(r+g+b > THRESHOLD) {
        led_array[led_pos] = strip.Color(r,g,b);
      } else {
        led_array[led_pos] = strip.Color(0,0,0);
      }
      led_pos = (led_pos+1) % NUM_LEDS;
      for(int i=0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, led_array[(led_pos + i) % NUM_LEDS]);
      }
      strip.show();
  }
}

ISR(ADC_vect) {
    sampleready = true;
}