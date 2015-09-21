/*
 * temperature_demo.cpp - Motate
 * This file is part of the Motate project.
 *
 * Copyright (c) 2015c Robert Giseburt
 *
 *  This file is part of the Motate Library.
 *
 *  This file ("the software") is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2 as published by the
 *  Free Software Foundation. You should have received a copy of the GNU General Public
 *  License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  As a special exception, you may use this file as part of a software library without
 *  restriction. Specifically, if other files instantiate templates or use macros or
 *  inline functions from this file, or you compile this file and link it with  other
 *  files to produce an executable, this file does not by itself cause the resulting
 *  executable to be covered by the GNU General Public License. This exception does not
 *  however invalidate any other reasons why the executable file might be covered by the
 *  GNU General Public License.
 *
 *  THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 *  WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 *  SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *  OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. *
 */


// Here, we'll read a thermistor, and drive a heater to maintain a set temperature.

#include "MotatePins.h"
#include "MotateSerial.h"
#include "MotateJSON.h"
#include <cmath>
#include <algorithm> // for std::max

// Grab our local pin assignments, customized per board
#include "pin_assignments.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

/****** Create file-global objects ******/


OutputPin<kDebug1_PinNumber> debug_pin1;
OutputPin<kDebug2_PinNumber> debug_pin2;


const float kSystemVoltage = 3.3;


template<pin_number adc_pin_num, uint16_t min_temp = 0, uint16_t max_temp = 300, uint32_t table_size=64>
struct Thermistor {
    float c1, c2, c3, pullup_resistance;
    // We'll pull adc top value from the adc_pin.getTop()

    ADCPin<adc_pin_num> adc_pin;
    uint16_t raw_adc_value = 0;

    typedef Thermistor<adc_pin_num, min_temp, max_temp, table_size> type;

    // References for thermistor formulas:
    //  http://assets.newport.com/webDocuments-EN/images/AN04_Thermistor_Calibration_IX.PDF
    //  http://hydraraptor.blogspot.com/2012/11/more-accurate-thermistor-tables.html

    Thermistor(const float temp_low, const float temp_med, const float temp_high, const float res_low, const float res_med, const float res_high, const float pullup_resistance_)
    : pullup_resistance{ pullup_resistance_ }    {
        setup(temp_low, temp_med, temp_high, res_low, res_med, res_high);
        adc_pin.setInterrupts(kPinInterruptOnChange|kInterruptPriorityLow);
    }

    void setup(const float temp_low, const float temp_med, const float temp_high, const float res_low, const float res_med, const float res_high) {
        float temp_low_fixed = temp_low + 273.15;
        float temp_med_fixed = temp_med + 273.15;
        float temp_high_fixed = temp_high + 273.15;

        // Intermediates - using cryptic names from the calibration paper for consistency.

        float a1 = log(res_low);
        float a2 = log(res_med);
        float a3 = log(res_high);

        float z = a1 - a2;
        float y = a1 - a3;
        float x = 1/temp_low_fixed - 1/temp_med_fixed;
        float w = 1/temp_low_fixed - 1/temp_high_fixed;

        float v = pow(a1,3) - pow(a2,3);
        float u = pow(a1,3) - pow(a3,3);

        c3 = (x-z*w/y)/(v-z*u/y);
        c2 = (x-c3*v)/z;
        c1 = 1/temp_low_fixed-c3*pow(a1,3)-c2*a1;

//        int16_t temp = min_temp;
//        for (int i=0; temp < max_temp && i < table_size; i++) {
//            lookup_table[i][0] = adc_value(temp);
//            lookup_table[i][0] = temp;
//
//            temp += (min_temp-max_temp)/(table_size-1);
//        }
    };

    uint16_t adc_value_(int16_t temp) {
        float y = (c1 - (1/(temp+273.15))) / (2*c3);
        float x = sqrt(pow(c2 / (3*c3),3) + pow(y,2));
        float r = exp((x-y) - (x+y)); // resistance of thermistor
        return (r / (pullup_resistance + r)) * (adc_pin.getTop());
    };

    float temperature_exact() {
        // Sanity check:
        if (raw_adc_value < 1) {
            return -1; // invalid temperature from a thermistor
        }

        float v = (float)raw_adc_value * kSystemVoltage / (adc_pin.getTop()); // convert the 10 bit ADC value to a voltage
        float r = (pullup_resistance * v) / (kSystemVoltage - v);   // resistance of thermistor
        float lnr = log(r);
        float Tinv = c1 + (c2*lnr) + (c3*pow(lnr,3));
        return (1/Tinv) - 273.15; // final temperature
    };

    float resistance() {
        if (raw_adc_value < 1) {
            return -1; // invalid temperature from a thermistor
        }

        float v = (float)raw_adc_value * kSystemVoltage / (adc_pin.getTop()); // convert the 10 bit ADC value to a voltage
        return (pullup_resistance * v) / (kSystemVoltage - v);   // resistance of thermistor
    }

    struct ResistanceProperty_t {
        type &parent;

