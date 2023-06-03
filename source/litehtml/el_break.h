#pragma once
#include "element.h"

namespace litehtml
{
	class el_break : public element
	{
	public:
		el_break(document* doc);
		~el_break() override;

		bool is_break() const override;
	};
}
