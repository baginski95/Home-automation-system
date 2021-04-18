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
#define ADRES_3 2        // Adres uk³adu odbiorczego nr 3
#define KOMENDA_4 4      // Komendy przypisane do okreœlonych przycisków
#define KOMENDA_5 5      // KOMENDA_4 przydzielona jest do przycisku 2,
                         // KOMENDA_5 wysy³ana jest przy wciœniêtym przycisku 3
#define T1 (1<<PB1)
#define T2 (1<<PB2)    // Ustawienie pod³aczonych do sterownika wyjœc mikrokontrolera
#define T3 (1<<PB3)
#define T4 (1<<PB4)

#define KROK_1 PORTB |= T1; PORTB &= ~(T2|T3|T4)  // Makrodefinicje wykorzystywane podczas
#define KROK_2 PORTB |= T2; PORTB &= ~(T1|T3|T4)  // podawania stanów logicznych w celu
#define KROK_3 PORTB |= T3; PORTB &= ~(T1|T2|T4)  // w celu sterowania silnikiem
#define KROK_4 PORTB |= T4; PORTB &= ~(T1|T2|T3)

int main(void) {
    uint16_t czas = 0;
	DDRB |= (T1|T2|T3|T4);  // Ustawienie pinu PB0 portu B jako wyjœcie dla sterowania silnikiem
    PORTB &= ~(T1|T2|T3|T4);// wy³¹czenie wszystkich pinów steruj¹cych

	inicjalizacja();	/* inicjalizacja dekodowania IR */

	sei();	/* w³¹czamy globalne przerwania */

	/* pêtla nieskoñczonapozwalaj¹ca na ci¹g³¹ pracê programu */
	while(1) {

		if(flaga_dostepu) {	/* jeœli odebrano prawid³owe kody z pilota */

			if( (adres == ADRES_3) && (komenda == KOMENDA_4) ) {
				czas = 0;                                                // Jeœli adres jest równy ADRES_3
				                  while(czas < 1000){                    // Za pomoc¹ petli whileustawienie czasu trwania dzia³ania silnika
				                   KROK_1;                               // A komenda równa KOMENDA_4
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
				czas = 0;                                                     // Jeœli adres jest równy ADRES_3
                                   while(czas < 1000){
				                   KROK_4;                                    // A komenda równa KOMENDA_5
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

			/* wyzerowanie flagi odbioru ramki oraz wartoœci odebranych kodów */
			flaga_dostepu=0;             //Wyzerowanie wartoœci pozwoli na odbiór kolejnej ramki danych
			komenda=0xff;                // z nowym zestawem bitów ramiki danych
			adres=0xff;
		}

	}
