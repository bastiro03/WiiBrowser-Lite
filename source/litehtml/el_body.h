#pragma once
#include "element.h"

namespace litehtml
{
	class el_body : public element
	{
	public:
		el_body(document* doc);
		~el_body() override;

		bool is_body() const override;
	};
}
