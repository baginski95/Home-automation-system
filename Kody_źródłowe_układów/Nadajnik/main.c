/*
 * nadawanie.c
 *
 *  Created on: 15 wrz 2018
 *      Author: Pi
 */
#include <avr/io.h>                   //Do³¹czenie niezbêdnych przez producenta
#include <avr/interrupt.h>            //bibliotek umo¿liwi napisanie kodu wykorzystuj¹c
#include <avr/sleep.h>               // mikrokontrolera w celu wykorzystania
#include <inttypes.h>
#include <util/delay.h>
#define F_CPU 8000000UL  // Zawarcie informacji dla kompilatora o ustawionej czêstotliwoœci taktowania mikrokontrolera
/*Makra adresów nadawanych przez pilot dla 3 uk³adów odbiorczych*/
#define ADRES_1 0                 //adresy s¹ interpretowane przez uk³ady odbiorcze
#define ADRES_2 1                 //jako ci¹g szeœciu bitów. Kompilator podczas kompilacji
#define ADRES_3 2                 //zamieni zdefiniowane makro na wartoœæ bitow¹
/* Makra u³atwiaj¹ce przydzielenie kodów komendy dla poszczególnych przycisków */
#define KOMENDA_1 1
#define KOMENDA_2 2
#define KOMENDA_3 3
#define KOMENDA_4 4
#define KOMENDA_5 5
/* Makrodefinicja portu obs³uguj¹ca diodê podczerwieni IR */
#define IR_PORT B
#define IR_PIN  B
#define IR_DDR  B
#define NR_PINU  2               // Baza tranzystora zosta³a pod³¹czona poprzez rezystory do pinu 2 portu B
#define IR (1<<NR_PINU)          // Makrodefinicja u³atwiaj¹ca dostêp do pinu pod³¹czonego
                                 // pod bazê tranzystora, który steruje diod¹ IR bed¹ca obci¹¿eniem tego tranzystora
#define IR_ON  PORTB &= ~(IR)    // Stan niski na PORTB.1 za³¹cza tranzystor
#define IR_OFF PORTB |= IR       // Stan wysoki zatyka tranzystor
#define CHWILA 200
/* Definicja portu obsluguj¹cego przyciski */
#define P_PORT B                   // Zarówno przyciski jak i baza tranzystora sterujacego diod¹ podczerwieni
#define P_PIN  B                   // s¹ pod³¹czone do portu B ,jednak inne makra zosta³y u¿yte, aby program
#define P_DDR  B                   // by³ bardziej czytelny

#define P1 (1 << PB1)               // Makra uproszczaj¹ce dostêp do rejestru portu B
#define P2 (1 << PB3)               // odpowiedzialnego za stan pinów pod³¹czonych
#define P3 (1 << PB4)               // do przycisków
#define P4 (1 << PB5)
#define P5 (1 << PB6)
#define P_MASKA (P1 | P2 | P3 | P4 | P5) // Maska dla pinów dotycz¹cych obslugi przycisków
                                         // wykorzystana w programie w celu programowego wykrycia wciskanego przycisku
/*Makra u¿yte w celu przejrzystoœci kodu podczas dostêpu do portów */
#define PORT(x) XPORT(x)           // sklejanie nazw oraz u¿ycie operatora # do uzyskania
#define XPORT(x) (PORT##x)         // stringa z liter¹ odpowiedniego portu
#define PIN(x) XPIN(x)
#define XPIN(x) (PIN##x)
#define DDR(x) XDDR(x)
#define XDDR(x) (DDR##x)

uint8_t adres;                // zmienna przechowuj¹ca adres urz¹dzenia
uint8_t komenda;              // zmienna przechowuj¹ca przypisan¹ funkcjê dla uk³adu wykonawczego

