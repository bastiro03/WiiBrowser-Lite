#include "entities.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define UNICODE_MAX 0x10FFFFul

static const char *named_entities[][2] =
	{
		{"AElig;", "Æ"},
		{"Aacute;", "Á"},
		{"Acirc;", "Â"},
		{"Agrave;", "À"},
		{"Alpha;", "Α"},
		{"Aring;", "Å"},
		{"Atilde;", "Ã"},
		{"Auml;", "Ä"},
		{"Beta;", "Β"},
		{"Ccedil;", "Ç"},
		{"Chi;", "Χ"},
		{"Dagger;", "‡"},
		{"Delta;", "Δ"},
		{"ETH;", "Ð"},
		{"Eacute;", "É"},
		{"Ecirc;", "Ê"},
		{"Egrave;", "È"},
		{"Epsilon;", "Ε"},
		{"Eta;", "Η"},
		{"Euml;", "Ë"},
		{"Gamma;", "Γ"},
		{"Iacute;", "Í"},
		{"Icirc;", "Î"},
		{"Igrave;", "Ì"},
		{"Iota;", "Ι"},
		{"Iuml;", "Ï"},
		{"Kappa;", "Κ"},
		{"Lambda;", "Λ"},
		{"Mu;", "Μ"},
		{"Ntilde;", "Ñ"},
		{"Nu;", "Ν"},
		{"OElig;", "Œ"},
		{"Oacute;", "Ó"},
		{"Ocirc;", "Ô"},
		{"Ograve;", "Ò"},
		{"Omega;", "Ω"},
		{"Omicron;", "Ο"},
		{"Oslash;", "Ø"},
		{"Otilde;", "Õ"},
		{"Ouml;", "Ö"},
		{"Phi;", "Φ"},
		{"Pi;", "Π"},
		{"Prime;", "″"},
		{"Psi;", "Ψ"},
		{"Rho;", "Ρ"},
		{"Scaron;", "Š"},
		{"Sigma;", "Σ"},
		{"THORN;", "Þ"},
		{"Tau;", "Τ"},
		{"Theta;", "Θ"},
		{"Uacute;", "Ú"},
		{"Ucirc;", "Û"},
		{"Ugrave;", "Ù"},
		{"Upsilon;", "Υ"},
		{"Uuml;", "Ü"},
		{"Xi;", "Ξ"},
		{"Yacute;", "Ý"},
		{"Yuml;", "Ÿ"},
		{"Zeta;", "Ζ"},
		{"aacute;", "á"},
		{"acirc;", "â"},
		{"acute;", "´"},
		{"aelig;", "æ"},
		{"agrave;", "à"},
		{"alefsym;", "ℵ"},
		{"alpha;", "α"},
		{"amp;", "&"},
		{"and;", "∧"},
		{"ang;", "∠"},
		{"apos;", "'"},
		{"aring;", "å"},
		{"asymp;", "≈"},
		{"atilde;", "ã"},
		{"auml;", "ä"},
		{"bdquo;", "„"},
		{"beta;", "β"},
		{"brvbar;", "¦"},
		{"bull;", "•"},
		{"cap;", "∩"},
		{"ccedil;", "ç"},
		{"cedil;", "¸"},
		{"cent;", "¢"},
		{"chi;", "χ"},
		{"circ;", "ˆ"},
		{"clubs;", "♣"},
		{"cong;", "≅"},
		{"copy;", "©"},
		{"crarr;", "↵"},
		{"cup;", "∪"},
		{"curren;", "¤"},
		{"dArr;", "⇓"},
		{"dagger;", "†"},
		{"darr;", "↓"},
		{"deg;", "°"},
		{"delta;", "δ"},
		{"diams;", "♦"},
		{"divide;", "÷"},
		{"eacute;", "é"},
		{"ecirc;", "ê"},
		{"egrave;", "è"},
		{"empty;", "∅"},
		{"emsp;", " "},
		{"ensp;", " "},
		{"epsilon;", "ε"},
		{"equiv;", "≡"},
		{"eta;", "η"},
		{"eth;", "ð"},
		{"euml;", "ë"},
		{"euro;", "€"},
		{"exist;", "∃"},
		{"fnof;", "ƒ"},
		{"forall;", "∀"},
		{"frac12;", "½"},
		{"frac14;", "¼"},
		{"frac34;", "¾"},
		{"frasl;", "⁄"},
		{"gamma;", "γ"},
		{"ge;", "≥"},
		{"gt;", ">"},
		{"hArr;", "⇔"},
		{"harr;", "↔"},
		{"hearts;", "♥"},
		{"hellip;", "…"},
		{"iacute;", "í"},
		{"icirc;", "î"},
		{"iexcl;", "¡"},
		{"igrave;", "ì"},
		{"image;", "ℑ"},
		{"infin;", "∞"},
		{"int;", "∫"},
		{"iota;", "ι"},
		{"iquest;", "¿"},
		{"isin;", "∈"},
		{"iuml;", "ï"},
		{"kappa;", "κ"},
		{"lArr;", "⇐"},
		{"lambda;", "λ"},
		{"lang;", "〈"},
		{"laquo;", "«"},
		{"larr;", "←"},
		{"lceil;", "⌈"},
		{"ldquo;", "“"},
		{"le;", "≤"},
		{"lfloor;", "⌊"},
		{"lowast;", "∗"},
		{"loz;", "◊"},
		{"lrm;", "\xE2\x80\x8E"},
		{"lsaquo;", "‹"},
		{"lsquo;", "‘"},
		{"lt;", "<"},
		{"macr;", "¯"},
		{"mdash;", "—"},
		{"micro;", "µ"},
		{"middot;", "·"},
		{"minus;", "−"},
		{"mu;", "μ"},
		{"nabla;", "∇"},
		{"nbsp;", " "},
		{"ndash;", "–"},
		{"ne;", "≠"},
		{"ni;", "∋"},
		{"not;", "¬"},
		{"notin;", "∉"},
		{"nsub;", "⊄"},
		{"ntilde;", "ñ"},
		{"nu;", "ν"},
		{"oacute;", "ó"},
		{"ocirc;", "ô"},
		{"oelig;", "œ"},
		{"ograve;", "ò"},
		{"oline;", "‾"},
		{"omega;", "ω"},
		{"omicron;", "ο"},
		{"oplus;", "⊕"},
		{"or;", "∨"},
		{"ordf;", "ª"},
		{"ordm;", "º"},
		{"oslash;", "ø"},
		{"otilde;", "õ"},
		{"otimes;", "⊗"},
		{"ouml;", "ö"},
		{"para;", "¶"},
		{"part;", "∂"},
		{"permil;", "‰"},
		{"perp;", "⊥"},
		{"phi;", "φ"},
		{"pi;", "π"},
		{"piv;", "ϖ"},
		{"plusmn;", "±"},
		{"pound;", "£"},
		{"prime;", "′"},
		{"prod;", "∏"},
		{"prop;", "∝"},
		{"psi;", "ψ"},
		{"quot;", "\""},
		{"rArr;", "⇒"},
		{"radic;", "√"},
		{"rang;", "〉"},
		{"raquo;", "»"},
		{"rarr;", "→"},
		{"rceil;", "⌉"},
		{"rdquo;", "”"},
		{"real;", "ℜ"},
		{"reg;", "®"},
		{"rfloor;", "⌋"},
		{"rho;", "ρ"},
		{"rlm;", "\xE2\x80\x8F"},
		{"rsaquo;", "›"},
		{"rsquo;", "’"},
		{"sbquo;", "‚"},
		{"scaron;", "š"},
		{"sdot;", "⋅"},
		{"sect;", "§"},
		{"shy;", "\xC2\xAD"},
		{"sigma;", "σ"},
		{"sigmaf;", "ς"},
		{"sim;", "∼"},
		{"spades;", "♠"},
		{"sub;", "⊂"},
		{"sube;", "⊆"},
		{"sum;", "∑"},
		{"sup;", "⊃"},
		{"sup1;", "¹"},
		{"sup2;", "²"},
		{"sup3;", "³"},
		{"supe;", "⊇"},
		{"szlig;", "ß"},
		{"tau;", "τ"},
		{"there4;", "∴"},
		{"theta;", "θ"},
		{"thetasym;", "ϑ"},
		{"thinsp;", " "},
		{"thorn;", "þ"},
		{"tilde;", "˜"},
		{"times;", "×"},
		{"trade;", "™"},
		{"uArr;", "⇑"},
		{"uacute;", "ú"},
		{"uarr;", "↑"},
		{"ucirc;", "û"},
		{"ugrave;", "ù"},
		{"uml;", "¨"},
		{"upsih;", "ϒ"},
		{"upsilon;", "υ"},
		{"uuml;", "ü"},
		{"weierp;", "℘"},
		{"xi;", "ξ"},
		{"yacute;", "ý"},
		{"yen;", "¥"},
		{"yuml;", "ÿ"},
		{"zeta;", "ζ"},
		{"zwj;", "\xE2\x80\x8D"},
		{"zwnj;", "\xE2\x80\x8C"}};

