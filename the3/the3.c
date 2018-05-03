//Omer CETIN 2257541
//Oguzcan BUDUMLU 2098820

//timer0 is used to generate 5 ms delays
//timer1 is used to generate 10 ms delay 
//each timers has its own internal counters to generate the desired time
/*These mentioned above and used ports for button initialized in 
 * initialization() function.But note that we did not 
 * enable global and peripheral interrupt in initialization part and did not 
 * initialize PORTB not to cause any conflict.
 * 
 * After initialization, necessary message is displayed and RE1 button is 
 * waited to push and release.
 * 
 * After pushing and releasing of RE1 button, we used busy-wait to generate
 * 3 s interrupt.
 * 
 * After this waiting, interrupts takes part in the scenario.Hence, global and
 * peripheral interrupts are enabled and Timer0 starts to count.
 * 
 * In the following stage, program is created with three (while) loop.
 * 
 * In the outmost loop, the pin is set.
 * 
 * After setting pit, in the middle loop, the user try to find correct pin. Also
 * note that Timer1 starts to count just before middle loop.
 * 
 * In the middle loop, while entering pin, the same code structure is used with
 * setting the pin.
 * 
 * If the user does not find correct pin in 120 seconds, a flag is set and used
 * to get out of the loops respectively and main function starts again.
 * 
 * If the user finds correct pin in 120 seconds, with a function of comparing 
 * the program enters innermost loop and stops there.
 
 
 */


#include <p18cxxx.h>
#include <p18f8722.h>
#pragma config OSC = HSPLL, FCMEN = OFF, IESO = OFF, PWRT = OFF, BOREN = OFF, WDT = OFF, MCLRE = ON, LPT1OSC = OFF, LVP = OFF, XINST = OFF, DEBUG = OFF

#define _XTAL_FREQ   40000000

#include "Includes.h"
#include "LCD.h"

unsigned int flagADCSampling = 0; //used when comparing ADC samples to check if there is a change in ADC value
unsigned int firstADCSample = 20000, secondADCSample = 30000; //used for taking samples from ADRES
unsigned int valuePortB; //used for reading PORTB
unsigned int counterFor100ms = 20, counterFor250ms = 50, counterFor500ms = 100; //counters to generate specific times under Timer0
unsigned int flagBlinking = 1; //if 0 then " ", else "#"
unsigned int initialValueTMR0 = 61; // timer0 initial value to enter timer0 interrupt in every 5 ms
unsigned int pinIndex = 0; // used when blinking the character and setting digits of the pin
unsigned char pinDigits[] = {'#', '#', '#', '#'}, setPinDigits[4]; //used for pin, each index is a digit of the pin
unsigned int flagSetPinUnfinished = 1; //pin setting session is not over yet
unsigned int flagDisplaySetPin = 0; //it allows to display the pin setting panel
unsigned char flagPinIsSet = 0; //when it is set, it shows the pin is designated
unsigned int flagDisplayNewPin = 0; //it allows to display the new pin just after the pin is designated
unsigned int initialValueTMR1 = 53036; //timer1 initial value to enter timer1 interrupt in every 10 ms
unsigned int counterFor1sec = 100; //counter to generate 1 sec under Timer1
unsigned int counterFor120sec = 120; //used for countdown when entering the pin
unsigned int d0 = 0, d1 = 1, d2 = 2, d3 = 0; //digits of the 7segment display
unsigned int flag1secPassed = 0; //when it is set it indicates 1sec has passed and under this flag update the 7segment with remaining time
unsigned int flag120secIsOver = 0; //when the 120 sec countdown is over it is set
unsigned int flagNewPinBlinking = 2; //when the pin is set this flag is used to blink the new pin message in 500 ms
unsigned int flagEnterPin = 0; //set when user reenters the designated pin 
unsigned int counterForNewPin = 0; //used to show the new pin message for 3 sec blinking in each 500 ms
unsigned int numberOfAttemps = 2; //number of attempts that user has 
unsigned int counterForBlocked20sec = 20; //counter for 20 sec when the user is blocked from entering the pin
unsigned int flagBlockedPanel = 0; //set when the user is blocked after 2 consecutive unsuccessful try
unsigned int digits7Segment[] = {//7segment display digits
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01100111, // 9
    0b01000000, // -
    0b00000000 // nothing
};

