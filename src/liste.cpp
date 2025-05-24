#include "liste.h"
#include "settings.h"
#include "main.h"

/*************************************************************
					 IMPLEMENTAZIONE
*************************************************************/

// Text
ListaDiTesto InitText(void)
{
	return nullptr;
}

int NoText(ListaDiTesto lista)
{
	return lista == nullptr;
}

ListaDiTesto InsText(ListaDiTesto lista)
{
	ListaDiTesto punt;
	punt = new Text;
	punt->prox = lista;
	return punt;
}

void DistruggiText(ListaDiTesto lista)
{
	if (NoText(lista))
		return;
	DistruggiText(lista->prox);
	delete (lista->txt);
	delete (lista);
}

// Button
ListaDiBottoni InitButton(void)
{
	return nullptr;
}

int NoButton(ListaDiBottoni lista)
{
	return lista == nullptr;
}

ListaDiBottoni InsButton(ListaDiBottoni lista, int outline)
{
	ListaDiBottoni punt;
	punt = new Button;
	punt->refs = nullptr;
	punt->label = NULL;
	punt->outline = outline;
	punt->prox = lista;
	return punt;
}

void DistruggiButton(ListaDiBottoni lista)
{
	if (NoButton(lista))
		return;
	DistruggiButton(lista->prox);
	delete (lista->btn);
	delete (lista);
}

// Input
ListaDiInput InitInput(void)
{
	return nullptr;
}

int NoInput(ListaDiInput lista)
{
	return lista == nullptr;
}

ListaDiInput InsInFondo(ListaDiInput lista, TipoElemento name, TipoElemento type, TipoElemento value,
						TipoElemento label)
{
	ListaDiInput punt;
	if (NoInput(lista))
	{
		punt = new Input;
		punt->name = name;
		punt->type = type;
		punt->value = value;
		punt->label = label;
		punt->prox = nullptr;
		return punt;
	}
	lista->prox = InsInFondo(lista->prox, name, type, value, label);
	return lista;
}

void SetOption(ListaDiInput element, TipoElemento value)
{
	if (!element)
		return;

	while (element->prox)
		element = element->prox;

	element->option = value;
}

void DistruggiInput(ListaDiInput lista)
{
	if (NoInput(lista))
		return;
	DistruggiInput(lista->prox);
	delete (lista);
}

// Image
ListaDiImg InitImg(void)
{
	return nullptr;
}

int NoImg(ListaDiImg lista)
{
	return lista == nullptr;
}

ListaDiImg InsImg(ListaDiImg lista)
{
	ListaDiImg punt;
	punt = new Image;
	punt->tag = nullptr;
	punt->imgdata = NULL;
	punt->fetched = false;
	punt->prox = lista;
	return punt;
}

void DistruggiImg(ListaDiImg lista)
{
	if (NoImg(lista))
		return;
	DistruggiImg(lista->prox);
	delete (lista->img);
	delete (lista->imgdata);
	delete (lista);
}

// Index
Indice InitIndex(void)
{
	return nullptr;
}

int NoIndex(Indice lista)
{
	return lista == nullptr;
}

Indice InsIndex(Indice lista)
{
	auto punt = new Index;
	if (!NoIndex(lista))
		lista->prec = punt;
	punt->prec = nullptr;
	punt->prox = lista;
	return punt;
}

void DistruggiIndex(Indice lista)
{
	if (NoIndex(lista))
		return;
	DistruggiIndex(lista->prox);
	delete (lista);
}

// History
History InitHistory(void)
{
	return nullptr;
}

History RewindHistory(History lista)
{
	if (!lista)
		return nullptr;
	while (!NoUrls(lista->prec))
		lista = lista->prec;
	return lista;
}

int NoUrls(History lista)
{
	return lista == nullptr;
}

History InsUrl(History lista, char *url)
{
	auto punt = new Url;
	punt->url = url;
	if (!NoUrls(lista))
	{
		DistruggiHistory(lista->prox, 1);
		lista->prox = punt;
	}
	punt->prox = nullptr;
	punt->prec = lista;
	return punt;
}

void DistruggiHistory(History lista, int prox)
{
	if (NoUrls(lista))
		return;
	if (prox)
		DistruggiHistory(lista->prox, prox);
	else
		DistruggiHistory(lista->prec, prox);
	delete (lista);
}

void FreeHistory(History lista)
{
	if (lista)
		DistruggiHistory(lista->prox, 1);
	DistruggiHistory(lista, 0);
}

void DumpList(History lista, const char *file)
{
	char path[256];
	snprintf(path, 256, "%s/appdata/history.txt", Settings.AppPath);
	FILE *fp = fopen(path, "w");
	if (!fp)
		return;

	lista = RewindHistory(lista);
	while (!NoUrls(lista))
	{
		fprintf(fp, "%s\r\n", lista->url.c_str());
		lista = lista->prox;
	}
	fprintf(fp, "# End List");
	fclose(fp);
}

History LoadList(const char *file)
{
	char path[512];
	snprintf(path, 512, "%s/appdata/history.txt", Settings.AppPath);
	FILE *fp = fopen(path, "r");
	if (!fp)
		return nullptr;

	History lista = nullptr;
	while (!feof(fp))
	{
		fscanf(fp, "%s", path);
		if (strchr(path, '#'))
			break;
		lista = InsUrl(lista, path);
	}
	fclose(fp);
	return lista;
}
