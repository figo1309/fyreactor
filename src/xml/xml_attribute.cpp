
#include <xml/define.hpp>
#include <xml/xml_util.hpp>
#include <xml/xml_attribute.hpp>

namespace fyreactor
{
	namespace xml
	{
		const char* XMLAttribute::Name() const 
		{
			return _name.GetStr();
		}

		const char* XMLAttribute::Value() const 
		{
			return _value.GetStr();
		}

		char* XMLAttribute::ParseDeep( char* p, bool processEntities )
		{
			// Parse using the name rules: bug fix, was using ParseText before
			p = _name.ParseName( p );
			if ( !p || !*p ) {
				return 0;
			}

			// Skip white space before =
			p = XMLUtil::SkipWhiteSpace( p );
			if ( !p || *p != '=' ) {
				return 0;
			}

			++p;	// move up to opening quote
			p = XMLUtil::SkipWhiteSpace( p );
			if ( *p != '\"' && *p != '\'' ) {
				return 0;
			}

			char endTag[2] = { *p, 0 };
			++p;	// move past opening quote

			p = _value.ParseText( p, endTag, processEntities ? StrPair::ATTRIBUTE_VALUE : StrPair::ATTRIBUTE_VALUE_LEAVE_ENTITIES );
			return p;
		}


		void XMLAttribute::SetName( const char* n )
		{
			_name.SetStr( n );
		}


		XMLError XMLAttribute::QueryIntValue( int* value ) const
		{
			if ( XMLUtil::ToInt( Value(), value )) {
				return XML_NO_ERROR;
			}
			return XML_WRONG_ATTRIBUTE_TYPE;
		}


		XMLError XMLAttribute::QueryUnsignedValue( unsigned int* value ) const
		{
			if ( XMLUtil::ToUnsigned( Value(), value )) {
				return XML_NO_ERROR;
			}
			return XML_WRONG_ATTRIBUTE_TYPE;
		}


		XMLError XMLAttribute::QueryBoolValue( bool* value ) const
		{
			if ( XMLUtil::ToBool( Value(), value )) {
				return XML_NO_ERROR;
			}
			return XML_WRONG_ATTRIBUTE_TYPE;
		}


		XMLError XMLAttribute::QueryFloatValue( float* value ) const
		{
			if ( XMLUtil::ToFloat( Value(), value )) {
				return XML_NO_ERROR;
			}
			return XML_WRONG_ATTRIBUTE_TYPE;
		}


		XMLError XMLAttribute::QueryDoubleValue( double* value ) const
		{
			if ( XMLUtil::ToDouble( Value(), value )) {
				return XML_NO_ERROR;
			}
			return XML_WRONG_ATTRIBUTE_TYPE;
		}


		void XMLAttribute::SetAttribute( const char* v )
		{
			_value.SetStr( v );
		}


		void XMLAttribute::SetAttribute( int v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			_value.SetStr( buf );
		}


		void XMLAttribute::SetAttribute( unsigned v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			_value.SetStr( buf );
		}


		void XMLAttribute::SetAttribute( bool v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			_value.SetStr( buf );
		}

		void XMLAttribute::SetAttribute( double v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			_value.SetStr( buf );
		}

		void XMLAttribute::SetAttribute( float v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			_value.SetStr( buf );
		}
	} // namespace xml
} // namespace fyreactor
