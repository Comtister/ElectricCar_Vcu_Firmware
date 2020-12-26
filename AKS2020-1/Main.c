//KULLANILACAK KÜTÜPHANELERIN DAHIL EDILMESI
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "defines.h"
#include "stm32f4xx_tim.h"
#include "attributes.h"
#include "stdlib.h"
#include <stdio.h>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_onewire.h"
#include "tm_stm32f4_ds18b20.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_timer_properties.h"


//DEGISTIRILEN TIMER AYARLARININ BELIRTILMESI
#define TM_DELAY_TIM				TIM2
#define TM_DELAY_TIM_IRQ			TIM2_IRQn
#define TM_DELAY_TIM_IRQ_HANDLER	TIM2_IRQHandler

//BATARYAYA BAGLI SICAKLIK SENSÖRÜ SAYISI
#define EXPECTING_SENSORS 9

//MOTORA BAGLI SICAKLIK SENSÖRÜ SAYISI
#define EXPECTING_SENSORS_M 1

//BATARYA YÜZDE HESABI IÇIN SABITLER
#define oldMinVoltage 50.4
#define oldMaxVoltage 75.6
#define newMinPoint 0
#define newMaxPoint 100

//LCD'DEN GELEN VERILERIN TUTULDUGU DEGISKENLER
uint8_t buton=0, buton2=0, buton3=0; //BUTON EVENTLERINDEN DÖNEN DEGERLERI TUTAN DEGISKENLER
char arayuz[30];
uint8_t butondurum1 = 0; //BUTON DURUMUNUN TUTULDUGU DEGISKEN

//GUC ILE ALAKALI VERILERIN TUTULDUGU DEGISKENLER
float power;
char powerBuffer[10];

//AKIM ILE ALAKALI VERILERIN TUTULDUGU DEGISKENLER
float akim;
float finalcurrent;
int akimAdcBuffer;
int stabilAkim;
float deger = 0.1540;	
float wastedPower = 0;
float batteryAh;
float akmMaxGerilim;
int ccc = 0;
char akimBuffer[10];
char wastedPowerBuffer[10];

//HIZ ILE ALAKALI VERILERIN TUTULDUGU DEGISKENLER
uint16_t rpm;
uint8_t speed;
uint16_t sayac,timer;
char verim[20];
char devir[12];
char hiz[20];

//GERILIM ILE ALAKALI VERILERIN TUTULDUGU DEGISKENLER
char maxGerilimBuffer[10];
uint16_t ADC1_ValArray[4]; 
uint16_t ADC2_ValArray[9];
uint16_t ADC3_ValArray[8];
float VoltageValue[18]; 
float reelVoltage[18]; 
float totalVoltage; 
int bataryaYuzdesi;
char bataryaYuzdesiBuffer[10];
//MEDIAN FILTRESI IÇIN TAMPON DIZILER
float voltageBuffer1[10],voltageBuffer2[10],voltageBuffer3[10],voltageBuffer4[10],voltageBuffer5[10],voltageBuffer6[10],voltageBuffer7[10],voltageBuffer8[10],voltageBuffer9[10],voltageBuffer10[10],voltageBuffer11[10],voltageBuffer12[10],voltageBuffer13[10],voltageBuffer14[10],voltageBuffer15[10],voltageBuffer16[10],voltageBuffer17[10],voltageBuffer18[10];
//MEDIAN FILTRESI IÇIN SAYACLAR
int c1 = 0,c2 = 0,c3 = 0,c4 = 0,c5 = 0,c6 = 0,c7 = 0,c8 = 0,c9 = 0,c10 = 0,c11 = 0,c12 = 0,c13 = 0,c14 = 0,c15 = 0,c16 = 0,c17 = 0,c18 = 0;
// FILTRE EDILMIS ANALOG VERININ TUTULDUGU DIZI
float filteredADC[18]; 

//sicaklik degiskenleri
float totalTemp;
float avarageTemp;
float topTempBuffer;
float newTemps[9];
char maxTempBuffer[10];
char avarageTempBuffer[10];
//döngü degiskenleri
uint8_t i,j,k;
//BATARYA SICAKLIGININ DEGISKENLERI
char sicaklikbuf[40];
uint8_t devices,t,y,count,alarm_count;
uint8_t device[EXPECTING_SENSORS][8];
uint8_t alarm_device[EXPECTING_SENSORS][8];
float temps[EXPECTING_SENSORS];

//Motor SICAKLIGININ DEGISKENLERI
char sicaklikbufM[40];
uint8_t devicesM,tM,yM,countM,alarm_countM;
uint8_t deviceM[EXPECTING_SENSORS_M][8];
uint8_t alarm_deviceM[EXPECTING_SENSORS_M][8];
float tempsM[EXPECTING_SENSORS_M];
	

float avg,val1,val2,val3,val4,val5,val6,val7,val8,val9,val10,val11,val12,val13,val14,
	val15,val16,val17,val18,val19;
int m,d;
int hc = 0;
float hareketliOrtalama(float deger){
	
	avg = 0;
	m = 0;
	hc ++;
	if(hc == 1){
		val1 = deger;
		avg = val1;
	}
	if(hc == 2){
		val2 = deger;
		avg = val2;
	}
	if(hc == 3){
		val3= deger;
		avg = val3;
	}
	if(hc == 4){
		val4 = deger;
		avg = val4;
	}
	if(hc == 5){
		val5 =deger;
		avg = val5;
	}
	
	
	else if(hc > 5){
		hc = 6;
		
		if(val1 == 0){
			m = m+1;
		}
		if(val2 == 0)
	{
		m = m+1;
	}
	if(val3 == 0){
		m = m+1;
	}
	if(val4 == 0){
		m = m+1;
	}
	if(val5 == 0){
		m = m+1;
	}
	
	d = 6-m;
	
	if(d==0){
		avg = deger;
		hc = 1;
	}
	else{
		avg = (val1+val2+val3+val4+val5+deger)/d;
	}
	val1 = val2;
	val2 = val3;
	val3 = val4;
	val4 = val5;
	val5 = deger;
		
	}
	
	return avg;
	
}





