#include "html_lt.h"
#include "document.h"
#include "elements.h"
#include "tokenizer.h"
#include "stylesheet.h"
#include "el_table.h"
#include "el_td.h"
#include "el_link.h"
#include "el_title.h"
#include "el_style.h"
#include "el_comment.h"
#include "el_base.h"
#include "el_anchor.h"
#include "el_break.h"

#include <math.h>
#include <stdlib.h>

#define FW_LIGHT        300
#define FW_NORMAL       400
#define FW_SEMIBOLD     600
#define FW_BOLD         700

int
wtoi(const wchar_t* str)
{
	return static_cast<int>(wcstol(str, 0, 10));
}

wchar_t*
itow(int value, wchar_t* string, int radix)
{
	wchar_t tmp[33];
	wchar_t* tp = tmp;
	int i;
	unsigned v;
	int sign;
	wchar_t* sp;

	if (radix > 36 || radix <= 1)
	{
		return nullptr;
	}

	sign = (radix == 10 && value < 0);
	if (sign)
		v = -value;
	else
		v = static_cast<unsigned>(value);
	while (v || tp == tmp)
	{
		i = v % radix;
		v = v / radix;
		if (i < 10)
			*tp++ = i + L'0';
		else
			*tp++ = i + L'a' - 10;
	}

	sp = string;
	if (sign)
		*sp++ = L'-';
	while (tp > tmp)
		*sp++ = *--tp;
	*sp = 0;
	return string;
}

const wchar_t* g_empty_tags[] =
{
	L"base",
	L"meta",
	L"link",
	L"br",
	L"hr",
	L"input",
	L"button",
	L"img",
	L"param",
	L"col",
	nullptr
};

litehtml::document::document(document_container* objContainer, context* ctx)
{
	m_container = objContainer;
	m_font_name = L"Times New Roman";
	m_context = ctx;
}

litehtml::document::~document()
{
}

litehtml::document::ptr litehtml::document::createFromString(const wchar_t* str, document_container* objPainter,
                                                             context* ctx)
{
	ptr doc = new document(objPainter, ctx);
	str_istream si(str);
	scanner sc(si);

	doc->begin_parse();

	int t = 0;
	while ((t = sc.get_token()) != scanner::TT_EOF && !doc->m_parse_stack.empty())
	{
		switch (t)
		{
		case scanner::TT_COMMENT_START:
			doc->parse_comment_start();
			break;
		case scanner::TT_COMMENT_END:
			doc->parse_comment_end();
			break;
		case scanner::TT_DATA:
			doc->parse_data(sc.get_value());
			break;
		case scanner::TT_TAG_START:
			doc->parse_tag_start(sc.get_tag_name());
			break;
		case scanner::TT_TAG_END:
			doc->parse_tag_end(sc.get_tag_name());
			break;
		case scanner::TT_ATTR:
			doc->parse_attribute(sc.get_attr_name(), sc.get_value());
			break;
		case scanner::TT_WORD:
			doc->parse_word(sc.get_value());
			break;
		case scanner::TT_SPACE:
			doc->parse_space();
			break;
		}
	}

	if (doc->m_root)
	{
		doc->m_root->finish();

		doc->m_root->apply_stylesheet(ctx->master_css());

		for (css_text::vector::iterator css = doc->m_css.begin(); css != doc->m_css.end(); css++)
		{
			doc->m_styles.parse_stylesheet(css->text.c_str(), css->baseurl.c_str(), doc->container());
		}
		doc->m_styles.sort_selectors();

		doc->m_root->apply_stylesheet(doc->m_styles);

		doc->m_root->parse_styles();
	}

	return doc;
}

