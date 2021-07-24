#include <stdio.h>
#include <string.h>
#include "nrf24l01.h"

u8 RX_BUF[NRF24L01_PLOAD_WIDTH];
u8 TX_BUF[NRF24L01_PLOAD_WIDTH];

void NRF24L01_SPI_Init(void);
void NRF24L01_EXTILine_Config(void);

void NRF24L01_Init(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = NRF24L01_GPIO_CE|NRF24L01_GPIO_CSN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(NRF24L01_GPIOx, &GPIO_InitStructure);
  // IRQ Initialize
  if (NRF24L01_INT_MODE) { 
    NRF24L01_EXTILine_Config();
  } else {
    GPIO_InitStructure.GPIO_Pin = NRF24L01_GPIO_IRQ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(NRF24L01_GPIOx, &GPIO_InitStructure);
  }
  NRF24L01_SPI_Init();
  /*
    CSN is high initially (active low).
    CE is low initially (active high).
  */
  CSN(1);
  printf("## nRF24L01 Initialized ##\r\n");
}

void NRF24L01_SPI_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  if(NRF24L01_SPIx == SPI1) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // SCK:PA5, MISO:PA6, MOSI:PA7 or PB3, PB4, PB5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE); // reset SPI1
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);// stop reset SPI1

  } else if(NRF24L01_SPIx == SPI2) {
    // B13, B14, B15
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  } else { // SPI3,4,5,6
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE);
    
  }
  
  SPI_InitTypeDef SPI_InitStructure;
  SPI_StructInit(&SPI_InitStructure); // set default settings 
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; // data sampled at first edge
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;   // clock is low when idle
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // data is transmitted MSB first
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; // transmit in master mode, NSS pin has to be always high
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // set the NSS management to internal and pull internal NSS high
	SPI_Init(NRF24L01_SPIx, &SPI_InitStructure);
	SPI_Cmd(NRF24L01_SPIx, ENABLE);
}

void NRF24L01_EXTILine_Config(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  GPIO_InitTypeDef   GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Connect EXTI Line13 to PG13 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource13);
  
  EXTI_InitTypeDef   EXTI_InitStructure;
  EXTI_InitStructure.EXTI_Line    = EXTI_Line13;
  EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  NVIC_InitTypeDef   NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
