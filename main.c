// spin.c
//
// Example program for bcm2835 library
// Shows how to interface with SPI to transfer a number of bytes to and from an SPI device
//
// After installing bcm2835, you can build this 
// with something like:
// gcc -o spin spin.c -l bcm2835
// sudo ./spin
//
// Or you can test it before installing with:
// gcc -o spin -I ../../src ../../src/bcm2835.c spin.c
// sudo ./spin
//
// Author: Mike McCauley
// Copyright (C) 2012 Mike McCauley
// $Id: RF22.h,v 1.21 2012/05/30 01:51:25 mikem Exp $

#include <bcm2835.h>
#include <stdio.h>
#include "bitop.h"

unsigned int reverse_bits(unsigned int input) {
	const unsigned int bit_count = 8;
	unsigned int output=0;
	for(unsigned char i=0;i<bit_count;i++) {
		change_bit(output,(bit_count-1)-i,test_bit(input,i))
	}
	return output;
}

#define START_BIT 0
#define STOP_BIT 1

// Writes an number of bytes to SPI
void spi_uart_tx(char c) {
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;
	
	bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0);
	
	//BUG: The start bit is always 1.5 periods long, probably unfixable
	
	// This is Polled transfer as per section 10.6.1
	// BUG ALERT: what happens if we get interupted in this section, and someone else
	// accesses a different peripheral?
	
	// Clear TX and RX fifos
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);
	
	// Set TA = 1
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);
	
		// Maybe wait for TXD
		while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
			;
		
		// Write to FIFO, no barrier
		bcm2835_peri_write_nb(fifo, reverse_bits(c));
		
		// Read from FIFO to prevent stalling
		while (bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD)
			(void) bcm2835_peri_read_nb(fifo);
// 		bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0);
	
	// Wait for DONE to be set
	while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE)) {
// 		while (bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD)
// 			(void) bcm2835_peri_read_nb(fifo);
	}
	
// 	bcm2835_delayMicroseconds(10);
	
	// Set TA = 0, and also set the barrier
	bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
	
	//TODO: THe program might be interrupted in here, corrupting the character.
	
	bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_OUTP); // MOSI
	bcm2835_gpio_set(RPI_GPIO_P1_19); //idle high
	bcm2835_gpio_set_pud(RPI_GPIO_P1_19,BCM2835_GPIO_PUD_UP);
	bcm2835_delayMicroseconds(40);
}

int main(int argc, char **argv) {
	if (!bcm2835_init())
		return 1;
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);
	bcm2835_spi_setClockDivider(2170-200);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_LEN, BCM2835_SPI0_CS_LEN); //make it 9 bit
	
	volatile uint32_t* ltoh = bcm2835_spi0+BCM2835_SPI0_LTOH/4;
	bcm2835_peri_set_bits(ltoh,0b1111,0);
	
	unsigned int i='a';
	
	while(1) {
		for(char c='a';c<='z';c++) {
			spi_uart_tx(c);
			spi_uart_tx(' ');
		}
		spi_uart_tx('\n');
		
// 		bcm2835_delay(10);
// 		printf(" ");
	}
	
	bcm2835_spi_end();
	bcm2835_close();
	
	return 0;
}

