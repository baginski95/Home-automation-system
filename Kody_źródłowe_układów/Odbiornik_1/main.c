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

#define ADRES_1 0        // Adres uk�adu odbiorczego nr 2
#define KOMENDA_1 1      // Komendy przypisane do okre�lonych przycisk�w
                      // KOMENDA_2 przydzielona jest do przycisku 2,
                         // KOMENDA_3 wysy�ana jest przy wci�ni�tym przycisku 3

int main(void) {

	DDRB |= (1<<PB0);  // Ustawienie pinu PB0 portu B jako wyj�cie
	PORTB |= (1<<PB0); // Podanie stanu wysokiego na pin PB0 przez okres 1,5 sekundy
	_delay_ms(1500);   // w celu sprawdzenia dzia�ania kodu programu
	PORTB &= ~(1<<PB0);

	inicjalizacja();	/* inicjalizacja dekodowania IR */

	sei();	/* w��czamy globalne przerwania */



	/* p�tla niesko�czonapozwalaj�ca na ci�g�� prac� programu */
	while(1) {

		if(flaga_dostepu) {	/* je�li odebrano prawid�owe kody z pilota */

			if( (adres == ADRES_1) && (komenda == KOMENDA_1) ) {              // Je�li adres jest r�wny ADRES_2
				PORTB ^= (1<<PB0);                                            // to zmie� stan na pinie PORTB.0

			}


			/* wyzerowanie flagi odbioru ramki oraz warto�ci odebranych kod�w */
			flaga_dostepu=0;             //Wyzerowanie warto�ci pozwoli na odbi�r kolejnej ramki danych
			komenda=0xff;                // z nowym zestawem bit�w ramiki danych
			adres=0xff;
		}

	}
}