* Basic SPI operation: Write to SPIx and read
*/
static u8 SPI_Write_Then_Read(u8 data)
{
  while(SPI_I2S_GetFlagStatus(NRF24L01_SPIx, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(NRF24L01_SPIx, data);

  while(SPI_I2S_GetFlagStatus(NRF24L01_SPIx, SPI_I2S_FLAG_RXNE)==RESET);
  return SPI_I2S_ReceiveData(NRF24L01_SPIx);
}

/**
* Read a 1-bit register
*/
u8 NRF24L01_Read_Reg(u8 reg)
{
  u8 value;
  CSN(0);
  SPI_Write_Then_Read(reg);
  value = SPI_Write_Then_Read(NRF24L01_CMD_NOP);
  CSN(1);
  return value;
}

/**
* Write a 1-byte register
*/
u8 NRF24L01_Write_Reg(u8 reg, u8 value)
{
  u8 status;
  CSN(0);
  if (reg < NRF24L01_CMD_REGISTER_W) {
    // This is a register access
		status = SPI_Write_Then_Read(NRF24L01_CMD_REGISTER_W | (reg & NRF24L01_MASK_REG_MAP));
		SPI_Write_Then_Read(value);

  } else {
    // This is a single byte command or future command/register
		status = SPI_Write_Then_Read(reg);
		if ((reg != NRF24L01_CMD_FLUSH_TX) 
        && (reg != NRF24L01_CMD_FLUSH_RX) 
        && (reg != NRF24L01_CMD_REUSE_TX_PL) 
        && (reg != NRF24L01_CMD_NOP)) {
			// Send register value
			SPI_Write_Then_Read(value);
		}
  }
  CSN(1); //停止SPI传输	
  return status; 
}

/**
* Read a multi-byte register
*  reg  - register to read
*  buf  - pointer to the buffer to write
*  len  - number of bytes to read
*/
u8 NRF24L01_Read_To_Buf(u8 reg, u8 *buf, u8 len)
{
  CSN(0);
  u8 status = SPI_Write_Then_Read(reg);
  while (len--) {
		*buf++ = SPI_Write_Then_Read(NRF24L01_CMD_NOP);
	}
  CSN(1);
  return status;
}

/**
* Write a multi-byte register
*  reg - register to write
*  buf - pointer to the buffer with data
*  len - number of bytes to write
*/
u8 NRF24L01_Write_From_Buf(u8 reg, u8 *buf, u8 len)
{
  CSN(0);
  u8 status = SPI_Write_Then_Read(reg);
  while (len--) {
    SPI_Write_Then_Read(*buf++);
  }
  CSN(1);
  return status;
}


u8 NRF24L01_Check(void)
{
  u8 rxbuf[5];
	u8 i;
	u8 *ptr = (u8 *)NRF24L01_TEST_ADDR;

	// Write test TX address and read TX_ADDR register
	NRF24L01_Write_From_Buf(NRF24L01_CMD_REGISTER_W | NRF24L01_REG_TX_ADDR, ptr, 5);
	NRF24L01_Read_To_Buf(NRF24L01_CMD_REGISTER_R | NRF24L01_REG_TX_ADDR, rxbuf, 5);

	// Compare buffers, return error on first mismatch
	for (i = 0; i < 5; i++) {
		if (rxbuf[i] != *ptr++) return 1;
	}
	return 0;
}

/**
* Flush the RX FIFO
*/
void NRF24L01_FlushRX(void)
{
	NRF24L01_Write_Reg(NRF24L01_CMD_FLUSH_RX, NRF24L01_CMD_NOP);
}

/**
* Flush the TX FIFO
*/
void NRF24L01_FlushTX(void)
{
	NRF24L01_Write_Reg(NRF24L01_CMD_FLUSH_TX, NRF24L01_CMD_NOP);
}

/**
* Clear IRQ bit of the STATUS register
*   reg - NRF24L01_FLAG_RX_DREADY
*         NRF24L01_FLAG_TX_DSENT
*         NRF24L01_FLAG_MAX_RT
*/
void NRF24L01_ClearIRQFlag(u8 reg) {
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_STATUS, reg);
}

/**
* Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
*/
void NRF24L01_ClearIRQFlags(void) {
	u8 reg;
	reg  = NRF24L01_Read_Reg(NRF24L01_REG_STATUS);
	reg |= NRF24L01_MASK_STATUS_IRQ;
	NRF24L01_Write_Reg(NRF24L01_REG_STATUS, reg);
}

/**
* Common configurations of RX and TX, internal function
*/
void _NRF24L01_Config(u8 *tx_addr)
{
  // TX Address
  NRF24L01_Write_From_Buf(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_TX_ADDR, tx_addr, NRF24L01_ADDR_WIDTH);
  // RX P0 Payload Width
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RX_PW_P0, NRF24L01_PLOAD_WIDTH);
  // 使能接收通道0自动应答
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_EN_AA, 0x3f);
  // 使能接收通道0
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_EN_RXADDR, 0x3f);
  // 选择射频通道40 - 2.440GHz = 2.400G  + 0.040G
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RF_CH, 40);
  // 000+0+[0:1Mbps,1:2Mbps]+[00:-18dbm,01:-12dbm,10:-6dbm,11:0dbm]+[0:LNA_OFF,1:LNA_ON]
  // 01:1Mbps,-18dbm; 03:1Mbps,-12dbm; 05:1Mbps,-6dbm; 07:1Mbps,0dBm
  // 09:2Mbps,-18dbm; 0b:2Mbps,-12dbm; 0d:2Mbps,-6dbm; 0f:2Mbps,0dBm, 
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RF_SETUP, 0x03);
  // 0A:delay=250us,count=10, 1A:delay=500us,count=10
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_SETUP_RETR, 0x0a);
}