void akimKesme(){
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	TIM_TimeBaseStructure.TIM_Prescaler = 8399;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 9999;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 50;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	TIM_Cmd(TIM3,ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		
}

void TIM3_IRQHandler(){
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update != RESET)){
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
		
		
		akimAdcBuffer = stabilAkim - 2915;
		akim = akimAdcBuffer*0.065;
		//Akim (-) ise (+) Ya Çevir
		if(akim < 0){
			
			akim = -1 * akim;
			
		}
		//Saniyede Harcanan Güç Hesabi
		wastedPower += (akim * akmMaxGerilim) / 3600;
		//Saatte Çekilen Akim Miktari
		batteryAh += akim / 3600;
		//AKS Ye Yollama Kismi
		ccc++;
		
	}
		
	}


/***********************************************************
* Function Name  : ReadCurrent
* Description    : Dijital Veriyi Akim Degerine Dönüstürür
* Input          : Int Dijital Analog Verisini
* Return         : float Akim Degeri
***********************************************************/
	
float ReadCurrent(uint16_t adc){
 adc = adc - 2740;
 akim= adc*0.065;
 //akim= akim/13.3;
 
	return akim;
}



	
/***********************************************************
* Function Name  : gerilimCikarim
* Description    : Ham Gerilim Degerlerini Huvcelere Böler
* Input          : int Çevrim Kanali
* Return         : float Hücre Gerilimi
***********************************************************/
void gerilimCikarim(uint8_t kanal){
	
	if(kanal == 0)
		reelVoltage[0] = VoltageValue[0];
	if(kanal == 1)
		reelVoltage[1] = VoltageValue[1] /2;
	if(kanal == 2)
		reelVoltage[2] = VoltageValue[2] / 3;
	if(kanal == 3)
		reelVoltage[3] = VoltageValue[3] / 4;
	if(kanal == 4)
		reelVoltage[4] = VoltageValue[4] / 5;
	if(kanal == 5)
		reelVoltage[5] = VoltageValue[5] / 6;
	if(kanal == 6)
		reelVoltage[6] = VoltageValue[6] / 7;
	if(kanal == 7)
		reelVoltage[7] = VoltageValue[7] / 8;
	if(kanal == 8)
		reelVoltage[8] = VoltageValue[8] / 9;
	if(kanal == 9)
		reelVoltage[9] = VoltageValue[9] / 10;
	if(kanal == 10)
		reelVoltage[10] = VoltageValue[10] / 11;
	if(kanal == 11)
		reelVoltage[11] = VoltageValue[11] / 12;
	if(kanal == 12)
		reelVoltage[12] = VoltageValue[12] / 13;
	if(kanal == 13)
		reelVoltage[13] = VoltageValue[13] / 14;
	if(kanal == 14)
		reelVoltage[14] = VoltageValue[14] / 15;
	if(kanal == 15)
		reelVoltage[15] = VoltageValue[15] / 16;
	if(kanal == 16)
		reelVoltage[16] = VoltageValue[16]  / 17;
	if(kanal == 17)
		reelVoltage[17] = VoltageValue[17] / 18;
	
	
}
/***********************************************************
* Function Name  : gerilimHesabiSon
* Description    : Ham Gerilim Degerlerini Hesaplar
* Input          : int Çevrim Kanali
* Return         : float Gerilim
***********************************************************/
void gerilimHesabiSon(uint8_t kanal){
	
	if(kanal == 0)
		VoltageValue[0] = filteredADC[0] * 0.0376470588235;
	if(kanal == 1)
		VoltageValue[1] = filteredADC[1] * 0.0380693069306;
	if(kanal == 2)
		VoltageValue[2] = filteredADC[2] *0.038178807947;
	if(kanal == 3)
		VoltageValue[3] = filteredADC[3] *0.1183076923076;
	if(kanal == 4)
		VoltageValue[4] = filteredADC[4] *0.0382868525896;
	if(kanal == 5)
		VoltageValue[5] = filteredADC[5] *0.0383056478405;
	if(kanal == 6)
		VoltageValue[6] = filteredADC[6] *0.038319088319;
	if(kanal == 7)
		VoltageValue[7] = filteredADC[7] *0.0383541147132;
	if(kanal == 8)
		VoltageValue[8] = filteredADC[8] *0.0383592017738;
	if(kanal == 9)
		VoltageValue[9] = VoltageValue[8] + VoltageValue[0] ;
	if(kanal == 10)
		VoltageValue[10] = filteredADC[10] *0.0400189214758;
	if(kanal == 11)
		VoltageValue[11] = filteredADC[11] *0.0383527454242;
	if(kanal == 12)
		VoltageValue[12] = VoltageValue[11] + VoltageValue[0];
	if(kanal == 13)
		VoltageValue[13] = filteredADC[13] *0.0383737517831;
	if(kanal == 14)
		VoltageValue[14] = filteredADC[14] *0.0177265745007;
	if(kanal == 15)
		VoltageValue[15] = filteredADC[15] *0.0382700684505;
	if(kanal == 16)
		VoltageValue[16] = filteredADC[16] *0.0563793103448;
	if(kanal == 17)
		VoltageValue[17] = filteredADC[17] *0.0319219089;
	
}




