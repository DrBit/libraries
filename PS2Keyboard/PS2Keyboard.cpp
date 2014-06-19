/*
  PS2Keyboard.cpp - PS2Keyboard library
  Copyright (c) 2007 Free Software Foundation.  All right reserved.
  Written by Christian Weichel <info@32leaves.net>

  ** Mostly rewritten Paul Stoffregen <paul@pjrc.com> 2010, 2011
  ** Modified for use beginning with Arduino 13 by L. Abraham Smith, <n3bah@microcompdesign.com> * 
  ** Modified for easy interrup pin assignement on method begin(datapin,irq_pin). Cuningan <cuninganreset@gmail.com> **

  for more information you can read the original wiki in arduino.cc
  at http://www.arduino.cc/playground/Main/PS2Keyboard
  or http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html

  Version 2.3 (October 2011)
  - Minor bugs fixed

  Version 2.2 (August 2011)
  - Support non-US keyboards - thanks to Rainer Bruch for a German keyboard :)

  Version 2.1 (May 2011)
  - timeout to recover from misaligned input
  - compatibility with Arduino "new-extension" branch
  - TODO: send function, proposed by Scott Penrose, scooterda at me dot com

  Version 2.0 (June 2010)
  - Buffering added, many scan codes can be captured without data loss
    if your sketch is busy doing other work
  - Shift keys supported, completely rewritten scan code to ascii
  - Slow linear search replaced with fast indexed table lookups
  - Support for Teensy, Arduino Mega, and Sanguino added

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "PS2Keyboard.h"

#define BUFFER_SIZE_INT 25			// Number of scancodes that can be pre-recorded
#define BUFFER_SIZE_PRESSED 15			// Number of scancodes that can be pressed at a time
static volatile uint16_t buffer[BUFFER_SIZE_INT];	// Buffer for the scancodes
static volatile uint16_t pressed[BUFFER_SIZE_PRESSED];	// Buffer for the pressed scancodes
static volatile uint8_t head, tail;			// Head number of received scancodes, tail is last readed scancode
static uint8_t DataPin;
static uint16_t CharBuffer=0;
static uint16_t CharRelBuffer=0;

// The ISR for the external interrupt
void ps2interrupt(void)
{
	static uint8_t bitcount=0;
	static uint8_t incoming=0;
	static uint32_t prev_ms=0;
	uint32_t now_ms;
	uint8_t n, val;

	val = digitalRead(DataPin);
	now_ms = millis();
	if (now_ms - prev_ms > 250) {
		bitcount = 0;
		incoming = 0;
	}
	prev_ms = now_ms;
	n = bitcount - 1;
	if (n <= 7) {
		incoming |= (val << n);
	}
	bitcount++;
	if (bitcount == 11) {
		uint8_t i = head + 1;
		if (i >= BUFFER_SIZE_INT) i = 0;
		if (i != tail) {
			buffer[i] = incoming;
			head = i;
		}
		bitcount = 0;
		incoming = 0;
	}
}

static inline uint8_t get_scan_code(void)
{
	uint8_t c, i;

	i = tail;
	if (i == head) return 0;
	i++;
	if (i >= BUFFER_SIZE_INT) i = 0;
	c = buffer[i];
	tail = i;
	return c;
}

// http://www.quadibloc.com/comp/scan.htm
// http://www.computer-engineering.org/ps2keyboard/scancodes2.html


#define BREAK     0x01
#define MODIFIER  0x02
#define SHIFT_L   0x04
#define SHIFT_R   0x08
#define ALTGR     0x10
#define CNTRL     0x20


static uint16_t get_iso8859_code(void)
{
	static uint8_t state=0;
	uint16_t s;
	uint16_t c;

	while (1) {
		s = get_scan_code();
		if (!s) return 0;
		if (s == 0xF0) {				// Code to releas a key 0xF0
			state |= BREAK;				// State is realeasing a key
		} else if (s == 0xE0) {			// Code of a special key
			state |= MODIFIER;			// Key is a modifier (like windows key)
		} else {
			if (state & BREAK) {		// If state is realeasing a key
				if (s == 0x12) {
					state &= ~SHIFT_L;
					c = s;
				} else if (s == 0x59) {
					state &= ~SHIFT_R;
					c = s;
				} else if (s == 0x11 && (state & MODIFIER)) {
					state &= ~ALTGR;
					c = s;
				} else if (s == 0x14) {
					state &= ~CNTRL;
					c = s;
				} else if (state & CNTRL) {
					if (s < PS2_KEYMAP_SIZE)
						c = s + (3*PS2_KEYMAP_SIZE);
				} else if (state & ALTGR) {
					if (s < PS2_KEYMAP_SIZE)
						c = s + (2*PS2_KEYMAP_SIZE);
				} else if (state & (SHIFT_L | SHIFT_R)) {
					if (s < PS2_KEYMAP_SIZE)
						c = s + PS2_KEYMAP_SIZE;
				} else {
					if (s < PS2_KEYMAP_SIZE)
						c = s;
				}
				state &= ~(BREAK | MODIFIER);
				if (c) {
					CharRelBuffer = c;
					return c;
				} 

			} else {			// If is pressing a key (Modifiers)
				if (s == 0x12) {
					state |= SHIFT_L;
					//continue;				// At this point skips everything and starts over at the begining of the while
					c = s;
				} else if (s == 0x59) {
					state |= SHIFT_R;
					//continue;
					c = s;
				} else if (s == 0x11 && (state & MODIFIER)) {
					state |= ALTGR;
					//continue;
					c = s;
				} else if (s == 0x14) {
					state |= CNTRL;
					//continue;
					c = s;
				} else if (state & CNTRL) {		// Final keycode (with or without modifier)
					if (s < PS2_KEYMAP_SIZE)
						c = s + (3*PS2_KEYMAP_SIZE);
				} else if (state & ALTGR) {
					if (s < PS2_KEYMAP_SIZE)
						c = s + (2*PS2_KEYMAP_SIZE);
				} else if (state & (SHIFT_L | SHIFT_R)) {
					if (s < PS2_KEYMAP_SIZE)
						c = s + PS2_KEYMAP_SIZE;
				} else {
					if (s < PS2_KEYMAP_SIZE)
						c = s;
				}
				state &= ~(BREAK | MODIFIER);
				if (c) {
					CharBuffer = c;
					return c;
				}
			}
		}
	}
}

// recall error
// Shift + A

// release shift
// keep A pressed


bool PS2Keyboard::available() {
	if ((CharBuffer) || (CharRelBuffer)) return true;		// Before reading a new one checlk if we have something on the buffer
	get_iso8859_code();
	if ((CharBuffer) || (CharRelBuffer)) return true;
	return false;
}

bool PS2Keyboard::key_pressed_available() {
	if (CharBuffer) return true;
	return false;
}

uint16_t PS2Keyboard::read() {
	uint16_t result = CharBuffer;
	CharBuffer = 0;
	if (!add_buffer (result)) return '\0'; // Buffer FULL -> return '\0'
	return result;
}

bool PS2Keyboard::add_buffer (uint16_t data) {
	unsigned int num_positions =  positions_buffer();
	for (int a = 0; a <= num_positions; a++) {
		if (pressed[a] == data) {	// This happens when auto repeat is enabled
			return true;			// We have a match so we dont have to continue
		}else if (a == num_positions) {		// If we have reached the end of filled slots...
			if (num_positions == BUFFER_SIZE_PRESSED) {		// We check if the buffers is full
				return false;		// BUFFER FULL!!!! we check only if we have a
			}else{
				pressed[a] = data;
				return true;
			}
		}
	}
}

bool PS2Keyboard::key_released_available() {
	if (CharRelBuffer) return true;
	return false;
}

uint16_t PS2Keyboard::read_released() {
	uint16_t result;
	result = CharRelBuffer;
	CharRelBuffer = 0;
	if (result) {
		// em de comprovar que s'ha premut amb anterioritat i si es un multiple (control/alt/shift)
		result = remove_buffer (result);
		return result;
	} else {
		return -1;
	}
}

uint16_t PS2Keyboard::remove_buffer (uint16_t data) {
	uint16_t temp_result = 0;
	boolean match = false;
	unsigned int num_positions =  positions_buffer();
	unsigned int i = num_positions-1;

		Serial.print (i);
		Serial.print (" - ");
		Serial.print (pressed[i]);
		Serial.print (" - ");
		Serial.print (data);
		Serial.print (" - ");
		Serial.println ((data == 0x12) || (data == 0x59) || (data == 0x14) || (data == 0x11)) ;

	if (num_positions == 0) {		// WE have reach the end and did not find the key, so its not in
		return data;
	}
	// If shift or any other modifier is pressed then we will remove shift + everything after shift (if any)
	if ((data == 0x12) || (data == 0x59) || (data == 0x14) || (data == 0x11)) {			// Shift L - SHIFT_R - CNTRL
		// If the last one is not == to data -> remove it

		if (data != pressed[i]) {		// If it is not already in the last positions...
			// If we encounter on the way another special key we have to take care of it and keep it in the list
			// We will cechk next in list and if ints not yet our the we will delete that instead anc move the actual to there
			if ((pressed[i] == 0x12) || (pressed[i] == 0x59) || (pressed[i] == 0x14) || (pressed[i] == 0x11)) {
				unsigned int t_i = i;
				while ((pressed[i] == 0x12) || (pressed[i] == 0x59) || (pressed[i] == 0x14) || (pressed[i] == 0x11)) {
					if (pressed[i] == data) {		// If it finally matches we quit 
						CharRelBuffer = data; 
						return -1;
					}
					if (t_i == 0) {		// WE have reach the end and did not find the key, so its not in
						return data;
					}
					// fer un canvi del penultim per l'ultim
					uint16_t last = pressed[i];
					uint16_t penultimum = pressed[t_i-1];
					pressed[t_i-1] = last;
					pressed[i] = penultimum;
					Serial.print ("Now last number is: ");
					Serial.print (pressed[i]);
					Serial.print (" - ");
					Serial.println (pressed[i] != data);
					t_i--;		// Next time one less
				} 
				// We have a normal number, we continue
				CharRelBuffer = data; 
				return -1;
			}

			//Keep doing it until we release all of them (Main loop will always check for available keys left)
			CharRelBuffer = data; 					// Pre-record shift (or modifier) to be able to come back to here again next time
													// If another key is inmediately released this will fail a better whay of doing shld be invented
			temp_result = pressed[num_positions-1];	// Pre-record data (last only)
			pressed[num_positions-1] = '\0';		// Delete last one
			Serial.println ("Relased via special");
			return temp_result;						// Send data to be released
		}
	}

	for (int a = 0; a < num_positions; a++) {
		if (pressed[a] == data) {
			match = true;
		}else if ((pressed[a] == (data + PS2_KEYMAP_SIZE)) || (pressed[a] == (data - PS2_KEYMAP_SIZE))) {
			match = true;
		}else if ((pressed[a] == (data + (2*PS2_KEYMAP_SIZE))) || (pressed[a] == (data - (2*PS2_KEYMAP_SIZE)))) {
			match = true;
		}else if ((pressed[a] == (data + (3*PS2_KEYMAP_SIZE))) || (pressed[a] == (data - (3*PS2_KEYMAP_SIZE)))) {
			match = true;
		}

		if (match) {
			temp_result = pressed[a];
			pressed[a] = pressed[num_positions-1];
			pressed[num_positions-1] = '\0';
			return temp_result;
		}
	}
	return -1;
}

uint8_t PS2Keyboard::positions_buffer () {	// returns the number of filled buffer slots
    for (int a=0; a<BUFFER_SIZE_PRESSED; a++) {
    	if (pressed[a] == '\0') {
    		return a;
    	}else if (a == (BUFFER_SIZE_PRESSED -1)) {
    		return BUFFER_SIZE_PRESSED;
    	}
    }
}


PS2Keyboard::PS2Keyboard() {
  // nothing to do here, begin() does it all
}

void PS2Keyboard::begin(uint8_t data_pin, uint8_t irq_pin) {
  uint8_t irq_num=0;

  DataPin = data_pin;

  // initialize the pins
#ifdef INPUT_PULLUP
  pinMode(irq_pin, INPUT_PULLUP);
  pinMode(data_pin, INPUT_PULLUP);
#else
  pinMode(irq_pin, INPUT);
  digitalWrite(irq_pin, HIGH);
  pinMode(data_pin, INPUT);
  digitalWrite(data_pin, HIGH);
#endif
  
  switch(irq_pin) {
    #ifdef CORE_INT0_PIN
    case CORE_INT0_PIN:
      irq_num = 0;
      break;
    #endif
    #ifdef CORE_INT1_PIN
    case CORE_INT1_PIN:
      irq_num = 1;
      break;
    #endif
    #ifdef CORE_INT2_PIN
    case CORE_INT2_PIN:
      irq_num = 2;
      break;
    #endif
    #ifdef CORE_INT3_PIN
    case CORE_INT3_PIN:
      irq_num = 3;
      break;
    #endif
    #ifdef CORE_INT4_PIN
    case CORE_INT4_PIN:
      irq_num = 4;
      break;
    #endif
    #ifdef CORE_INT5_PIN
    case CORE_INT5_PIN:
      irq_num = 5;
      break;
    #endif
    #ifdef CORE_INT6_PIN
    case CORE_INT6_PIN:
      irq_num = 6;
      break;
    #endif
    #ifdef CORE_INT7_PIN
    case CORE_INT7_PIN:
      irq_num = 7;
      break;
    #endif
    default:
      irq_num = 0;
      break;
  }
  head = 0;
  tail = 0;
  attachInterrupt(irq_num, ps2interrupt, FALLING);
}


