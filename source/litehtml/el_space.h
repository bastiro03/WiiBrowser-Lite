#pragma once
#include "element.h"
#include "el_text.h"

namespace litehtml
{
	class el_space : public el_text
	{
	public:
		el_space(document* doc);
		~el_space() override;

		bool is_white_space() override;
	};
}
