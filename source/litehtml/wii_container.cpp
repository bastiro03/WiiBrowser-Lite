#include <wctype.h>
#include <iconv.h>
#include <algorithm>

#include "..\include\litehtml.h"
#include "..\litehtml\tokenizer.h"

#include "..\FreeTypeGX.h"
#include "..\filelist.h"
#include "..\httplib.h"
#include "..\video.h"

#include "wii_container.h"
#include "common.h"

using namespace std;

extern GuiWindow *mainWindow;
extern GuiImage *bgImg;

wchar_t *load_text_file( const wchar_t *url );
struct block get_image( const wchar_t *url );

litehtml::wii_container::wii_container()
{
}

litehtml::wii_container::~wii_container(void)
{
    clear_images();
}

void litehtml::wii_container::open_url(wstring path)
{
    m_doc_path = path;
    m_base_path = path;
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
    double width = pos.right() - pos.left();
    double height = pos.bottom() - pos.top();

    GXColor col = { color.blue, color.green, color.red, color.alpha };
    GuiImage *image = new GuiImage(width, height, col);
    image->SetPosition(pos.left(), pos.top());

    mainWindow->Append(image);
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

	images_map::iterator img = m_images.find(url.c_str());
	if(img != m_images.end())
	{
        GuiImage *image = new GuiImage(img->second);

        image->SetSize(pos.width, pos.height);
        image->SetPosition(pos.x, pos.y);

        mainWindow->Append(image);
	}
}

void litehtml::wii_container::draw_borders(uint_ptr hdc, const css_borders& borders, const litehtml::position& draw_pos)
{

}

void litehtml::wii_container::draw_background(uint_ptr hdc, const wchar_t* image, const wchar_t* baseurl,
											const litehtml::position& draw_pos,
											const litehtml::css_position& bg_pos,
											litehtml::background_repeat repeat,
											litehtml::background_attachment attachment)
{
	std::wstring url, tmp(image);
	url = make_url(tmp, baseurl);

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
	double y = pos.top();

	GXColor col = { color.blue, color.green, color.red, color.alpha };

    GuiText *show = new GuiText(text, 0, col);
    show->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    show->SetPosition(x, y);
    show->SetFont(fnt);
    show->SetEffect(EFFECT_FADE, 50);

    // show->SetWrap(true, pos.right() - pos.left());
    mainWindow->Append(show);
}

wchar_t* converIconv(const char* src, const char *charset)
{
    size_t size = strlen(src) + 1;
    wchar_t *dst = new (std::nothrow) wchar_t[size];
    char *input = new (std::nothrow) char[size];

    if (!dst || !input)
    return NULL;
    memcpy(input, src, size);

    size_t outlen = size * sizeof(wchar_t);
    char *pIn = (char *) input;
    char *pOut = (char *) dst;

    iconv_t conv = iconv_open ("WCHAR_T", charset);
    if (conv == (iconv_t) -1)
        return NULL; // Charset not available

    size_t nconv = iconv(conv, &pIn, &size, &pOut, &outlen);
    if (nconv == (size_t) -1)
        return NULL; // Something went wrong

    iconv_close(conv);
    delete(input);

    if (outlen >= sizeof(wchar_t))
        *((wchar_t *) pOut) = L'\0';
    return (wchar_t *) dst;
}

wchar_t *load_text_file( const wchar_t *url )
{
    struct block HTML;
    CURL *curl_upd = curl_easy_init();

    char *ascii = new char[wcslen(url) + 1];
    int bt = wcstombs(ascii, url, wcslen(url));
    ascii[bt] = 0;

    HTML = downloadfile(curl_upd, ascii, NULL);
    delete(ascii);
    if(HTML.size == 0)
        return NULL;

	curl_easy_cleanup(curl_upd);
	wchar_t *text = converIconv(HTML.data, "UTF-8");
	free(HTML.data);

	return text;
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

wchar_t *wcsndup( const wchar_t *wideString, size_t  length )
{
    /* Local variables. */
    wchar_t  *duplicate ;

    duplicate = (wchar_t *) calloc (length + 1, sizeof (wchar_t)) ;
    if (duplicate == NULL) {
        return (NULL) ;
    }

    if (wideString == NULL) {
        duplicate[0] = 0 ;
    } else {
        wcsncpy (duplicate, wideString, length) ;
        duplicate[length] = 0 ;
    }

    return (duplicate) ;
}

wchar_t *getHost( wchar_t *url )
{
    wchar_t *p = wcschr(url, L'/')+2;
    wchar_t *c = wcschr(p, L'/');
    if (c != NULL)
        return wcsndup(url,(c+1)-url);
    return url;
}

wstring getRoot( wchar_t *url )
{
    wchar_t *i = wcschr(url, L'/');
    wchar_t *p = wcsrchr(i+2, L'/');
    wstring root(url);
    if (p != NULL)
        root.assign(url,(p+1)-url);
    else root.append(L"/");
    return root;
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

void litehtml::wii_container::import_css( std::wstring& text, const std::wstring& url, std::wstring& baseurl, const string_vector& media )
{
	if(media.empty() || std::find(media.begin(), media.end(), std::wstring(L"all")) != media.end() || std::find(media.begin(), media.end(), std::wstring(L"screen")) != media.end())
	{
		std::wstring css_url;
		css_url = make_url(url, baseurl.c_str());
		wchar_t *css = load_text_file(css_url.c_str());
		if(css)
		{
			baseurl = css_url;
			text = css;
			delete css;
		}
	}
}

void litehtml::wii_container::set_base_url( const wchar_t* base_url )
{
	if(base_url && base_url[0])
	{
        wstring temp(base_url);
        m_base_path = make_url(temp, m_base_path.c_str());
	}
	else
	{
		m_base_path = m_doc_path;
	}
}

void litehtml::wii_container::link( litehtml::document* doc, litehtml::element::ptr el )
{
	const wchar_t* rel = el->get_attr(L"rel");
	if(rel && !wcscmp(rel, L"stylesheet"))
	{
		const wchar_t* media = el->get_attr(L"media", L"screen");
		if(media && (wcsstr(media, L"screen") || wcsstr(media, L"all")))
		{
			const wchar_t* href = el->get_attr(L"href");
			if(href)
			{
				std::wstring url, temp(href);
				url = make_url(temp, m_base_path.c_str());
				wchar_t *css = load_text_file(url.c_str());
				if(css)
				{
					doc->add_stylesheet(css, url.c_str());
					delete css;
				}
			}
		}
	}
}

wstring litehtml::wii_container::make_url( wstring link, const wchar_t* url )
{
    if(!url || !url[0])
        url = m_base_path.c_str();

    wstring result;
    if (link.find(L"http://")==0 || link.find(L"https://")==0)
        return link;
    else if (link.at(0) == L'/' && link.at(1) == L'/')
        result.assign(L"http:"); // https?
    else if (link.at(0) == L'/')
    {
        result = getHost((wchar_t *)url);
        if (*result.rbegin() == '/')
            link.erase(link.begin());
    }
    else result = getRoot((wchar_t *)url);
    result.append(link);
    return result;
}

void litehtml::wii_container::on_anchor_click( const wchar_t* url, litehtml::element::ptr el )
{
    wstring temp(url);
    m_anchor = make_url(temp, m_base_path.c_str());
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
