#ifndef _DS3231_H_
#define _DS3231_H_

#ifdef USING_STC12C5A60S2_
#	include <STC/STC12C5A60S2.h>
#endif

#ifdef USING_STC89C58RD_
#	include <STC/STC89C5xRC.h>
#endif

#include <intrins.h>
#include <mcudef.h>

SET_IO( IO(0,0), SCL);
SET_IO( IO(0,1), SDA);
SET_IO( IO(0,2), INT);

typedef struct Time_t{
	uint8 year, month, day, week, hour, minute, second;
}Time, TimeBCD;

typedef struct Temp_t{
	uint8 temp, tempp;
}Temp, TempBCD;

bit ack;

#define WriteAddress	0xD0
#define ReadAddress		0xD1

#define Second			0x00
#define Minute			0x01
#define Hour			0x02
#define Week			0x03
#define Day				0x04
#define Month			0x05
#define Year			0x06

// Clock 1
#define Alarm1Second	0x07
#define Alarm1Minute	0x08
#define Alarm1Hour		0x09
#define Alarm1Week		0x0A
// Clock 2
#define Alarm2Minute	0x0B
#define Alarm2Hour		0x0C
#define Alarm2Week		0x0D

#define Control			0x0E
#define Status			0x0F

#define Busy			2
#define CrystalStopped	7
#define Xtal			0x10
#define TempH			0x11
#define TempL			0x12

uint8 bcd2Hex(uint8 val){
	uint8 temp;
	temp = val & 0x0F;
	val >>= 4;
	val &= 0x0F;
	val *= 10;
	temp += val;
	return temp;
}

uint8 hex2Bcd(uint8 val){
	uint8 i, j, k;
	i = val / 10;
	j = val;
	k = j + (i << 4);
	return k;
}

void delayUs(uint16 us){
	while(us--);
}

void startIIC(){
	SDA = 1;
	delayUs(1);
	SCL = 1;
	delayUs(5);
	SDA = 0;
	delayUs(5);
	SCL = 0;
	delayUs(2);
}

void stopIIC(){
	SDA = 0;
	delayUs(1);
	SCL = 1;
	delayUs(5);
	SDA = 1;
	delayUs(4);
}

void sendByte(uint8 c){
	uint8 bitCnt;
	for(bitCnt = 0; bitCnt < 8; bitCnt++){
		if((c<<bitCnt)&0x80){
			SDA = 1;
		}else{
			SDA = 0;
		}
		delayUs(1);
		SCL = 1;
		delayUs(5);
		SCL = 0;
	}
	delayUs(2);
	SDA = 1;
	delayUs(2);
	SCL = 1;
	delayUs(3);
	if(SDA == 1){
		ack = 0;
	}else{
		ack = 1;
	}
	SCL = 0;
	delayUs(2);
}

uint8 recvByte(){
	uint8 retc = 0;
	uint8 bitCnt;
	
	for(bitCnt = 0; bitCnt < 8; bitCnt++){
		delayUs(1);
		SCL = 0;
		delayUs(5);
		SCL = 1;
		delayUs(3);
		retc = retc << 1;
		if(SDA == 1){
			retc++;
		}
		delayUs(2);
	}
	SCL = 0;
	delayUs(2);
	return retc;
}

void ackIIC(bit a){
	if(a){
		SDA = 1;
	}else{
		SDA = 0;
	}
	delayUs(3);
	SCL = 1;
	delayUs(5);
	SCL = 0;
	delayUs(2);
}

uint8 writeByte(uint8 addr, uint8 writeData){
	startIIC();
	sendByte(WriteAddress);
	if(!ack) return 0;
	sendByte(addr);
	if(!ack) return 0;
	sendByte(writeData);
	if(!ack) return 0;
	stopIIC();
	delayUs(10);
	return 1;
}

uint8 readCurrent(){
	uint8 readData;
	startIIC();
	sendByte(ReadAddress);
	if(!ack) return 0;
	readData = recvByte();
	ackIIC(1);
	stopIIC();
	return readData;
}

uint8 readRandom(uchar addr){
	startIIC();
	sendByte(WriteAddress);
	if(!ack) return 0;
	sendByte(addr);
	if(!ack) return 0;
	return readCurrent();
}

void setTime(Time t){
	writeByte(Year	, hex2Bcd(t.year));
	writeByte(Month	, hex2Bcd(t.month));
	writeByte(Day	, hex2Bcd(t.day));
	writeByte(Hour	, hex2Bcd(t.hour));
	writeByte(Minute, hex2Bcd(t.minute));
	writeByte(Second, hex2Bcd(t.second));
	writeByte(Week	, hex2Bcd(t.week));
}

