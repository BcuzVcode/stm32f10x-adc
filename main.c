#include "stm32f10x.h"
#include <stdio.h>
#include <stdint.h>


void ADC_Init(void);
uint16_t ADC_Read(void);
void initialise_monitor_handles(void);
int putc(int ch, FILE *f);
void USART_Init(void);

// Function to initialize semihosting
void USART_Init(void) {
    initialise_monitor_handles(); // Initialize semihosting
}
void initialise_monitor_handles(void) {
    // Simple breakpoint for semihosting initialization
    __asm volatile("bkpt 0xAB");
}

void ADC_Init(void){

	// 1. Enabling clock for GPIO and ADC 
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;		// ENABLE GPIO CLOCK
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;		// ENABLE ADC1 CLOCK
	
	// 2. Configure PA0 as Analog input (ADC1 Channel 0)
	GPIOA->CRL &= ~GPIO_CRL_CNF0;			// CLEAR BITS FOR ANALOG MODE
	GPIOA->CRL &= ~GPIO_CRL_MODE0;		// MODE == 00 (INPUT MODE)
	
	// 3. Configure ADC1
	ADC1->CR2 &= ~ADC_CR2_EXTSEL;
	ADC1->CR2 |= (0b111 << 17);
	
	ADC1->CR2 |= ADC_CR2_ADON;				// ENABLE ADC1
	ADC1->CR2 |= ADC_CR2_CAL;					// START ADC1 CALIBRATION
	
	while(ADC1->CR2 & ADC_CR2_CAL);
	
	ADC1->SMPR2 &= ~ADC_SMPR2_SMP0;
	ADC1->SMPR2 |= (0b111 << 0);
	
}

uint16_t ADC_Read(void){

	ADC1->SQR3 = 0;
	ADC1->CR2 |= ADC_CR2_SWSTART;
	
	while (!(ADC1->SR & ADC_SR_EOC));
	
	return ((ADC1->DR) & 0x0FFF);
}
// Redirect output to semihosting
int fputc(int ch, FILE *f) {
    if (ch == '\n') {
        fputc('\r', f); // Send carriage return for new lines
    }
    
    // Prepare semihosting call parameters
    int r0 = 0x3;   // SYS_WRITE (stdout)
    int r1 = ch;    // Character to write
    int r2 = 1;     // Number of bytes to write

    // Use inline assembly to call the semihosting function
    __asm volatile (
        "mov r0, %0\n"   // Move r0 into the first parameter
        "mov r1, %1\n"   // Move r1 into the second parameter
        "mov r2, %2\n"   // Move r2 into the third parameter
        "bkpt 0xAB"      // Call semihosting function
        : 
        : "r"(r0), "r"(r1), "r"(r2)  // Input operands
        : "r0", "r1", "r2"           // Clobbered registers
    );

    return ch; // Return character
}

int main(void){

	SystemInit();
	USART_Init();
	ADC_Init();
	
	uint16_t adcValue;
	
	while(1){
		adcValue = ADC_Read();
		printf("Current ADC Reading is %d", adcValue);
		
		uint32_t delay_ms = 1000;
		while(delay_ms--){
			__NOP();
		}
	}
	
}