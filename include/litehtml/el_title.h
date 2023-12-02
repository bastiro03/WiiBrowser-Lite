#pragma once
#include "element.h"

namespace litehtml
{
	class el_title : public element
	{
	public:
		el_title(document* doc);
		~el_title() override;

	protected:
		void finish() override;
	};
}
