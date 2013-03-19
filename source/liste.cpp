#include "liste.h"

/*************************************************************
                     IMPLEMENTAZIONE
*************************************************************/

//Text
ListaDiTesto InitText( void ) {
	return NULL;
}

int NoText( ListaDiTesto lista ) {
    return lista == NULL;
}

ListaDiTesto InsText( ListaDiTesto lista ) {
	ListaDiTesto punt;
    punt=new Text;
    punt->prox = lista;
	return  punt;
}

void DistruggiText( ListaDiTesto lista ) {
    if (NoText(lista)) return;
    DistruggiText(lista->prox);
    delete (lista->txt);
    delete (lista);
}

//Button
ListaDiBottoni InitButton( void ) {
	return NULL;
}

int NoButton( ListaDiBottoni lista ) {
    return lista == NULL;
}

ListaDiBottoni InsButton( ListaDiBottoni lista ) {
	ListaDiBottoni punt;
    punt=new Button;
    punt->refs=NULL;
    punt->label=NULL;
    punt->prox = lista;
	return  punt;
}

void DistruggiButton( ListaDiBottoni lista ) {
    if (NoButton(lista)) return;
    DistruggiButton(lista->prox);
    delete (lista->btn);
    delete (lista);
}

//Input
ListaDiInput InitInput( void ) {
	return NULL;
}

int NoInput( ListaDiInput lista ) {
    return lista == NULL;
}

ListaDiInput InsInFondo( ListaDiInput lista, TipoElemento name, TipoElemento type, TipoElemento value, TipoElemento label ) {
    ListaDiInput punt;
    if( NoInput(lista) ) {
        punt=new Input;
        punt->name=name;
        punt->type=type;
        punt->value=value;
        punt->label=label;
        punt->prox=NULL;
        return punt;
    }
    lista->prox=InsInFondo(lista->prox, name, type, value, label);
    return lista;
}

void DistruggiInput( ListaDiInput lista ) {
    if (NoInput(lista)) return;
    DistruggiInput(lista->prox);
    delete (lista);
}

//Image
ListaDiImg InitImg( void ) {
	return NULL;
}

int NoImg( ListaDiImg lista ) {
    return lista == NULL;
}

ListaDiImg InsImg( ListaDiImg lista ) {
	ListaDiImg punt;
    punt=new Image;
    punt->tag = NULL;
    punt->imgdata = NULL;
    punt->fetched = false;
    punt->prox = lista;
	return  punt;
}

void DistruggiImg( ListaDiImg lista ) {
    if (NoImg(lista)) return;
    DistruggiImg(lista->prox);
    delete (lista->img);
    delete (lista->imgdata);
    delete (lista);
}

//Index
Indice InitIndex( void ) {
	return NULL;
}

int NoIndex( Indice lista ) {
    return lista == NULL;
}

Indice InsIndex( Indice lista ) {
	Indice punt=new Index;
    if (!NoIndex(lista))
        lista->prec = punt;
    punt->prec = NULL;
    punt->prox = lista;
	return  punt;
}

void DistruggiIndex( Indice lista ) {
    if (NoIndex(lista)) return;
    DistruggiIndex(lista->prox);
    delete (lista);
}

//History
History InitHistory( void ) {
	return NULL;
}

int NoUrls( History lista ) {
    return lista == NULL;
}

History InsUrl( History lista, char *url ) {
	History punt=new Url;
	punt->url=url;
    if (!NoUrls(lista)) {
        DistruggiHistory(lista->prox, 1);
        lista->prox = punt;
    }
    punt->prox = NULL;
    punt->prec = lista;
	return  punt;
}

void DistruggiHistory( History lista, int prox ) {
    if (NoUrls(lista)) return;
    if (prox)
        DistruggiHistory(lista->prox, prox);
    else
        DistruggiHistory(lista->prec, prox);
    delete (lista);
}

void FreeHistory( History lista ) {
    if (lista)
        DistruggiHistory(lista->prox,1);
    DistruggiHistory(lista,0);
}