void DelayMS(int time) { //counter to generate 1ms or 3000ms delays
    long int counter = 0;
    if (time == 1) counter = 910;
    else if (time == 3000) counter = 2720000;
    while (counter--) {
        ;
    }
}

void InitializeADC() { //initializing the ADC module
    TRISH4 = 1; //ADC input pin to use potentiometer
    ADCON1 = 0b00000000; //all ports are set as analog
    ADCON0 = 0b00110000; //channel 12 for PORTH+ pn, GO_DONE is 0, ADC module is off
    ADCON2 = 0b10001111; //right justified
    ADON = 1; //turn on ADC module(ADCON0)
    ADIF = 0; //AD interrupt flag is 0 
    ADIE = 1; //AD interrupt is enabled
}

void InitializeTimer0() { //initializing the Timer0
    T0CON = 0b01000111; //pre-scale is 1:256, TMR0 as 8-bit
    TMR0IF = 0; //timer0 interrupt flag is 0
    TMR0IE = 1; //timer0 interrupt is enabled
    TMR0 = initialValueTMR0; //TMR0 starts from initialValueTMR0 to generate 5ms interrupt
}

void InitializePortB() { //initializing PORTB and PORTB interrupts
    TRISB7 = 1; //RB7 is set as input
    TRISB6 = 1; //RB6 is set as input
    TRISB5 = 0; //RB5 is set as output
    TRISB4 = 0; //RB4 is set as output
    PORTB = 0; //write to PORTB to clear portb flag 
    RBPU = 0; //PORTB pull up is 0
    valuePortB = PORTB; //read the PORTB to cleat portb flag
    LATB = 0; //clear latch b 
    RBIF = 0; //portb interrupt flag is cleared
    RBIE = 1; //portb interrupt is enabled
}

void InitializeRE1() { //initializing RE1
    TRISE1 = 1; //RE1 is set as in put
    PORTE = 0; //clear porte
    LATE = 0; //cleat late
}

void RE1PushRelease() { //RE1 push and release task
    while (RE1); //RE1 is hold with no action
    while (!RE1); //RE1 is hold with pushing
}

void Initialize7Segment() { //initializing 7segment display
    PORTH = 0; //clear porth
    LATH = 0; //clear latch h
    TRISJ = 0; //port j pins are set as output
    LATJ = 0; //clear latch j
    TRISH = TRISH & 0b11110000; //port h <0:3> pins are set as output
}

void InitializeTimer1() { //initializing the timer1
    T1CON = 0b10110000; //TMR1 as 16bit, pre-scale 1:8, internal clock is used, timer1 is off
    TMR1 = initialValueTMR1; //TMR1 is started from the given value to generate 10 ms interrupts
    TMR1IE = 1; //timer1 interrupt is enabled
    TMR1IF = 0; //timer1 interrupt flag is cleared
}

void InitializeAll() { //initializing needed parts
    InitLCD();
    InitializeRE1();
    InitializeTimer0();
    InitializeADC();
    Initialize7Segment();
    InitializeTimer1();
}

void DisplayIntroductionMessage() { //display introduction message
    ClearLCDScreen(); //clear LCD
    WriteCommandToLCD(0x80); //go to the first line of LCD
    WriteStringToLCD(" $>Very  Safe<$ "); //display $>very safe<$ on LCD
    WriteCommandToLCD(0xC0); //go to the second line of LCD
    WriteStringToLCD(" $$$$$$$$$$$$$$ "); //display $$$$$$$$$$$$$ on LCD
}

