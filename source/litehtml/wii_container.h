#pragma once

namespace litehtml
{
	// call back interface to draw text, images and other elements
	class wii_container : public document_container
	{
    std::wstring			m_anchor;
    std::wstring			m_base_path;
    std::wstring			m_doc_path;

    litehtml::position::vector	m_clips;

	public:
        //wii_container();
		//virtual ~wii_container();

		//virtual uint_ptr	create_font(const wchar_t* faceName, int size, int weight, font_style italic, unsigned int decoration) = 0;
		//virtual void		delete_font(uint_ptr hFont) = 0;
		//virtual int			line_height(uint_ptr hdc, uint_ptr hFont) = 0;
		//virtual int			text_width(uint_ptr hdc, const wchar_t* text, uint_ptr hFont) = 0;
		//virtual void		draw_text(uint_ptr hdc, const wchar_t* text, uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) = 0;
		virtual void		fill_rect(uint_ptr hdc, const litehtml::position& pos, const litehtml::web_color color, const litehtml::css_border_radius& radius) = 0;
		virtual uint_ptr	get_temp_dc() = 0;
		virtual void		release_temp_dc(uint_ptr hdc) = 0;
		virtual int			pt_to_px(int pt) = 0;
		virtual int			get_default_font_size() = 0;
		virtual int			get_text_base_line(uint_ptr hdc, uint_ptr hFont) = 0;
		virtual void		draw_list_marker(uint_ptr hdc, list_style_type marker_type, int x, int y, int height, const web_color& color) = 0;
		virtual void		load_image(const wchar_t* src, const wchar_t* baseurl) = 0;
		virtual void		get_image_size(const wchar_t* src, const wchar_t* baseurl, litehtml::size& sz) = 0;
		virtual void		draw_image(uint_ptr hdc, const wchar_t* src, const wchar_t* baseurl, const litehtml::position& pos) = 0;
		virtual void		draw_background(uint_ptr hdc,
											const wchar_t* image,
											const wchar_t* baseurl,
											const litehtml::position& draw_pos,
											const litehtml::css_position& bg_pos,
											litehtml::background_repeat repeat,
											litehtml::background_attachment attachment) = 0;
		virtual void		draw_borders(uint_ptr hdc, const css_borders& borders, const litehtml::position& draw_pos) = 0;

		//virtual	void		set_caption(const wchar_t* caption)		= 0;
		//virtual	void		set_base_url(const wchar_t* base_url)	= 0;
		//virtual	void		link(litehtml::document* doc, litehtml::element::ptr el)	= 0;
		//virtual	void		on_anchor_click(const wchar_t* url, litehtml::element::ptr el)	= 0;
		//virtual	void		set_cursor(const wchar_t* cursor)	= 0;
		//virtual	wchar_t		toupper(const wchar_t c) = 0;
		//virtual	wchar_t		tolower(const wchar_t c) = 0;
		//virtual void		import_css(std::wstring& text, const std::wstring& url, std::wstring& baseurl, const string_vector& media) = 0;
		//virtual void		set_clip(const litehtml::position& pos, bool valid_x, bool valid_y) = 0;
		//virtual void		del_clip() = 0;

    protected:
        virtual std::wstring make_url(std::wstring link, const wchar_t* url);
        void get_client_rect(litehtml::position& client);
	};
}
