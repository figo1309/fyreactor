
#include <xml/define.hpp>
#include <xml/xml_util.hpp>
#include <xml/xml_node.hpp>
#include <xml/xml_document.hpp>
#include <xml/xml_comment.hpp>
#include <xml/xml_visitor.hpp>

namespace fyreactor
{
	namespace xml
	{
		XMLComment::XMLComment( XMLDocument* doc ) : XMLNode( doc )
		{
		}

		XMLComment::~XMLComment()
		{
		}

		char* XMLComment::ParseDeep( char* p, StrPair* )
		{
			// Comment parses as text.
			const char* start = p;
			p = _value.ParseText( p, "-->", StrPair::COMMENT );
			if ( p == 0 ) {
				_document->SetError( XML_ERROR_PARSING_COMMENT, start, 0 );
			}
			return p;
		}

		XMLNode* XMLComment::ShallowClone( XMLDocument* doc ) const
		{
			if ( !doc ) {
				doc = _document;
			}
			XMLComment* comment = doc->NewComment( Value() );	// fixme: this will always allocate memory. Intern?
			return comment;
		}

		bool XMLComment::ShallowEqual( const XMLNode* compare ) const
		{
			return ( compare->ToComment() && XMLUtil::StringEqual( compare->ToComment()->Value(), Value() ));
		}

		bool XMLComment::Accept( XMLVisitor* visitor ) const
		{
			return visitor->Visit( *this );
		}
	} // namespace xml
} // namespace fyreactor