litehtml::uint_ptr litehtml::document::add_font(const wchar_t* name, int size, const wchar_t* weight,
                                                const wchar_t* style, const wchar_t* decoration)
{
	uint_ptr ret = 0;

	if (!name || name && !wcscasecmp(name, L"inherit"))
	{
		name = m_font_name.c_str();
	}

	if (!size)
	{
		size = container()->get_default_font_size();
	}

	wchar_t strSize[20];
	itow(size, strSize, 10);

	std::wstring key = name;
	key += L":";
	key += strSize;
	key += L":";
	key += weight;
	key += L":";
	key += style;
	key += L":";
	key += decoration;

	if (m_fonts.find(key) == m_fonts.end())
	{
		auto fs = static_cast<font_style>(value_index(style, font_style_strings, fontStyleNormal));
		int fw = value_index(weight, font_weight_strings, -1);
		if (fw >= 0)
		{
			switch (fw)
			{
			case fontWeightBold:
				fw = FW_BOLD;
				break;
			case fontWeightBolder:
				fw = FW_SEMIBOLD;
				break;
			case fontWeightLighter:
				fw = FW_LIGHT;
				break;
			default:
				fw = FW_NORMAL;
				break;
			}
		}
		else
		{
			fw = wtoi(weight);
			if (fw < 100)
			{
				fw = FW_NORMAL;
			}
		}

		unsigned int decor = 0;

		if (decoration)
		{
			std::vector<std::wstring> tokens;
			tokenize(decoration, tokens, L" ");
			for (auto i = tokens.begin(); i != tokens.end(); ++i)
			{
				if (!wcscasecmp(i->c_str(), L"underline"))
				{
					decor |= font_decoration_underline;
				}
				else if (!wcscasecmp(i->c_str(), L"line-through"))
				{
					decor |= font_decoration_linethrough;
				}
				else if (!wcscasecmp(i->c_str(), L"overline"))
				{
					decor |= font_decoration_overline;
				}
			}
		}

		ret = m_container->create_font(name, size, fw, fs, decor);
		m_fonts[key] = ret;
	}
	return ret;
}

litehtml::uint_ptr litehtml::document::get_font(const wchar_t* name, int size, const wchar_t* weight,
                                                const wchar_t* style, const wchar_t* decoration)
{
	if (!name || name && !wcscasecmp(name, L"inherit"))
	{
		name = m_font_name.c_str();
	}

	if (!size)
	{
		size = container()->get_default_font_size();
	}

	wchar_t strSize[20];
	itow(size, strSize, 10);

	std::wstring key = name;
	key += L":";
	key += strSize;
	key += L":";
	key += weight;
	key += L":";
	key += style;
	key += L":";
	key += decoration;

	fonts_map::iterator el = m_fonts.find(key);

	if (el != m_fonts.end())
	{
		return el->second;
	}
	return add_font(name, size, weight, style, decoration);
}

litehtml::element* litehtml::document::add_root()
{
	if (!m_root)
	{
		m_root = new element(this);
		m_root->addRef();
		m_root->set_tagName(L"html");
	}
	return m_root;
}

litehtml::element* litehtml::document::add_body()
{
	if (!m_root)
	{
		add_root();
	}
	element* el = new el_body(this);
	el->set_tagName(L"body");
	m_root->appendChild(el);
	return el;
}

void litehtml::document::render(int max_width)
{
	if (m_root)
	{
		m_root->render(0, 0, max_width);
	}
}

void litehtml::document::draw(uint_ptr hdc, int x, int y, const position* clip)
{
	if (m_root)
	{
		m_root->draw(hdc, x, y, clip);
	}
}

int litehtml::document::cvt_units(const wchar_t* str, int fontSize, bool* is_percent/*= 0*/) const
{
	if (!str) return 0;

	css_length val;
	val.fromString(str);
	if (is_percent && val.units() == css_units_percentage && !val.is_predefined())
	{
		*is_percent = true;
	}
	return cvt_units(val, fontSize);
}

