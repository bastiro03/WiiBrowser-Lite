#include "html_lt.h"
#include "el_title.h"
#include "document.h"

litehtml::el_title::el_title(document* doc) : element(doc)
{
}

litehtml::el_title::~el_title()
{
}

void litehtml::el_title::finish()
{
	std::wstring text;
	get_text(text);
	m_doc->container()->set_caption(text.c_str());
}
