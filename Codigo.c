#include <p18f4550.h>
#include <string.h>
#include <stdio.h>
#include <delays.h>

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

#define PORT_LCD PORTD
#define PORT_CONT_LCD PORTE
#define TRIS_PORT_LCD TRISD
#define TRIS_CONT_LCD TRISE
#define _RS PORTEbits.RE0
#define _EN PORTEbits.RE1
#define _RW PORTEbits.RE2  //Pinos do LCD

#define c1 PORTBbits.RB3
#define c2 PORTBbits.RB2
#define c3 PORTBbits.RB1
#define l1 PORTBbits.RB4
#define l2 PORTBbits.RB5
#define l3 PORTBbits.RB6
#define l4 PORTBbits.RB7 //Pinos para leitura do keyboard

//declara as suas variaveis de software


void DelayFor20TCY(void){
	Nop();Nop();Nop();Nop();Nop();
	Nop();Nop();Nop();Nop();Nop();
	Nop();Nop();Nop();Nop();Nop();
}

void Pulse(void){
	DelayFor20TCY();
	_EN = 1;
	DelayFor20TCY();
	_EN = 0;
}

//Testa se o LCD está ocupado
unsigned char TesteBusyFlag(void){
	_RW = 1;
	_RS = 0;
	DelayFor20TCY();
	_EN = 1;
	DelayFor20TCY();
	if(PORT_LCD & 0x80){
		_EN = 0;
		_RW = 0;
		return 1;
	}
	else{
		_EN = 0;
		_RW = 0;
		return 0;
	}
}

//Envia instrução para o LCD
void EscInstLCD(unsigned char _inst){
	TRIS_PORT_LCD = 0;
	PORT_LCD = _inst;
	_RS = 0;
	_RW = 0;
	Pulse();
	_RS = 0;
	DelayFor20TCY();
	TRIS_PORT_LCD = 0xFF;
}

//Inicia LCD
void IniciaLCD(void){
	const unsigned char seq_Inic[3] = {0x0F,0x06,0x01};
	unsigned char i;
	char x;
	_EN = 0;
	_RS = 0;
	_RW = 0;
	ADCON1 = 0x0F;
	TRIS_CONT_LCD = 0;
	TRIS_PORT_LCD = 0;
	
	for(i = 0; i<3 ; i++){
		PORT_LCD = 0x30;
		Pulse();
		Delay10KTCYx(1);
	}
	
	PORT_LCD = 0x038;
	Pulse();
	Delay10KTCYx(1);
	
	for(i = 0 ; i<3 ; i++){
		PORT_LCD = seq_Inic[i];
		Pulse();
		Delay10KTCYx(1);
	}
	TRIS_PORT_LCD = 0xFF;
}

//Escreve uma caracter no LCD
void EscDataLCD(char _data){
	TRIS_PORT_LCD = 0;
	PORT_LCD = _data;
	_RS = 1;
	_RW = 0;
	Pulse();
	_RS = 0;
	DelayFor20TCY();
	TRIS_PORT_LCD = 0xFF;
}

//Escreve string da memória RAM
void EscStringLCD(char *buff){
	while(*buff){
		while(TesteBusyFlag());
		 EscDataLCD(*buff);
		buff++;
	}
	return;
}

void TestPixelsLCD(void){
	unsigned char BffCheio[32];
	unsigned char i;
	EscInstLCD(0x80);
	while(TesteBusyFlag());
	
	for(i=0;i<32;i++){
		if(i<16){
			EscDataLCD(0xFF);
			while(TesteBusyFlag());
		}
		else if(i==16){
			EscInstLCD(0xC0);
			while(TesteBusyFlag());
			
			EscDataLCD(0xFF);
			while(TesteBusyFlag());
		}
		else{
			EscDataLCD(0xFF);
			while(TesteBusyFlag());
		}
	}
	
}


