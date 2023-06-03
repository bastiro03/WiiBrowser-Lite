#pragma once
#include "element.h"

namespace litehtml
{
	class el_para : public element
	{
	public:
		el_para(document* doc);
		~el_para() override;

		void parse_styles(bool is_reparse = false) override;
	};
}
