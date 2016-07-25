/***************************************
** >MODULE NAME< Module
** Made By Ruilx
****************************************
** Title:          >PUT TITLE HERE<
** Time:           >Year</>Month</>Day<
** Author:         Ruilx
** Organization:   GT-Soft Studio
** License:        GPL
**
** Target:         >PUT TARGET HERE<
** Version:        0.1.0
****************************************
** Copyright(C) GT-Soft Studio
**                  All Rights Reserved.
***************************************/
//#define _NORMAL_PROGRAMMING_
//#define _TEST_PROGRAMMING_
#define _SLIVER_MECHINE_
#define USING_STC12C5A60S2_

#ifdef USING_STC12C5A60S2_
#	include <STC/STC12C5A60S2.h>
#endif

#ifdef USING_STC89C58RD_
#	include <STC/STC89C5xRC.h>
sfr IAP_CONTR = 0xE7;
#endif

#include <intrins.h>
#include <mcudef.h>
#define _DISABLE_STOP_TIMER_BEFORE_INTERRUPT_0_

#ifndef __REG52_H__
#define __REG52_H__
#endif
#include <stdTimerInterrupt.h>

#include <DS3231.h>

#include <ascii.h>

SET_IO( IO(1, 0), DS);
SET_IO( IO(1, 1), SH);
SET_IO( IO(1, 2), OE);
SET_IO( IO(1, 3), ST);
SET_IO( IO(1, 4), LA);
SET_IO( IO(1, 5), LB);
SET_IO( IO(1, 6), LC);
SET_IO( IO(1, 7), LD);

SET_IO( IO(3, 2), K1);
SET_IO( IO(3, 3), K2);
SET_IO( IO(3, 4), K3);
SET_IO( IO(3, 5), K4);

uchar data disBuf[2][8][5];
uchar bufIndex = 0;

void changeBufIndex(){
	uchar i = 0;
	uchar j = 0;
	if(bufIndex){
		bufIndex = 0;
		for(i=0; i<8; i++){
			for(j=0; j<5; j++){
				disBuf[1][i][j] = disBuf[0][i][j];
			}
		}
	}else{
		bufIndex = 1;
		for(i=0; i<8; i++){
			for(j=0; j<5; j++){
				disBuf[0][i][j] = disBuf[1][i][j];
			}
		}
	}
}

uchar code empty[][5] = {	//Empty Screen
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0xFF,0xFF,0xFF,0xFF,0xFF},
};

uchar code logo[][5] = {			//GT-Soft
{0xC1,0xFF,0xFF,0xFF,0xFF},
{0xCF,0xFF,0xC3,0xF8,0xFF},
{0xCF,0x9F,0xCF,0xF9,0x9F},
{0xC8,0x88,0x42,0x10,0x8F},
{0xCC,0x9F,0xF2,0x59,0x9F},
{0xCC,0x97,0xD2,0x59,0x97},
{0xC0,0x87,0xC2,0x10,0x87},
{0xFF,0xFF,0xFF,0xFF,0xFF},
};

uchar code rebooting[][5] = {
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0xC4,0x44,0x44,0x45,0xA3},
{0xD5,0xD5,0x56,0xEC,0xAF},
{0xC4,0x4D,0x56,0xED,0x2B},
{0xCD,0xD5,0x56,0xED,0xAB},
{0xD4,0x44,0x46,0xC5,0xA3},
{0xFF,0xFF,0xFF,0xFF,0xFF},
{0x00,0x00,0x00,0x00,0x00},
};

uchar code colonFlashing[][8] = {
{0xFF,0xDF,0x9F,0xFF,0xDF,0x9F,0xFF,0xFF},
{0xFF,0xDF,0x9F,0xFF,0xDF,0x9F,0xFF,0xFF},
{0xFF,0xBF,0x9F,0xFF,0xBF,0x9F,0xFF,0xFF},
{0xFF,0xBF,0x9F,0xFF,0xBF,0x9F,0xFF,0xFF},
{0xFF,0x9F,0xBF,0xFF,0x9F,0xBF,0xFF,0xFF},
{0xFF,0x9F,0xBF,0xFF,0x9F,0xBF,0xFF,0xFF},
{0xFF,0x9F,0xDF,0xFF,0x9F,0xDF,0xFF,0xFF},
{0xFF,0x9F,0xDF,0xFF,0x9F,0xDF,0xFF,0xFF},
};