/**
* Switch NRF24L01 to RX mode
*/
void NRF24L01_RX_Mode(u8 *rx_addr, u8 *tx_addr)
{
  CE(0);
  _NRF24L01_Config(tx_addr);
  NRF24L01_Write_From_Buf(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RX_ADDR_P0, rx_addr, NRF24L01_ADDR_WIDTH);
  /**
  REG 0x00: 
  0)PRIM_RX     0:TX             1:RX
  1)PWR_UP      0:OFF            1:ON
  2)CRCO        0:8bit CRC       1:16bit CRC
  3)EN_CRC      Enabled if any of EN_AA is high
  4)MASK_MAX_RT 0:IRQ low        1:NO IRQ
  5)MASK_TX_DS  0:IRQ low        1:NO IRQ
  6)MASK_RX_DR  0:IRQ low        1:NO IRQ
  7)Reserved    0
  */
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_CONFIG, 0x0f); //RX,PWR_UP,CRC16,EN_CRC
  CE(1);
}

/**
* Switch NRF24L01 to TX mode
*/
void NRF24L01_TX_Mode(u8 *rx_addr, u8 *tx_addr)
{
  CE(0);
  _NRF24L01_Config(tx_addr);
  // On the PTX the **TX_ADDR** must be the same as the **RX_ADDR_P0** and as the pipe address for the designated pipe
  // RX_ADDR_P0 will be used for receiving ACK, otherwise you will get MAX_RT Interrupt
  NRF24L01_Write_From_Buf(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RX_ADDR_P0, tx_addr, NRF24L01_ADDR_WIDTH);
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_CONFIG, 0x0e); //TX,PWR_UP,CRC16,EN_CRC
  CE(1);
}

/**
* Hold till data received and written to rx_buf
*/
u8 NRF24L01_RxPacket(u8 *rx_buf)
{
  u8 status, result = 0;
  while(IRQ);
  CE(0);
  status = NRF24L01_Read_Reg(NRF24L01_REG_STATUS);
  printf("Interrupted, status: %02X\r\n", status);

  if(status & NRF24L01_FLAG_RX_DREADY) {
    NRF24L01_Read_To_Buf(NRF24L01_CMD_RX_PLOAD_R, rx_buf, NRF24L01_PLOAD_WIDTH);
    for (int i = 0; i < 32; i++) {
      printf("%02X ", RX_BUF[i]);
    }
    result = 1;
    NRF24L01_FlushRX();
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_RX_DREADY);
  }
  CE(1);
  return result;
}

/**
* Send data in tx_buf and wait till data is sent or max re-tr reached
*/
u8 NRF24L01_TxPacket(u8 *tx_buf, u8 len)
{
  u8 status = 0x00;
  CE(0);
  len = len > NRF24L01_PLOAD_WIDTH? NRF24L01_PLOAD_WIDTH : len;
  NRF24L01_Write_From_Buf(NRF24L01_CMD_TX_PLOAD_W, tx_buf, len);
  CE(1);
  while(IRQ != 0); // Waiting send finish

  CE(0);
  status = NRF24L01_Read_Reg(NRF24L01_REG_STATUS);
  printf("Interrupted, status: %02X\r\n", status);
  if(status & NRF24L01_FLAG_TX_DSENT) {
    /*printf("Data sent: ");
    for (u8 i = 0; i < len; i++) {
      printf("%02X ", tx_buf[i]);
    }
    printf("\r\n");*/
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_TX_DSENT);
  } else if(status & NRF24L01_FLAG_MAX_RT) {
    printf("Sending exceeds max retries\r\n");
    NRF24L01_FlushTX();
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_MAX_RT);
  }
  CE(1);
  return status;
}

/**
* Read received data and written to rx_buf, No blocking.
*/
u8 NRF24L01_IRQ_Handler(u8 *rx_buf)
{
  u8 status, result = 0;
  CE(0);
  status = NRF24L01_Read_Reg(NRF24L01_REG_STATUS);
  printf("Reg status: %02X\r\n", status);
  if(status & NRF24L01_FLAG_RX_DREADY) {
    NRF24L01_Read_To_Buf(NRF24L01_CMD_RX_PLOAD_R, rx_buf, NRF24L01_PLOAD_WIDTH);
    for (int i = 0; i < 32; i++) {
      printf("%02X ", RX_BUF[i]);
    }
    result = 1;
    NRF24L01_FlushRX();
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_RX_DREADY);

  } else if(status & NRF24L01_FLAG_TX_DSENT) {
    printf("Data sent\r\n");
    NRF24L01_FlushTX();
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_TX_DSENT);
  } else if(status & NRF24L01_FLAG_MAX_RT) {
    printf("Sending exceeds max retries\r\n");
    NRF24L01_FlushTX();
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_MAX_RT);
  }
  CE(1);
  return result;
}


