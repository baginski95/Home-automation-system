/*
 * odbieranie.h
 *
 *  Created on: 12 wrz 2018
 *      Author: Pi
 */

#ifndef ODBIERANIE_H_
#define ODBIERANIE_H_

 /* DEFINICJA PINU pod�aczonego do Odbiornika */
#define IR_PORT D
#define IR_PIN 6
#define IR_IN (1<<IR_PIN)
/* DEFINICJA PINU pod�aczonego do Odbiornika */


/*Przeliczenie na mikrosekundy liczby takt�w zegara dla ustawionego preskalera */
#define PRESKALER 8
#define mikro_sec(us) ((us)*(8000000/1000000)/PRESKALER) // Gdy podzielimy przez 10^3 to pewna liczba takt�w zegara
                                                       // w zale�no�ci od taktowania i preskalera
                                                       // b�dzie okre�la�a 1 mikrosekund�

/*sta�e czasowe i zmienne potrzebne do obslugi standardu RC5*/
#define TOLERANCJA 200
#define MAX_POL_BIT mikro_sec(889 + TOLERANCJA)
#define MIN_POL_BIT mikro_sec(889 - TOLERANCJA)
#define MAX_BIT mikro_sec((889+889) + TOLERANCJA)


/*Makra u�atwiaj�ce dost�p do port�w */
#define PORT(x) XPORT(x)       // sklejanie nazw oraz u�ycie operatora # do uzyskania
#define XPORT(x) (PORT##x)     // stringa z liter� odpowiedniego portu

#define PIN(x) XPIN(x)
#define XPIN(x) (PIN##x)

#define DDR(x) XDDR(x)
#define XDDR(x) (DDR##x)

/* DEKLARACJE ZMIENNYCH DLA KA�DEGO PLIKU PROGRAMU */

extern volatile uint8_t adres;
extern volatile uint8_t komenda;
extern volatile uint8_t toggle;
extern volatile uint8_t flaga_dostepu;

/* DEKLARACJE FUNKCJI */
void inicjalizacja();


#endif /* ODBIERANIE_H_ */
