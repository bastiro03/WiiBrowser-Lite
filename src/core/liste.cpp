static bool historyWritable = false;
#include <sys/stat.h>
#include <cerrno>
#include <sstream>
#include "liste.h"
#include "file_io.h"
#include "safety.h"
#include "settings.h"
#include "main.h"

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
    punt->prox=lista;
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

ListaDiBottoni InsButton( ListaDiBottoni lista, int outline ) {
	ListaDiBottoni punt;
    punt=new Button{};
    punt->refs=NULL;
    punt->label=NULL;
    punt->outline=outline;
    punt->prox=lista;
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

void SetOption( ListaDiInput element, TipoElemento value )
{
    if (!element)
        return;

    while (element->prox)
        element = element->prox;

    element->option = value;
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

History RewindHistory( History lista ) {
	if (!lista)
        return NULL;
	while (!NoUrls(lista->prec))
        lista=lista->prec;
	return lista;
}

int NoUrls( History lista ) {
    return lista == NULL;
}

History InsUrl( History lista, char *url ) {
    if (!url || !wbl::httpUrl(url)) return lista;
	History punt=new Url;
	punt->url=url;
    if (!NoUrls(lista)) {
        DistruggiHistory(lista->prox, 1);
        lista->prox = punt;
    }
    punt->prox = NULL;
    punt->prec = lista;
    History oldest = RewindHistory(punt);
    size_t count = 0;
    for (History entry = oldest; entry; entry = entry->prox) ++count;
    while (count-- > 128 && oldest != punt) {
        History next = oldest->prox;
        next->prec = NULL;
        delete oldest;
        oldest = next;
    }
    return punt;
}

void DistruggiHistory( History lista, int prox ) {
    while (lista) { History next = prox ? lista->prox : lista->prec; delete lista; lista = next; }
}

void FreeHistory( History lista ) {
    DistruggiHistory(RewindHistory(lista), 1);
}

void DumpList ( History lista, const char * file ) {
    char path[256];
    snprintf(path, 256, "%s/appdata/history.txt", Settings.AppPath);
    FILE * fp=fopen(path, "w");
    if (!fp)
        return;

    lista=RewindHistory(lista);
    while (!NoUrls(lista)) {
        fprintf(fp, "%s\r\n", lista->url.c_str());
        lista=lista->prox;
    }
    fprintf(fp, "# End List");
    fclose(fp);
}

History LoadList ( const char * file ) {
    (void)file;
    std::string data;
    const std::string path = std::string(Settings.AppPath) + "appdata/history.txt";
    if (!wbl::readBounded(path.c_str(), data, 128 * (wbl::MaxUrl + 1))) {
        struct stat status;
        historyWritable = stat(path.c_str(), &status) != 0 && errno == ENOENT;
        return NULL;
    }
    std::istringstream input(data);
    std::string line;
    History lista = NULL;
    bool valid = true, sentinel = false;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line == "# End List") { sentinel = true; break; }
        if (wbl::httpUrl(line)) lista = InsUrl(lista, &line[0]);
        else valid = false;
    }
    historyWritable = valid && sentinel;
    return lista;
}