void DisplaySetPinPanel() { //display the message when setting the pin
    WriteCommandToLCD(0x80); //go to the first line of LCD
    WriteStringToLCD(" Set a pin:"); //display set a pin on LCD
    WriteDataToLCD(pinDigits[0]); //display the first digit of the pin on LCD
    WriteDataToLCD(pinDigits[1]); //display the second digit of the pin on LCD
    WriteDataToLCD(pinDigits[2]); //display the third digit of the pin on LCD
    WriteDataToLCD(pinDigits[3]); //display the fourth digit of the pin on LCD
    WriteDataToLCD(' '); //display a space at the end of the line
}

void DisplayBlockedPanel() { //display the message when the user is block from entering the pin
    WriteCommandToLCD(0x80); //go to the first line of LCD
    WriteStringToLCD(" Enter pin:XXXX"); //display enter pin:XXXX on LCD on the first line
    WriteCommandToLCD(0xC0); //go to the second line of LCD
    WriteStringToLCD("Try after 20sec."); //display try after 20sec. on the second line
}

void DisplayNewPinPanel() { //display the message when the new pin is set
    WriteCommandToLCD(0x80); //go to the first line of LCD
    WriteStringToLCD(" The new pin is "); //display the new pin is on the first line of LCD
    WriteCommandToLCD(0xC0); //go to the second line of LCD
    WriteStringToLCD("   ---"); //display ---  
    WriteDataToLCD(pinDigits[0]); //display the first digit of the pin 
    WriteDataToLCD(pinDigits[1]); //display the second digit of the pin
    WriteDataToLCD(pinDigits[2]); //display the third digit of the pin
    WriteDataToLCD(pinDigits[3]); //display the fourth digit of the pin
    WriteStringToLCD("---   "); //display ---
}

void DisplaySafeIsOpening() { //display the message when the user reenters the correct pin
    ClearLCDScreen(); //clear the LCD screen
    WriteCommandToLCD(0x80); //go to the first line of LCD
    WriteStringToLCD("Safe is opening!"); //display safe is opening on the first line
    WriteCommandToLCD(0xC0); //go to the second line of LCD
    WriteStringToLCD("$$$$$$$$$$$$$$$$"); //display $$$$$$$$$$$$$$$
}

char MapADCValueToNumber(unsigned int analog) { //mapping ADC value to a number in between 0 and 9
    if (analog < 100) return '0';
    if (analog < 200) return '1';
    if (analog < 300) return '2';
    if (analog < 400) return '3';
    if (analog < 500) return '4';
    if (analog < 600) return '5';
    if (analog < 700) return '6';
    if (analog < 800) return '7';
    if (analog < 900) return '8';
    if (analog < 1024) return '9';
}

unsigned int ToggleBlinkingCharacter() { //toggling flagBlinking to generate blinking on digits of the pin
    if (flagBlinking) return 0; //toggle flagBlinking       ex: flagBlinking = ToggleBlinkingCharacter
    else return 1; //toggle flagBlinking       ex: flagBlinking = ToggleBlinkingCharacter
}

unsigned int DecideToBlink() { //it decides if the digit of the pin will blink or not
    if (pinDigits[pinIndex] != '#' && pinDigits[pinIndex] != ' ') return 0; //if digit of the pin is set dont blink
    else return 1; //continue to blink if the digit is still '#' or ' '
}

void DisplayDashOn7Segment() { //display dash on 7segment display
    LATJ = 0b01000000; //corresponding to '-' for 7segment display
    LATH3 = 1; //write '-' to 7segment display
    LATH2 = 1; //write '-' to 7segment display
    LATH1 = 1; //write '-' to 7segment display
    LATH0 = 1; //write '-' to 7segment display
}

void Display7Segment() {
    LATH = LATH & 0xF0; // <0:3> pins are set as 0 to clear 7-segment
    LATJ = digits7Segment[d3]; // take necessary number from array
    LATH3 = 1; //activate d0 part of 7-segment to light
    DelayMS(1);
    LATH3 = 0; //deactivate d0 part of 7-segment not to light another number
    LATJ = digits7Segment[d2]; // take necessary number from array
    LATH2 = 1; //activate d1 part of 7-segment to light
    DelayMS(1);
    LATH2 = 0; //deactivate d1 part of 7-segment not to light another number
    LATJ = digits7Segment[d1]; // take necessary number from array
    LATH1 = 1; //activate d2 part of 7-segment to light
    DelayMS(1);
    LATH1 = 0; //deactivate d2 part of 7-segment not to light another number
    LATJ = digits7Segment[d0]; // take necessary number from array
    LATH0 = 1; //activate d3 part of 7-segment to light
    DelayMS(1);
    LATH0 = 0; //deactivate d3 part of 7-segment not to light another number
}

