/*
 * nadawanie.c
 *
 *  Created on: 15 wrz 2018
 *      Author: Pi
 */
#include <avr/io.h>                   //Do��czenie niezb�dnych przez producenta
#include <avr/interrupt.h>            //bibliotek umo�liwi napisanie kodu wykorzystuj�c
#include <avr/sleep.h>               // mikrokontrolera w celu wykorzystania
#include <inttypes.h>
#include <util/delay.h>
#define F_CPU 8000000UL  // Zawarcie informacji dla kompilatora o ustawionej cz�stotliwo�ci taktowania mikrokontrolera
/*Makra adres�w nadawanych przez pilot dla 3 uk�ad�w odbiorczych*/
#define ADRES_1 0                 //adresy s� interpretowane przez uk�ady odbiorcze
#define ADRES_2 1                 //jako ci�g sze�ciu bit�w. Kompilator podczas kompilacji
#define ADRES_3 2                 //zamieni zdefiniowane makro na warto�� bitow�
/* Makra u�atwiaj�ce przydzielenie kod�w komendy dla poszczeg�lnych przycisk�w */
#define KOMENDA_1 1
#define KOMENDA_2 2
#define KOMENDA_3 3
#define KOMENDA_4 4
#define KOMENDA_5 5
/* Makrodefinicja portu obs�uguj�ca diod� podczerwieni IR */
#define IR_PORT B
#define IR_PIN  B
#define IR_DDR  B
#define NR_PINU  2               // Baza tranzystora zosta�a pod��czona poprzez rezystory do pinu 2 portu B
#define IR (1<<NR_PINU)          // Makrodefinicja u�atwiaj�ca dost�p do pinu pod��czonego
                                 // pod baz� tranzystora, kt�ry steruje diod� IR bed�ca obci��eniem tego tranzystora
#define IR_ON  PORTB &= ~(IR)    // Stan niski na PORTB.1 za��cza tranzystor
#define IR_OFF PORTB |= IR       // Stan wysoki zatyka tranzystor
#define CHWILA 200
/* Definicja portu obsluguj�cego przyciski */
#define P_PORT B                   // Zar�wno przyciski jak i baza tranzystora sterujacego diod� podczerwieni
#define P_PIN  B                   // s� pod��czone do portu B ,jednak inne makra zosta�y u�yte, aby program
#define P_DDR  B                   // by� bardziej czytelny

#define P1 (1 << PB1)               // Makra uproszczaj�ce dost�p do rejestru portu B
#define P2 (1 << PB3)               // odpowiedzialnego za stan pin�w pod��czonych
#define P3 (1 << PB4)               // do przycisk�w
#define P4 (1 << PB5)
#define P5 (1 << PB6)
#define P_MASKA (P1 | P2 | P3 | P4 | P5) // Maska dla pin�w dotycz�cych obslugi przycisk�w
                                         // wykorzystana w programie w celu programowego wykrycia wciskanego przycisku
/*Makra u�yte w celu przejrzysto�ci kodu podczas dost�pu do port�w */
#define PORT(x) XPORT(x)           // sklejanie nazw oraz u�ycie operatora # do uzyskania
#define XPORT(x) (PORT##x)         // stringa z liter� odpowiedniego portu
#define PIN(x) XPIN(x)
#define XPIN(x) (PIN##x)
#define DDR(x) XDDR(x)
#define XDDR(x) (DDR##x)

uint8_t adres;                // zmienna przechowuj�ca adres urz�dzenia
uint8_t komenda;              // zmienna przechowuj�ca przypisan� funkcj� dla uk�adu wykonawczego