        operator float() {
            return parent.resistance();
        };

        void operator=(float) {;};
    };
    ResistanceProperty_t resistance_property {*this};

    template <uint16_t token_len, uint16_t description_len>
    auto json_bindings(const char (&token)[token_len], const char (&description)[description_len]) {
        return JSON::bind_object(token, description,
                                 JSON::bind_typed<float>("temp", *this, "temperature (ºC)", /*print precision:*/2),
                                 JSON::bind_typed<float>("res", resistance_property, "resistance measured", /*print precision:*/2)
                                 );
    }

    void adc_has_new_value() {
        raw_adc_value = (adc_pin.getRaw() + (9 * raw_adc_value))/10;
    };

    operator float() {
        return temperature_exact();
    };

    void operator=(float) {;};

};

namespace Motate {
    namespace JSON {
        bool relaxed_json = false;


//        template <pin_number adc_pin_num, uint16_t min_temp, uint16_t max_temp, uint32_t table_size>
//        struct binderWriter_t<Thermistor<adc_pin_num, min_temp, max_temp, table_size>> {
//            const int precision_ = 4;
//
//            constexpr binderWriter_t(int precision) : precision_{precision} {};
//
//            bool write_(str_buf &buf, Thermistor<adc_pin_num, min_temp, max_temp, table_size> &value_, bool verbose = 0) const {
//                return buf.copy(value_.temperature_exact(), precision_);
//            };
//        };
        
    }
}


// Output 1 FET info
const int16_t fet_pin1_freq = 100;
const int16_t fet_pin1_sample_freq = 1; // every fet_pin1_sample_freq interrupts, sample
int16_t fet_pin1_sample_counter = fet_pin1_sample_freq;

PWMOutputPin<kFET1_PinNumber> fet_pin1;
namespace Motate {
    template<>
    void PWMOutputPin<kFET1_PinNumber>::parentTimerType::interrupt() {
        if (!--fet_pin1_sample_counter) {
            ADC_Module::startSampling();
            fet_pin1_sample_counter = fet_pin1_sample_freq;
        }
    };
}

Thermistor<kADCInput1_PinNumber> thermistor1 {
    /*T1:*/    25, /*T2:*/  160, /*T3:*/ 235,
    /*R1:*/ 86500, /*R2:*/ 800, /*R3:*/ 190, /*pullup_resistance:*/ 4700};
void ADCPin<kADCInput1_PinNumber>::interrupt() {
    thermistor1.adc_has_new_value();
};



// Output 2 FET info
const int16_t fet_pin2_freq = 100;
// Sampling piggybacks off of the first fet timer

PWMOutputPin<kFET2_PinNumber> fet_pin2;
Thermistor<kADCInput2_PinNumber> thermistor2 {
    /*T1:*/    25, /*T2:*/  160, /*T3:*/ 235,
    /*R1:*/ 86500, /*R2:*/ 800, /*R3:*/ 190, /*pullup_resistance:*/ 4700};
void ADCPin<kADCInput2_PinNumber>::interrupt() {
    thermistor2.adc_has_new_value();
};

// Output 3 FET info
// Sampling piggybacks off of the first fet timer

// NOT a PWM pin -- bang bang!
OutputPin<kFET3_PinNumber> fet_pin3;
Thermistor<kADCInput3_PinNumber> thermistor3 {
    /*T1:*/    25, /*T2:*/  160, /*T3:*/ 235,
    /*R1:*/ 86500, /*R2:*/ 800, /*R3:*/ 190, /*pullup_resistance:*/ 4700};
void ADCPin<kADCInput3_PinNumber>::interrupt() {
    thermistor3.adc_has_new_value();
};

struct PID {
    static constexpr float output_max = 1.0;
    static constexpr float derivative_contribution = 0.05;

    float kP_;
    float kI_;
    float kD_;

    float proportional_ = 0.0; // integral_ storage
    float integral_ = 0.0; // integral_ storage
    float derivative_ = 0.0; // derivative_ storage
    float previous_input_ = 0.0; // derivative_ storage

    float setPoint_;

    constexpr PID(float P, float I, float D, float startSetPoint = 0.0) : kP_{P}, kI_{I}, kD_{D}, setPoint_{startSetPoint} {};

    float getNewOutput(float input) {
        // Calculate the e (error)
        float e = setPoint_ - input;

        float p = kP_ * e;
        // For output's sake, we'll store this, otherwise we don't need it:
        proportional_ = p;

        integral_ += e;

        if (integral_ < 0.0) {
            integral_ = 0.0;
        }

        float i = integral_ * kI_;

        if (i > output_max) {
            integral_ = output_max / kI_;
            i = output_max;
        }


        derivative_ = (kD_ * (input - previous_input_))*(derivative_contribution) + (derivative_ * (1.0-derivative_contribution));
        previous_input_ = input;

        return std::min(output_max, p + i - derivative_);
    };

