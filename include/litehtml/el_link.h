#pragma once
#include "element.h"

namespace litehtml
{
	class el_link : public element
	{
	public:
		el_link(document* doc);
		~el_link() override;

	protected:
		void finish() override;
	};
}
