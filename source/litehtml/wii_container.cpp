#include <wctype.h>
#include <algorithm>

#include "..\include\litehtml.h"

#include "..\FreeTypeGX.h"
#include "..\filelist.h"
#include "..\video.h"

#include "wii_container.h"
#include "common.h"

using namespace std;

wchar_t *load_text_file( const wchar_t *url );
struct block get_image( const wchar_t *url );

litehtml::wii_container::wii_container()
{
}

litehtml::wii_container::~wii_container(void)
{
    clear_images();
}

void litehtml::wii_container::set_cursor(const wchar_t* cursor)
{

}

void litehtml::wii_container::set_caption(const wchar_t* caption)
{

}

litehtml::uint_ptr litehtml::wii_container::get_temp_dc()
{
	return 0;
}

void litehtml::wii_container::release_temp_dc(uint_ptr hdc)
{

}

void litehtml::wii_container::fill_rect(uint_ptr hdc, const litehtml::position& pos, const litehtml::web_color color, const litehtml::css_border_radius& radius)
{
    GXColor col = { color.red, color.green, color.blue, color.alpha };
    GuiImage *image = new GuiImage(pos.width, pos.height, col);
    image->SetPosition(pos.x, pos.y);

    m_list.push_back(image);
    // m_wind->Append(image);
}

void litehtml::wii_container::draw_list_marker(uint_ptr hdc, list_style_type marker_type, int x, int y, int height, const web_color& color)
{
	int top_margin = height / 3;

	int draw_x		= x;
	int draw_y		= y + top_margin;
	int draw_width	= height - top_margin * 2;
	int draw_height	= height - top_margin * 2;

	switch(marker_type)
	{
        case litehtml::list_style_type_circle:
        case litehtml::list_style_type_disc:
        case litehtml::list_style_type_square:
        default:
		{
			fill_rect(hdc, litehtml::position(draw_x, draw_y, draw_width, draw_height), color, litehtml::css_border_radius());
		}
		break;
	}
}

void litehtml::wii_container::load_image(const wchar_t* src, const wchar_t* baseurl)
{
	std::wstring url, str(src);
	url = make_url(str, baseurl);

	if(m_images.find(url.c_str()) == m_images.end())
	{
		struct block img = get_image(url.c_str());
		if(img.size > 0 && strstr(img.type, "image"))
		{
		    GuiImageData *imagedata = new GuiImageData((u8 *)img.data, img.size);
			m_images[url.c_str()] = imagedata;
		}
		free(img.data);
	}
}

void litehtml::wii_container::get_image_size(const wchar_t* src, const wchar_t* baseurl, litehtml::size& sz)
{
	std::wstring url, str(src);
	url = make_url(str, baseurl);

	images_map::iterator img = m_images.find(url.c_str());
	if(img != m_images.end())
	{
		sz.width	= img->second->GetWidth();
		sz.height	= img->second->GetHeight();
	}
}

void litehtml::wii_container::draw_image(uint_ptr hdc, const wchar_t* src, const wchar_t* baseurl, const litehtml::position& pos)
{
    std::wstring url, str(src);
	url = make_url(str, baseurl);

    litehtml::position cpos;
	if(!m_clips.empty())
        cpos = *(m_clips.rbegin());

	images_map::iterator img = m_images.find(url.c_str());
	if(img != m_images.end())
	{
        GuiImage *image = new GuiImage(img->second);

        image->SetScaleX((float)pos.width/image->GetWidth());
        image->SetScaleY((float)pos.height/image->GetHeight());
        image->SetPosition(pos.x, pos.y);

        if(!m_clips.empty())
            image->SetClip(cpos.x, cpos.y, cpos.width, cpos.height);

		// m_wind->Append(image);
		m_list.push_back(image);
	}
	del_clip();
}

void litehtml::wii_container::line_to(int xleft, int yleft, int xright, int yright, litehtml::web_color color)
{
    litehtml::position pos(xleft, yleft, xright - xleft, yright - yleft);
    fill_rect(0, pos, color, litehtml::css_border_radius());
}