int litehtml::document::cvt_units(css_length& val, int fontSize) const
{
	if (val.is_predefined() || !val.is_predefined() && val.units() == css_units_percentage)
	{
		return 0;
	}
	int ret = 0;
	switch (val.units())
	{
	case css_units_px:
		ret = static_cast<int>(val.val());
		break;
	case css_units_em:
		ret = round_f(val.val() * fontSize);
		val.set_value(static_cast<float>(ret), css_units_px);
		break;
	case css_units_pt:
		ret = m_container->pt_to_px(static_cast<int>(val.val()));
		val.set_value(static_cast<float>(ret), css_units_px);
		break;
	case css_units_in:
		ret = m_container->pt_to_px(static_cast<int>(val.val() * 72));
		val.set_value(static_cast<float>(ret), css_units_px);
		break;
	case css_units_cm:
		ret = m_container->pt_to_px(static_cast<int>(val.val() * 0.3937 * 72));
		val.set_value(static_cast<float>(ret), css_units_px);
		break;
	case css_units_mm:
		ret = m_container->pt_to_px(static_cast<int>(val.val() * 0.3937 * 72) / 10);
		val.set_value(static_cast<float>(ret), css_units_px);
		break;
	}
	return ret;
}

int litehtml::document::width() const
{
	return m_root ? m_root->width() : 0;
}

int litehtml::document::height() const
{
	return m_root ? m_root->height() : 0;
}

void litehtml::document::add_stylesheet(const wchar_t* str, const wchar_t* baseurl)
{
	if (str && str[0])
	{
		m_css.push_back(css_text(str, baseurl));
	}
}

bool litehtml::document::on_mouse_over(int x, int y, position::vector& redraw_boxes)
{
	if (!m_root)
	{
		return false;
	}
	bool res = m_root->on_mouse_over(x, y);
	const wchar_t* cursor = m_root->get_cursor();
	m_container->set_cursor(cursor ? cursor : L"auto");
	if (res)
	{
		m_root->find_styles_changes(redraw_boxes, 0, 0);
		return true;
	}
	return false;
}

bool litehtml::document::on_mouse_leave(position::vector& redraw_boxes)
{
	if (!m_root)
	{
		return false;
	}
	if (m_root->on_mouse_leave())
	{
		m_root->find_styles_changes(redraw_boxes, 0, 0);
		return true;
	}
	return false;
}

bool litehtml::document::on_lbutton_down(int x, int y, position::vector& redraw_boxes)
{
	if (!m_root)
	{
		return false;
	}
	if (m_root->on_lbutton_down(x, y))
	{
		m_root->find_styles_changes(redraw_boxes, 0, 0);
		return true;
	}
	return false;
}

bool litehtml::document::on_lbutton_up(int x, int y, position::vector& redraw_boxes)
{
	if (!m_root)
	{
		return false;
	}
	if (m_root->on_lbutton_up(x, y))
	{
		m_root->find_styles_changes(redraw_boxes, 0, 0);
		return true;
	}
	return false;
}

litehtml::element::ptr litehtml::document::create_element(const wchar_t* tag_name)
{
	element::ptr newTag = nullptr;
	if (!wcscasecmp(tag_name, L"br"))
	{
		newTag = new el_break(this);
	}
	else if (!wcscasecmp(tag_name, L"p"))
	{
		newTag = new el_para(this);
	}
	else if (!wcscasecmp(tag_name, L"img"))
	{
		newTag = new el_image(this);
	}
	else if (!wcscasecmp(tag_name, L"table"))
	{
		newTag = new el_table(this);
	}
	else if (!wcscasecmp(tag_name, L"td") || !wcscasecmp(tag_name, L"th"))
	{
		newTag = new el_td(this);
	}
	else if (!wcscasecmp(tag_name, L"link"))
	{
		newTag = new el_link(this);
	}
	else if (!wcscasecmp(tag_name, L"title"))
	{
		newTag = new el_title(this);
	}
	else if (!wcscasecmp(tag_name, L"a"))
	{
		newTag = new el_anchor(this);
	}
	else if (!wcscasecmp(tag_name, L"tr"))
	{
		newTag = new element(this);
	}
	else if (!wcscasecmp(tag_name, L"style"))
	{
		newTag = new el_style(this);
	}
	else if (!wcscasecmp(tag_name, L"base"))
	{
		newTag = new el_base(this);
	}
	else if (!wcscasecmp(tag_name, L"body"))
	{
		newTag = new el_body(this);
	}
	else
	{
		newTag = new element(this);
	}

	if (newTag)
	{
		newTag->set_tagName(tag_name);
	}

	return newTag;
}

