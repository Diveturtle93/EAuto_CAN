//----------------------------------------------------------------------
// Titel	:	CAN_Bus.c
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	Aug 23, 2022
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	STM32F767ZI
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------
#include "can.h"
//----------------------------------------------------------------------

// Einfuegen der eigenen Include Dateien
//----------------------------------------------------------------------
#include "CAN_Bus.h"
//----------------------------------------------------------------------

// Ringpuffer definieren
//----------------------------------------------------------------------
ring_buffer rx_buffer;
ring_buffer tx_buffer;

ring_buffer *_rx_buffer;
ring_buffer *_tx_buffer;
//----------------------------------------------------------------------

void Ringbuf_init(void)
{
	_rx_buffer = &rx_buffer;
	_tx_buffer = &tx_buffer;

	/* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
	__HAL_CAN_ENABLE_IT(&hcan3, CAN_IT_ERROR);

	/* Enable the UART Data Register not empty Interrupt */
	__HAL_CAN_ENABLE_IT(&hcan3, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN_isr(CAN_HandleTypeDef* hcan)
{
	uint32_t isrflags   = READ_REG(hcan->Instance->MSR);
	uint32_t cr1its     = READ_REG(hcan->Instance->IER);

	/*If interrupt is caused due to Transmit Data Register Empty */
	if (((isrflags & CAN_MSR_TXM) != RESET) && ((cr1its & CAN_IER_TMEIE) != RESET))
	{
		if(tx_buffer.head == tx_buffer.tail)
		{
			// Buffer empty, so disable interrupts
			__HAL_CAN_DISABLE_IT(hcan, CAN_IT_TX_MAILBOX_EMPTY);
		}
		else
		{
			// There is more data in the output buffer. Send the next byte
			tx_buffer.tail = (tx_buffer.tail + 1) % CAN_BUFFER_SIZE;

			hcan->Instance->MSR;
		}

		return;
	}
}

void CAN_write(CAN_HandleTypeDef* hcan)
{
	if (((_tx_buffer->head)-(_tx_buffer->tail)) >= 0)
	{
		int i = (_tx_buffer->head + 1) % CAN_BUFFER_SIZE;

		// If the output buffer is full, there's nothing for it other than to
		// wait for the interrupt handler to empty it a bit
		// ???: return 0 here instead?
		while (i == _tx_buffer->tail);

		_tx_buffer->Paket[_tx_buffer->head];
		_tx_buffer->head = i;

		__HAL_CAN_ENABLE_IT(hcan, CAN_IT_TX_MAILBOX_EMPTY);
	}
}

Output_Paket Test(uint16_t id, uint8_t length, uint16_t sendeintervall, uint16_t sende_init_time)
{
	Output_Paket test = {id, length, sendeintervall, sende_init_time};

	return test;
}

void konfiguriere_CAN(void)
{
	Output_Paket CAN_Output_Paket_Liste[ANZAHL_OUTPUT_PAKETE];

	// Paket: SENDE_BAMOCAR_MOTOR_RPM Bamocar Sendeauftrag
	CAN_Output_Paket_Liste[0] = Test(0x108, 1, 25, 17);
	CAN_Output_Paket_Liste[1] = Test(0x283, 1, 25, 13);
	CAN_Output_Paket_Liste[2] = Test(0x372, 2, 25, 14);
	CAN_Output_Paket_Liste[3] = Test(0x182, 1, 25, 7);
}