/* MAIN */
int main(void)
{
	/********* INICJALIZACJA DIODY IR i LED********/

	DDR(IR_DDR)  |= (IR);      // Ustawienie PORTB: pin2 (0C0A) jako wyjœcie
	PORT(IR_PORT) |= (IR);     // Stan wysoki na pinie podl¹czonym do bazy tranzysotra PNP
	                             // zamyka przep³yw pr¹du przez tranzystor

	// INICJALIZACJA PRZYCISKÓW
    DDR(P_DDR) |= (P1 | P2 | P3 | P4 | P5);    // piny pod³¹czone do przycisków ustawione jako wyjœcia
    PORT(P_PORT) &= ~(P1 | P2 | P3 | P4 | P5);   // oraz ustawienie na nich stanu niskiego

    // Za³¹czenie diody przy pod³¹czeniu zasilania w celach diagnostycznych
    IR_ON;          // Ustawienie stanu niskiego na pinie steruj¹cym pr¹dem bazy
    _delay_ms(CHWILA); // Przez 200 ms dioda przy w³¹czeniu zasilania bêsiê œwieci³a
    IR_OFF;

    /********TIMER0 - USTAWIENIA**********************/
                                       // Timer0 Bêdzie s³u¿y³ do generowania
                                       //fali noœcnej wed³ug standardu RC(~36kHz)
   OCR0A = 110;                         // OCR0A= (F_CPU/(2*preskaler)*Fout)-1
   TCCR0A |= (1<<WGM01);               // Tryb CTC dla timera0
   TCCR0B |= (1<<CS00);                // Ustawienie preskalera timera0 na wartoœc = 1
   /**************************************************/

   /*********TIMER1 - USTAWIENIA**********************/
   //Timer1 przeznaczony jest do odmierzania opóŸnieñ w funkcji czekaj_us() (z dok³adnoœci¹ do wielekrotnoœci 1us)
   //Preskaler ustawiany na wartoœc = 8 (podczs jego za³¹czania)
   // U¿ywana jest wtedy flaga 0CF0A
   // nie u¿ywamy przerwañ
   TCCR1B |= (1<<WGM12); //Tryb CTC dla Timera1
   /***************USTAWIENIE OBNI¯ONEGO POBORU PR¥DU**********************************/
  ACSR |= (1<<ACD);                   // Ustawienia komparatora analogowego w celu obni¿enia poboru pr¹du
  WDTCSR = 0x00;                      // Wy³¹czenie tryby watch-dog
  MCUCR |= ((1<<SM0 | (1<<SM1)));   // Ustawienie tryby POWER_DOWN
  /************************************************************************************/

 // Ustawienie Pinu PD2 (INT0) jako wejœcie  (po³¹czone s¹ z nim jedn¹ koñcówk¹ przyciski)
 DDR(D) &= ~(1<<PD2);
 PORT(D) |= (1<<PD2);            // Podciagniecie pod VCC
 GIMSK |= (1<<INT0);             // zezwolenie na przerwania zewnêtrzne od pinu PD2(INTO)
 EIFR |= (1<<INTF0);             // Skasowanie flagi wyst¹pienia przerwania

 /*** GLOBALNE ZEZWOLENIE NA PRZERWANIA ***/
                  sei();                       // funkcja udostêpniona przez producenta w jednej
/**************************************/       // z bibliotek w celu u³atwienia dostêpu do rejestru
/****** PÊTLA G£ÓWNA ********/                 // zarz¹dzacaj¹cego przerwaniami mikrokontrolera
  while(1)
  {
	  sleep_mode();             // wprowadzneie procesora w tryb uœpienia w celu oszczêdnoœci energii
  }                             // oczekuj¹c na przerwanie od wciœniêtego przycisku
}
//--------------------FUNKCJE U£ATWIAJ¥CE KODOWANIE PRZY U¯YCIU STANDARDU RC5------------------------//
void czekaj_us(uint16_t usekundy)           // Dok³adna pêtla opóŸniaj¹ca wielokrotnoœci 1us
{                                           // przy pomocy TIMERA1 , taktowanie ustawione na 8Mhz, preskaler 8.
OCR1A = usekundy;                           // Dziêki tej funkcji mo¿na ³atwo w³¹czyæ diodê podczerwieni
TIFR |= (1<<OCF1A);                         // w celu emitowania czêstotliwoœci noœnej przez œciœle okreœlony czas
TCCR1B |= (1<<CS11);                        // wyra¿ony w mikrosekundach
while(!(TIFR & (1<<OCF1A))) {};
TCCR1B &= ~(1<<CS11);
}

void wyslij_1()
{
czekaj_us(889);
TCCR0A |= (1<<COM0A0); //Toggle OC0A on Compare Match (zmieñ stan pinu PB2(OC0A) przy porównaniu licznika)
czekaj_us(889);
TCCR0A &= ~(1<<COM0A0);
}

