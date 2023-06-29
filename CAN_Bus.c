//----------------------------------------------------------------------
// Titel	:	CAN_Bus.c
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	Jun 28, 2023
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	EAuto_CAN
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der eigenen Include Dateien
//----------------------------------------------------------------------
#include "CAN_Bus.h"
//----------------------------------------------------------------------

// Variablen definieren
//----------------------------------------------------------------------
RingbufferTypeDef rxRing;													// Empfangsring initialisieren
RingbufferTypeDef txRing;													// Sendering initialisieren

uint16_t sizeRxBuffer = 0;													// Groesse Empfangsring
uint16_t sizeTxBuffer = 0;													// Groesse Sendering

volatile CAN_massage_t *rxBuffer;											// Empfangsbuffer
volatile CAN_massage_t *txBuffer;											// Sendebuffer

bool canIsActive = false;													// Status
//----------------------------------------------------------------------

// Initialisiere und starte CAN-Bus
//----------------------------------------------------------------------
void CANinit(RXQUEUE_TABLE rxSize, TXQUEUE_TABLE txSize)
{
	// Beende wenn CAN-Bus aktive ist
	if (canIsActive)
		return;

	// Status CAN aktiv
	canIsActive = true;

	// Lege Groesse des Ringbuffers fest
	sizeRxBuffer = rxSize;
	sizeTxBuffer = txSize;

	// Initialisiere Ringbusbuffer
	initializeBuffers();

	// Konfiguriere CAN
	MX_CAN1_init();
}
//----------------------------------------------------------------------

// Schreibe Nachricht auf CAN-Bus oder in den Ringbuffer
//----------------------------------------------------------------------
bool CANwrite(CAN_massage_t *CAN_tx_msg, bool sendMB)
{
	bool ret = true;
	uint32_t TxMailbox;
	CAN_TxHeaderTypeDef TxHeader;

	// Schalte Sendeinterrupt aus
	__HAL_CAN_DISABLE_IT(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);

	// CAN-Nachricht hat extended ID
	if (CAN_tx_msg->flags.extended == 1)
	{
		TxHeader.ExtId = CAN_tx_msg->id;
		TxHeader.IDE = CAN_ID_EXT;
	}
	// CAN-Nachricht hat standart ID
	else
	{
		TxHeader.StdId = CAN_tx_msg->id;
		TxHeader.IDE = CAN_ID_EXT;
	}

	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = CAN_tx_msg->len;
	TxHeader.TransmitGlobalTime = DISABLE;

	// Nachricht auf Bus schreiben
	if (HAL_CAN_AddTxMessage(&hadc1, &TxHeader, CAN_tx_msg->buf, &TxMailbox) != 0)
	{
		// Wenn Nachricht nicht gesendet werden kann in Ring schreiben
		if (sendMB != true)
		{
			// Wenn Ring keinen Platz mehr hat
			if (addToRingBuffer(&txRing, CAN_tx_msg) == false)
			{
				ret = false;												// Kein Platz mehr im Ringbuffer
			}
		}
		// Wenn Nachricht nicht in den Ring geschrieben werden soll
		else
		{
			ret = false;
		}
	}

	// Schalte Sendeinterrupt ein
	__HAL_CAN_ENABLE_IT(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);
	return ret;
}
//----------------------------------------------------------------------

// Abfrage, ob CAN-Nachricht verfuegbar ist
//----------------------------------------------------------------------
uint8_t CAN_availible(void)
{
	if (rxRing.head >= rxRing.tail)
		return rxRing.head - rxRing.tail;
	else
		return rxRing.size - (rxRing.tail - rxRing.head);
}
//----------------------------------------------------------------------

