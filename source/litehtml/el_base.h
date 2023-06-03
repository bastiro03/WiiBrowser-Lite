#pragma once
#include "element.h"

namespace litehtml
{
	class el_base : public element
	{
	public:
		el_base(document* doc);
		~el_base() override;

		void finish() override;
	};
}
