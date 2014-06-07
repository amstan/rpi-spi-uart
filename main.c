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

// Writes an number of bytes to SPI
void bcm2835_spi_writenb_ex(char* tbuf, uint32_t len)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;

	// This is Polled transfer as per section 10.6.1
	// BUG ALERT: what happens if we get interupted in this section, and someone else
	// accesses a different peripheral?
	
	// Clear TX and RX fifos
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);
	
	// Set TA = 1
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);
	
	uint32_t i;
	for (i = 0; i < len; i++) {
		// Maybe wait for TXD
// 		while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
// 			;
		
		// Write to FIFO, no barrier
		bcm2835_peri_write_nb(fifo, tbuf[i]);
		
		// Read from FIFO to prevent stalling
// 		while (bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD)
// 			(void) bcm2835_peri_read_nb(fifo);
	}
	
	// Wait for DONE to be set
	while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE)) {
		while (bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD)
			(void) bcm2835_peri_read_nb(fifo);
	}
	
	// Set TA = 0, and also set the barrier
	bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}

int main(int argc, char **argv)
{
	// If you call this, it will not actually access the GPIO
	// Use for testing
// 	bcm2835_set_debug(1);
	
	if (!bcm2835_init())
		return 1;
	
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
	bcm2835_spi_setClockDivider(2170);     // The default
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
	
	// Send a some bytes to the slave and simultaneously read 
	// some bytes back from the slave
	// Most SPI devices expect one or 2 bytes of command, after which they will send back
	// some data. In such a case you will have the command bytes first in the buffer,
	// followed by as many 0 bytes as you expect returned data bytes. After the transfer, you 
	// Can the read the reply bytes from the buffer.
	// If you tie MISO to MOSI, you should read back what was sent.
	
	while(1) {
		char buf[] = { 0xff,0xff }; // Data to send
		bcm2835_spi_writenb_ex(buf, sizeof(buf));
		// buf will now be filled with the data that was read from the slave
		printf("Read from SPI: 0x%02X, 0x%02X \n", buf[0], buf[1]);
		bcm2835_delay(10);
	}
	
	bcm2835_spi_end();
	bcm2835_close();
	return 0;
}

