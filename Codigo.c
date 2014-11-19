#include <p18f4550.h>
#include <string.h>
#include <stdio.h>
#include <delays.h>
#include <stdlib.h>

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
typedef struct usuario Usuario;

struct usuario{
	char id[3];
	char senha[5];
};

short numeroUser = 1;
Usuario vUser[5] = {{"99","9999"},
					{"11","0000"},
					{"ff","ffff"},
					{"ff","ffff"},
					{"ff","ffff"}};

void DelayFor20TCY(void){
	Nop();Nop();Nop();Nop();Nop();
	Nop();Nop();Nop();Nop();Nop();
	Nop();Nop();Nop();Nop();Nop();
}

void DelayFor500ms(void){
	int i;
	for(i=0;i<100;i++){
		Delay10KTCYx(1);
	}
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
	const unsigned char seq_Inic[7] = {0x30,0x30,0x30,0x038,0x0F,0x06,0x01};
	unsigned char i;
	char x;
	_EN = 0;
	_RS = 0;
	_RW = 0;
	ADCON1 = 0x0F;
	TRIS_CONT_LCD = 0;
	TRIS_PORT_LCD = 0;
	
	for(i = 0 ; i<7 ; i++){
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
	while(TesteBusyFlag());
	return;
}

//Escreve string da memória ROM
void EscStringLCD_ROM(const rom char *buff){
	while(*buff){
		while(TesteBusyFlag());
		 EscDataLCD(*buff);
		buff++;
	}
	while(TesteBusyFlag());
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
char VarrerKey(void){  //Função para Varredura do tecla
					
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
//Iniciar Pinos
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

//Confirgurar saida serial
void configura_UART(void){
	
	TRISCbits.TRISC7 = 1;
	TRISCbits.TRISC7 = 1;
	TXSTA = 0b00100100;
	RCSTA = 0b10010000;
	BAUDCON = 0b00000000;
	SPBRG = 51;
}

//Transmite um caractere pela serial
void transmiteCaracter(char dado){
	
	TXREG = dado;
	while(TXSTAbits.TRMT);
	
}

void transmiteString (const rom char *buff){
	while(*buff){
		while(!PIR1bits.TXIF);
		TXREG = *buff;
		buff++;
	}

}

void transmiteStringRAM (char *buff){
	while(*buff){
		while(!PIR1bits.TXIF);
		TXREG = *buff;
		buff++;
	}

}

//Limpa e escreve na primeira linha
void printLimpo(const rom char *buff){
	
	EscInstLCD(0x01); //Limpa LCD e mostra cursor piscando na primeira linha
	while(TesteBusyFlag());
	
	while(*buff){
		while(TesteBusyFlag());
		EscDataLCD(*buff);
		buff++;
	}
	while(TesteBusyFlag());
	return;
}

//Escreve na segunda linha
void print2Linha(const rom char *buff){
	
	EscInstLCD(0xC0); //cursor para segunda linha
	while(TesteBusyFlag());
	
	while(*buff){
		while(TesteBusyFlag());
		EscDataLCD(*buff);
		buff++;
	}
	while(TesteBusyFlag());
	return;
}

//Exclui Usuario
void excluirUsuario(void){
	char idUser[3];
	char digitado;
	char confirme;
	int i = 0;
	int local = 1;
	
	printLimpo("Excluir Usuario ");
	print2Linha("ID: ");
	
	while(i<2){	
		digitado = VarrerKey();
		if(digitado != 'G') {
			idUser[i] = digitado;
			EscDataLCD(digitado);
			i++;
		}
	}
	idUser[3] = '\0';
	
	printLimpo("Excluir ID: ");
	EscStringLCD(idUser);
	print2Linha("  1-Sim 2-Nao   ");
	
	while(1){	
		confirme = VarrerKey();
		if(confirme == '1' || confirme == '2') {
			break;
		}
	}
	
	if(confirme == '2'){
		printLimpo("    Operacao");
		print2Linha("   Cancelada!   ");
		DelayFor500ms();
	}
	else{
		while(local<5){
			if((idUser[0] == vUser[local].id[0]) && (idUser[1] == vUser[local].id[1])){
				break;
			}
			local++;
		}
		vUser[local].id[0] = 'f';
		vUser[local].id[1] = 'f';
		printLimpo("Usuario: ");
		EscStringLCD(idUser);
		print2Linha("   Excluido!   ");
		DelayFor500ms();
		return;		
	}
}

//Cadastra novo usuario
void cadastrar(void){
	Usuario userTemp;
	short aux,i;
	char numero[11] ={"0123456789"};
	char digitado;
	char senha[5];
	
//	userTemp = malloc (sizeof (Usuario));

	for(i=1;(vUser[i].id[0] != 'f') && i<5 ; i++){}
	if(i >= 5){
		printLimpo("     Maximo     ");
		print2Linha("    Usuarios    ");
		DelayFor500ms();
		return;
	}
	
	aux = numeroUser/10;
	userTemp.id[0] = numero[aux];
	aux = numeroUser%10;
	userTemp.id[1] = numero[aux];
	userTemp.id[2] = '\0';
	aux = 0;
	
	do{
		printLimpo("Novo ID: ");
		EscStringLCD(userTemp.id);
		print2Linha("Senha: ");
	
		for(i=0;i<4;){	
			digitado = VarrerKey();
			if(digitado != 'G') {
				userTemp.senha[i] = digitado;
				EscDataLCD('*');
				i++;
			}
		}
		
		printLimpo("Confirmar");
		print2Linha("Senha: ");
		for(i=0;i<4;){	
			digitado = VarrerKey();
			if(digitado != 'G') {
				senha[i] = digitado;
				EscDataLCD('*');
				i++;
			}
		}
		
		if((userTemp.senha[0] != senha[0]) || (userTemp.senha[1] != senha[1]) ||
		(userTemp.senha[2] != senha[3]) ||(userTemp.senha[3] != senha[3])){
				printLimpo("   Senhas nao   ");
				print2Linha("    Conferem    ");
				aux++;
				DelayFor500ms();
		}
		else {
			printLimpo("Usuario Criado");
			print2Linha("  com Sucesso   ");
			
			for(i=1;(vUser[i].id[0] != 'f') && i<5 ; i++){}
			vUser[i] = userTemp;
			numeroUser++;
			DelayFor500ms();
			break;
		}
	
	}while(aux<3);
	
	if(aux >= 3){
		printLimpo("Tente Novamente ");
		print2Linha("    Depois      ");
		DelayFor500ms();
		return;
	}
	
}

//Menu Administrador
void adminMenu (void){
	char confirme;
	
	printLimpo("1-Novo 2-Excluir");
	print2Linha("3-Sair          ");
	
	while(1){	
		confirme = VarrerKey();
		if(confirme == '1' || confirme == '2' || confirme == '3') {
			break;
		}
	}
	
	if(confirme == '1'){
		cadastrar();
	}else if(confirme == '2'){
		excluirUsuario();
	}else{
		return;
	}
	
}

//Recebe usuario e senha e valida entrada
void receberSenha(void){
	int i,local;
	char digitado;
	Usuario userTemp;

	printLimpo("Usuario: ");
	print2Linha("Senha: ");
	
	EscInstLCD(0x89); //Coloca cursor apos a palavra usuario
	while(TesteBusyFlag());
	i = 0;
	while(i<2){	
		digitado = VarrerKey();
		if(digitado != 'G') {
			userTemp.id[i] = digitado;
			EscDataLCD(digitado);
			i++;
		}
	}
	local = 0;
	while(local<5){
		if((userTemp.id[0] == vUser[local].id[0]) && (userTemp.id[1] == vUser[local].id[1])){
			break;
		}
		local++;
	}
	
	if((userTemp.id[0] != vUser[local].id[0]) || (userTemp.id[1] != vUser[local].id[1])){

		printLimpo("Usuario Invalido");
		DelayFor500ms();
		return;
	}
	
	EscInstLCD(0xC7);
	while(TesteBusyFlag());
	i=0;

	while(i<4){	
		digitado = VarrerKey();
		if(digitado != 'G') {
			userTemp.senha[i] = digitado;
			EscDataLCD('*');
			i++;
		}
	}
		
	if((userTemp.senha[0] != vUser[local].senha[0]) || (userTemp.senha[1] != vUser[local].senha[1]) || 
		(userTemp.senha[2] != vUser[local].senha[2]) || (userTemp.senha[3] != vUser[local].senha[3])){
		printLimpo(" Senha Invalida");
		DelayFor500ms();
			return;
	}
	else {
		
		if((userTemp.id[0] == '9') && (userTemp.id[1] == '9')){
			adminMenu();
			return;
		}
		else {
			userTemp.id[3] = '\0';
			printLimpo(" Aesso Liberado ");
			transmiteString("Usuario: ");
			transmiteStringRAM(userTemp.id);
			transmiteString(", Acesso Liberado.\n");
			
		}
		DelayFor500ms();
		return;
	}
}

void main(void){
	
	Inic_Regs ();
	IniciaLCD();
	configura_UART();
	
	while(1){
		receberSenha();
	}
}