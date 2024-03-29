#include "html_lt.h"
#include "el_image.h"
#include "document.h"

litehtml::el_image::el_image(document* doc) : element(doc)
{
}

litehtml::el_image::~el_image(void)
{
}

void litehtml::el_image::get_content_size(size& sz, int max_width)
{
	m_doc->container()->get_image_size(m_src.c_str(), 0, sz);
}

void litehtml::el_image::draw_content(uint_ptr hdc, const position& pos)
{
	m_doc->container()->draw_image(hdc, m_src.c_str(), 0, pos);
}

void litehtml::el_image::parse_styles(bool is_reparse)
{
	element::parse_styles(is_reparse);
	m_src = get_attr(L"src", L"");
	m_doc->container()->load_image(m_src.c_str(), NULL);
	size sz;
	m_doc->container()->get_image_size(m_src.c_str(), 0, sz);

	if (!m_css_height.val())
	{
		m_css_height.fromString(get_attr(L"height", L"auto"), L"auto");
		if (m_css_height.is_predefined())
		{
			m_css_height.set_value(static_cast<float>(sz.height), css_units_px);
		}
	}
	if (!m_css_width.val())
	{
		m_css_width.fromString(get_attr(L"width", L"auto"), L"auto");
		if (m_css_width.is_predefined())
		{
			m_css_width.set_value(static_cast<float>(sz.width), css_units_px);
		}
	}
}

int litehtml::el_image::line_height() const
{
	return height();
}
