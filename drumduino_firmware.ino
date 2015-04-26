#include <SoftwareSerial.h>

#define USE_DISPLAY 1

#if USE_DISPLAY
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
#endif


enum DrumduinoFirmwareSettings {
	PORT_CNT = 1,
	CHAN_PER_PORT_CNT = 1,
	PAD_CNT = PORT_CNT * CHAN_PER_PORT_CNT,
	FRAME_BUFFER_SIZE = 3,
};

enum Pins {
	PIN_MULTIPLEX_A = 2,
	PIN_MULTIPLEX_B = 3 ,
	PIN_MULTIPLEX_C = 4 ,

	PIN_SOFTSERIAL_RX = 5,
	PIN_SOFTSERIAL_TX = 6,
};


//=================================================================================
// Set Bit and Clwear Bit Helpers
//=================================================================================
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


//=================================================================================
// Prescaler
//=================================================================================
// Maximum sampling frequency    // Resolution
enum Prescaler {
	Prescaler_2   = B00000000, // 16 MHz / 2 = 8 MHz            //
	Prescaler_4   = B00000010, // 16 MHz / 4 = 4 MHz            // ~5.9
	Prescaler_8   = B00000011, // 16 MHz / 8 = 2 MHz            // ~7.4
	Prescaler_16  = B00000100, // 16 MHz / 16 = 1 MHz           // ~8.6
	Prescaler_32  = B00000101, // 16 MHz / 32 = 500 kHz         // ~8.9
	Prescaler_64  = B00000110, // 16 MHz / 64 = 250 kHz         // ~9.0
	Prescaler_128 = B00000111, // 16 MHz / 128 = 125 kHz        // ~9.1
};

inline void setPrescaler(int prescaler) {
	ADCSRA &= B11111000;
	ADCSRA |= prescaler;
}


//=================================================================================
//  Ad Pin
//=================================================================================
enum AdPin {
	AdPin_0   = B00000000,
	AdPin_1   = B00000001,
	AdPin_2   = B00000010,
	AdPin_3   = B00000011,
	AdPin_4   = B00000100,
	AdPin_5   = B00000101,
	AdPin_6   = B00000110, // Bei Atmega8 nur in der Gehäusebauform TQFP und MLF verfügbar, nicht in PDIP
	AdPin_7   = B00000111, // Bei Atmega8 nur in der Gehäusebauform TQFP und MLF verfügbar, nicht in PDIP
	AdPin_Vbg = B00001110, // 1.23V
	AdPin_GND = B00001111, // 0V
};

inline void setAdPin(int adPin) {
	ADMUX &= B11110000;
	ADMUX |= adPin;
}

//=================================================================================
// ADC Alignment
//=================================================================================
// Das Ergebnis wird in den Registern ADCH/ADCL linksbündig ausgerichtet.
// Die 8 höchstwertigen Bits des Ergebnisses werden in ADCH abgelegt.
// Die verbleibenden 2 niederwertigen Bits werden im Register ADCL in den Bits 6 und 7 abgelegt.
enum AdcAlignment {
	ADAlignmentLeft  = B00100000,
	ADAlignmentRight = B00000000,
};


inline void setADAlignment(int align) {
	ADMUX &= ~B00100000;
	ADMUX |= align;
}

//=================================================================================
inline void startADCConversion() {
	ADCSRA |= B01000000;
}

//=================================================================================

inline void disableAnalogComparator() {
	ACSR = B10000000;
}

inline void multiplexSelectChan(uint8_t chan) {
	PORTD = B00011100 & (chan << 2) | (B11100011 & PORTD);
}

//=================================================================================
// MIDI
//=================================================================================
namespace midi {
	/// http://www.midi.org/techspecs/midimessages.php
#if 0
	struct SysexFrame {
		byte begin = 0xf0;
		byte manufacturer = 42;
		unsigned long time1 = 0;
		unsigned long time2 = 0;
		byte values[PAD_CNT] = { 0 };
		byte end = 0xF7;
	};
#endif

	/**
	Note Off event.
	This message is sent when a note is released (ended).
	*/
	template<typename SERIAL_IF>
	void noteOn(SERIAL_IF& serial, uint8_t note, uint8_t velocity) {
		serial.write((uint8_t)0x90);
		serial.write((uint8_t)note);
		serial.write((uint8_t)velocity);
	}

	/**
	Note On event.
	This message is sent when a note is depressed (start).
	*/
	template<typename SERIAL_IF>
	void noteOff(SERIAL_IF& serial, uint8_t note, uint8_t velocity) {
		serial.write((uint8_t)0x80);
		serial.write((uint8_t)note);
		serial.write((uint8_t)velocity);
	}

