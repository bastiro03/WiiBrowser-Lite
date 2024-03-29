#pragma once
#include "style.h"

namespace litehtml
{
	class document_container;

	class css
	{
		css_selector::vector m_selectors;

	public:
		css()
		{
		}

		~css()
		{
		}

		const css_selector::vector& selectors() const
		{
			return m_selectors;
		}

		void add_selector(css_selector::ptr selector);
		void parse_stylesheet(const wchar_t* str, const wchar_t* baseurl, document_container* doc);
		void parse_selectors(const std::wstring& txt, style::ptr styles);
		void sort_selectors();
		static void parse_css_url(const std::wstring& str, std::wstring& url);
	};

	inline void css::add_selector(css_selector::ptr selector)
	{
		selector->m_order = static_cast<int>(m_selectors.size());
		m_selectors.push_back(selector);
	}
}