/***********************************************************
* Function Name  : EkranGonder
* Description    : LCD'ye Komut Gönderir
* Input          : char[] komut , char deger
* Return         : Void
***********************************************************/
void EkranGonder(char nesne[],char deger[]){
	      TM_USART_Puts(USART1,nesne);
				TM_USART_Putc(USART1,'"');
				TM_USART_Puts(USART1,deger);
				TM_USART_Putc(USART1,'"');
				TM_USART_Putc(USART1,0xFF);
				TM_USART_Putc(USART1,0xFF);
				TM_USART_Putc(USART1,0xFF);
	 //USART1 için veri gönderme fonsiyonu ekran haberlesmesi
 }

 /***********************************************************
* Function Name  : EkranGonderInt
* Description    : LCD'ye Komut Gönderir
* Input          : char[] komut , Int deger
* Return         : Void
***********************************************************/
 void EkranGonderInt(char nesne[],int deger){
	 TM_USART_Puts(USART1,nesne);
	 TM_USART_Putc(USART1,deger);
	 TM_USART_Putc(USART1,0xFF);
	 TM_USART_Putc(USART1,0xFF);
	 TM_USART_Putc(USART1,0xFF);
	 //USART1 icin sayi gonderme metodu
 }

 /***********************************************************
* Function Name  : TIM2_IRQHandler
* Description    : Kesme Sayacini ayarlar
* Input          : Void
* Return         : Void
***********************************************************/
void TIM2_IRQHandler()
	{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	timer=sayac;  
	sayac=0;
	
	}
	
	
	
	/***********************************************************
* Function Name  : EXTI_II
* Description    : Kesmeyi Aktif Hale Getirir
* Input          : Void
* Return         : Void
***********************************************************/
	void EXTI_II(void)

{

    // Clock for GPIOA

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    // Clock for SYSCFG

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

 

    // GPIOA initialization as an input from user button (GPIOA0)

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;

    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;

    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		

    GPIO_Init(GPIOD, &GPIO_InitStruct);

 

    // Selects the GPIOA pin 0 used as external interrupt source

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource6);


    // External interrupt settings

    EXTI_InitTypeDef EXTI_InitStruct;

    EXTI_InitStruct.EXTI_Line = EXTI_Line6;

    EXTI_InitStruct.EXTI_LineCmd = ENABLE;

    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;

    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;

    EXTI_Init(&EXTI_InitStruct);

 

    // Nested vectored interrupt settings

    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;

    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    // EXTI0_IRQn has Most important interrupt

    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;

    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;

    NVIC_Init(&NVIC_InitStruct);

}
/***********************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : Kesme Sayacini arttirir
* Input          : Void
* Return         : Void
***********************************************************/
void EXTI9_5_IRQHandler(void)

{

    // Checks whether the interrupt from EXTI0 or not

    if (EXTI_GetITStatus(EXTI_Line6))

    {

        // Toggle orange LED (GPIO13)

       sayac= sayac+1;

         

        // Clears the EXTI line pending bit

        EXTI_ClearITPendingBit(EXTI_Line6);

    }

}

/***********************************************************
* Function Name  : EkranDevirHiz
* Description    : LCD'ye Devir,Hiz Ve Verimlilik Degerlerini Gonderir
* Input          : Void
* Return         : Void
***********************************************************/
void EkranDevirHiz(){
	/*
	sprintf(devir,"%d",timer*15);
	speed=((((timer*30)*60)*160)/1000)/60;
	sprintf(hiz,"%d",speed);
  EkranGonder("t1.txt=",hiz);
	EkranGonder("t0.txt=",devir);
	*/
	rpm = timer*15*1.5151;
	speed = rpm*0.094248;
	sprintf(devir,"%d",rpm);
	sprintf(hiz,"%d",speed);
	EkranGonder("t1.txt=",hiz);
	EkranGonder("t0.txt=",devir);
	
	
	if(rpm>0 && rpm<650){
		EkranGonderInt("j0.val=",rpm*0.08);
		EkranGonderInt("j0.pco=",57344);
	}else if(rpm>650&& rpm<750){
		EkranGonderInt("j0.val=",rpm*0.08);
		EkranGonderInt("j0.pco=",57120);
	}else if(rpm>750 && rpm<850){
		EkranGonderInt("j0.val=",rpm*0.08);
		EkranGonderInt("j0.pco=",3648);
	}else if(rpm>850 && rpm<950){
		EkranGonderInt("j0.val=",rpm*0.08);
		EkranGonderInt("j0.pco=",57120);
	}else if(rpm>950){
		EkranGonderInt("j0.val=",rpm*0.08);
		EkranGonderInt("j0.pco=",57344);
	}
	
	
	if(timer==0){
				sprintf(verim,"%d",0);
		EkranGonder("t7.txt=",verim);}
  if(timer>400 && timer<=480){
		sprintf(verim,"%d",70);
		EkranGonder("t7.txt=",verim);}
	
	else if(timer>480 && timer<=600){
		sprintf(verim,"%d",80);
		EkranGonder("t7.txt=",verim);}
	
	else if(timer>600 && timer<=800){
		sprintf(verim,"%d",88);
		EkranGonder("t7.txt=",verim);}
	
  else if(timer>0 && timer<=160){
		sprintf(verim,"%d",20);
		EkranGonder("t7.txt=",verim);}
	
  else if(timer>160 && timer<=200){
		sprintf(verim,"%d",32);
		EkranGonder("t7.txt=",verim);}
		
	else if(timer>200 && timer<=240){
		sprintf(verim,"%d",40);
		EkranGonder("t7.txt=",verim);
}
	else if(timer>240 && timer<=360){
		sprintf(verim,"%d",60);
		EkranGonder("t7.txt=",verim);
	}
	else if(timer>360 && timer<=400){
		sprintf(verim,"%d",64);
		EkranGonder("t7.txt=",verim);}
	
		else if(timer>800 && timer<=1000){
		sprintf(verim,"%d",90);
		EkranGonder("t7.txt=",verim);}
	}

	
/***********************************************************
* Function Name  : USART_Puts
* Description    : USART'Tan Veri Gönderir
* Input          : USARTX,veri
* Return         : Void
***********************************************************/
void USART_Puts(USART_TypeDef* USARTx, volatile char *s)
{
	
	while(*s){
	while ( !(USARTx -> SR & 0x00000040));
  USART_SendData(USARTx, *s);
	*s++;
}}
	
