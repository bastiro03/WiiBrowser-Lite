#pragma once
#include "element.h"

namespace litehtml
{
	class el_style : public element
	{
	public:
		el_style(document* doc);
		~el_style() override;

		void finish() override;
	};
}
