/*
 * main.c
 *
 *  Created on: 3 sty 2019
 *      Author: Pi
 */
#define F_CPU 8000000UL  // Przypisane warto�ci cz�stotliwo�ci taktowania mikrokontrolera dla kompilatora
#include <avr/io.h>      // Do��czenie niezb�dnych bibliotek od prodecenta w celu napisania oprogramowania
#include <avr/interrupt.h>
#include <util/delay.h>

#include "odbieranie.h"  // do��czenie pliku nag��wkowego biblioteki, kt�ra pozwala
                         // na dekodowanie sygna��w w standardzie RC5
#define ADRES_3 2        // Adres uk�adu odbiorczego nr 3
#define KOMENDA_4 4      // Komendy przypisane do okre�lonych przycisk�w
#define KOMENDA_5 5      // KOMENDA_4 przydzielona jest do przycisku 2,
                         // KOMENDA_5 wysy�ana jest przy wci�ni�tym przycisku 3
#define T1 (1<<PB1)
#define T2 (1<<PB2)    // Ustawienie pod�aczonych do sterownika wyj�c mikrokontrolera
#define T3 (1<<PB3)
#define T4 (1<<PB4)

#define KROK_1 PORTB |= T1; PORTB &= ~(T2|T3|T4)  // Makrodefinicje wykorzystywane podczas
#define KROK_2 PORTB |= T2; PORTB &= ~(T1|T3|T4)  // podawania stan�w logicznych w celu
#define KROK_3 PORTB |= T3; PORTB &= ~(T1|T2|T4)  // w celu sterowania silnikiem
#define KROK_4 PORTB |= T4; PORTB &= ~(T1|T2|T3)

int main(void) {
    uint16_t czas = 0;
	DDRB |= (T1|T2|T3|T4);  // Ustawienie pinu PB0 portu B jako wyj�cie dla sterowania silnikiem
    PORTB &= ~(T1|T2|T3|T4);// wy��czenie wszystkich pin�w steruj�cych

	inicjalizacja();	/* inicjalizacja dekodowania IR */

	sei();	/* w��czamy globalne przerwania */

	/* p�tla niesko�czonapozwalaj�ca na ci�g�� prac� programu */
	while(1) {

		if(flaga_dostepu) {	/* je�li odebrano prawid�owe kody z pilota */

			if( (adres == ADRES_3) && (komenda == KOMENDA_4) ) {
				czas = 0;                                                // Je�li adres jest r�wny ADRES_3
				                  while(czas < 1000){                    // Za pomoc� petli whileustawienie czasu trwania dzia�ania silnika
				                   KROK_1;                               // A komenda r�wna KOMENDA_4
				            	   _delay_ms(5);                         // to steruj silnikiem w prawo
				            	   KROK_2;
				            	   _delay_ms(5);
				            	   KROK_3;
				            	   _delay_ms(5);
				            	   KROK_4;
				            	   _delay_ms(5);
                                   czas++;
				                  }

			}
			else if( (adres == ADRES_3) && (komenda == KOMENDA_5) ) {
				czas = 0;                                                     // Je�li adres jest r�wny ADRES_3
                                   while(czas < 1000){
				                   KROK_4;                                    // A komenda r�wna KOMENDA_5
         	                       _delay_ms(5);                              // to steruj silnikiem w lewo
								   KROK_3;
								   _delay_ms(5);
								   KROK_2;
								   _delay_ms(5);
								   KROK_1;
								   _delay_ms(5);
                                   czas++;
 				                  }
                      }
			}

			/* wyzerowanie flagi odbioru ramki oraz warto�ci odebranych kod�w */
			flaga_dostepu=0;             //Wyzerowanie warto�ci pozwoli na odbi�r kolejnej ramki danych
			komenda=0xff;                // z nowym zestawem bit�w ramiki danych
			adres=0xff;
		}

	}
