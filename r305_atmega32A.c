/*
 * Author : Avarjana Panditha (Faculty of Information Technology, University of Moratuwa)
 * Support : Atmega 328p
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL // Changing this value is not recommended and use an external 16Mhz oscillator
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "r305.h"

/////////////////////////////////////SERIAL COMMUNICATION////////////////////////////////////////////////
void finger_init (void)
{
	UBRRH = (BAUDRATE>>8);							// shift the register right by 8 bits
	UBRRL = BAUDRATE;								// set baud rate
	UCSRB|= (1<<TXEN)|(1<<RXEN);					// enable receiver and transmitter
	UCSRB &= ~(1 << RXCIE);
	UCSRC|= (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);	// 8bit data format
}

void finger_uart_transmit (unsigned char data)
{
	while (!( UCSRA & (1<<UDRE)));				// wait while register is free
	UDR = data;									// load data in the register
}

unsigned char finger_uart_recieve (void)
{
	while(!((UCSRA) & (1<<RXC)));					// wait while data is being received
	return UDR;									// return 8-bit data
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void finger_transmit_header(uint16_t length){
	//Header
	finger_uart_transmit(239);
	finger_uart_transmit(1);
	
	//Adder
	finger_uart_transmit(0xFF);
	finger_uart_transmit(0xFF);
	finger_uart_transmit(0xFF);
	finger_uart_transmit(0xFF);
	
	//Package identifier (PID)
	finger_uart_transmit(1); //Command Packet Identifier
	
	//Package length
	finger_uart_transmit(length & 0xFF00);
	finger_uart_transmit(length & 0x00FF);
}

uint16_t finger_receive(){
	uint16_t len,ret;
	//header
	finger_uart_recieve();
	finger_uart_recieve();
	
	//module address
	finger_uart_recieve();
	finger_uart_recieve();
	finger_uart_recieve();
	finger_uart_recieve();
	
	//pid
	finger_uart_recieve();
	
	//package length
	len = finger_uart_recieve()<<8 | finger_uart_recieve();
	len-=2;
	
	ret = finger_uart_recieve();
	if(len==0x0002){
		ret = (finger_uart_recieve()<<8);
	}else if(len==0x0003){
		if(ret==0x00){
			ret = finger_uart_recieve()<<8 | finger_uart_recieve();
		}
	}
	
	//checksum
	finger_uart_recieve(); 
	finger_uart_recieve();
		
	return ret;
}

uint16_t finger_search_receive(){
	uint16_t ret=0;
	//header
	finger_uart_recieve();
	finger_uart_recieve();
	
	//module address
	finger_uart_recieve();
	finger_uart_recieve();
	finger_uart_recieve();
	finger_uart_recieve();
	
	//package_id
	finger_uart_recieve();
	
	//package length
	finger_uart_recieve();
	finger_uart_recieve();
	
	//conf code
	uint8_t temp;
	temp = finger_uart_recieve();
	//pageid
	if(temp==0x01){
		uint8_t low = finger_uart_recieve();
		uint8_t high = finger_uart_recieve();
		ret = (low <<8)+high;
	}
	else if(temp==0x09){
		ret = 0;
		finger_uart_recieve();	
		finger_uart_recieve();	
	}
	else{
		ret = finger_uart_recieve();
		ret = (finger_uart_recieve()<<8);
	}
	
	//score
	finger_uart_recieve();
	finger_uart_recieve();
	
	//checksum
	finger_uart_recieve();
	finger_uart_recieve();
	
	return ret;
	//return temp;
}

char* finger_get_response_string(uint8_t code){
	switch(code){
		case 0x00: return "complete";
		case 0x01: return "error receiving data";
		case 0x02: return "no finger";
		case 0x03: return "fail to enroll";
		case 0x06: return "disorderly fingerprint image";
		case 0x07: return "small fingerprint image";
		case 0x08: return "finger doesn’t match";
		case 0x09: return "fail to find the matching";
		case 0x0A: return "fail to combine";
		case 0x0B: return "invalid PageID";
		case 0x0C: return "invalid template";
		case 0x0D: return "error uploading template";
		case 0x0E: return "can’t data packages.";
		case 0x0F: return "error uploading image";
		case 0x10: return "fail to delete template";
		case 0x11: return "fail to clear library";
		case 0x13: return "wrong password!";
		case 0x15: return "invalid primary image";
		case 0x18: return "error writing flash";
		case 0x19: return "undefined error";
		case 0x1A: return "invalid register";
		case 0x1B: return "incorrect configuration";
		case 0x1C: return "wrong notepad page";
		case 0x1D: return "failed communication port";
	}
	return "Unknown error";
}

uint16_t finger_read(){
	uint16_t code = 2;
	
	while(code!=0x0000){
		finger_transmit_header(0x0003);
	
		finger_uart_transmit(0x01); //instruction
	
		//sum
		finger_uart_transmit(0x00);
		finger_uart_transmit(0x05);
		
		code = finger_receive();
		
		_delay_ms(500);
	}
	
	return code;
}

uint16_t upImg(){
	finger_transmit_header(0x0003);
	
	finger_uart_transmit(0x0A);
	
	finger_uart_transmit(0x00);
	finger_uart_transmit(0x0E);
	
	return finger_receive();
}

uint16_t upChar(uint16_t buff_id){
	finger_transmit_header(0x0004);
	
	finger_uart_transmit(0x08); //instruction
	
	finger_uart_transmit(buff_id); //buffer number
	
	buff_id += 7; //sum
	finger_uart_transmit(buff_id & 0xFF00);
	finger_uart_transmit(buff_id & 0x00FF);
	
	return finger_receive();
}

uint16_t finger_generate_char_file(uint16_t buff_id){
	finger_transmit_header(0x0004);
	
	finger_uart_transmit(0x02); //instruction
	
	finger_uart_transmit(buff_id); //buffer number
	
	buff_id += 7; //sum
	finger_uart_transmit(buff_id & 0xFF00);
	finger_uart_transmit(buff_id & 0x00FF);
	
	return finger_receive();
}

uint16_t finger_generate_template(){
	finger_transmit_header(0x0003);
	
	finger_uart_transmit(0x05); //instruction
	
	//sum
	finger_uart_transmit(0x00);
	finger_uart_transmit(0x09);
	
	return finger_receive();
}

uint16_t finger_get_storage_location(){
	finger_transmit_header(0x0003);
	
	finger_uart_transmit(0x1d); //instruction
	
	//sum
	finger_uart_transmit(0x00);
	finger_uart_transmit(0x21);
	
	return finger_receive();
}

uint16_t finger_delete_all(){
	finger_transmit_header(0x0003);
	
	finger_uart_transmit(0x0d); //instrucction
	
	finger_uart_transmit(0x00);
	finger_uart_transmit(0x11);
	
	return finger_receive();
}

uint16_t finger_store(uint16_t location){
	finger_transmit_header(0x0006);
	
	finger_uart_transmit(0x06); //instruciton
	
	finger_uart_transmit(0x01); //buffer id
	
	//location
	
	finger_uart_transmit(location & 0xFF00);
	finger_uart_transmit(location & 0x00FF);
	
	//sum
	location += 14;
	finger_uart_transmit(location & 0xFF00);
	finger_uart_transmit(location & 0x00FF);
	
	return finger_receive();
}

uint16_t finger_delete(uint16_t location){
	finger_transmit_header(0x0007);
	
	finger_uart_transmit(0x0C); //instruction
	
	//location
	finger_uart_transmit(location & 0xFF00);
	finger_uart_transmit(location & 0x00FF);
	
	//number of templates to delete
	finger_uart_transmit(0x00);
	finger_uart_transmit(0x01);
	
	//sum
	location += 21;
	finger_uart_transmit(location & 0xFF00);
	finger_uart_transmit(location & 0x00FF);
	
	return finger_receive();
}

uint16_t finger_search(){
	
	
	finger_transmit_header(0x0008);
	
	finger_uart_transmit(0x04); //instruction
	
	finger_uart_transmit(0x01); //buffer = 1
	
	//start location
	finger_uart_transmit(0x00);
	finger_uart_transmit(0x01);
	
	//length
	finger_uart_transmit(0x00);
	finger_uart_transmit(0xFF);
	
	//sum
	finger_uart_transmit(0x01);
	finger_uart_transmit(0x0E);
	
	return finger_search_receive();
}