volatile uchar timeCount = 0;
uchar timeStamp = 0;

typedef enum Font_e{
	SquareBoldFont = 0,
	SquareThinFont = 0x10,
	NormalFont = '0'
}Font;

bit hour24 = 1;		//使用24小时制
uchar font = SquareBoldFont;

void delay(uint time){
	while(time--);
}

void upload(uchar unit, uchar c){
	/****************************************
	** Mask:
	** ..111111 22222233 33334444 44555555 666666..
	** 
	****************************************/
	uchar code location[] = {0, 1, 1, 2, 3, 4};
	uchar code mask1[] = {0x3F, 0xFC, 0x03, 0x0F, 0x3F, 0xFC};
	uchar code mask2[] = {0x00, 0x00, 0xF0, 0xC0, 0x00, 0x00};
	uchar code adjust1[] = {2, 0, 6, 4, 2, 0};
	uchar code adjust2[] = {0, 0, 2, 4, 0, 0};
	uchar line;
	if(unit > 5) return;
	ET0 = 0;
	for(line=0; line<8; line++){
		disBuf[bufIndex ^ 0x01][line][location[unit]] = (disBuf[bufIndex ^ 0x01][line][location[unit]] & ~mask1[unit]) | ((ascii[c][line] >> adjust1[unit]) & mask1[unit]);
		if(mask2[unit]){
			disBuf[bufIndex ^ 0x01][line][location[unit]+1] = (disBuf[bufIndex ^ 0x01][line][location[unit]+1] & ~mask2[unit]) | ((ascii[c][line] << adjust2[unit]) & mask2[unit]);
		}
	}
	changeBufIndex();
	ET0 = 1;
}

void uploadData(uchar unit, uchar cData[8]){
	uchar code location[] = {0, 1, 1, 2, 3, 4};
	uchar code mask1[] = {0x3F, 0xFC, 0x03, 0x0F, 0x3F, 0xFC};
	uchar code mask2[] = {0x00, 0x00, 0xF0, 0xC0, 0x00, 0x00};
	uchar code adjust1[] = {2, 0, 6, 4, 2, 0};
	uchar code adjust2[] = {0, 0, 2, 4, 0, 0};
	uchar line;
	if(unit > 5) return;
	ET0 = 0;
	for(line=0; line<8; line++){
		disBuf[bufIndex ^ 0x01][line][location[unit]] = (disBuf[bufIndex ^ 0x01][line][location[unit]] & ~mask1[unit]) | ((cData[line] >> adjust1[unit]) & mask1[unit]);
		if(mask2[unit]){
			disBuf[bufIndex ^ 0x01][line][location[unit]+1] = (disBuf[bufIndex ^ 0x01][line][location[unit]+1] & ~mask2[unit]) | ((cData[line] << adjust2[unit]) & mask2[unit]);
		}
	}
	changeBufIndex();
	ET0 = 1;
}

void uploadScreen(uchar unit, uchar c, uchar disBuf[2][8][5]){
	uchar code location[] = {0, 1, 1, 2, 3, 4};
	uchar code mask1[] = {0x3F, 0xFC, 0x03, 0x0F, 0x3F, 0xFC};
	uchar code mask2[] = {0x00, 0x00, 0xF0, 0xC0, 0x00, 0x00};
	uchar code adjust1[] = {2, 0, 6, 4, 2, 0};
	uchar code adjust2[] = {0, 0, 2, 4, 0, 0};
	uchar line;
	if(unit > 5) return;
	
	for(line=0; line<8; line++){
		disBuf[bufIndex][line][location[unit]] = (disBuf[bufIndex][line][location[unit]] & ~mask1[unit]) | ((ascii[c][line] >> adjust1[unit]) & mask1[unit]);
		if(mask2[unit]){
			disBuf[bufIndex][line][location[unit]+1] = (disBuf[bufIndex][line][location[unit]+1] & ~mask2[unit]) | ((ascii[c][line] << adjust2[unit]) & mask2[unit]);
		}
	}
	changeBufIndex();
}

