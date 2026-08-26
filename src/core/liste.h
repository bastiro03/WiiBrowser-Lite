#ifndef _LISTE_H_
#define _LISTE_H_

#include "common.h"

/*************************************************************
                        PROTOTIPI
*************************************************************/

ListaDiTesto InitText( void );
int NoText( ListaDiTesto lista );
ListaDiTesto InsText( ListaDiTesto lista );
void DistruggiText( ListaDiTesto lista );

ListaDiBottoni InitButton( void );
int NoButton( ListaDiBottoni lista );
ListaDiBottoni InsButton( ListaDiBottoni lista, int outline = 0 );
void DistruggiButton( ListaDiBottoni lista );

ListaDiInput InitInput( void );
int NoInput( ListaDiInput lista );
ListaDiInput InsInFondo(ListaDiInput lista, TipoElemento name, TipoElemento type, TipoElemento value, TipoElemento label );
void SetOption( ListaDiInput element, TipoElemento value );
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
void DumpList ( History lista, const char * file = NULL );
History LoadList ( const char * file = NULL );

#endif
