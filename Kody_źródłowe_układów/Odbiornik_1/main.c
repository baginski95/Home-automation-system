/*
 * main.c
 *
 *  Created on: 3 sty 2019
 *      Author: Pi
 */
#define F_CPU 8000000UL  // Przypisane wartoœci czêstotliwoœci taktowania mikrokontrolera dla kompilatora
#include <avr/io.h>      // Do³¹czenie niezbêdnych bibliotek od prodecenta w celu napisania oprogramowania
#include <avr/interrupt.h>
#include <util/delay.h>

#include "odbieranie.h"  // do³¹czenie pliku nag³ówkowego biblioteki, która pozwala
                         // na dekodowanie sygna³ów w standardzie RC5

#define ADRES_1 0        // Adres uk³adu odbiorczego nr 2
#define KOMENDA_1 1      // Komendy przypisane do okreœlonych przycisków
                      // KOMENDA_2 przydzielona jest do przycisku 2,
                         // KOMENDA_3 wysy³ana jest przy wciœniêtym przycisku 3

int main(void) {

	DDRB |= (1<<PB0);  // Ustawienie pinu PB0 portu B jako wyjœcie
	PORTB |= (1<<PB0); // Podanie stanu wysokiego na pin PB0 przez okres 1,5 sekundy
	_delay_ms(1500);   // w celu sprawdzenia dzia³ania kodu programu
	PORTB &= ~(1<<PB0);

	inicjalizacja();	/* inicjalizacja dekodowania IR */

	sei();	/* w³¹czamy globalne przerwania */



	/* pêtla nieskoñczonapozwalaj¹ca na ci¹g³¹ pracê programu */
	while(1) {

		if(flaga_dostepu) {	/* jeœli odebrano prawid³owe kody z pilota */

			if( (adres == ADRES_1) && (komenda == KOMENDA_1) ) {              // Jeœli adres jest równy ADRES_2
				PORTB ^= (1<<PB0);                                            // to zmieñ stan na pinie PORTB.0

			}


			/* wyzerowanie flagi odbioru ramki oraz wartoœci odebranych kodów */
			flaga_dostepu=0;             //Wyzerowanie wartoœci pozwoli na odbiór kolejnej ramki danych
			komenda=0xff;                // z nowym zestawem bitów ramiki danych
			adres=0xff;
		}

	}
}

