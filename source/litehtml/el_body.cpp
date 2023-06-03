#include "html_lt.h"
#include "el_body.h"
#include "document.h"

litehtml::el_body::el_body(document* doc) : element(doc)
{
}

litehtml::el_body::~el_body()
{
}

bool litehtml::el_body::is_body() const
{
	return true;
}
