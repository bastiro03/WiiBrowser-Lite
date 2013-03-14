#pragma once
#include "libwiigui/gui.h"
#include "list"

namespace litehtml
{
	// call back interface to draw text, images and other elements
	class wii_container : public document_container
	{
	    typedef std::map<std::wstring, GuiImageData *>	images_map;

    public:
        litehtml::position::vector	m_clips;
        images_map				    m_images;
        std::list<GuiElement *>     m_list;

        wii_container();
		virtual ~wii_container();

		virtual uint_ptr	create_font(const wchar_t* faceName, int size, int weight, font_style italic, unsigned int decoration);
		virtual void		delete_font(uint_ptr hFont);
		virtual int			line_height(uint_ptr hdc, uint_ptr hFont);
		virtual int			text_width(uint_ptr hdc, const wchar_t* text, uint_ptr hFont);
		virtual void		draw_text(uint_ptr hdc, const wchar_t* text, uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos);
		virtual void		fill_rect(uint_ptr hdc, const litehtml::position& pos, const litehtml::web_color color, const litehtml::css_border_radius& radius);
		virtual uint_ptr	get_temp_dc();
		virtual void		release_temp_dc(uint_ptr hdc);
		virtual int			pt_to_px(int pt);
		virtual int			get_default_font_size();
		virtual int			get_text_base_line(uint_ptr hdc, uint_ptr hFont);
		virtual void		draw_list_marker(uint_ptr hdc, list_style_type marker_type, int x, int y, int height, const web_color& color);
		virtual void		load_image(const wchar_t* src, const wchar_t* baseurl);
		virtual void		get_image_size(const wchar_t* src, const wchar_t* baseurl, litehtml::size& sz);
		virtual void		draw_image(uint_ptr hdc, const wchar_t* src, const wchar_t* baseurl, const litehtml::position& pos);
		virtual void		draw_background(uint_ptr hdc,
											const wchar_t* image,
											const wchar_t* baseurl,
											const litehtml::position& draw_pos,
											const litehtml::css_position& bg_pos,
											litehtml::background_repeat repeat,
											litehtml::background_attachment attachment);
		virtual void		draw_borders(uint_ptr hdc, const css_borders& borders, const litehtml::position& draw_pos);

        virtual	void		set_caption(const wchar_t* caption);
		virtual	void		set_base_url(const wchar_t* base_url) = 0;
		virtual	void		link(litehtml::document* doc, litehtml::element::ptr el) = 0;
        virtual	void		on_anchor_click(const wchar_t* url, litehtml::element::ptr el) = 0;
        virtual	void		set_cursor(const wchar_t* cursor);
		virtual	wchar_t		toupper(const wchar_t c);
		virtual	wchar_t		tolower(const wchar_t c);
		virtual void		import_css(std::wstring& text, const std::wstring& url, std::wstring& baseurl, const string_vector& media) = 0;
		virtual void		set_clip(const litehtml::position& pos, bool valid_x, bool valid_y);
		virtual void		del_clip();

        void                line_to(int xleft, int yleft, int xright, int yright, litehtml::web_color color);

    protected:
        virtual std::wstring make_url(std::wstring link, const wchar_t* url) = 0;
        void get_client_rect(litehtml::position& client);
        void clear_images();
	};
}
