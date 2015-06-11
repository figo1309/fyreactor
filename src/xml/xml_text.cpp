
#include <xml/define.hpp>
#include <xml/xml_util.hpp>
#include <xml/xml_text.hpp>
#include <xml/xml_node.hpp>
#include <xml/xml_document.hpp>
#include <xml/xml_visitor.hpp>

namespace fyreactor
{
	namespace xml
	{
		char* XMLText::ParseDeep( char* p, StrPair* )
		{
			const char* start = p;
			if ( this->CData() ) {
				p = _value.ParseText( p, "]]>", StrPair::NEEDS_NEWLINE_NORMALIZATION );
				if ( !p ) {
					_document->SetError( XML_ERROR_PARSING_CDATA, start, 0 );
				}
				return p;
			}
			else {
				int flags = _document->ProcessEntities() ? StrPair::TEXT_ELEMENT : StrPair::TEXT_ELEMENT_LEAVE_ENTITIES;
				if ( _document->WhitespaceMode() == COLLAPSE_WHITESPACE ) {
					flags |= StrPair::COLLAPSE_WHITESPACE;
				}

				p = _value.ParseText( p, "<", flags );
				if ( !p ) {
					_document->SetError( XML_ERROR_PARSING_TEXT, start, 0 );
				}
				if ( p && *p ) {
					return p-1;
				}
			}
			return 0;
		}

		XMLNode* XMLText::ShallowClone( XMLDocument* doc ) const
		{
			if ( !doc ) {
				doc = _document;
			}
			XMLText* text = doc->NewText( Value() );	// fixme: this will always allocate memory. Intern?
			text->SetCData( this->CData() );
			return text;
		}

		bool XMLText::ShallowEqual( const XMLNode* compare ) const
		{
			return ( compare->ToText() && XMLUtil::StringEqual( compare->ToText()->Value(), Value() ));
		}

		bool XMLText::Accept( XMLVisitor* visitor ) const
		{
			return visitor->Visit( *this );
		}
	} // namespace xml
} // namespace fyreactor
