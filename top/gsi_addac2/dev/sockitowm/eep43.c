#include "ownet.h"
#include "uart.h"
#include "eep43.h"


#define READ_SCRATCH_CMD 0xaa
#define WRITE_SCRATCH_CMD 0x0f
#define COPY_SCRATCH_CMD 0x55
#define READ_MEM_CMD 0xf0
#define E_READ_MEM_CMD 0xa5

int Write43(int portnum, uchar *SerialNum, uchar *page_buffer)
{
	uchar rt=FALSE;
	ushort lastcrc16;
	int  i;

	owSerialNum(portnum, SerialNum, FALSE);

	if(owAccess(portnum))
	{
		mprintf(" Writing Scratchpad...\n");
		if (!owWriteBytePower(portnum, WRITE_SCRATCH_CMD)) 
			return FALSE;
		setcrc16(portnum, 0);
		docrc16(portnum,(ushort)WRITE_SCRATCH_CMD);
		
		owLevel(portnum, MODE_NORMAL);
		owWriteBytePower(portnum, 0x00);	//write LSB of target addr
		docrc16(portnum,(ushort)0x00);
		owLevel(portnum, MODE_NORMAL);
		owWriteBytePower(portnum, 0x00);	//write MSB of target addr
		docrc16(portnum,(ushort)0x00);


		for(i = 0; i < 32; i++)			//write 32 data bytes to scratchpad 
		{	
			owLevel(portnum,MODE_NORMAL);
			owWriteBytePower(portnum, i);
			lastcrc16 = docrc16(portnum,i);

		}
		for(i = 0; i < 2; i++)			//read two bytes CRC16
		{
			owLevel(portnum,MODE_NORMAL);
			lastcrc16 = docrc16(portnum,(ushort)owReadBytePower(portnum));
		}
		mprintf(" CRC16: %x\n", lastcrc16);
		if(lastcrc16 == 0xb001)
		{
			//copy to mem
			owLevel(portnum, MODE_NORMAL);
			if(Copy2Mem43(portnum, SerialNum))
				rt=TRUE;
	
		}
	}

	return rt;
}

int Copy2Mem43(int portnum, uchar *SerialNum)
{
	uchar rt=FALSE;
	uchar read_data;
	
	owSerialNum(portnum, SerialNum, FALSE);

	if(owAccess(portnum))
	{	
		if (!owWriteBytePower(portnum, COPY_SCRATCH_CMD)) 
			return FALSE;
		owLevel(portnum, MODE_NORMAL);
		owWriteBytePower(portnum, 0x00);	//write LSB of target addr
		owLevel(portnum, MODE_NORMAL);
		owWriteBytePower(portnum, 0x00);	//write MSB of target addr
		owLevel(portnum, MODE_NORMAL);
		owWriteBytePower(portnum, 0x1f);	//write E/S

		msDelay(500);
				
		
		owLevel(portnum,MODE_NORMAL);		
		read_data = owReadBytePower(portnum);
		
		if (read_data == 0xaa)			
			rt=TRUE;
	}

	return rt;
}

	

// routine for reading the scratchpad of a DS28EC20P EEPROM
// 80 pages of 32byte
// 32byte scratchpad
// expects 32 byte deep buffer
int ReadScratch43(int portnum, uchar *SerialNum, uchar *page_buffer)
{
	uchar rt=FALSE;
	ushort lastcrc16; 
	int i;
	ushort target_addr = 0;
	uchar read_data;
	
	owSerialNum(portnum, SerialNum, FALSE);

	if(owAccess(portnum))
	{	
		mprintf(" Reading Scratchpad...\n");
		if (!owWriteBytePower(portnum, READ_SCRATCH_CMD)) 
			return FALSE;

		setcrc16(portnum, 0);				//init crc
		docrc16(portnum,(ushort)READ_SCRATCH_CMD);
		
					
		
		owLevel(portnum,MODE_NORMAL);		//read 2 byte address and 1 byte status
		read_data = owReadBytePower(portnum);
		lastcrc16 = docrc16(portnum, read_data);
		target_addr = read_data;
 
		owLevel(portnum,MODE_NORMAL);		
		read_data = owReadBytePower(portnum);
		lastcrc16 = docrc16(portnum, read_data);
		target_addr |= read_data << 8;

		owLevel(portnum,MODE_NORMAL);		
		read_data = owReadBytePower(portnum);
		lastcrc16 = docrc16(portnum, read_data);

		mprintf("E/S: 0x%x\n", read_data);
		for(i = 0; i < 32; i++) 
		{	
			owLevel(portnum,MODE_NORMAL);
			page_buffer[i] = owReadBytePower(portnum);
			lastcrc16 = docrc16(portnum, page_buffer[i]);
		}
		
		for(i = 0; i < 2; i++)
		{
			owLevel(portnum,MODE_NORMAL);		
			read_data = owReadBytePower(portnum);
			lastcrc16 = docrc16(portnum, read_data);
		}	
		if (lastcrc16 == 0xb001)			
			rt=TRUE;
	}

	return rt;
}

// routine for reading a memory page of a DS28EC20P EEPROM
// expects 32 byte deep buffer
int ReadMem43(int portnum, uchar *SerialNum, uchar *page_buffer)
{
	uchar rt=FALSE;
	ushort lastcrc16; 
	int i;
	uchar read_data;
	
	owSerialNum(portnum, SerialNum, FALSE);

	if(owAccess(portnum))
	{	
		if (!owWriteBytePower(portnum, E_READ_MEM_CMD)) 
			return FALSE;

		setcrc16(portnum, 0);				//init crc
		docrc16(portnum,(ushort)E_READ_MEM_CMD);
		
		owLevel(portnum, MODE_NORMAL);
		owWriteBytePower(portnum, 0x00);	//write LSB of target addr
		docrc16(portnum,(ushort)0x00);
		owLevel(portnum, MODE_NORMAL);
		owWriteBytePower(portnum, 0x00);	//write MSB of target addr
		docrc16(portnum,(ushort)0x00);
		
		for(i = 0; i < 32; i++) 
		{	
			owLevel(portnum,MODE_NORMAL);
			page_buffer[i] = owReadBytePower(portnum);
			lastcrc16 = docrc16(portnum, page_buffer[i]);
		}
		
		for(i = 0; i < 2; i++)
		{
			owLevel(portnum,MODE_NORMAL);		
			read_data = owReadBytePower(portnum);
			lastcrc16 = docrc16(portnum, read_data);
		}	
		if (lastcrc16 == 0xb001)			
			rt=TRUE;
	}

	return rt;
}
