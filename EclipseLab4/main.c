#include "msp.h"
#define TICKS ((uint16_t)0x493F)
#define TICKS2 ((uint16_t)0x249F)

/**
 * main.c
 */

float TickLength = 1.333;   //uS
float SpeedOfSound = 0.034; //cm per uS
int c = 4;

int covDebug = 0;

volatile uint16_t CaptureValues [2] = {0};
volatile uint16_t ElapsedTicks = 0;
volatile float ElapsedTime = 0;
volatile float Distance = 0;

void timerA_stop(void){
    TIMER_A0->CTL &= TIMER_A_CTL_MC__STOP;
}

void timerA2_stop(void){
    TIMER_A2->CTL &= TIMER_A_CTL_MC__STOP;
}

void timerA2_start(void){
    TIMER_A2->CTL |= TIMER_A_CTL_MC__UPDOWN;
}

void timerA2_config(void){
    TIMER_A2->CTL       |= TIMER_A_CTL_CLR;
    TIMER_A2->CTL       |= TIMER_A_CTL_SSEL__SMCLK;
    TIMER_A2->CTL       |= TIMER_A_CTL_CLR;
    TIMER_A2->CTL       |= TIMER_A_CTL_ID_2; // Sets timer ID to 2 ---- division by 4
    TIMER_A2->CCR[0]    = TICKS2;
    TIMER_A2->CCR[4]    = TICKS2 -c;

    TIMER_A2->CCTL[4]   |= TIMER_A_CCTLN_OUTMOD_7; //Resets Output
    TIMER_A2->CCTL[4]   &= TIMER_A_CCTLN_OUTMOD_4; // Sets output to toggle


}


void timerA_config(void){

    //Configuring clock
    TIMER_A0->CTL       |= TIMER_A_CTL_CLR; // clears TimerA0
    TIMER_A0->CTL       |= TIMER_A_CTL_SSEL__SMCLK; //Use SMCLK
    TIMER_A0->CTL       |= TIMER_A_CTL_ID_2; // Sets timer ID to 2 ---- division by 4
    TIMER_A0->CCR[0]    = TICKS;
    TIMER_A0->CCR[1]    = TICKS;


    //configuring interrupts
    TIMER_A0->CCTL[1]   |= TIMER_A_CCTLN_CCIE;  //Enable interrupt for Trigger on sensor - P2.4
    TIMER_A0->CCTL[2]   |= TIMER_A_CCTLN_CCIE;  //Enable interrupt for  Echo on sensor - P2.5

    TIMER_A0->CCTL[2]   |= TIMER_A_CCTLN_CM__BOTH;   //set as capture input on rising and falling edge
    TIMER_A0->CCTL[2]   |= TIMER_A_CCTLN_CAP;   //set as capture mode

    TIMER_A0->CCTL[1]   &= ~TIMER_A_CCTLN_CCIFG; //clear interrupt flag
    TIMER_A0->CCTL[2]   &= ~TIMER_A_CCTLN_CCIFG; //clear interrupt flag


    //TIMER_A0->CCTL[1]   |= TIMER_A_CCTLN_OUTMOD_7; //Resets Output
    //TIMER_A0->CCTL[1]   &= TIMER_A_CCTLN_OUTMOD_4; // Sets output to toggle
}

void timerA_start(void){
    TIMER_A0->CTL |= TIMER_A_CTL_MC__UP;
}

void config_NVIC(void){
    __NVIC_EnableIRQ(TA0_N_IRQn); //enables timer A interrupt
}

void gpio_config(void){
    P2->DIR     |= BIT4;
    P6->DIR     |= BIT7;
    P6->OUT     &= ~(BIT7);
    P2->OUT     |= BIT4;            // Make sure that P2 is an output

    P6->SEL0    |= BIT7;
    P6->SEL1    &= ~(BIT7);
    P2->SEL0    &= ~BIT4;           // Select the mode
    P2->SEL1    &= ~(BIT4);

    P2->DIR     &= ~BIT5;           //configure P2.5 to input
    P2->OUT     &= ~BIT5;           //configure pull down on P2.5
    P2->REN     |= BIT5;            //enable pull down

    P2->SEL0    |=  BIT5;           //enable primary module function on P2.5 (TIMER_A0 CCR2)
    P2->SEL1    &=  ~BIT5;
}

void TA0_N_IRQHandler(void){
    __NVIC_DisableIRQ(TA0_N_IRQn); //disable since we're in the interrupt


    //if interrupt is on CCR[1] toggle the output
    if(TIMER_A0->CCTL[1] & TIMER_A_CCTLN_CCIFG){
        //Toggle P2.4
        P2->OUT ^= BIT4;

        // Clear the Interrupt Source Flag
        TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG;
    }

    if(TIMER_A0->CCTL[2] & TIMER_A_CCTLN_CCIFG){

        if(P2->IN & BIT5){
            CaptureValues[0] = TIMER_A0->CCR[2];
            TIMER_A0->CCTL[2] &= ~TIMER_A_CCTLN_COV;    //clear Overflow
        }
        if(~(P2->IN & BIT5)){
            CaptureValues[1] = TIMER_A0->CCR[2];
            if(TIMER_A0->CCTL[2] & TIMER_A_CCTLN_COV){
                CaptureValues[1] += TICKS;
                covDebug = 1;
            } else{
                covDebug = 0;
            }
        }
        // Clear the Interrupt Source Flag
        TIMER_A0->CCTL[2] &= ~TIMER_A_CCTLN_CCIFG;
    }

    __NVIC_EnableIRQ(TA0_N_IRQn); //enable interrupt since we are about to exit handler
}

//change
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

    timerA2_stop();
    timerA2_config();
    timerA2_start();

	timerA_stop();
	timerA_config();
	timerA_start();
	gpio_config();
	config_NVIC();

	while(1){
	    ElapsedTicks = CaptureValues[1] - CaptureValues[0]; //find elapsed ticks
	    ElapsedTime = ElapsedTicks * TickLength;        //convert ticks to time
	    Distance = ElapsedTime * SpeedOfSound / 2;              //centimeters
	}

}