	/**
	Polyphonic Key Pressure (Aftertouch).
	This message is most often sent by pressing down on the key after it "bottoms out".
	*/
	template<typename SERIAL_IF>
	void polyphonicKeyPressure(SERIAL_IF& serial, uint8_t note, uint8_t pressure) {
		serial.write((uint8_t)0xA0);
		serial.write((uint8_t)note);
		serial.write((uint8_t)pressure);
	}

	/**
	Control Change.
	This message is sent when a controller value changes.
	Controllers include devices such as pedals and levers.
	Controller numbers 120-127 are reserved as "Channel Mode Messages" (below).
	*/
	template<typename SERIAL_IF>
	void controlChange(SERIAL_IF& serial, uint8_t controllerNumber, uint8_t controllerValue) {
		serial.write((uint8_t)0xB0);
		serial.write((uint8_t)controllerNumber);
		serial.write((uint8_t)controllerValue);
	}
}



//=================================================================================
//
//=================================================================================
struct Configuration {
	enum Type {
		TypeDisabled,
		TypePiezo,
	} type[PAD_CNT] = { TypePiezo };

	struct CurveSettings {
		enum CurveType {
			CurveNormal,
			CurveExp,
			CurveLog,
			CurveSigma,
			CurveFlat,
			CurveExtra,
		};

		CurveType type = CurveNormal;
		uint8_t value = 127;
		int8_t offset = 0;
		uint8_t factor = 127;
	} curve[PAD_CNT];

	uint8_t note[PAD_CNT] = { 0 };
	uint8_t threshold[PAD_CNT] = { 25 };
	uint8_t scanTime[PAD_CNT] = { 25 };
	uint8_t maskTime[PAD_CNT] = { 35 };

} g_configuration;


//=================================================================================
//
//=================================================================================
struct Runtime {
	SoftwareSerial softSerial{ PIN_SOFTSERIAL_RX, PIN_SOFTSERIAL_TX }; // RX, TX

	uint8_t value[PAD_CNT][FRAME_BUFFER_SIZE];

	enum State {
		StateAwait,
		StateScan,
		StateMask,
	} state[PAD_CNT] = { StateAwait };

	uint8_t  trigger[PAD_CNT] = { 0 };
	uint8_t max[PAD_CNT] = { 0 };
	//uint64_t sum[PAD_CNT] = { 0 };

	uint64_t frameCounter = 0;
#if USE_DISPLAY
	bool displayTriggerEvent[PAD_CNT] = { 0 };
#endif
} g_runtime;


//=================================================================================
//
//=================================================================================
inline uint8_t calcCurve(const Configuration::CurveSettings& curveSettings, uint8_t value) {
	uint8_t ret = 0;

	float x = value * 8.0;
	float f = ((float)curveSettings.value) / 64.0; //[1;127]->[0.;2.0]

	switch(curveSettings.type) {
		//[0-1023]x[0-127]
		case Configuration::CurveSettings::CurveNormal:
			ret = x * f / 16.0;
			break;

		case Configuration::CurveSettings::CurveExp:
			ret = (127.0 / (exp(2.0 * f) - 1)) * (exp(f * x / 512.0) - 1.0);
			break; //Exp 4*(exp(x/256)-1)

		case Configuration::CurveSettings::CurveLog:
			ret = log(1.0 + (f * x / 128.0)) * (127.0 / log((8 * f) + 1));
			break; //Log 64*log(1+x/128)

		case Configuration::CurveSettings::CurveSigma:
			ret = (127.0 / (1.0 + exp(f * (512.0 - x) / 64.0)));
			break; //Sigma

		case Configuration::CurveSettings::CurveFlat:
			ret = (64.0 - ((8.0 / f) * log((1024 / (1 + x)) - 1)));
			break; //Flat

		case Configuration::CurveSettings::CurveExtra:
			ret = (x + 0x20) * f / 16.0;

	}

	ret = ret * (curveSettings.factor / 127.0) + curveSettings.offset;

	if(ret <= 0) {
		return 0;
	}

	if(ret >= 127) {
		return 127;    //127
	}

	return ret;
}


//=================================================================================
//
//=================================================================================
void setup() {
#if USE_DISPLAY
	lcd.begin(16, 2);
	delay(500);
	lcd.print("Drumduino  0.0.1");
	delay(1000);
#endif


	memset(g_runtime.value, 0, sizeof(g_runtime.value));

	// generate note mapping
	for(uint8_t pad = 0; pad < PAD_CNT; ++pad) {
		uint8_t note = 0x1E + pad;
		g_configuration.note[pad] = note;
	}

	// Setup MultiplexSelection Pins
	pinMode(PIN_MULTIPLEX_A, OUTPUT);
	pinMode(PIN_MULTIPLEX_B, OUTPUT);
	pinMode(PIN_MULTIPLEX_C, OUTPUT);

	// Setup ADCs
	analogReference(DEFAULT);
	disableAnalogComparator();
	setPrescaler(Prescaler_8);
	//setADAlignment(ADAlignmentLeft);

	// Disable digital input buffers on all analog input pins
	DIDR0 = DIDR0 | B00111111;

	// Setup Serial
	Serial.begin(2000000);
	Serial.flush();

	g_runtime.softSerial.begin(31250);
	//g_runtime.softSerial.flush();
}


