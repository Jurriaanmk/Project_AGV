#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pindefines.h"

#define SPEED 120
#define TURNADJUST 40
#define TURNTHRESH 20

#define SENSEWALL_SETTING  0
#define SENSETREES_SETTING 1

#define DIST_DIF (read_adc(0) - read_adc(1))

int timercounter2 = 0;  //used to limit distance measurements
int treeside = 1;       //defines which side sensor looks for trees, 0=right

ISR(TIMER2_OVF_vect)
{
    timercounter2++;
}

int read_adc(uint8_t analogreadpin)     //get distance values from IR sensors
{
    ADMUX &= 0b11111000;            //clear pin selection bits
    analogreadpin &= 0b00000111;    //filter pin selection bits
    ADMUX |= analogreadpin;         //set specified pin

    ADCSRA |= (1<<ADSC);    //start conversion
    loop_until_bit_is_clear(ADCSRA,ADSC);   //wait for conversion to finish

    return ADC;
}

void detect_tree(void)
{
    int sensor = treeside+2;    //select appropriate sensor
    if(bit_is_clear(INPUT_PIN,TREE_IR_R))
    {
        ledon();
        move_stop();
        buzzer(1);
        _delay_ms(500);    //stop at tree
        buzzer(0);

        while(bit_is_clear(INPUT_PIN,TREE_IR_R))    //continue until tree is passed
        {
            RSPEED = SPEED;
            LSPEED = SPEED;
        }
        ledoff();
    }
}

int main(void)
{
    pinsetup();
    pwm_init();
    adc_init();
    timer_init();
    sei();

    //loop_until_bit_is_clear(START_PIN,START_BUTTON);    //wait for start signal

    while(1)
    {
        //wall detection
        if (read_adc(3)<800 && SENSEWALL_SETTING)
        {
            turn_left();
        }

        //tree detection (move to ISR probably)
        if(SENSETREES_SETTING)
        {
            detect_tree();  //handles tree detection routine
        }

        if(DIST_DIF < TURNTHRESH && DIST_DIF > -TURNTHRESH)
        {
            RSPEED = SPEED;
            LSPEED = SPEED;
        }
        else if (read_adc(0) > read_adc(1))     //left turn
        {
            RSPEED = SPEED;
            LSPEED = SPEED - TURNADJUST;
            TIFR2 |= (1<<TOV2);
        }
        else if (read_adc(0) < read_adc(1))     //right turn
        {
            LSPEED = SPEED;
            RSPEED = SPEED - TURNADJUST;
            TIFR2 |= (1<<TOV2);
        }
    }
    return 0;
}
