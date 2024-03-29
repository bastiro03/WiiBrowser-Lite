#pragma once
#include "style.h"

namespace litehtml
{
	//////////////////////////////////////////////////////////////////////////

	struct selector_specificity
	{
		int a;
		int b;
		int c;
		int d;

		selector_specificity(int va = 0, int vb = 0, int vc = 0, int vd = 0)
		{
			a = va;
			b = vb;
			c = vc;
			d = vd;
		}

		void operator +=(const selector_specificity& val)
		{
			a += val.a;
			b += val.b;
			c += val.c;
			d += val.d;
		}

		bool operator==(const selector_specificity& val) const
		{
			if (a == val.a && b == val.b && c == val.c && d == val.d)
			{
				return true;
			}
			return false;
		}

		bool operator!=(const selector_specificity& val) const
		{
			if (a != val.a || b != val.b || c != val.c || d != val.d)
			{
				return true;
			}
			return false;
		}

		bool operator >(const selector_specificity& val) const
		{
			if (a > val.a)
			{
				return true;
			}
			if (a < val.a)
			{
				return false;
			}
			if (b > val.b)
			{
				return true;
			}
			if (b < val.b)
			{
				return false;
			}
			if (c > val.c)
			{
				return true;
			}
			if (c < val.c)
			{
				return false;
			}
			if (d > val.d)
			{
				return true;
			}
			if (d < val.d)
			{
				return false;
			}
			return false;
		}

		bool operator >=(const selector_specificity& val) const
		{
			if ((*this) == val) return true;
			if ((*this) > val) return true;
			return false;
		}

		bool operator <=(const selector_specificity& val) const
		{
			if ((*this) > val)
			{
				return false;
			}
			return true;
		}

		bool operator <(const selector_specificity& val) const
		{
			if ((*this) <= val && (*this) != val)
			{
				return true;
			}
			return false;
		}
	};

	//////////////////////////////////////////////////////////////////////////

	enum attr_select_condition
	{
		select_exists,
		select_equal,
		select_contain_str,
		select_start_str,
		select_end_str,
		select_pseudo_class,
	};

	//////////////////////////////////////////////////////////////////////////

	struct css_attribute_selector
	{
		using map = std::map<std::wstring, css_attribute_selector>;

		std::wstring val;
		attr_select_condition condition;

		css_attribute_selector()
		{
			condition = select_exists;
		}

		css_attribute_selector(const css_attribute_selector& val)
		{
			this->val = val.val;
			condition = val.condition;
		}
	};

	//////////////////////////////////////////////////////////////////////////

	class css_element_selector
	{
	public:
		std::wstring m_tag;
		css_attribute_selector::map m_attrs;

	public:
		css_element_selector()
		{
		}

		css_element_selector(const css_element_selector& val)
		{
			m_tag = val.m_tag;
			m_attrs = val.m_attrs;
		}

		void parse(const std::wstring& txt);
	};

	//////////////////////////////////////////////////////////////////////////

	enum css_combinator
	{
		combinator_descendant,
		combinator_child,
		combinator_adjacent_sibling,
		combinator_general_sibling
	};

	//////////////////////////////////////////////////////////////////////////

	class css_selector : public object
	{
	public:
		using ptr = object_ptr<css_selector>;
		using vector = std::vector<ptr>;

	public:
		selector_specificity m_specificity;
		css_element_selector m_right;
		ptr m_left;
		css_combinator m_combinator;
		style::ptr m_style;
		int m_order;

	public:
		css_selector()
		{
			m_combinator = combinator_descendant;
			m_order = 0;
		}

		~css_selector()
		{
		}

		css_selector(const css_selector& val)
		{
			m_right = val.m_right;
			if (val.m_left)
			{
				m_left = new css_selector(*val.m_left);
			}
			else
			{
				m_left = 0;
			}
			m_combinator = val.m_combinator;
			m_specificity = val.m_specificity;
			m_order = val.m_order;
		}

		bool parse(const std::wstring& text);
		void calc_specificity();
	};

	//////////////////////////////////////////////////////////////////////////

	inline bool operator >(const css_selector& v1, const css_selector& v2)
	{
		if (v1.m_specificity == v2.m_specificity)
		{
			return (v1.m_order > v2.m_order);
		}
		return (v1.m_specificity > v2.m_specificity);
	}

	inline bool operator <(const css_selector& v1, const css_selector& v2)
	{
		if (v1.m_specificity == v2.m_specificity)
		{
			return (v1.m_order < v2.m_order);
		}
		return (v1.m_specificity < v2.m_specificity);
	}

	inline bool operator >(css_selector::ptr v1, css_selector::ptr v2)
	{
		return (*v1 > *v2);
	}

	inline bool operator <(css_selector::ptr v1, css_selector::ptr v2)
	{
		return (*v1 < *v2);
	}

	//////////////////////////////////////////////////////////////////////////

	class used_selector : public object
	{
	public:
		using ptr = object_ptr<used_selector>;
		using vector = std::vector<ptr>;

		css_selector::ptr m_selector;
		bool m_used;

		used_selector(css_selector::ptr selector, bool used)
		{
			m_used = used;
			m_selector = selector;
		}
	};
}