/***********************************************************
* Function Name  : GPIOA_Init
* Description    : GPIOA Hattini Analog Giris Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOA_Init(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

/***********************************************************
* Function Name  : GPIOB_Init
* Description    : GPIOB Hattini Analog Giris Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOB_Init(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1  ;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	
}

/***********************************************************
* Function Name  : GPIOC_Init
* Description    : GPIOC Hattini Analog Giris Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOC_Init(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	
}

/***********************************************************
* Function Name  : GPIOF_Init
* Description    : GPIOF Hattini Analog Giris Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOF_Init(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	
	
}

/***********************************************************
* Function Name  : GPIOD_Init_OUTPUT
* Description    : GPIOD Hattini Dijital Çikis Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOD_Init_OUTPUT(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_13 |GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	
	
}

/***********************************************************
* Function Name  : GPIOG_Init_OUTPUT
* Description    : GPIOG Hattini Dijital Çikis Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOG_Init_OUTPUT(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	
	
}


void GPIOG_Disable_OUTPUT(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, DISABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	
	
}


void GPIOD_Disable_OUTPUT(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, DISABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_13 |GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	
	
}


/***********************************************************
* Function Name  : GPIOF_Init_DInput
* Description    : GPIOF Hattini Dijital Giris Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOF_Init_DInput(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
		
}

/***********************************************************
* Function Name  : GPIOG_Init_DInput
* Description    : GPIOG Hattini Dijital Giris Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOG_Init_DInput(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
		
}

/***********************************************************
* Function Name  : GPIOE_Init_DInput
* Description    : GPIOE Hattini Dijital Giris Yapacak Sekilde Tanimlar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIOE_Init_DInput(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
		
}

/***********************************************************
* Function Name  : Init_ADC1
* Description    : ADC1 Kanalini Aktif Hale Getirir
* Input          : Void
* Return         : Void
***********************************************************/
void Init_ADC1(void){
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay= ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	ADC_InitStructure.ADC_Resolution            = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode					= ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode		= ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge  = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign							= ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion 			= 4;
	

	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_480Cycles);//C5 18
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 2, ADC_SampleTime_480Cycles);//C4 19 yedek
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 3, ADC_SampleTime_480Cycles);//B1 20 Yedek
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 4, ADC_SampleTime_480Cycles);//B0 //Akim


	
	ADC_Init(ADC1,&ADC_InitStructure);
	  
	ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE);
	
	ADC_DMACmd(ADC1,ENABLE);
  
  ADC_Cmd(ADC1,ENABLE);


	
}

/***********************************************************
* Function Name  : Init_ADC2
* Description    : ADC2 Kanalini Aktif Hale Getirir
* Input          : Void
* Return         : Void
***********************************************************/
void Init_ADC2(void){
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
	
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay= ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	ADC_InitStructure.ADC_Resolution            = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode					= ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode		= ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge  = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign							= ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion 			= 9;
	
	
		ADC_RegularChannelConfig(ADC2, ADC_Channel_12,  1, ADC_SampleTime_480Cycles);//C2 9
		ADC_RegularChannelConfig(ADC2, ADC_Channel_1,  2, ADC_SampleTime_480Cycles);//A1 10
		ADC_RegularChannelConfig(ADC2, ADC_Channel_0,  3, ADC_SampleTime_480Cycles);//A0 11
		ADC_RegularChannelConfig(ADC2, ADC_Channel_3,  4, ADC_SampleTime_480Cycles);//A3 12
		ADC_RegularChannelConfig(ADC2, ADC_Channel_2,  5, ADC_SampleTime_480Cycles);//A2 13
		ADC_RegularChannelConfig(ADC2, ADC_Channel_5,  6, ADC_SampleTime_480Cycles);//A5 14
		ADC_RegularChannelConfig(ADC2, ADC_Channel_4,  7, ADC_SampleTime_480Cycles);//A4 15
		ADC_RegularChannelConfig(ADC2, ADC_Channel_7,  8, ADC_SampleTime_480Cycles);//A7 16
		ADC_RegularChannelConfig(ADC2, ADC_Channel_6,  9, ADC_SampleTime_480Cycles);//A6 17 
		
		 
	  	ADC_Init(ADC2,&ADC_InitStructure);
			ADC_Cmd(ADC2,ENABLE);
	  	ADC_DMARequestAfterLastTransferCmd(ADC2,ENABLE);

	
	ADC_DMACmd(ADC2,ENABLE);
  
 
	
}

/***********************************************************
* Function Name  : Init_ADC3
* Description    : ADC3 Kanalini Aktif Hale Getirir
* Input          : Void
* Return         : Void
***********************************************************/
void Init_ADC3(void){
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay= ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	ADC_InitStructure.ADC_Resolution            = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode					= ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode		= ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge  = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign							= ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion 			= 8;
	
 	  ADC_RegularChannelConfig(ADC3, ADC_Channel_4,  1, ADC_SampleTime_480Cycles);//F6 1
		ADC_RegularChannelConfig(ADC3, ADC_Channel_6,  2, ADC_SampleTime_480Cycles);//F8 2
		ADC_RegularChannelConfig(ADC3, ADC_Channel_5,  3, ADC_SampleTime_480Cycles);//F7 3
		ADC_RegularChannelConfig(ADC3, ADC_Channel_8,  4, ADC_SampleTime_480Cycles);//F10 4
		ADC_RegularChannelConfig(ADC3, ADC_Channel_7,  5, ADC_SampleTime_480Cycles);//F9 5
		ADC_RegularChannelConfig(ADC3, ADC_Channel_11,  6, ADC_SampleTime_480Cycles);//C1 6
		ADC_RegularChannelConfig(ADC3, ADC_Channel_10,  7, ADC_SampleTime_480Cycles);//C0 7
		ADC_RegularChannelConfig(ADC3, ADC_Channel_13,  8, ADC_SampleTime_480Cycles);//C3 8
 	 
	ADC_Init(ADC3,&ADC_InitStructure);
	  
	ADC_DMARequestAfterLastTransferCmd(ADC3,ENABLE);
	
	ADC_DMACmd(ADC3,ENABLE);
  
  ADC_Cmd(ADC3,ENABLE);
}