/**
* Dump nRF24L01 configuration
*/
void NRF24L01_DumpConfig(void) {
	uint8_t i,j;
	uint8_t aw;
	uint8_t buf[5];

	// CONFIG
	i = NRF24L01_Read_Reg(NRF24L01_REG_CONFIG);
	printf("[0x%02X] 0x%02X MASK:%02X CRC:%02X PWR:%s MODE:%s\r\n",
			NRF24L01_REG_CONFIG,
			i,
			i >> 4,
			(i & 0x0c) >> 2,
			(i & 0x02) ? "ON" : "OFF",
			(i & 0x01) ? "RX" : "TX"
		);
	// EN_AA
	i = NRF24L01_Read_Reg(NRF24L01_REG_EN_AA);
	printf("[0x%02X] 0x%02X ENAA: ", NRF24L01_REG_EN_AA, i);
	for (j = 0; j < 6; j++) {
		printf("[P%1u%s]%s",j,
				(i & (1 << j)) ? "+" : "-",
				(j == 5) ? "\r\n" : " "
			);
	}
	// EN_RXADDR
	i = NRF24L01_Read_Reg(NRF24L01_REG_EN_RXADDR);
	printf("[0x%02X] 0x%02X EN_RXADDR: ", NRF24L01_REG_EN_RXADDR, i);
	for (j = 0; j < 6; j++) {
		printf("[P%1u%s]%s",j,
				(i & (1 << j)) ? "+" : "-",
				(j == 5) ? "\r\n" : " "
			);
	}
	// SETUP_AW
	i = NRF24L01_Read_Reg(NRF24L01_REG_SETUP_AW);
	aw = (i & 0x03) + 2;
	printf("[0x%02X] 0x%02X EN_RXADDR=%03X (address width = %u)\r\n", NRF24L01_REG_SETUP_AW, i, i & 0x03, aw);
	// SETUP_RETR
	i = NRF24L01_Read_Reg(NRF24L01_REG_SETUP_RETR);
	printf("[0x%02X] 0x%02X ARD=%04X ARC=%04X (retr.delay=%uus, count=%u)\r\n",
			NRF24L01_REG_SETUP_RETR,
			i,
			i >> 4,
			i & 0x0F,
			((i >> 4) * 250) + 250,
			i & 0x0F
		);
	// RF_CH
	i = NRF24L01_Read_Reg(NRF24L01_REG_RF_CH);
	printf("[0x%02X] 0x%02X (%.3uGHz)\r\n",NRF24L01_REG_RF_CH,i,2400 + i);
	// RF_SETUP
	i = NRF24L01_Read_Reg(NRF24L01_REG_RF_SETUP);
	printf("[0x%02X] 0x%02X CONT_WAVE:%s PLL_LOCK:%s DataRate=",
			NRF24L01_REG_RF_SETUP,
			i,
			(i & 0x80) ? "ON" : "OFF",
			(i & 0x80) ? "ON" : "OFF"
		);
	switch ((i & 0x28) >> 3) {
		case 0x00:
			printf("1M");
			break;
		case 0x01:
			printf("2M");
			break;
		case 0x04:
			printf("250k");
			break;
		default:
			printf("???");
			break;
	}
	printf("pbs RF_PWR=");
	switch ((i & 0x06) >> 1) {
		case 0x00:
			printf("-18");
			break;
		case 0x01:
			printf("-12");
			break;
		case 0x02:
			printf("-6");
			break;
		case 0x03:
			printf("0");
			break;
		default:
			printf("???");
			break;
	}
	printf("dBm\r\n");
	// STATUS
	i = NRF24L01_Read_Reg(NRF24L01_REG_STATUS);
	printf("[0x%02X] 0x%02X IRQ:%03X RX_PIPE:%u TX_FULL:%s\r\n",
			NRF24L01_REG_STATUS,
			i,
			(i & 0x70) >> 4,
			(i & 0x0E) >> 1,
			(i & 0x01) ? "YES" : "NO"
		);

	// OBSERVE_TX
	i = NRF24L01_Read_Reg(NRF24L01_REG_OBSERVE_TX);
	printf("[0x%02X] 0x%02X PLOS_CNT=%u ARC_CNT=%u\r\n",NRF24L01_REG_OBSERVE_TX,i,i >> 4,i & 0x0F);

	// RPD
	i = NRF24L01_Read_Reg(NRF24L01_REG_RPD);
	printf("[0x%02X] 0x%02X RPD=%s\r\n",NRF24L01_REG_RPD,i,(i & 0x01) ? "YES" : "NO");

	// RX_ADDR_P0
	NRF24L01_Read_To_Buf(NRF24L01_REG_RX_ADDR_P0,buf,aw);
	printf("[0x%02X] RX_ADDR_P0 \"",NRF24L01_REG_RX_ADDR_P0);
	for (i = 0; i < aw; i++) printf("%X ",buf[i]);
	printf("\"\r\n");

  // RX_ADDR_P1
	NRF24L01_Read_To_Buf(NRF24L01_REG_RX_ADDR_P1,buf,aw);
	printf("[0x%02X] RX_ADDR_P1 \"",NRF24L01_REG_RX_ADDR_P1);
	for (i = 0; i < aw; i++) printf("%X ",buf[i]);
	printf("\"\r\n");

	// RX_ADDR_P2
	printf("[0x%02X] RX_ADDR_P2 \"",NRF24L01_REG_RX_ADDR_P2);
	for (i = 0; i < aw - 1; i++) printf("%X ",buf[i]);
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_ADDR_P2);
	printf("%X\"\r\n",i);

	// RX_ADDR_P3
	printf("[0x%02X] RX_ADDR_P3 \"",NRF24L01_REG_RX_ADDR_P3);
	for (i = 0; i < aw - 1; i++) printf("%X ",buf[i]);
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_ADDR_P3);
	printf("%X\"\r\n",i);

	// RX_ADDR_P4
	printf("[0x%02X] RX_ADDR_P4 \"",NRF24L01_REG_RX_ADDR_P4);
	for (i = 0; i < aw - 1; i++) printf("%X ",buf[i]);
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_ADDR_P4);
	printf("%X\"\r\n",i);

  // RX_ADDR_P5
	printf("[0x%02X] RX_ADDR_P5 \"",NRF24L01_REG_RX_ADDR_P5);
	for (i = 0; i < aw - 1; i++) printf("%X ",buf[i]);
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_ADDR_P5);
	printf("%X\"\r\n",i);

	// TX_ADDR
	NRF24L01_Read_To_Buf(NRF24L01_REG_TX_ADDR,buf,aw);
	printf("[0x%02X] TX_ADDR \"",NRF24L01_REG_TX_ADDR);
	for (i = 0; i < aw; i++) printf("%X ",buf[i]);
	printf("\"\r\n");

	// RX_PW_P0
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_PW_P0);
	printf("[0x%02X] RX_PW_P0=%u\r\n",NRF24L01_REG_RX_PW_P0,i);

	// RX_PW_P1
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_PW_P1);
	printf("[0x%02X] RX_PW_P1=%u\r\n",NRF24L01_REG_RX_PW_P1,i);

  // RX_PW_P2
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_PW_P2);
	printf("[0x%02X] RX_PW_P2=%u\r\n",NRF24L01_REG_RX_PW_P2,i);

	// RX_PW_P3
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_PW_P3);
	printf("[0x%02X] RX_PW_P3=%u\r\n",NRF24L01_REG_RX_PW_P3,i);

	// RX_PW_P4
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_PW_P4);
	printf("[0x%02X] RX_PW_P4=%u\r\n",NRF24L01_REG_RX_PW_P4,i);

	// RX_PW_P5
	i = NRF24L01_Read_Reg(NRF24L01_REG_RX_PW_P5);
	printf("[0x%02X] RX_PW_P5=%u\r\n",NRF24L01_REG_RX_PW_P5,i);
}
