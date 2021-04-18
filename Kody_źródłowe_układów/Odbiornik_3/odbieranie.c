/*
 * odbieranie.c
 *
 *  Created on: 11 wrz 2018
 *      Author: Pi
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#include "odbieranie.h"

 //  Zmienne przechowuj�ce poszczeg�lne bity odbieranej ramki

volatile uint8_t adres;     // zawiera 5 bit�w adresu urz�dzenia
volatile uint8_t komenda;  // zawiera 6 bit�w definiuj�cych rozkaz
volatile uint8_t toggle;  // 1 bit pomocniczy

volatile uint8_t flaga_dostepu;   // je�li == 1 informuje o odebranym nowym kodzie
                                  // po odczytaniu ramki nale�y wyzerowac

volatile uint8_t licznik_rc5;   // Licznik wyst�pujacych zboczy

/* Funkcja inicjalizuj�ca odpowiedni PORT oraz Timer */

void inicjalizacja()
{
	DDR(IR_PORT) &= ~(IR_IN);   // Ustawienie pinu jako wej�cie
	PORT(IR_PORT) |= IR_IN;     // Podciagni�cie pinu do Vcc

	/*Procedura ustawiajaca preskaler Timera */
#if PRESKALER == 1
	TCCR1B |= (1<<CS10);
#endif
#if PRESKALER == 8
	TCCR1B |= (1<<CS11);
#endif
#if PRESKALER == 64
	TCCR1B |= (1<<CS10) | (1<<CS11);;
#endif
#if PRESKALER == 256
	TCCR1B |= (1<<CS12);
#endif
  /* KONIEC PROCEDURY */

TCCR1B &= ~(1<<ICES1); // Reakcja na zbocze opadajace od pinu ICP1 (IR_IN)
licznik_rc5 = 0;       // Wyzerowanie licznika zboczy
TIMSK |= (1<<TICIE1); // Za��czenie przerwania od
flaga_dostepu = 0; // Zerowanie flagi otrzymania kodu
}
 /* PROCEDURA OBS�UGUJ�CE PRZERWANIE POCHODZ�CE OD PINU IPC1 */  // Przerwanie reaguje na zbocza wystepuj�ce na pinie ICP1
ISR(TIMER1_CAPT_vect)
{

#define RESTART 0
#define DALEJ   1
#define KONIEC  2
#define BLAD    3


/* Deklaracja zmiennych statycznych u�ywanych w przerwaniu */
       uint16_t Szerokosc_Impulsu; //Obliczanie czasu ka�dej po��wki
static uint16_t Poprzedni_Impuls;  // pomocnicza zmienna przy obliczaniu Szeroko�ci_impulsu
static uint16_t IR_dane;           //  16 bitowa zmienna przechwuj�ca adres
static uint8_t IR_impulsy;
static uint8_t Status_Ramki;

Szerokosc_Impulsu = ICR1 - Poprzedni_Impuls; // Pobranie czasu trwania impulsu
Poprzedni_Impuls = ICR1;

TCCR1B ^= (1<<ICES1); // Zmiana zbocza wyzwalaj�cego na przeciwne ,aby m�c wywo�ac kolejne przerwanie zboczem przeciwnym do poprzedzaj�cego


if (Szerokosc_Impulsu > MAX_BIT) licznik_rc5=0;

if (licznik_rc5 > 0) Status_Ramki = DALEJ;

if (licznik_rc5 == 0) //Pocz�tek ramki
 {
	IR_dane = 0;
	IR_impulsy = 0;
	TCCR1B |= (1<<ICES1);
	licznik_rc5++;
	Status_Ramki = KONIEC;
 }


if (Status_Ramki == DALEJ)
   {
	if (Szerokosc_Impulsu < MIN_POL_BIT ) Status_Ramki = RESTART; // W przypadku zak��ce� b�d� b��du odbioru ramki

	if (Szerokosc_Impulsu > MAX_BIT ) Status_Ramki = RESTART;

	if(Status_Ramki == DALEJ)
	 {
		if(Szerokosc_Impulsu > MAX_POL_BIT) licznik_rc5++;
		if(licznik_rc5 > 1)
		if( (licznik_rc5 % 2) == 0)
		     {
			   IR_dane = IR_dane << 1 ;
			   if((TCCR1B & (1<<ICES1))) IR_dane |= 0x0001;
		       IR_impulsy++;


		       if(IR_impulsy > 12)
		        {
		    	 if (flaga_dostepu == 0)
		    	  {
		    	   komenda = IR_dane & 0b0000000000111111;
		    	   adres = (IR_dane & 0b0000011111000000) >> 6;
		           toggle = (IR_dane & 0b0000100000000000) >> 11;
		    	}
		    		Status_Ramki = RESTART;
		    		flaga_dostepu = 1;
		     }
		     }
		   licznik_rc5++;
		}
	 }
		   if (Status_Ramki == RESTART)
		   {
			   licznik_rc5 = 0;
			   TCCR1B &= ~(1<<ICES1);
		   }
   }