int CompareSetAndEnteredPin() { //compare the designated pin and the entered pin
    for (int i = 0; i < 4; i++) {
        if (setPinDigits[i] != pinDigits[i]) return 0; //if there is a difference return false
    }
    return 1; //there is no difference return true
}

void PassNumberTo7Segment(unsigned int number) { //used to pass a number to the 7segment display
    d3 = number % 10; //calculate the units digit of the number
    number /= 10; //divide the number by 10
    d2 = number % 10; //calculate the tens digit of the number
    number /= 10; //divide the number by 10
    d1 = number % 10; //calculate the hundreds digit of the number
    d0 = 0; //the leftmost digit of the 7segment display is always 0
    Display7Segment(); //call Display7Segment function to write the value above to the 7segment display
}

void DisplayEnterPinPanel() { //display the message when the user reenter the pin to find the correct pin
    WriteCommandToLCD(0x80); //go to the first line of LCD
    WriteStringToLCD(" Enter pin:"); //display enter pin on the first line
    WriteDataToLCD(pinDigits[0]); //display the first digit of yhe pin
    WriteDataToLCD(pinDigits[1]); //display the second digit of the pin
    WriteDataToLCD(pinDigits[2]); //display the third digit of the pin
    WriteDataToLCD(pinDigits[3]); //display the fourth digit of the pin
    WriteStringToLCD(" "); //display a space
    WriteCommandToLCD(0xC0); //go to the second line of LCD
    WriteStringToLCD("  Attempts:"); //display attempts:
    WriteDataToLCD('0' + numberOfAttemps); //display the remaining attempts that the user has
    WriteStringToLCD("    "); //display spaces
}

void PreparePanelForEnterPin() { //when the pin is designated prepare the panel for user to reenter the pin
    setPinDigits[0] = pinDigits[0]; //save the first digit of the designated pin
    setPinDigits[1] = pinDigits[1]; //save the second digit of the designated pin
    setPinDigits[2] = pinDigits[2]; //save the third digit of the designated pin
    setPinDigits[3] = pinDigits[3]; //save the fourth digit of the designated pin
    pinDigits[0] = '#'; //prepare pinDigits for the user to reenter the pin
    pinDigits[1] = '#'; //prepare pinDigits for the user to reenter the pin
    pinDigits[2] = '#'; //prepare pinDigits for the user to reenter the pin
    pinDigits[3] = '#'; //prepare pinDigits for the user to reenter the pin
    ClearLCDScreen(); //clear the LCD screen
    DisplayEnterPinPanel(); //display pin entering panel
}

