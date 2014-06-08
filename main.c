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

char reverse_bits(char input) {
	unsigned char output=0;
	for(unsigned char i=0;i<8;i++) {
		change_bit(output,7-i,test_bit(input,i))
	}
	return output;
}

// Writes an number of bytes to SPI
void spi_uart_tx(char *tbuf, uint32_t len) {
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;

	// This is Polled transfer as per section 10.6.1
	// BUG ALERT: what happens if we get interupted in this section, and someone else
	// accesses a different peripheral?
	
	// Clear TX and RX fifos
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);
	
	
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_LEN, BCM2835_SPI0_CS_LEN); //make it 9 bit
	
	
	// Set TA = 1
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);
	
	uint32_t i;
	for (i = 0; i < len; i++) {
		// Maybe wait for TXD
		while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
			;
		
		// Write to FIFO, no barrier
		bcm2835_peri_write_nb(fifo, reverse_bits(tbuf[i])+0x000);
		
		// Read from FIFO to prevent stalling
		while (bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD)
			(void) bcm2835_peri_read_nb(fifo);
		bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0);
	}
	
	// Wait for DONE to be set
	while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE)) {
		while (bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD)
			(void) bcm2835_peri_read_nb(fifo);
	}
	
	// Set TA = 0, and also set the barrier
	bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
	
	bcm2835_spi_end();
	
	bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_OUTP); // MOSI
	bcm2835_gpio_set(RPI_GPIO_P1_19); //idle high
}

int main(int argc, char **argv) {
	if (!bcm2835_init())
		return 1;
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);
	bcm2835_spi_setClockDivider(2170);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	
	unsigned char i='a';
	
	while(1) {
		char buffer[] = {i++};
		spi_uart_tx(buffer,sizeof(buffer));
		
		if(i>'z')
			i='\n';
		
		if(i==('\n'+1))
			i='a';
		
		bcm2835_delay(10);
	}
	
	bcm2835_close();
	
	return 0;
}