void litehtml::wii_container::draw_borders(uint_ptr hdc, const css_borders& borders, const litehtml::position& draw_pos)
{
	int bdr_top		= 0;
	int bdr_bottom	= 0;
	int bdr_left	= 0;
	int bdr_right	= 0;

	if(borders.top.width.val() != 0 && borders.top.style > litehtml::border_style_hidden)
	{
		bdr_top = (int) borders.top.width.val();
	}
	if(borders.bottom.width.val() != 0 && borders.bottom.style > litehtml::border_style_hidden)
	{
		bdr_bottom = (int) borders.bottom.width.val();
	}
	if(borders.left.width.val() != 0 && borders.left.style > litehtml::border_style_hidden)
	{
		bdr_left = (int) borders.left.width.val();
	}
	if(borders.right.width.val() != 0 && borders.right.style > litehtml::border_style_hidden)
	{
		bdr_right = (int) borders.right.width.val();
	}

	// draw right border
	if(bdr_right)
	{
        line_to(draw_pos.right() - bdr_right, draw_pos.top() + bdr_top, draw_pos.right(), draw_pos.top(), borders.right.color);
        line_to(draw_pos.right(), draw_pos.top(), draw_pos.right(),	draw_pos.bottom(), borders.right.color);
        line_to(draw_pos.right(), draw_pos.bottom(), draw_pos.right() - bdr_right,	draw_pos.bottom() - bdr_bottom, borders.right.color);
	}

	// draw bottom border
	if(bdr_bottom)
	{
		line_to(draw_pos.left(), draw_pos.bottom(), draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom, borders.bottom.color);
		line_to(draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom, draw_pos.right() - bdr_right, draw_pos.bottom() - bdr_bottom, borders.bottom.color);
		line_to(draw_pos.right() - bdr_right, draw_pos.bottom() - bdr_bottom, draw_pos.right(),	draw_pos.bottom(), borders.bottom.color);
	}

	// draw top border
	if(bdr_top)
	{
        line_to(draw_pos.left(), draw_pos.top(), draw_pos.left() + bdr_left, draw_pos.top() + bdr_top, borders.top.color);
        line_to(draw_pos.left() + bdr_left, draw_pos.top() + bdr_top, draw_pos.right() - bdr_right,	draw_pos.top() + bdr_top, borders.top.color);
        line_to(draw_pos.right() - bdr_right,	draw_pos.top() + bdr_top, draw_pos.right(),	draw_pos.top(), borders.top.color);
	}

	// draw left border
	if(bdr_left)
	{
		line_to(draw_pos.left() + bdr_left, draw_pos.top() + bdr_top, draw_pos.left(), draw_pos.top(), borders.left.color);
		line_to(draw_pos.left(), draw_pos.top(), draw_pos.left(), draw_pos.bottom(), borders.left.color);
		line_to(draw_pos.left(), draw_pos.bottom(), draw_pos.left() + bdr_left,	draw_pos.bottom() - bdr_bottom, borders.left.color);
	}
}

void litehtml::wii_container::draw_background(uint_ptr hdc, const wchar_t* image, const wchar_t* baseurl,
											const litehtml::position& draw_pos,
											const litehtml::css_position& bg_pos,
											litehtml::background_repeat repeat,
											litehtml::background_attachment attachment)
{
	std::wstring url, tmp(image);
	url = make_url(tmp, baseurl);
	set_clip(draw_pos, 1, 1);

	images_map::iterator img_i = m_images.find(url.c_str());
	if(img_i != m_images.end())
	{
		GuiImageData* bgbmp = img_i->second;

		litehtml::size img_sz;
		img_sz.width	= bgbmp->GetWidth();
		img_sz.height	= bgbmp->GetHeight();

		int bg_x = draw_pos.x;
		int bg_y = draw_pos.y;

		if(bg_pos.x.units() != litehtml::css_units_percentage)
		{
			bg_x += (int) bg_pos.x.val();
		} else
		{
			bg_x += (int) ((float) (draw_pos.width - img_sz.width) * bg_pos.x.val() / 100.0);
		}

		if(bg_pos.y.units() != litehtml::css_units_percentage)
		{
			bg_y += (int) bg_pos.y.val();
		} else
		{
			bg_y += (int) ( (float) (draw_pos.height - img_sz.height) * bg_pos.y.val() / 100.0);
		}

		litehtml::position pos(bg_x, bg_y, bgbmp->GetWidth(), bgbmp->GetHeight());
		draw_image(0, image, baseurl, pos);

		switch(repeat)
		{
		case litehtml::background_repeat_no_repeat:
		/*	draw_image(0, image, baseurl, { bg_x, bg_y, bgbmp->GetWidth(), bgbmp->GetHeight() });
			break;*/

		case litehtml::background_repeat_repeat_x:
		/*	cairo_set_source(cr, pattern);
			cairo_rectangle(cr, draw_pos.left(), bg_y, draw_pos.width, bgbmp->getHeight());
			cairo_fill(cr);
			break;*/

		case litehtml::background_repeat_repeat_y:
		/*	cairo_set_source(cr, pattern);
			cairo_rectangle(cr, bg_x, draw_pos.top(), bgbmp->getWidth(), draw_pos.height);
			cairo_fill(cr);
			break;*/

		case litehtml::background_repeat_repeat:
		/*	cairo_set_source(cr, pattern);
			cairo_rectangle(cr, draw_pos.left(), draw_pos.top(), draw_pos.width, draw_pos.height);
			cairo_fill(cr);*/
			break;
		}
	}
}