// Nachricht von Ringbuffer einlesen
//----------------------------------------------------------------------
bool CANread(CAN_massage_t *CAN_rx_msg)
{
	bool ret;

	// Schalte Empfangsinterrupt aus
	__HAL_CAN_DISABLE_IT(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

	// Lese Nachricht
	ret = removeFromRingBuffer(&rxRing, CAN_rx_msg);

	// Schalte Empfangsinterrupt ein
	__HAL_CAN_ENABLE_IT(&hcan1, CAN_IT_RX_FIFO0_MSG_PENGING);

	return ret;
}
//----------------------------------------------------------------------

// Abfrage, ob Ringbuffer initialisiert ist
//----------------------------------------------------------------------
bool isInitialized(void)
{
	return rxBuffer != 0;
}
//----------------------------------------------------------------------

// Initialisiere Ringbuffer
//----------------------------------------------------------------------
void initializeBuffers(void)
{
	if (isInitialized())
		return;

	// Konfiguriere den Sende Ringbuffer
	if (txBuffer == 0)
	{
		txBuffer = malloc(sizeTxBuffer * sizeof(CAN_massage_t));
	}

	initRingBuffer(&txRing, txBuffer, sizeTxBuffer);

	// Konfiguriere den Empfang Ringbuffer
	if (rxBuffer == 0)
	{
		rxBuffer = malloc(sizeRxBuffer * sizeof(CAN_massage_t));
	}

	initRingBuffer(&rxRing, rxBuffer, sizeRxBuffer);
}
//----------------------------------------------------------------------

// Initialisiere Ringbuffer
//----------------------------------------------------------------------
void initRingBuffer(RingbufferTypeDef *ring, volatile CAN_massage_t *buffer, uint32_t size)
{
	ring->buffer = buffer;
	ring->size = size;
	ring->head = 0;
	ring->tail = 0;
}
//----------------------------------------------------------------------

// Nachricht zum Ring hinzufuegen
//----------------------------------------------------------------------
bool addToRingBuffer(RingbufferTypeDef *ring, CAN_massage_t *msg)
{
	unit16_t nextEntry;
	nextEntry = (ring->head + 1) % ring->size;

	// Pruefe, ob Ringbuffer gefuellt ist
	if (nextEntry == ring->tail)
		return false;

	// Fuege Element zum Ring hinzu
	memcpy((void *)&ring->buffer[ring->head], (void *)&msg, sizeof(CAN_massage_t));

	// Ringbuffer Kopf hochzaehlen
	ring->head = nextEntry;

	return true;
}
//----------------------------------------------------------------------

// Nachricht von Ring entfernen
//----------------------------------------------------------------------
bool removeFromRingBuffer(RingbufferTypeDef *ring, CAN_massage_t *msg)
{
	// Pruefen, ob Nachrichten im Ring sind
	if (isRingBufferEmpty(ring) == true)
		return false;

	// Kopiere Nachricht
	memcpy((void *)&msg, (void *)&ring->buffer[ring->tail], sizeof(CAN_massage_t));

	// Ringbuffer Schwanz hochzaehlen
	ring->tail = (ring->tail + 1) % ring->size;

	return true;
}
//----------------------------------------------------------------------

// Abfrage, ob Ringbuffer Nachrichten hat
//----------------------------------------------------------------------
bool isRingBufferEmpty(RingbufferTypeDef *ring)
{
	if (ring->head == ring->tail)
		return true;

	return false;
}
//----------------------------------------------------------------------

// Initialisiere Interruptfunktionen
//----------------------------------------------------------------------
//#ifndef
//----------------------------------------------------------------------

// Sendeinterrupt Mailbox 0
//----------------------------------------------------------------------
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *CanHandler)
{
	CAN_massage_t txmsg;

	// Wenn CAN1 Nachricht gesendet hat
	if (CanHandler->Instance == CAN1)
	{
		if (removeFromRingBuffer(&txRing, &txmsg) == true)
		{
			CANwrite(&txmsg, true);
		}
	}

	// TODO CAN2 und CAN3 hinzufuegen
}
//----------------------------------------------------------------------

// Sendeinterrupt Mailbox 1
//----------------------------------------------------------------------
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *CanHandler)
{
	CAN_massage_t txmsg;

	// Wenn CAN1 Nachricht gesendet hat
	if (CanHandler->Instance == CAN1)
	{
		if (removeFromRingBuffer(&txRing, &txmsg) == true)
		{
			CANwrite(&txmsg, true);
		}
	}

	// TODO CAN2 und CAN3 hinzufuegen
}
//----------------------------------------------------------------------

// Sendeinterrupt Mailbox 2
//----------------------------------------------------------------------
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *CanHandler)
{
	CAN_massage_t txmsg;

	// Wenn CAN1 Nachricht gesendet hat
	if (CanHandler->Instance == CAN1)
	{
		if (removeFromRingBuffer(&txRing, &txmsg) == true)
		{
			CANwrite(&txmsg, true);
		}
	}

	// TODO CAN2 und CAN3 hinzufuegen
}
//----------------------------------------------------------------------

// Empfanginterrupt FIFO0
//----------------------------------------------------------------------
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *CanHandler)
{
	CAN_massage_t rxmsg;
	CAN_RxHeaderTypeDef RxHeader;

	// Schreibe Nachricht von CAN-Bus in Ringbuffer
	if (HAL_CAN_GetRxMessage(CanHandler, CAN_RX_FIFO0, &RxHeader, rxmsg.buf) == HAL_OK)
	{
		if (RxHeader.IDE == CAN_ID_StdId)
		{
			rxmsg.id = RxHeader.StdId;
			rxmsg.flags.extended = 0;
		}
		else
		{
			rxmsg.id = RxHeader.ExtId;
			rxmsg.flags.extended = 1;
		}

		rxmsg.flags.remote = RxHeader.RTR;
		rxmsg.mb = RxHeader.FilterMatchIndex;
		rxmsg.timestamp = RxHeader.Timestamp;
		rxmsg.len = RxHeader.DLC;

		// TODO Ringbuffer fuer einzelne CAN-Busse erstellen
		if (CanHandler->Instance == CAN1)
		{
			rxmsg.bus = 1;
			addToRingBuffer(&rxRing, &rxmsg);
		}

		// TODO CAN2 und CAN3 hinzufuegen
	}
}
//----------------------------------------------------------------------

// RX IRQ Handler
//----------------------------------------------------------------------
void CAN1_RX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}
//----------------------------------------------------------------------

// TX IRQ Handler
//----------------------------------------------------------------------
void CAN1_TX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}
//----------------------------------------------------------------------
