#include <stdio.h>

#include "vsn.h"
#include "vsncrc32.h"
#include "vsnsetup.h"
#include "vsnpm.h"
#include "vsntemperature.h"
#include "vsnusart.h"

#include "vsnlogateccmdparser.h"
#include "vsnlogateccmdoutput.h"

static void sht21GetHandler(const char __attribute__((unused)) *args)
{
	float temp = vsnTemperature_read();
	float rh = 0.0;
	logatecParser_printResponse("T  = %f C\r\n", temp);
	logatecParser_printResponse("Rh = %f %%\r\n", rh);
}

const struct ResourceHandler_t sht21Handler = {
		.name = "sensors/sht21",
		.get = sht21GetHandler,
		.post = NULL
};

const struct ResourceHandler_t * const resources[] = {
	 &sht21Handler,
	 NULL
};

int main(void)
{
	/* Init the SNC board */
	SystemInit();
	vsnPM_init();
	vsnSetup_intClk(SNC_CLOCK_8MHZ);
	vsnSetup_initSnc();
	vsnSetup_calibHsi();
	vsnCRC32_init();
	vsnPM_mesureAdcBitVolt();

	/* Open USART1 for communication with SNA-LGTC */
	USART_InitTypeDef USART_InitStructure = {
		.USART_BaudRate = 115200,
		.USART_WordLength = USART_WordLength_8b,
		.USART_StopBits = USART_StopBits_1,
		.USART_Parity = USART_Parity_No,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx };
	vsnUSART_init(USART1, &USART_InitStructure);

	printf("SNC Initialized\n");

	/* Initialize parser. */
	locatecParser_init(logatecParser_outputUsart1, resources);

	while(1)
	{
		/* Check if any characters are available from USART1 and pass
		 * them to the parser. */
		char infrastructureMsg;
		if (vsnUSART_read(USART1, &infrastructureMsg, 1)) {
			logatecParser_inputUnicast(infrastructureMsg);
		}
	}
}