void litehtml::document::parse_tag_start(const wchar_t* tag_name)
{
	parse_pop_empty_element();

	// We add the html(root) element before parsing
	if (!wcscasecmp(tag_name, L"html"))
	{
		return;
	}

	element::ptr el = create_element(tag_name);
	if (el)
	{
		if (!wcscasecmp(m_parse_stack.back()->get_tagName(), L"html"))
		{
			// if last element is root we have to add head or body
			if (!value_in_list(tag_name, L"head;body"))
			{
				parse_push_element(create_element(L"body"));
			}
		}

		// fix <TD> and <TH>
		if (value_in_list(tag_name, L"td;th"))
		{
			if (value_in_list(m_parse_stack.back()->get_tagName(), L"th;td"))
			{
				parse_pop_element();
			}

			if (wcscasecmp(m_parse_stack.back()->get_tagName(), L"tr"))
			{
				parse_push_element(create_element(L"tr"));
			}
		}

		// fix <TR>: add tbody into the table
		if (!wcscasecmp(tag_name, L"tr"))
		{
			if (!value_in_list(m_parse_stack.back()->get_tagName(), L"tbody;thead;tfoot"))
			{
				parse_push_element(create_element(L"tbody"));
			}
		}

		parse_push_element(el);
	}
}

void litehtml::document::parse_tag_end(const wchar_t* tag_name)
{
	if (!m_parse_stack.empty())
	{
		if (!wcscasecmp(m_parse_stack.back()->get_tagName(), tag_name))
		{
			parse_pop_element();
		}
		else
		{
			parse_pop_element(tag_name);
		}
	}
}

void litehtml::document::begin_parse()
{
	m_root = create_element(L"html");
	m_parse_stack.push_back(m_root);
}

void litehtml::document::parse_push_element(element::ptr el)
{
	if (!m_parse_stack.empty())
	{
		m_parse_stack.back()->appendChild(el);
		m_parse_stack.push_back(el);
	}
}

void litehtml::document::parse_attribute(const wchar_t* attr_name, const wchar_t* attr_value)
{
	if (!m_parse_stack.empty())
	{
		m_parse_stack.back()->set_attr(attr_name, attr_value);
	}
}

void litehtml::document::parse_word(const wchar_t* val)
{
	parse_pop_empty_element();

	if (!m_parse_stack.empty())
	{
		element::ptr el = new el_text(val, this);
		m_parse_stack.back()->appendChild(el);
	}
}

void litehtml::document::parse_space()
{
	parse_pop_empty_element();

	if (!m_parse_stack.empty())
	{
		element::ptr el = new el_space(this);
		m_parse_stack.back()->appendChild(el);
	}
}

void litehtml::document::parse_comment_start()
{
	parse_pop_empty_element();
	parse_push_element(new el_comment(this));
}

void litehtml::document::parse_comment_end()
{
	parse_pop_element();
}

void litehtml::document::parse_data(const wchar_t* val)
{
	if (!m_parse_stack.empty())
	{
		m_parse_stack.back()->set_data(val);
	}
}

void litehtml::document::parse_pop_element()
{
	if (!m_parse_stack.empty())
	{
		m_parse_stack.pop_back();
	}
}

void litehtml::document::parse_pop_element(const wchar_t* tag)
{
	bool found = false;
	for (elements_vector::reverse_iterator iel = m_parse_stack.rbegin(); iel != m_parse_stack.rend(); iel++)
	{
		if (!wcscasecmp((*iel)->get_tagName(), tag))
		{
			found = true;
		}
	}

	while (found)
	{
		if (!wcscasecmp(m_parse_stack.back()->get_tagName(), tag))
		{
			found = false;
		}
		parse_pop_element();
	}
}

void litehtml::document::parse_pop_empty_element()
{
	if (!m_parse_stack.empty())
	{
		bool is_empty_tag = false;
		for (int i = 0; g_empty_tags[i]; i++)
		{
			if (!wcscasecmp(m_parse_stack.back()->get_tagName(), g_empty_tags[i]))
			{
				is_empty_tag = true;
				break;
			}
		}
		if (is_empty_tag)
		{
			parse_pop_element();
		}
	}
}
