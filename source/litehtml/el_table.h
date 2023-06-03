#pragma once
#include "element.h"
#include "table.h"

namespace litehtml
{
	struct col_info
	{
		int width;
		bool is_auto;
	};

	class el_table : public element
	{
		table_grid m_grid;
		css_length m_css_border_spacing_x;
		css_length m_css_border_spacing_y;
		int m_border_spacing_x;
		int m_border_spacing_y;

	public:
		el_table(document* doc);
		~el_table() override;

		int render(int x, int y, int max_width) override;
		bool appendChild(element* el) override;
		void parse_styles(bool is_reparse = false) override;
		void draw(uint_ptr hdc, int x, int y, const position* clip) override;

	protected:
		void init() override;
	};
}
