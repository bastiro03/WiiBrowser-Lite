#pragma once
#include "object.h"
#include "style.h"
#include "types.h"
#include "xh_scanner.h"
#include "context.h"

namespace litehtml
{
	struct str_istream : public instream
	{
		const wchar_t* p;
		const wchar_t* end;

		str_istream(const wchar_t* src) : p(src), end(src + wcslen(src))
		{
		}

		wchar_t get_char() override { return p < end ? *p++ : 0; }
	};

	struct css_text
	{
		using vector = std::vector<css_text>;

		std::wstring text;
		std::wstring baseurl;

		css_text()
		{
		}

		css_text(const wchar_t* txt, const wchar_t* url)
		{
			text = txt ? txt : L"";
			baseurl = url ? url : L"";
		}

		css_text(const css_text& val)
		{
			text = val.text;
			baseurl = val.baseurl;
		}
	};

	class element;

	class document : public object
	{
	public:
		using ptr = object_ptr<document>;

	private:
		element::ptr m_root;
		document_container* m_container;
		fonts_map m_fonts;
		css_text::vector m_css;
		css m_styles;
		std::wstring m_font_name;
		litehtml::web_color m_def_color;
		context* m_context;

		elements_vector m_parse_stack;

	public:
		document(document_container* objContainer, context* ctx);
		~document() override;

		document_container* container() { return m_container; }
		uint_ptr get_font(const wchar_t* name, int size, const wchar_t* weight, const wchar_t* style,
		                  const wchar_t* decoration);
		void render(int max_width);
		void draw(uint_ptr hdc, int x, int y, const position* clip);
		web_color get_def_color() { return m_def_color; }
		int cvt_units(const wchar_t* str, int fontSize, bool* is_percent = nullptr) const;
		int cvt_units(css_length& val, int fontSize) const;
		int width() const;
		int height() const;
		void add_stylesheet(const wchar_t* str, const wchar_t* baseurl);
		bool on_mouse_over(int x, int y, position::vector& redraw_boxes);
		bool on_lbutton_down(int x, int y, position::vector& redraw_boxes);
		bool on_lbutton_up(int x, int y, position::vector& redraw_boxes);
		bool on_mouse_leave(position::vector& redraw_boxes);
		element::ptr create_element(const wchar_t* tag_name);

		static ptr createFromString(const wchar_t* str, document_container* objPainter, context* ctx);

	private:
		//void			load_default_styles();
		element* add_root();
		element* add_body();
		uint_ptr add_font(const wchar_t* name, int size, const wchar_t* weight, const wchar_t* style,
		                  const wchar_t* decoration);

		void begin_parse();

		void parse_tag_start(const wchar_t* tag_name);
		void parse_tag_end(const wchar_t* tag_name);
		void parse_attribute(const wchar_t* attr_name, const wchar_t* attr_value);
		void parse_word(const wchar_t* val);
		void parse_space();
		void parse_comment_start();
		void parse_comment_end();
		void parse_data(const wchar_t* val);
		void parse_push_element(element::ptr el);
		void parse_pop_element();
		void parse_pop_element(const wchar_t* tag);
		void parse_pop_empty_element();
	};
}