void wyslij_0()
{
	TCCR0A |= (1<<COM0A0); //Toggle OC0A on Compare Match (zmieñ stan pinu PB2(OC0A) przy porównaniu licznika)
	czekaj_us(889);
	TCCR0A &= ~(1<<COM0A0);
	czekaj_us(889);
}
/* Przes³anie ca³ej ramki w standardzie RC5 */
 void wyslij_ramke ( uint8_t adr, uint8_t cmd, uint8_t tog)
 {
   uint16_t data = 0;
   uint8_t i = 15;
   // Formowanie ramki w standardzie RC5
   // ,aby to zrealizowac przesuwamy bity do wys³ania w lew¹ stronê
   // przy pomocy dekrementacji zmiennej i

   data |= ((1<<15) | (1<<14) | (tog<<13) | (adr<<8) | (cmd<<2));

	// wysy³amy kolejno:
   //         2 bity startu ,  1 bit toggle , 5 bitów adresu , 6 bitów komendy
   do
   {
     if (!(data & (1 << i )))  wyslij_0();
     else wyslij_1();

   }while(--i >1);

 }
 /******KONIEC DEFINICJI FUNKCJI POMOCNYCH PRZY NADAWANIU SYGNA£U W STANDARDZIE RC5********/

/******************************PRZERWANIE OD PINU INT0************************************/
                  /*Wywo³ane wciœniêciem, któregoœ z przycisków*/
 ISR(INT0_vect)
 {
	 uint8_t przyciski = PIN(P_PIN);    // zmienna do której przypisany zostanie rejest PINB
	                                    // zmienna pomocnicza przy wykrywaniu wciœniêtego przycisku
	 static uint8_t toggle_bit;
	 // Przez czas trwania przerwania INT0
	 // Przestawiony zostaje kierunek portów klawiszy .
	 //PORTB ustawiony jako wejœcia  ,a pin PORTD2(INTO) jako wyjœcie ze stanem niskim
	 DDR(P_DDR) &= ~(P1 | P2 | P3 | P4 | P5);    // piny pod³¹czone do przycisków ustawione jako wejœcia
	 PORT(P_PORT) |= (P1 | P2 | P3 | P4 | P5);   // podci¹gniêcie wejœœ do Uzas

	 DDR(D) |= (1<<PD2); //PORTD2 (INT0) jako wyjœcie
	 PORT(D) &= ~(1<<PD2); // ustawienie stanu niskiego (obs³uga klawiszy)
	 if ( (przyciski & P_MASKA) != P_MASKA )  //sprawdzenie, czy wciœniêty jakikolwiek klawisz
	 {
		 _delay_ms(45);                    // pozbycie siê drgañ styków
		 przyciski = PIN(P_PIN);           // Sprawdzamy dwa razy wciœniêty przycisk
         if( (przyciski & P_MASKA) != P_MASKA )
         {
           toggle_bit ^= (1<<0); // przed wys³aniem ramki danych zmieniamy wartoœc bitu toggle na przeciwn¹

           /* przy wykorzystaniu pêtli DO_WHILE przypisujemy dynamicznie do zmiennej komenda oraz adres odpowiednie
            wartoœci przypisane do poszczególnych przycisków
            */
           do {
        	   if(!(przyciski & P1))
        		               {
        		               komenda = KOMENDA_1;
        		               adres = ADRES_1;
        		               }

        	   else if(!(przyciski & P2))
        	           		   {
        	           		   komenda = KOMENDA_2;
        	           		   adres = ADRES_2;
        	           		   }
        	   else if(!(przyciski & P3))
        	           		   {
        	           		   komenda = KOMENDA_3;
        	           		   adres = ADRES_2;
        	           		   }
        	   else if(!(przyciski & P4))
        	           	   	   {
        	           	        komenda = KOMENDA_4;
        	           	        adres = ADRES_3;
        	           	        }
        	   else if(!(przyciski & P5))
        	           	       {
        	           	        komenda = KOMENDA_5;
        	           	        adres = ADRES_3;
        	           	       }

        	   wyslij_ramke(adres, komenda, toggle_bit);
        	   _delay_ms(120); //przerwa pomiedzy przesy³anymi ramkami
        	   przyciski =  PIN(P_PIN) ; // ponowne wczytanie stanów klawiszy

           } while ( (przyciski & P_MASKA) != P_MASKA ); // warunek powtarzaj¹cy przes³anie ramki ,jeœli przycisk wci¹¿ wciœniêty
         }
	 }
   /********** Przywrócenie ustawieñ pinów po przerwaniu wywo³anym wciœniêciem przycisku**********/

    DDR(D) &= ~(1<<PD2); // pin PORTD.2 (INTO) ustawiamy jako wejscie
    PORT(D) |= (1<<PD2);  // z podci¹gniêciem do stanu wysokiego w celu oczekiwania na impuls
                          // pochodz¹cy od
	DDR(P_DDR) |= (P1 | P2 | P3 | P4 | P5);    // piny pod³¹czone do przycisków ustawione jako wyjœcia
	PORT(P_PORT) &= ~(P1 | P2 | P3 | P4 | P5); // ustawnienie na pinach stanu niskiego
	EIFR |=(1<<INTF0);                         // kasowanie flagi przerwania
 }