void setTimeBCD(TimeBCD t){
	writeByte(Year	, t.year);
	writeByte(Month	, t.month);
	writeByte(Day	, t.day);
	writeByte(Hour	, t.hour);
	writeByte(Minute, t.minute);
	writeByte(Second, t.second);
	writeByte(Week	, t.week);
}

#define GetYear()	(readRandom(Year))
#define GetMonth()	(readRandom(Month))
#define GetDay()	(readRandom(Day))
#define GetWeek()	(readRandom(Week))
#define GetHour()	(readRandom(Hour) & 0x3F)
#define GetMinute()	(readRandom(Minute))
#define GetSecond()	(readRandom(Second))

TimeBCD getTimeBcd(){
	TimeBCD t;
	t.hour = GetHour();
	t.minute = GetMinute();
	t.second = GetSecond();
	return t;
}

TimeBCD getDateBcd(){
	TimeBCD t;
	t.year = GetYear();
	t.month = GetMonth();
	t.day = GetDay();
	return t;
}

TempBCD getTemp(){
	TempBCD t;
	t.temp = readRandom(TempH);
	t.tempp = readRandom(TempL);
	return t;
}

void initDs3231(){
	if((readRandom(Status) & 0x80)){
		//2015年12月9日 周三 1时59分45秒
		TimeBCD t = {0x15, 0x12, 0x11, 0x05, 0x22, 0x21, 0x40};
		setTimeBCD(t);
		writeByte(Status, readRandom(Status) & 0x7F);
	}
	writeByte(Control, 0x1C);
	//writeByte(Status, 0x00);
}

/************** AT24CXX PROGRAM *****************
** A2 = 1; A1 = 1; A0 = 1;
** Device Address:
** 2K:  1  0  1  0  A2  A1  A0 R/W
** 4K:  1  0  1  0  A2  A1  P0 R/W
** 8K:  1  0  1  0  A2  P1  P0 R/W
** 16K: 1  0  1  0  P2  P1  P0 R/W
************************************************/
#define AT24CXXWriteAddress 0xAE
#define AT24CXXReadAddress 0xAF

uint8 AT24CXXWriteByte(uint addr, uint8 writeData){
	startIIC();
	sendByte(AT24CXXWriteAddress);
	if(!ack) return 0;
	sendByte(addr >> 8);
	if(!ack) return 0;
	sendByte(addr);
	if(!ack) return 0;
	sendByte(writeData);
	if(!ack) return 0;
	stopIIC();
	delayUs(10);
	return 1;
}

uint8 AT24CXXReadCurrent(){
	uint8 readData;
	startIIC();
	sendByte(AT24CXXReadAddress);
	if(!ack) return 0;
	readData = recvByte();
	ackIIC(1);
	stopIIC();
	return readData;
}

uint8 AT24CXXReadRandom(uint addr){
	startIIC();
	sendByte(AT24CXXWriteAddress);
	if(!ack) return 0;
	sendByte(addr >> 8);
	if(!ack) return 0;
	sendByte(addr);
	if(!ack) return 0;
	return AT24CXXReadCurrent();
}

uint8 AT24CXXPageWrite(uint addr, uchar *dat, uchar len){
	startIIC();
	sendByte(AT24CXXWriteAddress);
	if(!ack) return 0;
	sendByte(addr >> 8);
	if(!ack) return 0;
	sendByte(addr);
	if(!ack) return 0;
	do{
		sendByte(*dat);
		if(!ack) return 0;
		dat++;
	}while(--len);
	stopIIC();
	return 1;
}

uint8 AT24CXXSequentialRead(uchar *dat, uchar len){
	startIIC();
	sendByte(AT24CXXReadAddress);
	if(!ack) return 0;
	do{
		*dat = recvByte();
		if(!ack) return 0;
		dat++;
	}while(--len);
	ackIIC(1);
	stopIIC();
	return 1;
}

void readAscii6x8(uchar c, uchar buf[8]){
//	uchar line;
	buf[0] = AT24CXXReadRandom((uint)(c) << 3);
//	for(line = 1; line < 8; line++){
//		buf[line] = AT24CXXReadCurrent();
//	}
	AT24CXXSequentialRead(&buf[1], 7);
}

uchar writeAscii6x8(uchar c, uchar figure[8]){
	if(AT24CXXPageWrite((uint)(c) << 3, figure, 8)){
		return 1;
	}else{
		return 0;
	}
}

#endif