//Varre o teclado
char VarrerKey(void){  //Função para Varredura do teclad
					
	static unsigned char x = 0; 		
	
	TRISB = 0xf0; 						//seta linhas para saida e colunas para entrada
	
	EnablePullups();
	
//Rastrear coluna 1
	PORTB = 0xf7;						 //ativa coluna 1, desativa restante //(0b11111011)
	Nop();
	if(!l1)								//Varre a linha 1
		if(x==1) return 'G'; 			//Se 1 foi teclado, verifica se a tecla já foi tratada
		else{							//Se não
			TRISB = 0;					//Seta porta B para leitura
			x = 1;						//Seta x, informando que existe uma tecla a ser tratada
			return '1';					//Retorna o digito correspondente a tecla
		}
	if(!l2)								//A mesma lógica para as demais linhas e colunas
		if(x==1) return 'G'; 
		else{
			TRISB = 0;
			x = 1;
			return '4';
		}
	if(!l3)
		if(x==1) return 'G';
	else{
		TRISB = 0;
		x = 1;
		return '7';
	}
	if(!l4)
		if(x==1) return 'G'; 
	else{
		TRISB = 0;
		x = 1;
		return '*';
	}
	
//Rastrear coluna 2
	PORTB = 0xfb; //ativa coluna 2, desativa restante
	Nop();
	if(!l1)
		if(x==1) return 'G'; 
	else{
		TRISB = 0;
		x = 1;
		return '2';
	}
	if(!l2)
		if(x==1) return 'G'; 
	else{
		TRISB = 0;
		x = 1;
		return '5';
	}
	if(!l3)
		if(x==1) return 'G'; 
	else{
		TRISB = 0;
		x = 1;
		return '8';
	}
	if(!l4)
		if(x==1) return 'G'; 
	else{
		TRISB = 0;
		x = 1;
		return '0';
	}
	
//Rastrear coluna 3
	PORTB = 0xfd;
	Nop();
	if(!l1)
		if(x==1) return 'G'; 
	else{
		TRISB = 0;
		x = 1;
		return '3';
	}
	if(!l2)
		if(x==1) return 'G';
	else{
		TRISB = 0;
		x = 1;
		return '6';
	}
	if(!l3)
		if(x==1) return 'G';
	else{
		TRISB = 0;
		x = 1;
		return '9';
	}
	if(!l4)
		if(x==1) return 'G'; 
	else{
		TRISB = 0;
		x = 1;
		return '#';
	}
	
	TRISB = 0;
	x = 0;
	return 'G';
	
}

/*void trataKeyboad (void){  								//função para trarar as teclas do keyboard

	short x = 0;
	short i;		
		x = VarrerKey();								//chama a função varrer teclado
		if(x!='G'){										//Se x='G', Nenhuma tecla foi teclada ou ainda não foi tratada
			 if(x==-2){              					//alterna entre os modos do relogio (* = -2)
				if(modo == 0) modo = 1; 				//modo=1, mostra MM:SS
				else if(modo == 1) modo = 2;			//modo=2, mostra alarme HH:MM
				else modo = 0;							//modo=0, mostra HH:MM
			}
			else if(x==-3){								//altera o setModo
				if(setModo == 0) setModo = 1; 			//modo=1, posivel setar a hora (relogio ou alarme)
				else if(setModo == 1) setModo = 2;		//modo=2, possivel setar o minuto (relogio ou alarme)
				else setModo = 0;						//modo=0, modo set desligado
			}			
			else for(i = 0; i<10 && setModo!=0; i++){  //Tratar o teclado numerico
					if(x==i){							//Valor digitado = i?
						if(dezena == 'G') {				//dezena está vazia?
							dezena = i;					//associa o valor digitado a dezena
							i = 10;						//i > que 10, para sair do for
						}
						else {							//se não(dezena ja possui um valor)
							unidade = i;				//associa o valor digitado a unidade
							i=10;						//sai do for
						}
					}
				}
			
		}
}*/


void Inic_Regs (void){   //Função para inicializar os pino
	
	TRISA = 0x00;
	TRISB =0x00;
	TRISC =0x00;
	TRISD =0x00;
	TRISE = 0x00; //seta portas como saida
	ADCON1 = 0x0F;
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTE = 0;  //Limpa portas

}


void main(void){
	char buf[9] = {"Usuário: "};
	int dly = 0;
	char buf02[7] = {"Senha: "};
	char teste;
	short i=0;
	
	Inic_Regs ();
	IniciaLCD();
	
	EscInstLCD(0x01); //Limpa LCD e mostra cursor pisando
	while(TesteBusyFlag());  //Espera LCD terminar execução
	
	EscStringLCD(buf);
	while(TesteBusyFlag());
	
	EscInstLCD(0xC0); //Cursor para primeira posição da segunda coluna
	while(TesteBusyFlag());
	
	EscStringLCD(buf02);
	while(TesteBusyFlag());
	
	EscInstLCD(0x89);
	while(TesteBusyFlag());
	
	while(i<3){	
		teste = VarrerKey();
		if(teste != 'G') {
			EscDataLCD(teste);
			i++;
		}
	}
	
	EscInstLCD(0xC7);
	while(TesteBusyFlag());
	i=0;
	
	while(i<4){	
		teste = VarrerKey();
		if(teste != 'G') {
			EscDataLCD('*');
			i++;
		}
	}
	
	EscInstLCD(0x80); //Cursor para primeira posição da segunda coluna
	while(TesteBusyFlag());  //Espera LCD terminar execução
	
	while(1);
}	
