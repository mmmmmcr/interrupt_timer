#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

#define VNTimer 11 /* Interrupt vector for Timer */ 
#define VNkeyboard 22 /* Interrupt vector for Keyboard */ 

typedef	unsigned char	uint8;
typedef	unsigned short	uint16;
typedef	unsigned long	uint32;

typedef	char			int8;
typedef	short			int16;
typedef	long			int32;

// To Clear or Set single bits in a byte variable.
#define	b_SetBit(bit_ID, varID)		(varID |= (uint8)(1<<bit_ID))
#define	b_ClearBit(bit_ID, varID)	(varID &= ~(uint8)(1<<bit_ID))
#define	b_XorBit(bit_ID, varID)		(varID ^= (uint8)(1<<bit_ID))

// To Clear or Set single bits in a word(16-bit) variable.
#define	w_SetBit(bit_ID, varID)		(varID |= (uint16)(1<<bit_ID))
#define	w_ClearBit(bit_ID, varID)	(varID &= ~(uint16)(1<<bit_ID))
#define	w_XorBit(bit_ID, varID)		(varID ^= (uint16)(1<<bit_ID))

// To check single bit status in a given variable in ZERO PAGE
#define	b_CheckBit(varID, bit_ID)	((varID & (uint8)(1<<bit_ID))?1:0)
//#define	b_CheckBit(varID, bit_ID)	(varID & (muint8)(1<<bit_ID))
#define	w_CheckBit(varID, bit_ID) ((varID & (uint16)(1<<bit_ID))?1:0)

// To check single bit status in a given variable in ZERO PAGE
#define		DummyRead(varID)			{__asm lda varID;}




long count=1;
int cycle=1;
int init=8;//initializam cu 8 ca sa ne asiguram ca init%8==0 este adevarat din start
void PeriphInit(void);

void main(void) 
{
	EnableInterrupts;
	PeriphInit(); /* Microcontroller initialization */
	for(;;){}
}

interrupt VNTimer void TPM1_overflow()
{ 
	byte varTOF; 
	varTOF = TPM1SC_TOF; // clear TOF; first read and then write 0 to the flag
	TPM1SC_TOF = 0;
	if(count%cycle==0){//va fi adevarata cand count atinge val cycle sau daca e multiplu de cycle
						//asa putem sa tinem evidenta  clockului si sa executam la cycle sec bazandu-ne pe faptul ca timer-ul nostru initial executa comanda la 1 sec; 
		if(init%8==0){//initializam PTFD cu 0b00000001 pentru a putea deplasa dupa bitul->asa aprindem becurile pe rand
					//se va initializa de fiecare data cand face un ciclu complet de 8 leduri
				PTFD=0b00000001;
				init=8;
				init++;
				count=1;//resetam count la 1 pt a numara x intreruperi de 1 secunda determinate de timer
						//nu il punem pe intreruperea data de push button deoarece ar trece instant la urmatorul led,
						//dar aici asteapta pana cand atinge x secunde chiar daca marim timpul de pe buton
				}
			else{
				init++;
				PTFD=(PTFD<<1);//deplasam bit-ul la stanga
				}
	}
	count++;
}

interrupt VNkeyboard void intKBI_SW()

{
	//__RESET_WATCHDOG();
	KBI1SC_KBACK = 1; //acknowledge interrupt
	//turn off and on led
	cycle=cycle*2;//marim cu 2 cand apasam butonul de fiecare data si initializam caunt cu 1 pentru a ne asigura ca prindem  cicli de clock  
	
}
//in loc de varianta curenta pt rezolvarea problemei puteam sa modificam registrii modulo cand aveam intrerupere push button 
//,dar depindeam de frecventa deoarece puteam depasi numarul de 65 535;
void PeriphInit()
{	
		SOPT = 0x00; // Disable watchdog
	
	
		/* enable interrupt for keyboard input */
		b_ClearBit(1, KBI1SC); /* KBI1SC: KBIE=0, disable KBI interrupt request */
		KBI1PE = 0b00100000; /* KBI1PE: enable KBI function for pin 5 only */
		b_ClearBit(0, KBI1SC); /* KBI1SC: KBIMOD=0, select edge-only detection */
		/* in defaut only falling edge events to be detected */
		b_SetBit(2, KBI1SC); /* KBI1SC: KBACK=1, to clear KBI flag */
		b_SetBit(1, KBI1SC); /* KBI1SC: KBIE=1, enable KBI */
		
		PTDPE_PTDPE2=1;//port D2 active for push button interrupt
		
	    PTFDD = 0xFF; // set PORTF direction as output
	    PTFD = 0x00; // Turn on LEDs
	    //timer:
	    	ICGC2 = 0X00; // Set up ICG control register 2
	    	ICGC1 = 0X78; // Set up ICG for FEE, 4MHz external crystal
	    	    // busclk = 4MHz

	    	// configure TPM module 1
	    	TPM1SC = 0x4F; // format: TOF(0) TOIE(1) CPWMS(0) CLKSB(0) CLKSA(1) PS2(1) PS1(1) PS0(1)	
	    	
	    	//4mhz = 4,000,000hz
	    	//prescaler 1:128 so our timer runs at
	    	//31 250 hz
	    	//and counts up to 655352.
	    	//to overflow once every second we need to count only up to 31 250, which is 7A 12
	    	TPM1MODH = 0xC; // set the counter modulo registers	
	    	TPM1MODL = 0x35; 	
		
		while ( b_CheckBit(ICGS1,3) == 0){}
}
