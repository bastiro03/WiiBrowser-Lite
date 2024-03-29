#pragma once
#include "attributes.h"

namespace litehtml
{
	class style : public object
	{
	public:
		using ptr = object_ptr<style>;
		using vector = std::vector<ptr>;

	private:
		string_map m_properties;

	public:
		style();
		style(const style& val);
		virtual ~style();

		void operator=(const style& val)
		{
			m_properties = val.m_properties;
		}

		void add(const wchar_t* txt, const wchar_t* baseurl)
		{
			parse(txt, baseurl);
		}

		void add_property(const wchar_t* name, const wchar_t* val, const wchar_t* baseurl);

		const wchar_t* get_property(const wchar_t* name) const
		{
			if (name)
			{
				string_map::const_iterator f = m_properties.find(name);
				if (f != m_properties.end())
				{
					return f->second.c_str();
				}
			}
			return nullptr;
		}

		void combine(const style& src);

		void clear()
		{
			m_properties.clear();
		}

	private:
		void parse_property(const std::wstring& txt, const wchar_t* baseurl);
		void parse(const wchar_t* txt, const wchar_t* baseurl);
		void parse_short_border(const std::wstring& prefix, const std::wstring& val);
		void parse_short_background(const std::wstring& val, const wchar_t* baseurl);
		void parse_short_font(const std::wstring& val);
	};

	/*
		class used_styles
		{
		public:
			typedef std::vector<used_styles>	vector;

			style_sheet::ptr	m_style_sheet;
			bool				m_used;

			used_styles()
			{
				m_used = false;
			}

			used_styles(style_sheet::ptr sh, bool used)
			{
				m_used			= used;
				m_style_sheet	= sh;
			}

			used_styles(const used_styles& val)
			{
				m_style_sheet	= val.m_style_sheet;
				m_used			= val.m_used;
			}
		};
	*/
}
