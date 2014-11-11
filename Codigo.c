#include <p18f4550.h>
#include <string.h>
#include <stdio.h>

// Configuração do microcontrolador para execução de instruções

#pragma config FOSC     = HS    /// EXTERN CLOCK 8MHZ
#pragma config IESO     = OFF   /// INTERNAL/EXTERNAL OSCILATOR DISABLE
#pragma config PWRT     = OFF   /// DISABLE POWER-UP TIMER
#pragma config BORV     = 3     /// BROWN-OUT RESET MINIMUM
#pragma config WDT      = OFF   /// DISABLE WATCHDOG TIMER
#pragma config WDTPS    = 32768 /// WATCHDOG TIMER 32768s
#pragma config MCLRE    = OFF   /// MASTER CLEAR PIN (RE3) DISBALE
#pragma config LPT1OSC  = OFF   /// TIMER1 LOW POWER OPERATION
#pragma config PBADEN   = OFF   /// PORTB.RB0,1,2,3,4 AS I/O DIGITAL
#pragma config STVREN   = ON    /// STACK FULL/UNDERFLOW CAUSE RESET
#pragma config LVP      = OFF   /// DISABLE LOW VOLTAGE PROGRAM (ICSP DISABLE)

//declarar as suas variaveis de hardware

#define Barramento_Lcd PORTD
#define RS PORTCbits.RE0
#define WR PORTCbits.RE2
#define E PORTCbits.RE1

//#define Start_Bit 0

//declara as suas variaveis de software
char mens []= "Exemplo de Programacao Serial\r";

void Delay_1200bps (){ // tempo = 833us -> 1200bps
	int i;
	for (i=0;i<400;i++) {}

}

void Envia_Comando_Lcd (unsigned char dado){
	RS=0;  //seleciona envio de instrução
	WR=0;  //seleciona modo de escrita no lcd
	E=1;
	Barramento_Lcd=dado;
	E=0;
	Delay_1200bps ();
}

void envia_Dado_Lcd (unsigned char dado)

{
RS=1;
WR=0;
E=1;
Barramento_Lcd=dado;
E=0;
Delay_1200bps ();
}

void Inicializa_Lcd ()

{
Envia_Comando_Lcd (0x38);
Envia_Comando_Lcd (0x38);
Envia_Comando_Lcd (0x06);
Envia_Comando_Lcd (0x0E);
Envia_Comando_Lcd (0x01);
}


void Envia_Frase_Lcd (char *mensagem){
	int i;
	for (i = 0;mensagem[i] != '\0' ; i++){}
	
	
	for(;i!=0;i--){
		envia_Dado_Lcd(mensagem[i]);
	}
}



void main(){
 
TRISC=0b00011111;  // 
TRISD=0b00000000;
Inicializa_Lcd ();
Envia_Frase_Lcd(mens);

while (1)
	{
//	Envia_Frase_Lcd ("Exemplo de Programacao Serial\r");
//	Envia_Frase_Lcd ("As mensagens estao inseridas direto no codigo\r");
//	Envia_Frase_Lcd ("e podem ter qualquer tamanho\r");
//	Envia_Frase_Lcd ("OBS: nao gastamos RAM para alocar as mensagens\r");
	while (1) {}; // parada forçada
	}
}