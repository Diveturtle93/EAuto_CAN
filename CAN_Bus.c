//----------------------------------------------------------------------
// Titel	:	CAN_Bus.c
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	Aug 23, 2022
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	EAuto_CAN
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der eigenen Include Dateien
//----------------------------------------------------------------------
#include "CAN_Bus.h"
#include "millis.h"
//----------------------------------------------------------------------

// Arrays definieren
//----------------------------------------------------------------------
CAN_Paket CAN_Output_Paket_Liste[ANZAHL_OUTPUT_PAKETE];
uint8_t CAN_Output_Signal_Liste[ANZAHL_OUTPUT_PAKETE * 8];
uint8_t CAN_Input_Signal_Liste[CAN_BUFFER_SIZE*8];

ring_buffer rxHeader;
//----------------------------------------------------------------------

// Empfangene CAN-Nachricht speichern
//----------------------------------------------------------------------
void CAN_rx_read (CAN_HandleTypeDef* hcan, uint32_t RxFifo)
{
	if (CAN_rx_available() < CAN_BUFFER_SIZE)
	{
		CAN_RxHeaderTypeDef _tmp_Rx;
		HAL_CAN_GetRxMessage(hcan, RxFifo, &_tmp_Rx, &CAN_Input_Signal_Liste[rxHeader.head*8]);
		rxHeader.Paket[rxHeader.head] = _tmp_Rx;
		rxHeader.head = (rxHeader.head == (CAN_BUFFER_SIZE - 1)) ? 0 : (rxHeader.head + 1);
	}
	else
	{

	}
}
//----------------------------------------------------------------------

// Nachrichten auf CAN-Bus senden
//----------------------------------------------------------------------
void CAN_write (CAN_HandleTypeDef* hcan)
{
	for (uint16_t i = 0; i < ANZAHL_OUTPUT_PAKETE; i++)
	{
		if (CAN_Output_Paket_Liste[i].sende_time < (millis() - CAN_Output_Paket_Liste[i].sendeintervall))
		{
			CAN_TxHeaderTypeDef TxHeader;

			TxHeader.StdId = CAN_Output_Paket_Liste[i].id;
			TxHeader.ExtId = 0x00;
			TxHeader.DLC = CAN_Output_Paket_Liste[i].length;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.TransmitGlobalTime=DISABLE;

			while (HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX0) == 1);

			if (HAL_CAN_AddTxMessage(hcan, &TxHeader, &CAN_Output_Signal_Liste[i*8], (uint32_t *)CAN_TX_MAILBOX0) != 0)
			{

			}
			else
			{
				CAN_Output_Paket_Liste[i].sende_time = millis();
			}
		}
	}
}
//----------------------------------------------------------------------

// CAN-Nachrichten definieren
//----------------------------------------------------------------------
CAN_Paket CAN_Nachricht (uint16_t id, uint8_t length, uint16_t sendeintervall, uint32_t sende_init_time)
{
	CAN_Paket TxHeader = {id, length, sendeintervall - 1, sende_init_time};

	return TxHeader;
}
//----------------------------------------------------------------------

// Abfrage, ob CAN-Nachricht verfuegbar
//----------------------------------------------------------------------
uint8_t CAN_rx_available (void)
{
	if (rxHeader.head >= rxHeader.tail)
		return rxHeader.head - rxHeader.tail;
	else
		return CAN_BUFFER_SIZE - (rxHeader.tail - rxHeader.head);
}
//----------------------------------------------------------------------
