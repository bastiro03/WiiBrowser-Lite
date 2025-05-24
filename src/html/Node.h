#ifndef __HTML_PARSER_NODE_H
#define __HTML_PARSER_NODE_H

#include <map>
#include <string>
#include <utility>

namespace htmlcxx
{
	namespace HTML
	{
		class Node
		{
		public:
			Node()
			{
			}

			// Node(const Node &rhs); //uses default
			~Node()
			{
			}

			void text(const std::string &text) { this->mText = text; }
			const std::string &text() const { return this->mText; }

			void closingText(const std::string &text) { this->mClosingText = text; }
			const std::string &closingText() const { return mClosingText; }

			void offset(unsigned int offset) { this->mOffset = offset; }
			unsigned int offset() const { return this->mOffset; }

			void length(unsigned int length) { this->mLength = length; }
			unsigned int length() const { return this->mLength; }

			void tagName(const std::string &tagname) { this->mTagName = tagname; }
			const std::string &tagName() const { return this->mTagName; }

			bool isTag() const { return this->mIsHtmlTag; }
			void isTag(bool is_html_tag) { this->mIsHtmlTag = is_html_tag; }

			bool isClosing() const { return !this->mIsHtmlClose; }
			void isClosing(bool is_html_close) { this->mIsHtmlClose = is_html_close; }

			bool isComment() const { return this->mComment; }
			void isComment(bool comment) { this->mComment = comment; }

			std::string content(const std::string &html) const
			{
				return html.substr(this->mOffset /*+ this->mText.length()*/,
								   this->mLength /*- (this->mText.length() + this->mClosingText.length())*/);
			}

			std::pair<bool, std::string> attribute(const std::string &attr) const
			{
				auto i = this->mAttributes.find(attr);
				if (i != this->mAttributes.end())
				{
					return make_pair(true, i->second);
				}
				return make_pair(false, std::string());
			}

			operator std::string() const;
			std::ostream &operator<<(std::ostream &stream) const;

			const std::map<std::string, std::string> &attributes() const { return this->mAttributes; }
			void parseAttributes();

			bool operator==(const Node &rhs) const;

		protected:
			std::string mText;
			std::string mClosingText;
			unsigned int mOffset;
			unsigned int mLength;
			std::string mTagName;
			std::map<std::string, std::string> mAttributes;
			bool mIsHtmlTag;
			bool mIsHtmlClose;
			bool mComment;
		};
	}
}

#endif