static int cmp(const void *key, const void *element)
{
	return strncmp(key, *(const char **)element,
				   strlen(*(const char **)element));
}

static const char *get_named_entity(const char *name)
{
	const char **entity = bsearch(name, named_entities,
								  sizeof(named_entities) / sizeof(*named_entities),
								  sizeof(*named_entities), cmp);

	return entity ? entity[1] : NULL;
}

static size_t putc_utf8(unsigned long cp, char *buffer)
{
	unsigned char *bytes = (unsigned char *)buffer;

	if (cp <= 0x007Ful)
	{
		bytes[0] = (unsigned char)cp;
		return 1;
	}

	if (cp <= 0x07FFul)
	{
		bytes[1] = (unsigned char)((2u << 6) | (cp & 0x3Fu));
		bytes[0] = (unsigned char)((6u << 5) | (cp >> 6));
		return 2;
	}

	if (cp <= 0xFFFFul)
	{
		bytes[2] = (unsigned char)((2u << 6) | (cp & 0x3Fu));
		bytes[1] = (unsigned char)((2u << 6) | ((cp >> 6) & 0x3Fu));
		bytes[0] = (unsigned char)((14u << 4) | (cp >> 12));
		return 3;
	}

	if (cp <= 0x10FFFFul)
	{
		bytes[3] = (unsigned char)((2u << 6) | (cp & 0x3Fu));
		bytes[2] = (unsigned char)((2u << 6) | ((cp >> 6) & 0x3Fu));
		bytes[1] = (unsigned char)((2u << 6) | ((cp >> 12) & 0x3Fu));
		bytes[0] = (unsigned char)((30u << 3) | (cp >> 18));
		return 4;
	}

	return 0;
}