void interrupt ISR() {
    if (RBIE && RBIF) {
        RBIF = 0;
        if (!RB6) {
            if (flagPinIsSet == 0) {
                if (pinIndex < 4 && DecideToBlink() == 0) pinIndex++;
            }
        }
        if (!RB7 && flagPinIsSet) flagDisplayNewPin = 1;
    } else if (TMR0IE && TMR0IF) { //TMR0IE may be disabled somewhere
        if (flagSetPinUnfinished) { //if the pin is not set
            TMR0IF = 0; //indicate the system is in interrupt
            TMR0 = initialValueTMR0; //to adjust timer0 interrupt for every 5 ms 
            counterFor100ms--; //to catch 100 ms
            counterFor250ms--; //to catch 250 ms
            if (counterFor100ms == 0) { //TIMER0 interrupt with a period of 100 ms 
                counterFor100ms = 20; //default value of counterFor100ms to catch 100  ms
                GO_DONE = 1; //start A/D conversion
            }
            if (counterFor250ms == 0) { //led blinking in every 250 when the cell is either '#' or ' '
                counterFor250ms = 50; //default value of counterFor100ms to catch 50  ms
                if (DecideToBlink()) { //decide whether setting of any value with potentiometer
                    flagBlinking = ToggleBlinkingCharacter(); //choose '#' or ' '
                    if (flagBlinking) pinDigits[pinIndex] = '#'; //if value in the cell ' ', then it becomes '#'
                    else pinDigits[pinIndex] = ' '; //if value in the cell '#', then it becomes ' '
                } else flagBlinking = 2; //if value in the cell is any number adjusted with potentiometer, flagBlinking becomes random value i.e. 2 except for 0 and 1
                flagDisplaySetPin = 1; // indicate to display updated set pin panel
            }
        } else { //if the pin is set
            TMR0IF = 0; //ADIF is reset.
            TMR0 = initialValueTMR0; //to adjust timer0 interrupt for every 5 ms 
            counterFor500ms--; //to catch 500 ms
            if (counterFor500ms == 0) { //lcd module blinking in every 500 ms during 3 s when the pin is set using the RB7 button.
                counterForNewPin++; //to catch 3 s in total while blinking for new message panel
                counterFor500ms = 100; //default value of counterFor500ms to catch 500 ms
                if (flagNewPinBlinking == 1) flagNewPinBlinking = 0;
                else if (flagNewPinBlinking == 0) flagNewPinBlinking = 1;
                else flagNewPinBlinking = 2;
            }
        }
    } else if (ADIE && ADIF && flagSetPinUnfinished) { //ADIE may be disabled somewhere
        ADIF = 0; //ADIF is reset.
        if (flagADCSampling == 0) { //if sample is not taken yet
            firstADCSample = ADRES; //take a sample read from potentiometer from potentiometer
            flagADCSampling = 1; //indicate taken value from potentiometer to display via main function
        } else { //if sample is taken
            secondADCSample = ADRES; //take another sample read from potentiometer from potentiometer
            if (firstADCSample - secondADCSample < 10 || secondADCSample - firstADCSample < 10) { //if there is any change in potentiometer, take the value. (because of sensibility of potenitometer, 10 is selected.)
                flagADCSampling = 0; //if two samples are taken, sampling stage is reset
            } else { //if there is no change in potentiometer
                pinDigits[pinIndex] = MapADCValueToNumber(secondADCSample); //set pin digit by taking from variable with respect to table in homework
                flagADCSampling = 0; //if two samples are taken, sampling stage is reset
                flagDisplaySetPin = 1; //display set pin panel 
            }
            firstADCSample = 100000; //default value 
            secondADCSample = 200000; //default value
        }
        GO_DONE = 0; //finish A/D conversion
    } else if (TMR1IE && TMR1IF) {
        TMR1IF = 0; // TMR1F is reset.
        TMR1 = initialValueTMR1; //default value of TMR1 to adjust 10 ms
        counterFor1sec--; //counter to catch 1 s
        if (!counterFor1sec && !flag120secIsOver) {
            counterFor1sec = 100; //default value
            flag1secPassed = 1; //indicate 1 sec is passed to display via main function
        } else Display7Segment(); //light 7-segment
    }
}

