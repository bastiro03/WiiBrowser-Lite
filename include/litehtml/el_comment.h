#pragma once
#include "element.h"

namespace litehtml
{
	class el_comment : public element
	{
		std::wstring m_text;

	public:
		el_comment(document* doc);
		~el_comment() override;

		void get_text(std::wstring& text) override;
		void set_data(const wchar_t* data) override;

		void apply_stylesheet(const css& stylesheet) override;
		void parse_styles(bool is_reparse) override;
		int get_base_line() override;
	};
}