static _Bool parse_entity(const char *current, char **to,
						  const char **from)
{
	const char *end = strchr(current, ';');
	if (!end)
		return 0;

	if (current[1] == '#')
	{
		char *tail = NULL;
		errno = 0;

		_Bool hex = current[2] == 'x' || current[2] == 'X';

		unsigned long cp = strtoul(
			current + (hex ? 3 : 2), &tail, hex ? 16 : 10);

		if (tail == end && !errno && cp <= UNICODE_MAX)
		{
			*to += putc_utf8(cp, *to);
			*from = end + 1;

			return 1;
		}
	}
	else
	{
		const char *entity = get_named_entity(&current[1]);
		if (entity)
		{
			size_t len = strlen(entity);
			memcpy(*to, entity, len);

			*to += len;
			*from = end + 1;

			return 1;
		}
	}

	return 0;
}

size_t decode_html_entities_utf8(char *dest, const char *src)
{
	if (!src)
		src = dest;

	char *to = dest;
	const char *from = src;

	const char *current;
	while ((current = strchr(from, '&')))
	{
		memcpy(to, from, current - from);
		to += current - from;

		if (parse_entity(current, &to, &from))
			continue;

		from = current;
		*to++ = *from++;
	}

	size_t remaining = strlen(from);

	memcpy(to, from, remaining);
	to += remaining;

	*to = 0;
	return to - dest;
}
