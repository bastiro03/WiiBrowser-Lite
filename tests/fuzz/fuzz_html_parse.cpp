/* fuzz_html_parse.cpp - libFuzzer harness for htmlcxx::HTML::ParserDom.
 *
 * Target: ParserDom::parseTree() in src/html/ParserDom.cpp. Already
 * exercised with real google/x.com HTML; this harness feeds arbitrary
 * bytes to surface SAX-level issues (malformed tags, attribute parsing
 * OOB, entity decoder overflow, UTF-8 handling).
 *
 * Bugs this should catch:
 *   - OOB read in tag/attribute parsing
 *   - Infinite loop on malformed comment / CDATA
 *   - Heap overflow in entity decoder for long/malformed entities
 *   - UAF in tree<Node> iterator invalidation
 */

#include <stddef.h>
#include <stdint.h>
#include <string>
#include "../../src/html/ParserDom.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (size > 262144) return 0;   /* 256 KB cap — matches x.com HTML size */

	try
	{
		std::string html((const char *)data, size);
		htmlcxx::HTML::ParserDom parser;
		const tree<htmlcxx::HTML::Node> &dom = parser.parseTree(html);

		/* Force iteration so any iterator-level UAF surfaces */
		for (auto it = dom.begin(); it != dom.end(); ++it)
		{
			(void)it->isTag();
		}
	}
	catch (...)
	{
		/* Exceptions are fine — we only care about memory safety. */
	}
	return 0;
}
