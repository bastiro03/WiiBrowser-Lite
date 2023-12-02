#pragma once

#include "element.h"

namespace litehtml
{
	class el_text : public element
	{
		std::wstring m_text;
		std::wstring m_transformed_text;
		size m_size;
		text_transform m_text_transform;

	public:
		el_text(const wchar_t* text, document* doc);
		~el_text() override;

		void apply_stylesheet(const css& stylesheet) override;
		void get_text(std::wstring& text) override;
		const wchar_t* get_style_property(const wchar_t* name, bool inherited, const wchar_t* def = nullptr) override;
		void parse_styles(bool is_reparse) override;
		int get_base_line() override;
		void draw(uint_ptr hdc, int x, int y, const position* clip) override;
		int line_height() const override;

	protected:
		void get_content_size(size& sz, int max_width) override;
		void draw_content(uint_ptr hdc, const position& pos) override;
	};
}
