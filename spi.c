#include "msp430g2553.h"

unsigned char MST_Data, SLV_Data;

void main(void)
{
  volatile unsigned int i;

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  P1OUT = 0x00;                             // P1 setup for LED & reset output
  P1DIR |= BIT0 + BIT4;                     //
  P1SEL = BIT5 + BIT6 + BIT7;
  P1SEL2 = BIT5 + BIT6 + BIT7;
  UCB0CTL0 = UCCKPL + UCMSB + UCMST + UCSYNC;  // 3-pin, 8-bit SPI master
  UCB0CTL1 = UCSSEL_2 + UCSWRST;                     // SMCLK
  UCB0BR0 = 8;                          // /8
  UCB0BR1 = 0;                              //
  UCB0MCTL = 0;                             // No modulation
  UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IFG2 &= ~UCB0RXIE;
  IE2 |= UCB0RXIE;                          // Enable USCI0 RX interrupt


  P1OUT &= ~BIT4;                           // Now with SPI signals initialized,
  P1OUT |= BIT4;                            // reset slave

  __delay_cycles(75);                 // Wait for slave to initialize

  MST_Data = 0x01;                          // Initialize data values
  SLV_Data = 0x00;

  UCB0TXBUF = MST_Data;                     // Transmit first character

  __bis_SR_register(LPM0_bits + GIE);       // CPU off, enable interrupts
}

// Test for valid RX and TX character
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
  volatile unsigned int i;

  while (!(IFG2 & UCB0TXIFG));              // USCI_B0 TX buffer ready?

  if (UCB0RXBUF == SLV_Data)                // Test for correct character RX'd
    P1OUT |= BIT0;                          // If correct, light LED
  else
    P1OUT &= ~BIT0;                         // If incorrect, clear LED

  MST_Data++;                               // Increment master value
  SLV_Data++;                               // Increment expected slave value
  UCB0TXBUF = MST_Data;                     // Send next value

  __delay_cycles(50);                     // Add time between transmissions to
}
