#pragma once

#include "element.h"

namespace litehtml
{
	class el_text : public element
	{
		std::wstring	m_text;
		std::wstring	m_transformed_text;
		size			m_size;
		text_transform	m_text_transform;
	public:
		el_text(const wchar_t* text, litehtml::document* doc);
		virtual ~el_text();

		virtual void				apply_stylesheet(const litehtml::css& stylesheet);
		virtual void				get_text(std::wstring& text);
		virtual const wchar_t*		get_style_property(const wchar_t* name, bool inherited, const wchar_t* def = 0);
		virtual void				parse_styles(bool is_reparse);
		virtual int					get_base_line();
		virtual void				draw(uint_ptr hdc, int x, int y, const position* clip);
		virtual int					line_height() const;

	protected:
		virtual void				get_content_size(size& sz, int max_width);
		virtual void				draw_content(uint_ptr hdc, const litehtml::position& pos);
	};
}