int main() {
    InitializeAll(); //initialize all necessary ports and variables
    DisplayIntroductionMessage(); // display "very safe" message
    RE1PushRelease(); //wait for RE1 push-release for next part
    DelayMS(3000); //wait for 3-seconds
    ClearLCDScreen(); //clear LCD panel for another stage
    GIE_GIEH = 1; //enable global interrupt
    PEIE_GIEL = 1; //enable peripheral interrupt
    TMR0ON = 1; //start to count for tmr0 interrupt
    InitializePortB(); //initialize port B for set pin stage
    while (1) {
        if (flagDisplaySetPin) { //if adjusted as 1 in interrupt
            DisplaySetPinPanel(); //display set pin panel in LCD
            flagDisplaySetPin = 0; //default value
        }
        if (pinIndex == 4) { //if all digits of pin is set
            flagPinIsSet = 1; //when pin is set
        }
        if (flagDisplayNewPin) { //if adjusted as 1 in interrupt
            flagNewPinBlinking = 1; //to display new pin message
            TMR0 = initialValueTMR0; //TMR0 is adjusted to catch 1 ms interrupt
            flagSetPinUnfinished = 0; //when set pin stage is finished
            flagDisplayNewPin = 0; //default value
            ClearLCDScreen(); //clear LCD panel for another stage
        }
        if (flagNewPinBlinking == 1) { //if adjusted as 1 in interrupt
            DisplayDashOn7Segment(); //while the system is started in enter pin stage
            DisplayNewPinPanel(); //display new pin in LCD
        }
        if (flagNewPinBlinking == 0) { //if adjusted as 1 in interrupt
            ClearLCDScreen(); //clear LCD panel for another stage
        }
        if (counterForNewPin == 6) { //lcd module blinks 3 times (means that 3 times the message is showed and 3 times lcd module is cleared) 
            flagEnterPin = 1; //indicates the system is entered "Enter pin" stage
            flagNewPinBlinking = 2; //default value
            flagSetPinUnfinished = 1; //when set pin stage is started
            pinIndex = 0; //when set pin stage is started
            PreparePanelForEnterPin(); //to store pin
            counterForNewPin = 0; //default value of it
            flagPinIsSet = 0; //for new pin set stage
        }
        if (flagEnterPin) { //when the system enters in enter pin stage
            TMR1ON = 1; //start to count for tmr0 interrupt
            while (1) {
                //Display7Segment();
                if (flagDisplaySetPin && !flagBlockedPanel) { //if the user is not punished for 20 seconds
                    DisplayEnterPinPanel(); // display enter pin panel in LCD 
                    flagDisplaySetPin = 0; // default value
                }
                if (pinIndex == 4) { //if all digits of pin is set
                    flagPinIsSet = 1; //when pin is set
                }

                if (flagDisplayNewPin && !flagBlockedPanel) { //if the user is not punished for 20 seconds
                    flagDisplayNewPin = 0;
                    if (CompareSetAndEnteredPin() == 1) { //when the entered pin matches with set pin
                        DisplaySafeIsOpening(); //display necessary message in LCD
                        while (1) {
                            //the system is unlocked
                        }
                    } else {
                        --numberOfAttemps; //decreases in every attemp
                        if (numberOfAttemps == 0) { //when the attemp right is not left, the user is punished with 20 sec?bds
                            ClearLCDScreen(); //clear LCD panel for another stage
                            numberOfAttemps = 2; //after punished, two rights is given.
                            DisplayBlockedPanel(); //display panel that the system is locked 
                            flagBlockedPanel = 1; //change the state of system 
                        }
                        pinDigits[0] = '#'; //prepare for new enter pin panel stage
                        pinDigits[1] = '#'; //prepare for new enter pin panel stage
                        pinDigits[2] = '#'; //prepare for new enter pin panel stage
                        pinDigits[3] = '#'; //prepare for new enter pin panel stage
                        pinIndex = 0; //prepare for new enter pin panel stage
                        flagPinIsSet = 0; //prepare for new enter pin panel stage
                    }
                }
                if (flag1secPassed) { //when the 1 sec is passed while counting from 120 to 0
                    if (flagBlockedPanel) { //if the user is punished for 20 seconds
                        if (--counterForBlocked20sec) DisplayBlockedPanel(); //while 20 seconds is passed
                        else flagBlockedPanel = 0; //punishment with 20 seconds is over
                    }
                    if (!counterFor120sec) { //when the counter reaches to 0
                        flag120secIsOver = 1; //indicate counter reaches to 0
                        break; //start again the system
                    } else PassNumberTo7Segment(counterFor120sec--); //display the numnber in 7-segment while counting
                    flag1secPassed = 0; //default value
                }
            }
        }
        if (flag120secIsOver) break; //start again the system
    }
}

