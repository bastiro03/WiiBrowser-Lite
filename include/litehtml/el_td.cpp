#include "html_lt.h"
#include "el_td.h"

litehtml::el_td::el_td(document* doc) : element(doc)
{
}

litehtml::el_td::~el_td()
{
}

void litehtml::el_td::parse_styles(bool is_reparse)
{
	const wchar_t* str = get_attr(L"width");
	if (str)
	{
		m_style.add_property(L"width", str, nullptr);
	}
	str = get_attr(L"background");
	if (str)
	{
		std::wstring url = L"url('";
		url += str;
		url += L"')";
		m_style.add_property(L"background-image", url.c_str(), nullptr);
	}
	str = get_attr(L"align");
	if (str)
	{
		m_style.add_property(L"text-align", str, nullptr);
	}

	str = get_attr(L"valign");
	if (str)
	{
		m_style.add_property(L"vertical-align", str, nullptr);
	}

	element::parse_styles(is_reparse);
}
