#pragma once
#include "element.h"

namespace litehtml
{
	class el_anchor : public element
	{
	public:
		el_anchor(document* doc);
		~el_anchor() override;

		void on_click(int x, int y) override;
	};
}
