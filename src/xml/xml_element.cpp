
#include <xml/define.hpp>
#include <xml/xml_util.hpp>
#include <xml/xml_node.hpp>
#include <xml/xml_text.hpp>
#include <xml/xml_element.hpp>
#include <xml/xml_document.hpp>

namespace fyreactor
{
	namespace xml
	{
		const XMLElement* XMLNode::FirstChildElement( const char* value ) const
		{
			for( XMLNode* node=_firstChild; node; node=node->_next ) {
				XMLElement* element = node->ToElement();
				if ( element ) {
					if ( !value || XMLUtil::StringEqual( element->Name(), value ) ) {
						return element;
					}
				}
			}
			return 0;
		}

		const XMLElement* XMLNode::LastChildElement( const char* value ) const
		{
			for( XMLNode* node=_lastChild; node; node=node->_prev ) {
				XMLElement* element = node->ToElement();
				if ( element ) {
					if ( !value || XMLUtil::StringEqual( element->Name(), value ) ) {
						return element;
					}
				}
			}
			return 0;
		}

		const XMLElement* XMLNode::NextSiblingElement( const char* value ) const
		{
			for( XMLNode* element=this->_next; element; element = element->_next ) {
				if (    element->ToElement()
						&& (!value || XMLUtil::StringEqual( value, element->Value() ))) {
					return element->ToElement();
				}
			}
			return 0;
		}

		const XMLElement* XMLNode::PreviousSiblingElement( const char* value ) const
		{
			for( XMLNode* element=_prev; element; element = element->_prev ) {
				if (    element->ToElement()
						&& (!value || XMLUtil::StringEqual( value, element->Value() ))) {
					return element->ToElement();
				}
			}
			return 0;
		}
		
		XMLElement::XMLElement( XMLDocument* doc ) : XMLNode( doc ),
			_closingType( 0 ),
			_rootAttribute( 0 )
		{
		}

		XMLElement::~XMLElement()
		{
			while( _rootAttribute ) {
				XMLAttribute* next = _rootAttribute->_next;
				DELETE_ATTRIBUTE( _rootAttribute );
				_rootAttribute = next;
			}
		}

		XMLAttribute* XMLElement::FindAttribute( const char* name )
		{
			XMLAttribute* a = 0;
			for( a=_rootAttribute; a; a = a->_next ) {
				if ( XMLUtil::StringEqual( a->Name(), name ) ) {
					return a;
				}
			}
			return 0;
		}

		const XMLAttribute* XMLElement::FindAttribute( const char* name ) const
		{
			XMLAttribute* a = 0;
			for( a=_rootAttribute; a; a = a->_next ) {
				if ( XMLUtil::StringEqual( a->Name(), name ) ) {
					return a;
				}
			}
			return 0;
		}

		const char* XMLElement::Attribute( const char* name, const char* value ) const
		{
			const XMLAttribute* a = FindAttribute( name );
			if ( !a ) {
				return 0;
			}
			if ( !value || XMLUtil::StringEqual( a->Value(), value )) {
				return a->Value();
			}
			return 0;
		}

		const char* XMLElement::GetText() const
		{
			if ( FirstChild() && FirstChild()->ToText() ) {
				return FirstChild()->ToText()->Value();
			}
			return 0;
		}

		void XMLElement::SetText( const char* inText )
		{
			if ( FirstChild() && FirstChild()->ToText() )
				FirstChild()->SetValue( inText );
			else {
				XMLText*	theText = GetDocument()->NewText( inText );
				InsertFirstChild( theText );
			}
		}

		void XMLElement::SetText( int v ) 
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			SetText( buf );
		}

