//#include <iostm8S105c6.h>
#define multiplier 4
#define DISCOVERY 1
#if defined DISCOVERY
#include <iostm8S105c6.h>
#elif defined PROTOMODULE
#include <iostm8s103k3.h>
#else
    #include <iostm8s103f3.h>
#endif
#include <intrinsics.h>


#define         CE       PD_ODR_ODR3 
#define         CSN      PD_ODR_ODR2 
#define         SCK      PC_ODR_ODR7
#define         MOSI     PC_ODR_ODR6
#define         MISO     PC_IDR_IDR5  
#define         IRQ      PD_IDR_IDR4 

///////////////////////////////////////////////////////

#define TX_ADR_WIDTH    5    // 5 uints TX address width
#define RX_ADR_WIDTH    5    // 5 uints RX address width
#define TX_PLOAD_WIDTH  32  // 20 uints TX payload
#define RX_PLOAD_WIDTH  32   // 20 uints TX payload
unsigned char  TX_ADDRESS[TX_ADR_WIDTH]= {0xE7,0xE7,0xE7,0xE7,0xE7}; 
unsigned char  RX_ADDRESS[RX_ADR_WIDTH]= {0xE7,0xE7,0xE7,0xE7,0xE7}; 


/////////////// SPI(NRF24l01) commands////////////////

#define READ_REG        0x00   
#define WRITE_REG       0x20  
#define RD_RX_PLOAD     0x61   
#define WR_TX_PLOAD     0xA0   
#define FLUSH_TX        0xE1  
#define FLUSH_RX        0xE2   
#define REUSE_TX_PL     0xE3   
#define NOP             0xFF   

///////////////SPI(nrf24l01) registers///////////////////////////////////////

#define CONFIG          0x00  
#define EN_AA           0x01  
#define EN_RXADDR       0x02 
#define SETUP_AW        0x03  
#define SETUP_RETR      0x04  
#define RF_CH           0x05 
#define RF_SETUP        0x06  
#define STATUS          0x07  
#define OBSERVE_TX      0x08 
#define CD              0x09           
#define RX_ADDR_P0      0x0A  
#define RX_ADDR_P1      0x0B  
#define RX_ADDR_P2      0x0C  
#define RX_ADDR_P3      0x0D  
#define RX_ADDR_P4      0x0E  
#define RX_ADDR_P5      0x0F  
#define TX_ADDR         0x10 
#define RX_PW_P0        0x11  
#define RX_PW_P1        0x12  
#define RX_PW_P2        0x13  
#define RX_PW_P3        0x14 
#define RX_PW_P4        0x15  
#define RX_PW_P5        0x16  
#define FIFO_STATUS     0x17  

///////////////////////////////////////////////////////////////////////////

unsigned char    sta=0x00;
#define   RX_DR  (sta & 0x40)
#define   TX_DS  (sta & 0x20)
#define   MAX_RT  (sta & 0x10)


////////////////////////////////////////////////////////////////////////////


 void init_io_config()
 {
   PC_DDR_DDR5=0;
   PC_DDR_DDR6=1;
   PC_DDR_DDR7=1;
   
   PD_DDR_DDR2=1;
   PD_DDR_DDR3=1;
   PD_DDR_DDR4=0;
   
   
   PC_CR1_C15=1;
   PC_CR1_C16=1;
   PC_CR1_C17=1;
   
   PD_CR1_C12=1;
   PD_CR1_C13=1;
   PD_CR1_C14=0;
   PC_CR2 = 0x00; 
   PD_CR2 = 0x00;
 }

 void delayms(unsigned int count)
{
 unsigned int i,j;
 for(i=0;i<count;i++)
 for(j=0;j<450;j++);
}
 
unsigned char SPI_RW(unsigned char byte)
{
    unsigned char i;
    for(i=0;i<8;i++) // output 8-bit
    {
      MOSI=(byte & 0x80);
      byte = (byte << 1); 
      SCK = 1;
      byte|=MISO;
      SCK = 0;
    }
    return (byte);
      
}

unsigned char SPI_RW_Reg(unsigned char reg, unsigned char value)
{
 unsigned int  status;
 CSN = 0;                   // CSN low, init SPI transaction
 status = SPI_RW(reg);      // select register
 SPI_RW(value);             // ..and write value to it..
 CSN = 1;                   // CSN high again
 return(status);            // return nRF24L01 status uchar
}

unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char num)
{
 unsigned char status=0x00,byte_ctr=0;
 CSN = 0;            
 status = SPI_RW(reg);   
 for(byte_ctr=0; byte_ctr < num; byte_ctr++)
 {   
   SPI_RW(*pBuf++);
 }
 CSN = 1;           
 return(status);   
}



void init_NRF24L01(void)
{
  delayms(1);
  CE=0;    // chip enable
  CSN=1;   // Spi disable 
  SCK=0;   // Spi clock line init high
 SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f); //set pwr up bit enable
 SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    
 SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); 
 SPI_RW_Reg(WRITE_REG + EN_AA, 0x00);       
 SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);   // enable pipe 0
 SPI_RW_Reg(WRITE_REG + SETUP_AW, 0x02);    // set up address width 5 bytes
 SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x00);  // retransmission disabled
 SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);   //pwr=0dbm,datarate =1 mbps,lna:hcurr
 SPI_RW_Reg(WRITE_REG + STATUS, 0x02);          //
  SPI_RW_Reg(WRITE_REG + RF_CH, 0x2c);          //

 SPI_RW_Reg(WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); //
 CE=1;               //make ce high
}


void SetTX_Mode(void)
{
 CE=0;
 SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);     
 CE = 1; 
 delayms(1);
}
void nRF24L01_TxPacket(unsigned char * tx_buf)
{
  
 CE=0;   
 SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); 
 SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH); 
 //while(TX_DS!=1);
 //SPI_RW_Reg(WRITE_REG + STATUS, 0x00);
 SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);

 CE=1;   
delayms(1);

 //SPI_RW_Reg(WRITE_REG + STATUS, 0x00);
 //sta = SPI_RW(STATUS);
}

void SetRX_Mode(void)
{
 CE=0;
 SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);     
 CE = 1; 
 delayms(1);
}

unsigned char SPI_Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char num)
{
 unsigned char  status,byte_ctr=0;
 
 CSN = 0;                      // Set CSN low, init SPI tranaction
 status = SPI_RW(reg);         // Select register to write to and read status uchar
 for(byte_ctr=0;byte_ctr < num; byte_ctr++)   
 pBuf[byte_ctr] = SPI_RW(0);     
 
 CSN = 1;                           
 
 return(status);                    // return nRF24L01 status uchar
}

unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
{
  unsigned char revale=0;
  SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);     
  CE = 1;    
  sta=SPI_RW(STATUS);
  while(RX_DR);
  if(RX_DR)
  {
  CE = 0;    
  SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer
  revale =1;  
 }
 SPI_RW_Reg(WRITE_REG + STATUS, 0x00);
 sta = SPI_RW(STATUS);
 SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);
 return revale;
}



void main()
{
  unsigned char TxBuf[32]={0};   
//unsigned char RxBuf[32]={0};
  init_io_config();
  init_NRF24L01() ;
  while(1)
  {
  TxBuf[1] =0x55;
  SetTX_Mode();
  nRF24L01_TxPacket(TxBuf);
  TxBuf[2]=0x00;
  
 for(int i=0;i<2000;i++)
 {
   for(int j=0;j<1275;j++);
 }

  delayms(500);
  }
 /* SetRX_Mode();
  nRF24L01_RxPacket(RxBuf);*/
  
 }
