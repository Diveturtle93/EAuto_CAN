//----------------------------------------------------------------------
// Titel	:	CAN_Bus.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	Jun 28, 2023
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	EAuto_CAN
//----------------------------------------------------------------------

// Dateiheader definieren
//----------------------------------------------------------------------
#ifndef INC_CAN_BUS_H_
#define INC_CAN_BUS_H_
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------
#include "can.h"
//----------------------------------------------------------------------

// Konstanten definieren
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Struktur definieren
//----------------------------------------------------------------------
typedef enum
{
	false,
	true
} bool;
//----------------------------------------------------------------------
typedef enum
{
	RX_SIZE_2 = (uint16_t)2,
	RX_SIZE_4 = (uint16_t)4,
	RX_SIZE_8 = (uint16_t)8,
	RX_SIZE_16 = (uint16_t)16,
	RX_SIZE_32 = (uint16_t)32,
	RX_SIZE_64 = (uint16_t)64,
	RX_SIZE_128 = (uint16_t)128,
	RX_SIZE_256 = (uint16_t)256,
	RX_SIZE_512 = (uint16_t)512,
	RX_SIZE_1024 = (uint16_t)1024,
} RXQUEUE_TABLE;
//----------------------------------------------------------------------
typedef enum
{
	TX_SIZE_2 = (uint16_t)2,
	TX_SIZE_4 = (uint16_t)4,
	TX_SIZE_8 = (uint16_t)8,
	TX_SIZE_16 = (uint16_t)16,
	TX_SIZE_32 = (uint16_t)32,
	TX_SIZE_64 = (uint16_t)64,
	TX_SIZE_128 = (uint16_t)128,
	TX_SIZE_256 = (uint16_t)256,
	TX_SIZE_512 = (uint16_t)512,
	TX_SIZE_1024 = (uint16_t)1024,
} TXQUEUE_TABLE;
//----------------------------------------------------------------------
typedef struct CAN_message_t
{
	uint32_t id;															// CAN Identifier
	uint16_t timestamp;														// Zeit wann die Nachricht angekommen ist
	uint8_t idhit;															// Filter der die Nachricht erkannt hat
	struct
	{
		bool extended;														// Erweiterte ID (29 Bit)
		bool remote;														// Remote Sendeanforderung
		bool overrun;														// Nachrichten ueberlauf
		bool reserved;
	} flags;
	uint8_t len;															// Nachrichtenlaenge
	uint8_t buf[8];															// Datenbuffer
	int8_t mb;																// Identifier welche Mailbox benutzt werden soll
	uint8_t bus;															// Identifier welche Bus benutzt werden soll
	bool seq;																// Sequentiale Rahmen
} CAN_massage_t;
//----------------------------------------------------------------------
typedef struct RingbufferTypeDef
{
	volatile uint16_t head;													// Kopf des Ringbusses
	volatile uint16_t tail;													// Schwanz des Ringbusses
	uint16_t size;															// Groesse des Ringbusses
	volatile CAN_massage_t *buffer;											// Nachrichtenbuffer
} RingbufferTypeDef;
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
void CANinit(RXQUEUE_TABLE rxSize, TXQUEUE_TABLE txSize);
bool CANwrite(CAN_massage_t *CAN_tx_msg, bool MB);
uint8_t CAN_available(void);
bool CANread(CAN_massage_t *CAN_rx_msg);
bool isInitialized(void);
void initializeBuffer(void);
void initRingBuffer(RingbufferTypeDef *ring, volatile CAN_massage_t *buffer, uint32_t size);
bool addToRingBuffer(RingbufferTypeDef *ring, CAN_massage_t *msg);
bool removeFromRingBuffer(RingbufferTypeDef *ring, CAN_massage_t *msg);
bool isRingBufferEmpty(RingbufferTypeDef *ring);
//----------------------------------------------------------------------

#endif /* INC_CAN_BUS_H_ */
//----------------------------------------------------------------------
