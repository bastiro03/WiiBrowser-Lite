/* html_parser_asan_wrapper.cpp - C wrapper for htmlcxx ParserDom
 *
 * Provides a simple C interface for html_parser_asan_test.c to call
 * htmlcxx::HTML::ParserDom::parseTree() and count the resulting nodes.
 */

#include <string>
#include "../src/html/ParserDom.h"

extern "C" {

/* Parse HTML and return the number of nodes in the resulting tree.
 * Returns -1 on failure (exception or error). */
int html_parse_and_count_nodes(const char *html, size_t len)
{
	try
	{
		std::string html_str(html, len);
		htmlcxx::HTML::ParserDom parser;
		const tree<htmlcxx::HTML::Node> &dom = parser.parseTree(html_str);

		/* Count nodes by iterating through the tree */
		int count = 0;
		for (auto it = dom.begin(); it != dom.end(); ++it)
			count++;

		return count;
	}
	catch (...)
	{
		return -1;
	}
}

} // extern "C"