//=================================================================================
//
//=================================================================================
void loop() {
	uint64_t& frameCounter = g_runtime.frameCounter;
	size_t curFrameIdx = frameCounter % FRAME_BUFFER_SIZE;
	size_t lastFrameIdx = (frameCounter - 1) % FRAME_BUFFER_SIZE;

	for(uint8_t chan = 0; chan < CHAN_PER_PORT_CNT; ++chan) {
		multiplexSelectChan(chan);

		for(uint8_t port = 0; port < PORT_CNT; ++port) {
			int pad = port * CHAN_PER_PORT_CNT + chan;

			// shortcuts!
			uint8_t& currentValue = g_runtime.value[pad][curFrameIdx];
			const uint8_t& lastValue = g_runtime.value[pad][lastFrameIdx];

			Runtime::State& state = g_runtime.state[pad];
			uint8_t& triggerFrame = g_runtime.trigger[pad];
			uint8_t& maxValue = g_runtime.max[pad];
			//const uint8_t& sumValue = g_runtime.sum[pad];

			const Configuration::Type& type = g_configuration.type[pad];
			const uint8_t& threshold = g_configuration.threshold[pad];
			const uint8_t& scanTime = g_configuration.scanTime[pad];
			const uint8_t& maskTime = g_configuration.maskTime[pad];

			// real processing
			currentValue = uint8_t(analogRead(port) >> 3);

			switch(type) {
				case Configuration::TypePiezo: {
					switch(state) {
						// In this state we wait for a signal to trigger
						default:
						case Runtime::StateAwait: {
STATE_AGAIN:

							if(currentValue < lastValue + threshold) {
								break;
							}

							state = Runtime::StateScan;
							triggerFrame = frameCounter;
							maxValue = currentValue;
							//sumValue = currentValue;
							//### fallthrough
						}

						// In this state we measure the value for the given time period to get the max value
						case Runtime::StateScan: {
							if(frameCounter < triggerFrame + scanTime) {
								maxValue = max(currentValue, maxValue);
								//sumValue += currentValue;
								break;
							}

							const Configuration::CurveSettings& curve = g_configuration.curve[pad];
							uint8_t velocity = calcCurve(curve, maxValue);

							const uint8_t& note = g_configuration.note[pad];
							midi::noteOn(g_runtime.softSerial, note, velocity);
							state = Runtime::StateMask;
#if USE_DISPLAY
							g_runtime.displayTriggerEvent[pad] = true;
#endif
							//### fallthrough
						}

						// In this state we do nothing to prevent retriggering
						case Runtime::StateMask: {
							if(frameCounter < triggerFrame + scanTime + maskTime) {
								break;
							}

							state = Runtime::StateAwait;
							//goto STATE_AGAIN;
						}
					}
				}
			}
		}
	}

#if USE_DISPLAY

	if(frameCounter % 1000 == 0) {
		lcd.clear();

		lcd.setCursor(0, 0);

		switch(g_configuration.type[0]) {
			default:
			case Configuration::TypeDisabled: {
				lcd.print("Off");
				break;
			}

			case Configuration::TypePiezo: {
				lcd.print("Piez");
				break;
			}
		}

		lcd.setCursor(5, 0);
		lcd.print(g_configuration.note[0]);
		lcd.setCursor(9, 0);
		lcd.print(g_configuration.scanTime[0]);
		lcd.setCursor(13, 0);
		lcd.print(g_configuration.threshold[0]);


		lcd.setCursor(0, 1);
		lcd.print("CurV:");
		lcd.setCursor(6, 1);
		lcd.print(g_runtime.value[0][curFrameIdx]);

		lcd.setCursor(11, 1);

		switch(g_runtime.state[0]) {
			default:
			case Runtime::StateAwait: {
				lcd.print("A");
				break;
			}

			case Runtime::StateScan: {
				lcd.print("S");
				break;
			}

			case Runtime::StateMask: {
				lcd.print("M");
				break;
			}
		}

		lcd.setCursor(13, 1);

		if(g_runtime.displayTriggerEvent[0]) {

			lcd.print("[x]");
		}
		else {
			lcd.print("[ ]");
		}
	}

	if(frameCounter % 1000 == 0) {
		memset(g_runtime.displayTriggerEvent, 0, sizeof(g_runtime.displayTriggerEvent));
	}

#endif

	++frameCounter;
}
