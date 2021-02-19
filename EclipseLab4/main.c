#include "msp.h"
#define TICKS ((uint16_t)0x493E)

/**
 * main.c
 */

//change
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
}

void timerA_config(void){
    TIMER_A0->CTL       |= TIMER_A_CTL_CLR; // clears TimerA0
    TIMER_A0->CTL       |= TIMER_A_CTL_SSEL__SMCLK; //Use SMCLK
    TIMER_A0->CTL       |= TIMER_A_CTL_ID_2; // Sets timer ID to 1
    TIMER_A0->CCR[0]    |= TICKS;
    TIMER_A0->CCTL[1]   |= TIMER_A_CCTLN_OUTMOD_7; //Resets Output
    TIMER_A0->CCTL[1]   |= TIMER_A_CCTLN_OUTMOD_4; // Sets output to toggle
    TIMER_A0->CCTL[1]   |= TIMER_A_CCTLN_CCIE;
}

void config_NVIC(void){
    __NVIC_EnableIRQ(TA0_0_IRQn); //enables timer A interrupt
}

void gpio_config(void){
    P2->DIR     |= BIT4;
    P2->OUT     |= BIT4;                    // Make sure that P2 is an output
  //P2->DS      |= BIT4;                    // Sets drive strength to high
    P2->SEL0    |= BIT4;                    // Select the mode
    P2->SEL1    &= ~(BIT4);
}

void TA0_0_IRQHandler(void){
//Toggle P2.0
    P2->OUT ^= BIT0;
// Clear the Interrupt Source Flag
    P6->IFG &= ~BIT1;
}
