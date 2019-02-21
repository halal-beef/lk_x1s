#include <sys/types.h>
#include <platform/exynos9830.h>

#define Outp32(addr, data) (*(volatile u32 *)(addr) = (data))
#define Inp32(addr) ((*(volatile u32 *)(addr)))

#define PWMTIMER_BASE			(EXYNOS9830_PWMTIMER_BASE)

#define rTCFG0					(PWMTIMER_BASE+0x00)
#define rTCFG1					(PWMTIMER_BASE+0x04)
#define rTCON					(PWMTIMER_BASE+0x08)
#define rTCNTB0					(PWMTIMER_BASE+0x0C)
#define rTCMPB0					(PWMTIMER_BASE+0x10)
#define rTCNTO0					(PWMTIMER_BASE+0x14)
#define rTCNTB1					(PWMTIMER_BASE+0x18)
#define rTCMPB1					(PWMTIMER_BASE+0x1C)
#define rTCNTO1					(PWMTIMER_BASE+0x20)
#define rTINT_CSTAT				(PWMTIMER_BASE+0x44)

void Dmc_StartTimer(u32 uTsel)
{
	u32 uTimer=uTsel;
	u32 uTemp0,uTemp1;
	u32 uEnInt=0;
	u32 uDivider=0;
	u32 uDzlen=0;
	u32 uPrescaler=66;					/*- Silicon : uPrescaler=66   /  FPGA : uPrescaler=24*/
	u32 uEnDz=0;
	u32 uAutoreload=1;
	u32 uEnInverter=0;
	u32 uTCNTB=0xffffffff;
	u32 uTCMPB=(u32)(0xffffffff/2);
	u32 i=0;
	u32 uCurrentTCNTO;

	{
		uTemp0 = Inp32(rTCFG1);			/* 1 changed into 0xf; dohyun kim 090211*/
		uTemp0 = (uTemp0 & (~ (0xf << 4*uTimer) )) | (uDivider<<4*uTimer);
		/* TCFG1			init. selected MUX 0~4, 	select 1/1~1/16*/

		Outp32(rTCFG1,uTemp0);

		uTemp0 = Inp32(rTINT_CSTAT);
		uTemp0 = (uTemp0 & (~(1<<uTimer)))|(uEnInt<<(uTimer));
		/*		selected timer disable, 		selected timer enable or diable.*/
		Outp32(rTINT_CSTAT,uTemp0);
	}

	switch(uTimer)
	{
		case 0:
			uTemp0 = Inp32(rTCON);
			uTemp0 = uTemp0 & (0xfffffffe);
			Outp32(rTCON, uTemp0);			/* Timer0 stop */

			uCurrentTCNTO = Inp32(rTCNTO0);

			uTemp0 = Inp32(rTCFG0);
			uTemp0 = (uTemp0 & (~(0xff00ff))) | ((uPrescaler-1)<<0) |(uDzlen<<16);
			/*			init. except prescaler 1 value, 	set the Prescaler 0 value,	set the dead zone length.*/
			Outp32(rTCFG0, uTemp0);

			Outp32(rTCNTB0, uTCNTB);
			Outp32(rTCMPB0, uTCMPB);


			uTemp1 = Inp32(rTCON);
			uTemp1 = (uTemp1 & (~(0x1f))) |(uEnDz<<4)|(uAutoreload<<3)|(uEnInverter<<2)|(1<<1)|(0<<0);
			/*		set all zero in the Tiemr 0 con., Deadzone En or dis, set autoload, 	set invert,  manual uptate, timer stop*/
			Outp32(rTCON, uTemp1);									/*timer0 manual update*/
			uTemp1 = (uTemp1 & (~(0x1f))) |(uEnDz<<4)|(uAutoreload<<3)|(uEnInverter<<2)|(0<<1)|(1<<0);
			/*		set all zero in the Tiemr 0 con., Deadzone En or dis, set autoload, 	set invert,  manual uptate disable, timer start*/
			Outp32(rTCON, uTemp1);									/* timer0 start*/

			do{
				if(i == 5000)
					break;
				i++;
			}while(Inp32(rTCNTO0) == uCurrentTCNTO);

			break;
		case 1:
			uTemp0 = Inp32(rTCON);
			uTemp0 = uTemp0 & (0xfffffeff);
			Outp32(rTCON, uTemp0);			/* Timer0 stop */

			uCurrentTCNTO = Inp32(rTCNTO0);

			uTemp0 = Inp32(rTCFG0);
			uTemp0 = (uTemp0 & (~(0xff00ff))) | ((uPrescaler-1)<<0) |(uDzlen<<16);
			/*			init. except prescaler 1 value, 	set the Prescaler 0 value,	set the dead zone length.*/
			Outp32(rTCFG0, uTemp0);

			Outp32(rTCNTB1, uTCNTB);
			Outp32(rTCMPB1, uTCMPB);


			uTemp1 = Inp32(rTCON);
			uTemp1 = (uTemp1 & (~(0xf00))) |(uAutoreload<<11)|(uEnInverter<<10)|(1<<9)|(0<<8);
			/*		set all zero in the Tiemr 0 con., Deadzone En or dis, set autoload, 	set invert,  manual uptate, timer stop*/
			Outp32(rTCON, uTemp1);			/*timer0 manual update*/
			uTemp1 = (uTemp1 & (~(0xf00))) |(uAutoreload<<11)|(uEnInverter<<10)|(0<<9)|(1<<8);
			/*		set all zero in the Tiemr 0 con., Deadzone En or dis, set autoload, 	set invert,  manual uptate disable, timer start*/
			Outp32(rTCON, uTemp1);			/* timer0 start */

			do{
				if(i == 5000)
					break;
				i++;
			}while(Inp32(rTCNTO1) == uCurrentTCNTO);

			break;
		default :
			break;
	}
}

void Dmc_PWM_stop(u32 uNum)
{
	u32 uTemp;

	uTemp = Inp32(rTCON);

	if(uNum == 0)
		uTemp &= ~(0x1);
	else
		uTemp &= ~((0x10)<<(uNum*4));

	Outp32(rTCON,uTemp);
}


u32 Dmc_StopTimer(u32 uTsel)
{
	u32 uVal = 0;

	Dmc_PWM_stop(uTsel);

	switch(uTsel)
	{
		case 0 :
			uVal = Inp32(rTCNTO0);
			break;
		case 1 :
		default :
			uVal = Inp32(rTCNTO1);
			break;
	}
	uVal = 0xffffffff - uVal;

	return uVal;
}


void DMC_Delay(u32 uTsel,u32 uSec)
{
	u32 uElapsedTime=0;

	Dmc_StartTimer(uTsel);

	while(1){
		if(uTsel==0)
			uElapsedTime = Inp32(rTCNTO0);
		else if(uTsel==1)
			uElapsedTime = Inp32(rTCNTO1);

		uElapsedTime = 0xffffffff - uElapsedTime;

		if(uElapsedTime > uSec){
			Dmc_StopTimer(uTsel);
			break;
		}
	}
}