/***********************************************************
* Function Name  : Init_DMA2_CH0_ADC1
* Description    : DMA2'nin 0.Kanali ADC1 Için Ayririr Ve Ayarlarini Yapar
* Input          : Void
* Return         : Void
***********************************************************/
void Init_DMA2_CH0_ADC1(void){  ///////////// FOR ADC1
	
	DMA_InitTypeDef DMA_InitStructure;
	
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_InitStructure.DMA_Channel            = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = ((uint32_t)0x4001204C);      /////////////////   ((uint32_t)0x4001224C)
	DMA_InitStructure.DMA_Memory0BaseAddr    = (uint32_t) ADC1_ValArray;
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize         = 4;
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
  
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream0, ENABLE);
	
	
	


	
}

/***********************************************************
* Function Name  : Init_DMA2_CH0_ADC1
* Description    : DMA2'nin 1.Kanali ADC3 Için Ayririr Ve Ayarlarini Yapar
* Input          : Void
* Return         : Void
***********************************************************/
void Init_DMA2_CH1_ADC2(void){  ///////////// FOR ADC2
	
	DMA_InitTypeDef DMA_InitStructure;
	
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_InitStructure.DMA_Channel            = DMA_Channel_1;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) 0x4001214C;     
	DMA_InitStructure.DMA_Memory0BaseAddr    = (uint32_t) ADC2_ValArray;
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize         = 9;
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
  
	
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream2, ENABLE);
	
  
}

/***********************************************************
* Function Name  : Init_DMA2_CH2_ADC3
* Description    : DMA2'nin 2.Kanali ADC3 Için Ayririr Ve Ayarlarini Yapar
* Input          : Void
* Return         : Void
***********************************************************/
void Init_DMA2_CH2_ADC3(void){  ///////////// FOR ADC3
	
	DMA_InitTypeDef  DMA_InitStructure;
	
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_InitStructure.DMA_Channel            = DMA_Channel_2;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) 0x4001224C;     
	DMA_InitStructure.DMA_Memory0BaseAddr    = (uint32_t) ADC3_ValArray;
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize         = 8;
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;

  
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream1, ENABLE);
	
	
}

/***********************************************************
* Function Name  : GPIO_USARTConf
* Description    : USART Ayarlarini Yapar
* Input          : Void
* Return         : Void
***********************************************************/
void GPIO_USARTConf(){
//SystemInit();
GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
	
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode =USART_Mode_Tx;
	USART_InitStructure.USART_Parity    =USART_Parity_No;
	USART_InitStructure.USART_StopBits  =USART_StopBits_1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	
	USART_Init(USART6,&USART_InitStructure);
	USART_Cmd(USART6, ENABLE);

}

/***********************************************************
* Function Name  : Hiz
* Description    : ??
* Input          : Void
* Return         : Void
***********************************************************/
void Hiz(){
	
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef        NVIC_InitStructure;
	
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	
	TIM_TimeBaseStructure.TIM_Prescaler      = 4199;
	TIM_TimeBaseStructure.TIM_CounterMode    =TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period         = 23000;
	TIM_TimeBaseStructure.TIM_ClockDivision  = TIM_CKD_DIV4;
  TIM_TimeBaseStructure.TIM_RepetitionCounter= 0;
	TIM_TimeBaseInit(TIM2, & TIM_TimeBaseStructure);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	
}

/***********************************************************
* Function Name  : telemetri
* Description    : Telemetri Yazilimina Verileri Gonderir
* Input          : Int Arayüz Kanali
* Return         : Void
***********************************************************/
void telemetri(uint8_t arayuz_kanali){
	
	    if(arayuz_kanali==1){	
		  sprintf(arayuz,"DS,1,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.1f,%d,END,\r\n",temps[0],temps[1],temps[2],reelVoltage[0],reelVoltage[1],reelVoltage[2],reelVoltage[3],reelVoltage[4],reelVoltage[5],finalcurrent,speed);
	    TM_USART_Puts(USART6,arayuz);
			TM_USART_Puts(USART3,arayuz);
			}
		
		else if(arayuz_kanali==2){
			sprintf(arayuz,"DS,2,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.1f,%d,END,\r\n",temps[3],temps[4],temps[5],reelVoltage[6],reelVoltage[7],reelVoltage[8],reelVoltage[9],reelVoltage[10],reelVoltage[11],finalcurrent,speed);
      TM_USART_Puts(USART6,arayuz);
			TM_USART_Puts(USART3,arayuz);
				
		}
		else if(arayuz_kanali==3){
			sprintf(arayuz,"DS,3,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.1f,%d,END,\r\n",temps[6],temps[7],temps[8],reelVoltage[12],reelVoltage[13],reelVoltage[14],reelVoltage[15],reelVoltage[16],VoltageValue[17],finalcurrent,speed);
			TM_USART_Puts(USART6,arayuz);
			TM_USART_Puts(USART3,arayuz);
		}
		else if(arayuz_kanali==4){
			sprintf(arayuz,"DS,4,%2.2f,%2.2f,0,%2.2f,%2.2f,0,0,0,0,%2.1f,%d,END,\r\n",0.0,temps[8],0.0,tempsM[0],finalcurrent,speed); //temps[10] Batarya kutusu sicakligimi evett
		  TM_USART_Puts(USART6,arayuz);
		  TM_USART_Puts(USART3,arayuz);
		}
		else if(arayuz_kanali==5){
      sprintf(arayuz,"DS,5,0,0,0,0,0,0,0,0,0,%2.1f,%d,END,\r\n",finalcurrent,speed); 
      TM_USART_Puts(USART6,arayuz);	
			TM_USART_Puts(USART3,arayuz);
		}
		else if(arayuz_kanali==6){
			 sprintf(arayuz,"DS,6,0,0,0,0,0,0,0,0,0,%2.1f,%d,END,\r\n",finalcurrent,speed);
		   TM_USART_Puts(USART6,arayuz); 
			TM_USART_Puts(USART3,arayuz);			
		}
		else if(arayuz_kanali==7){
			sprintf(arayuz,"DS,7,0,0,0,0,0,0,0,0,0,%2.1f,%d,END,\r\n",finalcurrent,speed);
		  TM_USART_Puts(USART6,arayuz);
			TM_USART_Puts(USART3,arayuz);
		}}

