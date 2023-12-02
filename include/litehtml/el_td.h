#pragma once
#include "element.h"

namespace litehtml
{
	class el_td : public element
	{
	public:
		el_td(document* doc);
		~el_td() override;

		void parse_styles(bool is_reparse) override;
	};
}
