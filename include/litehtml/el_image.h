#pragma once

#include "element.h"

namespace litehtml
{
	class el_image : public element
	{
		std::wstring m_src;

	public:
		el_image(document* doc);
		~el_image(void) override;

		void parse_styles(bool is_reparse = false) override;
		int line_height() const override;

	protected:
		void get_content_size(size& sz, int max_width) override;
		void draw_content(uint_ptr hdc, const position& pos) override;
	};
}