/***********************************************************
* Function Name  : powerHesap
* Description    : Anlik Gücü Hesaplar
* Input          : Int Akim, Int Gerilim
* Return         : Float Power
***********************************************************/
float powerHesap(uint16_t akim,uint16_t gerilim){
	
	return akim*gerilim;
	
}
		
/***********************************************************
* Function Name  : gerilimFiltre
* Description    : Tampon diziden sirasiyla okunan 20 gerilim degerini alir
küçükten büyüge siralar ortadaki 5 elemanini baska bir tampon diziye atar 
tampon dizinin ortalamasini filtre edilmis gerilim olarak geri döndürür.
* Input          : float sirali okunmus gerilimler
* Return         : float filtre edilmis gerilim
***********************************************************/
float gerilimFiltre(float datas[]){
	float geciciData[3];
	float voltageBuffer;
	for(int i=0;i<9;i++){
				for(int j=i;j<9;j++){
					if(datas[i]<datas[j+1]){
						voltageBuffer = datas[j];
						datas[i]=datas[j+1];
						datas[j+1]=voltageBuffer;
					}
				}
			}
	geciciData[0] = datas[4];
	geciciData[1] = datas[5];
  geciciData[2] = datas[6];
			
	return 		(geciciData[0] + geciciData[1] + geciciData[2])/3;
	
}

/***********************************************************
* Function Name  : bataryaYuzde
* Description    : Bataryadan Okunan Gerilim Degerini % Lige Dönüstürür.
* Input          : float Anlik Batarya Gerilimi
* Return         : Int Bataryanin Doluluk Durumu
***********************************************************/
int bataryaYuzde(int batteryVoltage){
	
	return ((batteryVoltage - oldMinVoltage) / (oldMaxVoltage - oldMinVoltage)) * (newMaxPoint - newMinPoint) + newMinPoint;
	
}

