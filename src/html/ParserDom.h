#ifndef __HTML_PARSER_DOM_H__
#define __HTML_PARSER_DOM_H__

#include "ParserSax.h"
#include "tree.h"

namespace htmlcxx
{
	namespace HTML
	{
		class ParserDom : public ParserSax
		{
		public:
			ParserDom()
			{
			}

			~ParserDom() override
			{
			}

			const tree<Node> &parseTree(const std::string &html);

			const tree<Node> &getTree()
			{
				return mHtmlTree;
			}

		protected:
			void beginParsing() override;

			void foundTag(Node node, bool isEnd) override;
			void foundText(Node node) override;
			void foundComment(Node node) override;

			void endParsing() override;

			tree<Node> mHtmlTree;
			tree<Node>::iterator mCurrentState;
		};

		std::ostream &operator<<(std::ostream &stream, const tree<Node> &tr);
	} // namespace HTML
} // namespace htmlcxx

#endif