void uploadFromEEPROM(uchar unit, uchar c){
	uchar code location[] = {0, 1, 1, 2, 3, 4};
	uchar code mask1[] = {0x3F, 0xFC, 0x03, 0x0F, 0x3F, 0xFC};
	uchar code mask2[] = {0x00, 0x00, 0xF0, 0xC0, 0x00, 0x00};
	uchar code adjust1[] = {2, 0, 6, 4, 2, 0};
	uchar code adjust2[] = {0, 0, 2, 4, 0, 0};
	uchar numberData[8];
	uchar line;
	if(unit > 5) return;
	readAscii6x8(c, numberData);
	
	for(line=0; line<8; line++){
		disBuf[bufIndex ^ 0x01][line][location[unit]] = (disBuf[bufIndex ^ 0x01][line][location[unit]] & ~mask1[unit]) | ((numberData[line] >> adjust1[unit]) & mask1[unit]);
		if(mask2[unit]){
			disBuf[bufIndex ^ 0x01][line][location[unit]+1] = (disBuf[bufIndex ^ 0x01][line][location[unit]+1] & ~mask2[unit]) | ((numberData[line] << adjust2[unit]) & mask2[unit]);
		}
	}
	changeBufIndex();
}

void uploadNumber(ulong num){
	upload(5, (num % 10) | font);
	num /= 10;
	upload(4, (num % 10) | font);
	num /= 10;
	upload(3, (num % 10) | font);
	num /= 10;
	upload(2, (num % 10) | font);
	num /= 10;
	upload(1, (num % 10) | font);
	num /= 10;
	upload(0, (num % 10) | font);
}

void uploadNumberHex(ulong num){
	upload(5, (num & 0xF) | font);
	num >>= 1;
	upload(4, (num & 0xF) | font);
	num >>= 1;
	upload(3, (num & 0xF) | font);
	num >>= 1;
	upload(2, (num & 0xF) | font);
	num >>= 1;
	upload(1, (num & 0xF) | font);
	num >>= 1;
	upload(0, (num & 0xF) | font);
}

#ifdef USING_STC12C5A60S2_
void delay500ms(){		//@22.1184MHz
	unsigned char i, j, k;
	i = 43;
	j = 6;
	k = 203;
	do{
		do{
			while (--k);
		}while (--j);
	}while (--i);
}
#endif

#ifdef USING_STC89C58RD_
void delay500ms(){		//@22.1184MHz
	unsigned char i, j, k;
	_nop_();
	i = 8;
	j = 1;
	k = 243;
	do{
		do{
			while (--k);
		} while (--j);
	} while (--i);
}
#endif

#ifdef USING_STC12C5A60S2_
void delay10us(){		//@22.1184MHz
	unsigned char i;
	_nop_();
	i = 52;
	while (--i);
}
#endif

#ifdef USING_STC89C58RD_
void delay10us(){		//@22.1184MHz
	unsigned char i;
	_nop_();
	i = 6;
	while (--i);
}
#endif

void send(uchar dat){
	uchar i = 0;
	for(; i < 8; i++){
		DS = dat & 0x80;
		SH = 1;
		SH = 0;
		dat <<= 1;
	}
}

#define line(line) {\
	LA = line & 0x01;\
	LB = line & 0x02;\
	LC = line & 0x04;\
}

void show(uchar line){
	OE = 1;
	line(line);
	ST = 1;
	ST = 0;
	OE = 0;
	delay(500);
	OE = 1;
}

void updateScreen(uchar screen[][5]){
	uchar *p = (uchar *)screen;
	uchar *pd = (uchar *)disBuf[bufIndex ^ 0x01];
	uchar *pe = p + 40;
	while(p < pe){
		*pd = *p;
		pd++;
		p++;
	}
	changeBufIndex();
}

void flashScreen(){

	static uchar currentLine = 0x01;
	uchar * lineHeadPointer;
	switch(currentLine){
		case 0x01: 
			lineHeadPointer = &disBuf[bufIndex][0][4];
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer));
			show(0);
			break;
		case 0x02: 
			lineHeadPointer = &disBuf[bufIndex][1][4];
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer));
			show(1);
			break;
		case 0x04: 
			lineHeadPointer = &disBuf[bufIndex][2][4];
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer));
			show(2);
			break;
		case 0x08: 
			lineHeadPointer = &disBuf[bufIndex][3][4];
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer));
			show(3);
			break;
		case 0x10: 
			lineHeadPointer = &disBuf[bufIndex][4][4];
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer));
			show(4);
			break;
		case 0x20: 
			lineHeadPointer = &disBuf[bufIndex][5][4];
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer));
			show(5);
			break;
		case 0x40: 
			lineHeadPointer = &disBuf[bufIndex][6][4];
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer));
			show(6);
			break;
		case 0x80: 
			lineHeadPointer = &disBuf[bufIndex][7][4];
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer--));
			send(*(lineHeadPointer));
			show(7);
			break;
	}
	currentLine = _crol_(currentLine, 1);
	timeCount++;
}