		void XMLElement::SetText( unsigned v ) 
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			SetText( buf );
		}

		void XMLElement::SetText( bool v ) 
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			SetText( buf );
		}

		void XMLElement::SetText( float v ) 
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			SetText( buf );
		}

		void XMLElement::SetText( double v ) 
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			SetText( buf );
		}

		XMLError XMLElement::QueryIntText( int* ival ) const
		{
			if ( FirstChild() && FirstChild()->ToText() ) {
				const char* t = FirstChild()->ToText()->Value();
				if ( XMLUtil::ToInt( t, ival ) ) {
					return XML_SUCCESS;
				}
				return XML_CAN_NOT_CONVERT_TEXT;
			}
			return XML_NO_TEXT_NODE;
		}

		XMLError XMLElement::QueryUnsignedText( unsigned* uval ) const
		{
			if ( FirstChild() && FirstChild()->ToText() ) {
				const char* t = FirstChild()->ToText()->Value();
				if ( XMLUtil::ToUnsigned( t, uval ) ) {
					return XML_SUCCESS;
				}
				return XML_CAN_NOT_CONVERT_TEXT;
			}
			return XML_NO_TEXT_NODE;
		}

		XMLError XMLElement::QueryBoolText( bool* bval ) const
		{
			if ( FirstChild() && FirstChild()->ToText() ) {
				const char* t = FirstChild()->ToText()->Value();
				if ( XMLUtil::ToBool( t, bval ) ) {
					return XML_SUCCESS;
				}
				return XML_CAN_NOT_CONVERT_TEXT;
			}
			return XML_NO_TEXT_NODE;
		}

		XMLError XMLElement::QueryDoubleText( double* dval ) const
		{
			if ( FirstChild() && FirstChild()->ToText() ) {
				const char* t = FirstChild()->ToText()->Value();
				if ( XMLUtil::ToDouble( t, dval ) ) {
					return XML_SUCCESS;
				}
				return XML_CAN_NOT_CONVERT_TEXT;
			}
			return XML_NO_TEXT_NODE;
		}

		XMLError XMLElement::QueryFloatText( float* fval ) const
		{
			if ( FirstChild() && FirstChild()->ToText() ) {
				const char* t = FirstChild()->ToText()->Value();
				if ( XMLUtil::ToFloat( t, fval ) ) {
					return XML_SUCCESS;
				}
				return XML_CAN_NOT_CONVERT_TEXT;
			}
			return XML_NO_TEXT_NODE;
		}

		XMLAttribute* XMLElement::FindOrCreateAttribute( const char* name )
		{
			XMLAttribute* last = 0;
			XMLAttribute* attrib = 0;
			for( attrib = _rootAttribute;
					attrib;
					last = attrib, attrib = attrib->_next ) {
				if ( XMLUtil::StringEqual( attrib->Name(), name ) ) {
					break;
				}
			}
			if ( !attrib ) {
				attrib = new (_document->_attributePool.Alloc() ) XMLAttribute();
				attrib->_memPool = &_document->_attributePool;
				if ( last ) {
					last->_next = attrib;
				}
				else {
					_rootAttribute = attrib;
				}
				attrib->SetName( name );
				attrib->_memPool->SetTracked(); // always created and linked.
			}
			return attrib;
		}

		void XMLElement::DeleteAttribute( const char* name )
		{
			XMLAttribute* prev = 0;
			for( XMLAttribute* a=_rootAttribute; a; a=a->_next ) {
				if ( XMLUtil::StringEqual( name, a->Name() ) ) {
					if ( prev ) {
						prev->_next = a->_next;
					}
					else {
						_rootAttribute = a->_next;
					}
					DELETE_ATTRIBUTE( a );
					break;
				}
				prev = a;
			}
		}

		char* XMLElement::ParseAttributes( char* p )
		{
			const char* start = p;
			XMLAttribute* prevAttribute = 0;

			// Read the attributes.
			while( p ) {
				p = XMLUtil::SkipWhiteSpace( p );
				if ( !p || !(*p) ) {
					_document->SetError( XML_ERROR_PARSING_ELEMENT, start, Name() );
					return 0;
				}

				// attribute.
				if (XMLUtil::IsNameStartChar( *p ) ) {
					XMLAttribute* attrib = new (_document->_attributePool.Alloc() ) XMLAttribute();
					attrib->_memPool = &_document->_attributePool;
					attrib->_memPool->SetTracked();

					p = attrib->ParseDeep( p, _document->ProcessEntities() );
					if ( !p || Attribute( attrib->Name() ) ) {
						DELETE_ATTRIBUTE( attrib );
						_document->SetError( XML_ERROR_PARSING_ATTRIBUTE, start, p );
						return 0;
					}
					// There is a minor bug here: if the attribute in the source xml
					// document is duplicated, it will not be detected and the
					// attribute will be doubly added. However, tracking the 'prevAttribute'
					// avoids re-scanning the attribute list. Preferring performance for
					// now, may reconsider in the future.
					if ( prevAttribute ) {
						prevAttribute->_next = attrib;
					}
					else {
						_rootAttribute = attrib;
					}
					prevAttribute = attrib;
				}
				// end of the tag
				else if ( *p == '/' && *(p+1) == '>' ) {
					_closingType = CLOSED;
					return p+2;	// done; sealed element.
				}
				// end of the tag
				else if ( *p == '>' ) {
					++p;
					break;
				}
				else {
					_document->SetError( XML_ERROR_PARSING_ELEMENT, start, p );
					return 0;
				}
			}
			return p;
		}

		//
		//	<ele></ele>
		//	<ele>foo<b>bar</b></ele>
		//
		char* XMLElement::ParseDeep( char* p, StrPair* strPair )
		{
			// Read the element name.
			p = XMLUtil::SkipWhiteSpace( p );
			if ( !p ) {
				return 0;
			}

			// The closing element is the </element> form. It is
			// parsed just like a regular element then deleted from
			// the DOM.
			if ( *p == '/' ) {
				_closingType = CLOSING;
				++p;
			}

			p = _value.ParseName( p );
			if ( _value.Empty() ) {
				return 0;
			}

			p = ParseAttributes( p );
			if ( !p || !*p || _closingType ) {
				return p;
			}

			p = XMLNode::ParseDeep( p, strPair );
			return p;
		}

		XMLNode* XMLElement::ShallowClone( XMLDocument* doc ) const
		{
			if ( !doc ) {
				doc = _document;
			}
			XMLElement* element = doc->NewElement( Value() );					// fixme: this will always allocate memory. Intern?
			for( const XMLAttribute* a=FirstAttribute(); a; a=a->Next() ) {
				element->SetAttribute( a->Name(), a->Value() );					// fixme: this will always allocate memory. Intern?
			}
			return element;
		}

		bool XMLElement::ShallowEqual( const XMLNode* compare ) const
		{
			const XMLElement* other = compare->ToElement();
			if ( other && XMLUtil::StringEqual( other->Value(), Value() )) {

				const XMLAttribute* a=FirstAttribute();
				const XMLAttribute* b=other->FirstAttribute();

				while ( a && b ) {
					if ( !XMLUtil::StringEqual( a->Value(), b->Value() ) ) {
						return false;
					}
					a = a->Next();
					b = b->Next();
				}
				if ( a || b ) {
					// different count
					return false;
				}
				return true;
			}
			return false;
		}

		bool XMLElement::Accept( XMLVisitor* visitor ) const
		{
			if ( visitor->VisitEnter( *this, _rootAttribute ) ) {
				for ( const XMLNode* node=FirstChild(); node; node=node->NextSibling() ) {
					if ( !node->Accept( visitor ) ) {
						break;
					}
				}
			}
			return visitor->VisitExit( *this );
		}
	} // namespace xml
} // namespace fyreactor