    template <uint16_t token_len, uint16_t description_len, class... subTypes>
    auto json_bindings(const char (&token_name)[token_len], const char (&object_description)[description_len]) {
        return JSON::bind_object(token_name, object_description,
                                 JSON::bind("set", setPoint_,     "set temperature",            /*print precision:*/2),
                                 JSON::bind("p",   proportional_, "proportional scale setting", /*print precision:*/2),
                                 JSON::bind("i",   integral_,     "integral scale setting",     /*print precision:*/5),
                                 JSON::bind("d",   derivative_,   "deriviative scale setting",  /*print precision:*/5)
                                 );
    }
};

PID pid1 { 22.2/255.0, 1.08/255.0, 114.0/255.0};

float test = 0.1;

const auto json_base = JSON::parent("Temperature demo",
//                                    thermistor1.json_bindings("t1", "Thermistor 1"),
//                                    thermistor2.json_bindings("t2", "Thermistor 2"),
//                                    thermistor3.json_bindings("thb", "Thermistor HeatBed"),
//                                    JSON::bind_object("t1",
//                                                      "Thermistor 1",
//                                                      JSON::bind_typed<float>("temp", thermistor1,                     "temperature (ºC)", /*print precision:*/2),
//                                                      JSON::bind_typed<float>("res",  thermistor1.resistance_property, "resistance",       /*print precision:*/2)
//                                                      ),
//                                    JSON::bind_object("t2",
//                                                      "Thermistor 2",
//                                                      JSON::bind_typed<float>("temp", thermistor2,                     "temperature (ºC)", /*print precision:*/2),
//                                                      JSON::bind_typed<float>("res",  thermistor2.resistance_property, "resistance",       /*print precision:*/2)
//                                                      ),
//                                    JSON::bind_object("thb",
//                                                      "Thermistor HeatBed",
//                                                      JSON::bind_typed<float>("temp", thermistor3,                     "temperature (ºC)", /*print precision:*/2),
//                                                      JSON::bind_typed<float>("res",  thermistor3.resistance_property, "resistance",       /*print precision:*/2)
//                                                      ),
                                    JSON::bind_typed<float>  ("h1",  fet_pin1, "heater 1", /*precision:*/ 3),
                                    JSON::bind_typed<float>  ("h2",  fet_pin2, "heater 2", /*precision:*/ 3),
                                    JSON::bind_typed<bool>   ("hhb", fet_pin3, "heater for the heat bed"),
                                    pid1.json_bindings("pid1", "Heater 1 PID")
                                    );

// Just manual t1: 564 b
// Manual t1, t2, and thb: 1,188 b
// bindings_t1:    354 b


JSON::instruction_list_t<20> sr;
JSON::instruction_list_t<20> commands;

//const char json_sr[] { "{t1:n,h1:n,t2:n,h2:n,thb:n,hhb:n}" };//,t2:n,h2:n,thb:n,hhb:n
const char json_sr[] { "{t1:n,h1:n,pid1:n}" };//,t2:n,h2:n,thb:n,hhb:n
char json_str_pool[] { };//,t2:n,h2:n,thb:n,hhb:n

char read_buffer[1024] {0};
char *read_buffer_pos = read_buffer;
char write_buffer[1024] {0};

Timeout sr_timeout;
Timeout pid_timeout;

/****** Optional setup() function ******/

void setup() {
    fet_pin1 = 0.0;
    fet_pin2 = 0.0;
    fet_pin3 = 0;

    fet_pin1.setFrequency(fet_pin1_freq);
    fet_pin1.setInterrupts(kInterruptOnOverflow|kInterruptPriorityLow);

    fet_pin2.setFrequency(fet_pin2_freq);

    debug_pin1 = 0;
    debug_pin2 = 0;

    sr_timeout.set(100);
    pid_timeout.set(100);
}

/****** Main run loop() ******/

void loop() {
//    debug_pin1 = 1;

    int16_t v = Serial.readByte();

    if (v > 0) {
        *(read_buffer_pos++) = v;

        switch ((char)v) {
            case '\n':
            case '\r':
                *(read_buffer_pos) = 0;
                JSON::parse_json(commands, (char *)read_buffer);
                commands.exec(&json_base);
                commands.write(&json_base, write_buffer, sizeof(write_buffer));
                Serial.write(write_buffer, strlen(write_buffer));
                Serial.write("\n", 1);
                read_buffer_pos = read_buffer;
                *(read_buffer_pos) = 0;
                *write_buffer = 0;
                break;
        }
    }

    if (sr_timeout.isPast()) {
        strncpy(json_str_pool, json_sr, strlen(json_sr));

        JSON::parse_json(sr, json_str_pool);
        sr.write(&json_base, write_buffer, sizeof(write_buffer));
        Serial.write(write_buffer, strlen(write_buffer));
        Serial.write("\n", 1);

        sr_timeout.set(250);
    }


    if (pid_timeout.isPast()) {
        fet_pin1 = pid1.getNewOutput(thermistor1.temperature_exact());
        pid_timeout.set(100);
    }

//    delay(100);
}
