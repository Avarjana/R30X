/*
 * Author : Avarjana Panditha (Faculty of Information Technology, University of Moratuwa)
 * Support : R30X Optical Fingerprint Series with Atmega 32A
 * Datasheet :	http://www.rhydolabz.com/documents/finger-print-module.pdf
 * Datasheet (mirror link) : https://drive.google.com/open?id=1o4q0tGSPaEOwfNWyWO5uH2c7qbCzvQpJ
 */ 

#ifndef R305_H_
#define R305_H_

#ifndef F_CPU
#define F_CPU 16000000UL // Changing this value is not recommended and use an external 16Mhz oscillator 
#endif

#define BAUD 57600 // 9600*N (N={1,2,3,....,11,12} [default N = 6]
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)

void	 finger_init();							// Initialize the finger print sensor. Use inside main function
uint16_t finger_read();							// reading fingerprint
uint16_t finger_generate_char_file(uint16_t);	// generate char file from buffer. send buff id 0x01 or 0x02
uint16_t finger_generate_template();			// comabine buffers to generate template
uint16_t finger_get_storage_location();			// get pageId to store the template in flash storage
char*	 finger_get_response_string(uint8_t);	// get the string representation of response codes
uint16_t finger_delete_all();					// delete all the templates in flash store
uint16_t finger_store(uint16_t);				// stores fingerprint template at given location. send location from finger_get_storage_location();
uint16_t finger_delete(uint16_t);				// delets the template at the location. send location to delete
uint16_t finger_search();						// search flash sotreage for matching templates

void enroll_finger(); //sequence for enrolling a finger

uint16_t upImg();
uint16_t upChar(uint16_t);
#endif /* R305_H_ */



