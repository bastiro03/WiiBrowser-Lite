#pragma once
#include "stylesheet.h"

namespace litehtml
{
	class context
	{
		css m_master_css;

	public:
		void load_master_stylesheet(const wchar_t* str);

		css& master_css()
		{
			return m_master_css;
		}
	};
}
