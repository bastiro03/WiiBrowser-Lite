#include "html_lt.h"
#include "iterators.h"
#include "element.h"

litehtml::element* litehtml::elements_iterator::next(bool ret_parent)
{
	next_idx();

	while (m_idx < static_cast<int>(m_el->children().size()))
	{
		element* el = m_el->children()[m_idx];
		if (el->children().size() && m_go_inside && m_go_inside->select(el))
		{
			stack_item si;
			si.idx = m_idx;
			si.el = m_el;
			m_stack.push_back(si);
			m_el = el;
			m_idx = -1;
			if (ret_parent)
			{
				return el;
			}
			next_idx();
		}
		else
		{
			if (!m_select || m_select && m_select->select(m_el->children()[m_idx]))
			{
				return m_el->children()[m_idx];
			}
			next_idx();
		}
	}

	return nullptr;
}

void litehtml::elements_iterator::next_idx()
{
	m_idx++;
	while (m_idx >= static_cast<int>(m_el->children().size()) && m_stack.size())
	{
		stack_item si = m_stack.back();
		m_stack.pop_back();
		m_idx = si.idx;
		m_el = si.el;
		m_idx++;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool litehtml::go_inside_inline::select(element* el)
{
	if (el->get_display() == display_inline || el->get_display() == display_inline_text)
	{
		return true;
	}
	return false;
}

bool litehtml::go_inside_table::select(element* el)
{
	if (el->get_display() == display_table_row_group ||
		el->get_display() == display_table_header_group ||
		el->get_display() == display_table_footer_group)
	{
		return true;
	}
	return false;
}

bool litehtml::table_rows_selector::select(element* el)
{
	if (el->get_display() == display_table_row)
	{
		return true;
	}
	return false;
}

bool litehtml::table_cells_selector::select(element* el)
{
	if (el->get_display() == display_table_cell)
	{
		return true;
	}
	return false;
}