/* MAIN */
int main(void)
{
	/********* INICJALIZACJA DIODY IR i LED********/

	DDR(IR_DDR)  |= (IR);      // Ustawienie PORTB: pin2 (0C0A) jako wyj�cie
	PORT(IR_PORT) |= (IR);     // Stan wysoki na pinie podl�czonym do bazy tranzysotra PNP
	                             // zamyka przep�yw pr�du przez tranzystor

	// INICJALIZACJA PRZYCISK�W
    DDR(P_DDR) |= (P1 | P2 | P3 | P4 | P5);    // piny pod��czone do przycisk�w ustawione jako wyj�cia
    PORT(P_PORT) &= ~(P1 | P2 | P3 | P4 | P5);   // oraz ustawienie na nich stanu niskiego

    // Za��czenie diody przy pod��czeniu zasilania w celach diagnostycznych
    IR_ON;          // Ustawienie stanu niskiego na pinie steruj�cym pr�dem bazy
    _delay_ms(CHWILA); // Przez 200 ms dioda przy w��czeniu zasilania b�si� �wieci�a
    IR_OFF;

    /********TIMER0 - USTAWIENIA**********************/
                                       // Timer0 B�dzie s�u�y� do generowania
                                       //fali no�cnej wed�ug standardu RC(~36kHz)
   OCR0A = 110;                         // OCR0A= (F_CPU/(2*preskaler)*Fout)-1
   TCCR0A |= (1<<WGM01);               // Tryb CTC dla timera0
   TCCR0B |= (1<<CS00);                // Ustawienie preskalera timera0 na warto�c = 1
   /**************************************************/

   /*********TIMER1 - USTAWIENIA**********************/
   //Timer1 przeznaczony jest do odmierzania op�nie� w funkcji czekaj_us() (z dok�adno�ci� do wielekrotno�ci 1us)
   //Preskaler ustawiany na warto�c = 8 (podczs jego za��czania)
   // U�ywana jest wtedy flaga 0CF0A
   // nie u�ywamy przerwa�
   TCCR1B |= (1<<WGM12); //Tryb CTC dla Timera1
   /***************USTAWIENIE OBNI�ONEGO POBORU PR�DU**********************************/
  ACSR |= (1<<ACD);                   // Ustawienia komparatora analogowego w celu obni�enia poboru pr�du
  WDTCSR = 0x00;                      // Wy��czenie tryby watch-dog
  MCUCR |= ((1<<SM0 | (1<<SM1)));   // Ustawienie tryby POWER_DOWN
  /************************************************************************************/

 // Ustawienie Pinu PD2 (INT0) jako wej�cie  (po��czone s� z nim jedn� ko�c�wk� przyciski)
 DDR(D) &= ~(1<<PD2);
 PORT(D) |= (1<<PD2);            // Podciagniecie pod VCC
 GIMSK |= (1<<INT0);             // zezwolenie na przerwania zewn�trzne od pinu PD2(INTO)
 EIFR |= (1<<INTF0);             // Skasowanie flagi wyst�pienia przerwania

 /*** GLOBALNE ZEZWOLENIE NA PRZERWANIA ***/
                  sei();                       // funkcja udost�pniona przez producenta w jednej
/**************************************/       // z bibliotek w celu u�atwienia dost�pu do rejestru
/****** P�TLA G��WNA ********/                 // zarz�dzacaj�cego przerwaniami mikrokontrolera
  while(1)
  {
	  sleep_mode();             // wprowadzneie procesora w tryb u�pienia w celu oszcz�dno�ci energii
  }                             // oczekuj�c na przerwanie od wci�ni�tego przycisku
}
//--------------------FUNKCJE U�ATWIAJ�CE KODOWANIE PRZY U�YCIU STANDARDU RC5------------------------//
void czekaj_us(uint16_t usekundy)           // Dok�adna p�tla op�niaj�ca wielokrotno�ci 1us
{                                           // przy pomocy TIMERA1 , taktowanie ustawione na 8Mhz, preskaler 8.
OCR1A = usekundy;                           // Dzi�ki tej funkcji mo�na �atwo w��czy� diod� podczerwieni
TIFR |= (1<<OCF1A);                         // w celu emitowania cz�stotliwo�ci no�nej przez �ci�le okre�lony czas
TCCR1B |= (1<<CS11);                        // wyra�ony w mikrosekundach
while(!(TIFR & (1<<OCF1A))) {};
TCCR1B &= ~(1<<CS11);
}

void wyslij_1()
{
czekaj_us(889);
TCCR0A |= (1<<COM0A0); //Toggle OC0A on Compare Match (zmie� stan pinu PB2(OC0A) przy por�wnaniu licznika)
czekaj_us(889);
TCCR0A &= ~(1<<COM0A0);
}