#ifdef USING_STC12C5A60S2_
void uartInit(void){	//115200BPS @ 22.1184MHz
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0x50;		//8位数据, 可变波特率
	AUXR |= 0x04;		//Fosc发生器, 1T
	BRT = 0xFA;			//设定独立波特率发生器重装值
	AUXR |= 0x01;		//串口1选择独立波特率发生器为波特率发生器
	AUXR |= 0x10;		//启动独立波特率发生器
	ES = 1;				//打开串口中断
	EA = 1;				//打开总中断
}

void _sendChar(uchar ch){
	SBUF = ch;
	while(TI == 0);
	TI = 0;
}

void _sendUart(uchar *str){
	ET0 = 0;
	while(*str){
		_sendChar(*str);
		str++;
	}
	ET0 = 1;
}
#endif

#ifdef USING_STC89C58RD_
void uartInit(void)		//115200bps@22.1184MHz
{
	SCON = 0x50;		//8???,?????
	AUXR &= 0xBF;		//???1???Fosc/12,?12T
	AUXR &= 0xFE;		//??1?????1???????
	TMOD &= 0x0F;		//?????1?16???????
	TL1 = 0xFC;		//??????
	TH1 = 0xFF;		//??????
	ET1 = 0;		//?????1??
	TR1 = 1;		//?????1
	ES = 1;
	EA = 1;
}
#endif


#ifdef _NORMAL_PROGRAMMING_
void main(void){
	TimeBCD time;
	TimeBCD timeOld = {0};
	uchar currentTimeCount = 0;
	uchar currentTimeStamp = 0;
	static uchar colonStatement = 0;
	
	uartInit();
	_sendUart("============== LED CLOCK ================\r\n");
	_sendUart("=            Program Begin              =\r\n");
	_sendUart("=========================================\r\n");
	_sendUart("\n");
	_sendUart("[Infor] Initializing DS3231...\r\n");
	initDs3231();
	_sendUart("[Infor] Initializing Timer0...\r\n");
	timer0Init(1041, 22.1184, TIMER_OF_16_BIT, flashScreen);
	_sendUart("[Infor] Initializing Logo to Screen...\r\n");
	updateScreen(logo);
	_sendUart("[Infor] Start Timer0 to Splash Screen...\r\n");
	timer0Start();
	delay500ms();
	delay500ms();
	_sendUart("[Infor] Clear Screen\r\n");
	updateScreen(empty);
	_sendUart("[Infor] Get First Time\r\n");
	time = getTimeBcd();
	while(1){
	if(!timeCount){ 
		currentTimeStamp = ++timeStamp;
	}
	currentTimeCount = timeCount;
	
	if(!(currentTimeStamp & 0x1F) && !currentTimeCount){ 	//Count: 64:0 TimeStamp to read a time from IIC
//		_sendUart("[Infor] [Loop] Getting Time...\n");
		time = getTimeBcd();
	}
	if(time.hour != timeOld.hour){
		if(!hour24){	//12小时制
			if(time.hour < 0x12){	//早上
				upload(0, 0x0A);
				time.hour >> 4 ? upload(1, (time.hour >> 4) + font) : upload(1, ' ');
				upload(2, (time.hour & 0x0F) + font);
			}else if(time.hour == 0x20 || time.hour == 0x21){	//20点至21点BCD码要减去0x18
				time.hour -= 0x18;
				upload(0, 0x0B);
				upload(1, (time.hour >> 4) + font);
				upload(2, (time.hour & 0x0F) + font);
			}else{
				time.hour -= 0x12; //其他下午时间只需要减去0x12即可
				upload(0, 0x0B);
				time.hour >> 4 ? upload(1, (time.hour >> 4) + font) : upload(1, ' '); 
				upload(2, (time.hour & 0x0F) + font);
			}
		}else{	//24小时制
			upload(0, 0x0C);
			upload(1, (time.hour >> 4) + font);
			upload(2, (time.hour & 0x0F) + font);
		}
		timeOld.hour = time.hour;
	}
	if(time.minute != timeOld.minute){
		upload(4, (time.minute >> 4) + font);
		upload(5, (time.minute & 0x0F) + font);
		timeOld.minute = time.minute;
	}
//	if(time.second != timeOld.second){
//		upload(4, (time.second >> 4));
//		upload(5, (time.second & 0x0F));
//		timeOld.second = time.second;
//	}
	if(!(currentTimeStamp & 0x0F) && !currentTimeCount){	//8:0 timeStamp to update a colon symbol
//		_sendUart("[Infor] [Loop] Update Colun... \n");
//		_sendUart((colonStatement & 0x07) + '0');
		uploadData(3, colonFlashing[colonStatement & 0x07]);
		colonStatement++;
	}
}
	}