litehtml::uint_ptr litehtml::wii_container::create_font( const wchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration )
{
    const u8 *fontbuffer = font_ttf;
    u32	filesize = font_ttf_size;

    if (italic == litehtml::fontStyleItalic)
    {
        fontbuffer = font_italic_ttf;
        filesize = font_italic_ttf_size;
    }

    // size = size >= get_default_font_size() ? size : get_default_font_size();
    FreeTypeGX *font = new FreeTypeGX(size, fontbuffer, filesize);

    return (uint_ptr) font;
}

void litehtml::wii_container::delete_font( litehtml::uint_ptr hFont )
{
	FreeTypeGX *fnt = (FreeTypeGX *)hFont;

	if(fnt)
	{
		delete fnt;
	}
}

int litehtml::wii_container::line_height( litehtml::uint_ptr hdc, litehtml::uint_ptr hFont )
{
	FreeTypeGX *fnt = (FreeTypeGX *)hFont;

    return fnt->ftPointSize + 6;
}

int litehtml::wii_container::text_width( litehtml::uint_ptr hdc, const wchar_t* text, litehtml::uint_ptr hFont )
{
	FreeTypeGX *fnt = (FreeTypeGX *)hFont;

    return fnt->getWidth(text);
}

void litehtml::wii_container::draw_text( litehtml::uint_ptr hdc, const wchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos )
{
    FreeTypeGX *fnt = (FreeTypeGX *)hFont;
    double x = pos.left();
	double y = pos.top() + 15;

	GXColor col = { color.red, color.green, color.blue, color.alpha };

    GuiText *show = new GuiText(text, 0, col);
    show->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    show->SetPosition(x, y);
    show->SetFont(fnt);

    show->SetEffect(EFFECT_FADE, 50);
    // show->SetWrap(true, pos.right() - pos.left());
    // m_wind->Append(show);
    m_list.push_back(show);
}

struct block get_image( const wchar_t *url )
{
    struct block HTML;
    CURL *curl_upd = curl_easy_init();

    char *ascii = new char[wcslen(url) + 1];
    int bt = wcstombs(ascii, url, wcslen(url));
    ascii[bt] = 0;

    HTML = downloadfile(curl_upd, ascii, NULL);
    delete(ascii);
    if(HTML.size == 0)
        return emptyblock;

	curl_easy_cleanup(curl_upd);

	return HTML;
}

int litehtml::wii_container::get_text_base_line( litehtml::uint_ptr hdc, litehtml::uint_ptr hFont )
{
    FreeTypeGX *fnt = (FreeTypeGX *)hFont;

    return fnt->ftFace->size->metrics.descender >> 6;
}

int litehtml::wii_container::get_default_font_size()
{
	return 16;
}

int litehtml::wii_container::pt_to_px( int pt )
{
    return (pt * 96) / 72;
}

wchar_t litehtml::wii_container::toupper( const wchar_t c )
{
	return (wchar_t) towupper(c);
}

wchar_t litehtml::wii_container::tolower( const wchar_t c )
{
	return (wchar_t) towlower(c);
}

void litehtml::wii_container::set_clip( const litehtml::position& pos, bool valid_x, bool valid_y )
{
	litehtml::position clip_pos = pos;
	litehtml::position client_pos;
	get_client_rect(client_pos);
	if(!valid_x)
	{
		clip_pos.x		= client_pos.x;
		clip_pos.width	= client_pos.width;
	}
	if(!valid_y)
	{
		clip_pos.y		= client_pos.y;
		clip_pos.height	= client_pos.height;
	}
	m_clips.push_back(clip_pos);
}

void litehtml::wii_container::del_clip()
{
	if(!m_clips.empty())
	{
		m_clips.pop_back();
		if(!m_clips.empty())
		{
			litehtml::position clip_pos = m_clips.back();
		}
	}
}

void litehtml::wii_container::get_client_rect( litehtml::position& client )
{
	client.x		= 0;
	client.y		= 0;
	client.width	= screenwidth;
	client.height	= screenheight;
}

void litehtml::wii_container::clear_images()
{
	for(images_map::iterator i = m_images.begin(); i != m_images.end(); i++)
	{
		if(i->second)
		{
			delete i->second;
		}
	}
	m_images.clear();
}