void wyslij_0()
{
	TCCR0A |= (1<<COM0A0); //Toggle OC0A on Compare Match (zmie� stan pinu PB2(OC0A) przy por�wnaniu licznika)
	czekaj_us(889);
	TCCR0A &= ~(1<<COM0A0);
	czekaj_us(889);
}
/* Przes�anie ca�ej ramki w standardzie RC5 */
 void wyslij_ramke ( uint8_t adr, uint8_t cmd, uint8_t tog)
 {
   uint16_t data = 0;
   uint8_t i = 15;
   // Formowanie ramki w standardzie RC5
   // ,aby to zrealizowac przesuwamy bity do wys�ania w lew� stron�
   // przy pomocy dekrementacji zmiennej i

   data |= ((1<<15) | (1<<14) | (tog<<13) | (adr<<8) | (cmd<<2));

	// wysy�amy kolejno:
   //         2 bity startu ,  1 bit toggle , 5 bit�w adresu , 6 bit�w komendy
   do
   {
     if (!(data & (1 << i )))  wyslij_0();
     else wyslij_1();

   }while(--i >1);

 }
 /******KONIEC DEFINICJI FUNKCJI POMOCNYCH PRZY NADAWANIU SYGNA�U W STANDARDZIE RC5********/

/******************************PRZERWANIE OD PINU INT0************************************/
                  /*Wywo�ane wci�ni�ciem, kt�rego� z przycisk�w*/
 ISR(INT0_vect)
 {
	 uint8_t przyciski = PIN(P_PIN);    // zmienna do kt�rej przypisany zostanie rejest PINB
	                                    // zmienna pomocnicza przy wykrywaniu wci�ni�tego przycisku
	 static uint8_t toggle_bit;
	 // Przez czas trwania przerwania INT0
	 // Przestawiony zostaje kierunek port�w klawiszy .
	 //PORTB ustawiony jako wej�cia  ,a pin PORTD2(INTO) jako wyj�cie ze stanem niskim
	 DDR(P_DDR) &= ~(P1 | P2 | P3 | P4 | P5);    // piny pod��czone do przycisk�w ustawione jako wej�cia
	 PORT(P_PORT) |= (P1 | P2 | P3 | P4 | P5);   // podci�gni�cie wej�� do Uzas

	 DDR(D) |= (1<<PD2); //PORTD2 (INT0) jako wyj�cie
	 PORT(D) &= ~(1<<PD2); // ustawienie stanu niskiego (obs�uga klawiszy)
	 if ( (przyciski & P_MASKA) != P_MASKA )  //sprawdzenie, czy wci�ni�ty jakikolwiek klawisz
	 {
		 _delay_ms(45);                    // pozbycie si� drga� styk�w
		 przyciski = PIN(P_PIN);           // Sprawdzamy dwa razy wci�ni�ty przycisk
         if( (przyciski & P_MASKA) != P_MASKA )
         {
           toggle_bit ^= (1<<0); // przed wys�aniem ramki danych zmieniamy warto�c bitu toggle na przeciwn�

           /* przy wykorzystaniu p�tli DO_WHILE przypisujemy dynamicznie do zmiennej komenda oraz adres odpowiednie
            warto�ci przypisane do poszczeg�lnych przycisk�w
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
        	   _delay_ms(120); //przerwa pomiedzy przesy�anymi ramkami
        	   przyciski =  PIN(P_PIN) ; // ponowne wczytanie stan�w klawiszy

           } while ( (przyciski & P_MASKA) != P_MASKA ); // warunek powtarzaj�cy przes�anie ramki ,je�li przycisk wci�� wci�ni�ty
         }
	 }
   /********** Przywr�cenie ustawie� pin�w po przerwaniu wywo�anym wci�ni�ciem przycisku**********/

    DDR(D) &= ~(1<<PD2); // pin PORTD.2 (INTO) ustawiamy jako wejscie
    PORT(D) |= (1<<PD2);  // z podci�gni�ciem do stanu wysokiego w celu oczekiwania na impuls
                          // pochodz�cy od
	DDR(P_DDR) |= (P1 | P2 | P3 | P4 | P5);    // piny pod��czone do przycisk�w ustawione jako wyj�cia
	PORT(P_PORT) &= ~(P1 | P2 | P3 | P4 | P5); // ustawnienie na pinach stanu niskiego
	EIFR |=(1<<INTF0);                         // kasowanie flagi przerwania
 }