#endif

#ifdef _TEST_PROGRAMMING_
void main(void){
	uchar i, j;
	uchar numData[8];
	uartInit();
	timer0Init(1041, 22.1184, TIMER_OF_16_BIT, flashScreen);
	timer0Start();
	updateScreen(empty);
	for(i = 0; i < 128; i++){
		uploadNumber(i);
		if(writeAscii6x8(i, ascii[i])){
			delay500ms();
		}else{
			upload(0, 'E');
			upload(1, 'r');
			upload(2, 'r');
			upload(3, 'o');
			upload(4, 'r');
			upload(5, '!');
			while(1);
		}
	}
	upload(0, 'F');
	upload(1, 'i');
	upload(2, 'n');
	upload(3, 'i');
	upload(4, 's');
	upload(5, 'h');
	for(i = 0; i < 128; i++){
		uploadNumber(i);
		delay500ms();
		readAscii6x8(i, numData);
		for(j = 0; j < 8; j++){
			if(numData[j] != ascii[i][j]){
				upload(0, 'E');
				upload(1, 'r');
				upload(2, 'r');
				upload(3, 'o');
				upload(4, 'r');
				upload(5, '2');
				while(1);
			}
		}
	}
	while(1);
}
#endif
	
#ifdef _DEMO_
void main(void){
	uint delay;
	ulong i=0;
	timer0Init(1041, 22.1184, TIMER_OF_16_BIT, flashScreen);
	timer0Start();
	uartInit();
	updateScreen(empty);
	uploadNumber(0);
	while(1){
		while(!K1){
			i++;
			uploadNumber(i);
		}
		while(!K2){
			i--;
			uploadNumber(i);
		}
		while(!K3){
			while(!K3 && delay){
				delay = 5000;
				while(delay--);
			}
			if(font == SquareBoldFont){
				font = SquareThinFont;
			}else if(font == SquareThinFont){
				font = NormalFont;
			}else{
				font = SquareBoldFont;
			}
			uploadNumber(i);
		}
		while(!K4){
			i = 0;
			uploadNumber(i);
		}
	}
}
#endif
	
#ifdef _SLIVER_MECHINE_
#include <stdExternalInterrupt.h>
SET_IO( IO(2, 5), RW);
SET_IO( IO(2, 6), RS);
SET_IO( IO(3, 2), EN);
#define dataPort P0
uchar *p = &disBuf[0][0][0];
uchar *pStart = &disBuf[0][0][0];
uchar *pEnd = &disBuf[1][7][4];
void enable(){
	if(RS == HIGH){ //Data
		if(RW == HIGH){ //Read
			
		}else{ //Write
			*p = dataPort;
			if(p != pEnd){
				p++;
			}else{
				p = pStart;
			}
		}
	}else{ //Cmd
		if(RW == HIGH){ //Read
			
		}else{ //Write
			uchar block = dataPort;
			if(block < 40){
				p = &disBuf[0][0][0] + block;
			}
		}
	}
}

void main(void){
	timer0Init(1041, 22.1184, TIMER_OF_16_BIT, flashScreen);
	timer0Start();
	uartInit();
	uploadScreen(logo);
	externalInterrupt0Init(FALLING_EDGE_TRIGGER, enable);
	while(1){
		
	}
}
#endif

void uartInterrupt(void) interrupt 4{
	if(RI){
		if(SBUF == 0xEA){
			RI = 0;
			IAP_CONTR = 0x60;
			return;
		}else if(SBUF == 0xEB){
		}
	}
	if(TI){
		TI = 0;
	}
}