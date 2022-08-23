//----------------------------------------------------------------------
// Titel	:	CAN_Bus.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	Aug 23, 2022
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	STM32F767ZI
//----------------------------------------------------------------------

// Dateiheader definieren
//----------------------------------------------------------------------
#ifndef INC_CAN_BUS_H_
#define INC_CAN_BUS_H_
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------
#include <inttypes.h>
//----------------------------------------------------------------------

// Konstanten definieren
//----------------------------------------------------------------------
#define CAN_BUFFER_SIZE					10									// Anzahl der Ringpuffer-Elemente
#define ANZAHL_OUTPUT_PAKETE 			4									// Anzahl der Pakete fuer den CAN-Bus
//----------------------------------------------------------------------

// Struktur definieren
//----------------------------------------------------------------------
typedef struct
{
	uint16_t id;
	uint8_t length;
	uint16_t sendeintervall;
	uint16_t sende_init_time;
} Output_Paket;
//----------------------------------------------------------------------
typedef struct
{
	Output_Paket Paket[ANZAHL_OUTPUT_PAKETE];								// Ringpuffer mit Anzahl Elementen
	volatile uint8_t head;													// Verfuegbare Elemente
	volatile uint8_t tail;													// Letztes gesendetes Element
} ring_buffer;
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
void Ringbuf_init(void);
void CAN_isr (CAN_HandleTypeDef* hcan);
void CAN_write(CAN_HandleTypeDef* hcan);
Output_Paket Test(uint16_t id, uint8_t length, uint16_t sendeintervall, uint16_t sende_init_time);
//----------------------------------------------------------------------

#endif /* INC_CAN_BUS_H_ */
//----------------------------------------------------------------------
