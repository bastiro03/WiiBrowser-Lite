#include "html.h"

/*************************************************************
                        PROTOTIPI
*************************************************************/

ListaDiElem Inizializza( void );
int ListaVuota( ListaDiElem lista );
ListaDiElem Inserisci( ListaDiElem lista, TipoElemento elem );
ListaDiElem InvertiRiusandoRic( ListaDiElem lista );

int DimensioneRic( ListaDiElem lista );
void DistruggiListaRic( ListaDiElem lista );

ListaDiTesto InitText( void );
int NoText( ListaDiTesto lista );
ListaDiTesto InsText( ListaDiTesto lista );
void DistruggiText( ListaDiTesto lista );

ListaDiBottoni InitButton( void );
int NoButton( ListaDiBottoni lista );
ListaDiBottoni InsButton( ListaDiBottoni lista );
void DistruggiButton( ListaDiBottoni lista );

ListaDiInput InitInput( void );
int NoInput( ListaDiInput lista );
ListaDiInput InsInFondo(ListaDiInput lista, TipoElemento name, TipoElemento type, TipoElemento value, TipoElemento label );
void DistruggiInput( ListaDiInput lista );

ListaDiImg InitImg( void );
int NoImg( ListaDiImg lista );
ListaDiImg InsImg( ListaDiImg lista );
void DistruggiImg( ListaDiImg lista );

Indice InitIndex( void );
int NoIndex( Indice lista );
Indice InsIndex( Indice lista );
void DistruggiIndex( Indice lista );

History InitHistory( void );
int NoUrls( History lista );
History InsUrl( History lista, char *url );

void DistruggiHistory( History lista, int prox );
void FreeHistory( History lista );