int main(){
RCC_HSEConfig(RCC_HSE_ON);
while(!RCC_WaitForHSEStartUp());

//OneWire Tanimlamalari
TM_OneWire_t OneWire1;
TM_OneWire_t OneWire2;	
//SISTEM KURULUMU	
SystemInit();	
//Internal Timer in Aktiflestirilmesi
akimKesme();
//Gecikme Kütüphanesinin Yüklenmesi
TM_DELAY_Init();	
//OneWire Pin Atamalari Ve Yüklenmesi
TM_OneWire_Init(&OneWire1,GPIOF,GPIO_Pin_11);	
TM_OneWire_Init(&OneWire2,GPIOF,GPIO_Pin_12);
//Usart Pin Atamalari Ve Yüklenmesi
TM_USART_Init(USART6, TM_USART_PinsPack_1, 9600); //Telemetri
TM_USART_Init(USART1, TM_USART_PinsPack_2, 9600); //Ekran
TM_USART_Init(USART3, TM_USART_PinsPack_3, 9600); //DORJI
TM_USART_Init(USART2,TM_USART_PinsPack_2,9600); // PD5 TX PD6 RX
 
	//Pin Ayarlarinin Çagirilmasi Ve Aktif Hale Getirilmesi
	GPIOD_Init_OUTPUT();
	GPIOG_Init_OUTPUT();
	GPIOA_Init();
	GPIOB_Init();
	GPIOC_Init();
	GPIOF_Init();
	GPIOF_Init_DInput();
	GPIOG_Init_DInput();
	GPIOE_Init_DInput();
	GPIO_USARTConf();
	//ADC Kanallarinin Ayarlarinin Aktif Hale Getirilmesi
	Init_ADC1();
	Init_ADC2();
	Init_ADC3();
	//DMA Donaniminin Aktif Hale Getirilmesi
	Init_DMA2_CH0_ADC1();
	Init_DMA2_CH1_ADC2();
  Init_DMA2_CH2_ADC3();
	//Analog-Dijital Çevrimin Baslatilmasi
	ADC_SoftwareStartConv(ADC1);
	ADC_SoftwareStartConv(ADC2);
	ADC_SoftwareStartConv(ADC3);
	//Kesme Isleminin Baslatilmasi
	EXTI_II();
  Hiz();
			
		
//Sicaklik Tanimlamalari BATARYA
// Sensörlerin Tanimlanmasi Ve Adreslenmesi 
	count=0;
	devices=TM_OneWire_First(&OneWire1);
	while(devices){
		
		count++;
		
		TM_OneWire_GetFullROM(&OneWire1,device[count-1]);
		
		devices=TM_OneWire_Next(&OneWire1);
	}
		
 if(count>0){

		sprintf(sicaklikbuf,"devices found on 1-wire:  %d\n",count);
	  TM_USART_Puts(USART6,sicaklikbuf);
	  for(t=0;t<count;t++){
			for(y=0;y<8;y++){
						sprintf(sicaklikbuf,"0x%02X",device[t][y]);
						TM_USART_Puts(USART6,"\n");
			}
		}
			
 }else{

		TM_USART_Puts(USART6,"No devices on OneWire\n");
	 
 }	 

  for(t=0;t<count;t++){
		
		TM_DS18B20_SetResolution(&OneWire1,device[t],TM_DS18B20_Resolution_10bits);
		
	}
 
	
	
	
	//SICAKLIK TANIMLAMALARI MOTOR////////////////////////////////////7
	// Sensörlerin Tanimlanmasi Ve Adreslenmesi
	countM=0;
	devicesM=TM_OneWire_First(&OneWire2);
	while(devicesM){
		
		countM++;
		
		TM_OneWire_GetFullROM(&OneWire2,deviceM[countM-1]);
		
		devicesM=TM_OneWire_Next(&OneWire2);
	}
		
 if(countM>0){

		sprintf(sicaklikbufM,"devices found on 1-wire:  %d\n",countM);
	  TM_USART_Puts(USART6,sicaklikbufM);
	  for(tM=0;tM<countM;tM++){
			for(yM=0;yM<8;yM++){
						sprintf(sicaklikbufM,"0x%02X",deviceM[tM][yM]);
						TM_USART_Puts(USART6,"\n");
			}
		}
			
 }else{

		TM_USART_Puts(USART6,"No devices on OneWire\n");
	 
 }	 

  for(tM=0;tM<countM;tM++){
		
		TM_DS18B20_SetResolution(&OneWire2,deviceM[tM],TM_DS18B20_Resolution_10bits);
		
	}
 
	
	
	
	
	while(1){
	
		
		 
			//Batarya Sicakliginin Okunamsi
			TM_DS18B20_StartAll(&OneWire1);
		 
		  //Sicaklik Bagli degilse Açma
			
		while(!TM_DS18B20_AllDone(&OneWire1));
		 
		  for(t=0;t<count;t++){
				   
				if(TM_DS18B20_Read(&OneWire1,device[t],&temps[t])){
					sprintf(sicaklikbuf,"TEMP %d: %3.5f; \n",t,temps[t]);
           
					TM_USART_Puts(USART6,sicaklikbuf);
				}else{
					TM_USART_Puts(USART6,"Reading error;\n");
				}
							
			}
			
			
			//Sicakliklari tampon diziye yerlestirme
			for(int i=0;i<9;i++){
				
				newTemps[i] = temps[i];
				
			}
			
			//Tampon Diziyi Siralama 
			for(int i=0;i<9;i++){
				for(int j=i;j<9;j++){
					if(newTemps[i]<newTemps[j+1]){
						topTempBuffer = newTemps[j];
						newTemps[i]=newTemps[j+1];
						newTemps[j+1]=topTempBuffer;
					}
				}
			}
			
			//ORTAMALA SICAKLIGI ALMA
			for(int i=0;i<9;i++){
				
				avarageTemp += temps[i];
				
			}
			/*
			avarageTemp = avarageTemp/9;
			
			sprintf(avarageTempBuffer,"%2.1f",avarageTemp);
			EkranGonder("t2.txt=",avarageTempBuffer);
			*/
			avarageTemp = 0;
			//MAX Sicakligi Ekrana Gonderme
			sprintf(maxTempBuffer,"%2.1f",newTemps[0]);
			EkranGonder("t3.txt=",maxTempBuffer);
			
		  Delayms(5);
			
			
			
			//MOTOR SICAKLIGI //////////////////////////////////////////7
			
			TM_DS18B20_StartAll(&OneWire2);
		 // USART_Puts(USART6,"aa");
		//Sicaklik Bagli degilse Açma
			while(!TM_DS18B20_AllDone(&OneWire2));
		 
		  for(tM=0;tM<countM;tM++){
				   
				if(TM_DS18B20_Read(&OneWire2,deviceM[tM],&tempsM[tM])){
					sprintf(sicaklikbufM,"TEMP %d: %3.5f; \n",tM,tempsM[tM]);
           
					TM_USART_Puts(USART6,sicaklikbufM);
				}else{
					TM_USART_Puts(USART6,"Reading error;\n");
				}
							
			}
			
			
		//////////////////////////////////////////////////////////////////////
		//GERILIM OKUMA	
			
			GPIOG_Disable_OUTPUT();
			
		
		voltageBuffer1[c1] = ADC3_ValArray[0];
		c1++;
		
		if(c1 == 9){
			c1 = 0;
			filteredADC[0] = gerilimFiltre(voltageBuffer1);
		}

		voltageBuffer2[c2] = ADC3_ValArray[1];
		c2++;
		
		if(c2 == 9){
			c2 = 0;
			filteredADC[1] = gerilimFiltre(voltageBuffer2);
		}
		
		voltageBuffer3[c3] = ADC3_ValArray[2];
		c3++;
		
		if(c3 == 9){
			c3 = 0;
			filteredADC[2] = gerilimFiltre(voltageBuffer3);
		}
		
		voltageBuffer4[c4] = ADC3_ValArray[3];
		c4++;
		
		if(c4 == 9){
			c4 = 0;
			filteredADC[3] = gerilimFiltre(voltageBuffer4);
		}
		
		voltageBuffer5[c5] = ADC3_ValArray[4];
		c5++;
		
		if(c5 == 9){
			c5 = 0;
			filteredADC[4] = gerilimFiltre(voltageBuffer5);
		}
		
		voltageBuffer6[c6] = ADC3_ValArray[5];
		c6++;
		
		if(c6 == 9){
			c6 = 0;
			filteredADC[5] = gerilimFiltre(voltageBuffer6);
		}
		
		voltageBuffer7[c7] = ADC3_ValArray[6];
		c7++;
		
		if(c7 == 9){
			c7 = 0;
			filteredADC[6] = gerilimFiltre(voltageBuffer7);
		}
		
		voltageBuffer8[c8] = ADC3_ValArray[7];
		c8++;
		
		if(c8 == 9){
			c8 = 0;
			filteredADC[7] = gerilimFiltre(voltageBuffer8);
		}
		
		voltageBuffer9[c9] = ADC2_ValArray[0];
		c9++;
		
		if(c9 == 9){
			c9 = 0;
			filteredADC[8] = gerilimFiltre(voltageBuffer9);
		}
		
		voltageBuffer10[c10] = ADC2_ValArray[1];
		c10++;
		
		if(c10 == 9){
			c10 = 0;
			filteredADC[9] = gerilimFiltre(voltageBuffer10);
		}
		
		voltageBuffer11[c11] = ADC2_ValArray[2];
		c11++;
		
		if(c11 == 9){
			c11 = 0;
			filteredADC[10] = gerilimFiltre(voltageBuffer11);
		}
		
		voltageBuffer12[c12] = ADC2_ValArray[3];
		c12++;
		
		if(c12 == 9){
			c12 = 0;
			filteredADC[11] = gerilimFiltre(voltageBuffer12);
		}
		
		voltageBuffer13[c13] = ADC2_ValArray[4];
		c13++;
		
		if(c13 == 9){
			c13 = 0;
			filteredADC[12] = gerilimFiltre(voltageBuffer13);
		}
		
		voltageBuffer14[c14] = ADC2_ValArray[5];
		c14++;
		
		if(c14 == 9){
			c14 = 0;
			filteredADC[13] = gerilimFiltre(voltageBuffer14);
		}
		
		voltageBuffer15[c15] = ADC2_ValArray[6];
		c15++;
		
		if(c15 == 9){
			c15 = 0;
			filteredADC[14] = gerilimFiltre(voltageBuffer15);
		}
		
		voltageBuffer16[c16] = ADC2_ValArray[7];
		c16++;
		
		if(c16 == 9){
			c16 = 0;
			filteredADC[15] = gerilimFiltre(voltageBuffer16);
		}
		
		voltageBuffer17[c17] = ADC2_ValArray[8];
		c17++;
		
		if(c17 == 9){
			c17 = 0;
			filteredADC[16] = gerilimFiltre(voltageBuffer17);
		}
		
		voltageBuffer18[c18] = ADC1_ValArray[0];
		c18++;
		
		if(c18 == 9){
			c18 = 0;
			filteredADC[17] = gerilimFiltre(voltageBuffer18);
		}
			
		for(int i=0;i<18;i++){
			
			gerilimHesabiSon(i);
			
		}
		
		for(int i=0;i<18;i++){
			
			gerilimCikarim(i);
			
		}
		
		
		//Max gerilim
		totalVoltage = VoltageValue[17];
		
		
		
		//Batarya Yüzdesi
		bataryaYuzdesi = bataryaYuzde(totalVoltage);
		//Batarya Yüzdesi LCD'Ye Yazdirildi.
		sprintf(bataryaYuzdesiBuffer,"%d",bataryaYuzdesi);
		EkranGonder("t36.txt=",bataryaYuzdesiBuffer);	
		//Max Gerilim LCD'ye Yazdirildi
		sprintf(maxGerilimBuffer,"%2.1f",totalVoltage);
		EkranGonder("t5.txt=",maxGerilimBuffer);	
		
		
		
		EkranDevirHiz();
   
    //AKIM OKUMA
		
		
		
		stabilAkim = hareketliOrtalama(ADC1_ValArray[3]);
		akmMaxGerilim = totalVoltage;
		
		sprintf(wastedPowerBuffer,"%2.1f",wastedPower);
		EkranGonder("t2.txt=",wastedPowerBuffer);
		
		sprintf(akimBuffer,"%2.1f",akim);
		EkranGonder("t4.txt=",akimBuffer);
		
		power = powerHesap(akim,VoltageValue[17]);
		
		sprintf(powerBuffer,"%2.1f",power);
		EkranGonder("t6.txt=",powerBuffer);
		//////////////////////////////////////////////////////////////
		
		EkranDevirHiz();
		
		//Telemetri
		telemetri(1);
		Delayms(5);
		telemetri(2);
		Delayms(5);
		telemetri(3);
		Delayms(5);
		telemetri(4);
		Delayms(5);
		telemetri(5);
		Delayms(5);
		telemetri(6);
		Delayms(5);
		telemetri(7);
		
	//Ekrandan Gelen Veriye Göre Rölelerin Tetiklenmesi
	
	GPIOG_Init_OUTPUT();
	GPIOD_Init_OUTPUT();
		
		//Switchlerden Gelen Veriye Göre Rölelerin Tetiklenmesi
    if(!GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_13)){
			
			GPIO_SetBits(GPIOD,GPIO_Pin_14);
			
		}else{
			GPIO_ResetBits(GPIOD,GPIO_Pin_14);
		}
		
		if(!GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_0)){
			
			GPIO_SetBits(GPIOG,GPIO_Pin_3);
			
		}else{
			GPIO_ResetBits(GPIOG,GPIO_Pin_3);
		}
		
		 if(!GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_15)){
			
			GPIO_SetBits(GPIOG,GPIO_Pin_2);
			
		}else{
			GPIO_ResetBits(GPIOG,GPIO_Pin_2);
		}
		
		 if(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_7)){
			
			GPIO_SetBits(GPIOD,GPIO_Pin_15);
			
		}else{
			GPIO_ResetBits(GPIOD,GPIO_Pin_15);
		}
		
		 if(!GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_1)){
			
			GPIO_SetBits(GPIOD,GPIO_Pin_12);
			
		}else{
			GPIO_ResetBits(GPIOD,GPIO_Pin_12);
		}
		
		 if(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_9)){
			
			GPIO_SetBits(GPIOD,GPIO_Pin_10);
			
		}else{
			GPIO_ResetBits(GPIOD,GPIO_Pin_10);
		}
		
		 if(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8)){
			
			GPIO_SetBits(GPIOD,GPIO_Pin_13);
			
		}else{
			GPIO_ResetBits(GPIOD,GPIO_Pin_13);
		}
		
			
}
	
}

    
	

