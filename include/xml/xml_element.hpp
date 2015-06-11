
#pragma once

#include "define.hpp"
#include "xml_node.hpp"
#include "xml_attribute.hpp"
#include "xml_visitor.hpp"

namespace fyreactor
{
	namespace xml
	{
		/** The element is a container class. It has a value, the element name,
			and can contain other elements, text, comments, and unknowns.
			Elements also contain an arbitrary number of attributes.
		*/
		class XMLElement : public XMLNode
		{
			friend class XMLBase;
			friend class XMLDocument;

		public:
			/// Get the name of an element (which is the Value() of the node.)
			const char* Name() const		{
				return Value();
			}

			/// Set the name of the element.
			void SetName( const char* str, bool staticMem=false )	{
				SetValue( str, staticMem );
			}

			virtual XMLElement* ToElement()				{
				return this;
			}

			virtual const XMLElement* ToElement() const {
				return this;
			}

			virtual bool Accept( XMLVisitor* visitor ) const;

			/** Given an attribute name, Attribute() returns the value
    			for the attribute of that name, or null if none
    			exists. For example:

    			@verbatim
    			const char* value = ele->Attribute( "foo" );
    			@endverbatim

    			The 'value' parameter is normally null. However, if specified,
    			the attribute will only be returned if the 'name' and 'value'
    			match. This allow you to write code:

    			@verbatim
    			if ( ele->Attribute( "foo", "bar" ) ) callFooIsBar();
    			@endverbatim

    			rather than:
    			@verbatim
    			if ( ele->Attribute( "foo" ) ) {
    				if ( strcmp( ele->Attribute( "foo" ), "bar" ) == 0 ) callFooIsBar();
    			}
    			@endverbatim
			*/
			const char* Attribute( const char* name, const char* value=0 ) const;

			/** Given an attribute name, IntAttribute() returns the value
    			of the attribute interpreted as an integer. 0 will be
    			returned if there is an error. For a method with error
    			checking, see QueryIntAttribute()
			*/
			int		 IntAttribute( const char* name ) const		{
				int i=0;
				QueryIntAttribute( name, &i );
				return i;
			}
			/// See IntAttribute()
			unsigned UnsignedAttribute( const char* name ) const {
				unsigned i=0;
				QueryUnsignedAttribute( name, &i );
				return i;
			}
			/// See IntAttribute()
			bool	 BoolAttribute( const char* name ) const	{
				bool b=false;
				QueryBoolAttribute( name, &b );
				return b;
			}
			/// See IntAttribute()
			double 	 DoubleAttribute( const char* name ) const	{
				double d=0;
				QueryDoubleAttribute( name, &d );
				return d;
			}
			/// See IntAttribute()
			float	 FloatAttribute( const char* name ) const	{
				float f=0;
				QueryFloatAttribute( name, &f );
				return f;
			}

			/** Given an attribute name, QueryIntAttribute() returns
    			XML_NO_ERROR, XML_WRONG_ATTRIBUTE_TYPE if the conversion
    			can't be performed, or XML_NO_ATTRIBUTE if the attribute
    			doesn't exist. If successful, the result of the conversion
    			will be written to 'value'. If not successful, nothing will
    			be written to 'value'. This allows you to provide default
    			value:

    			@verbatim
    			int value = 10;
    			QueryIntAttribute( "foo", &value );		// if "foo" isn't found, value will still be 10
    			@endverbatim
			*/
			XMLError QueryIntAttribute( const char* name, int* value ) const				{
				const XMLAttribute* a = FindAttribute( name );
				if ( !a ) {
					return XML_NO_ATTRIBUTE;
				}
				return a->QueryIntValue( value );
			}
			/// See QueryIntAttribute()
			XMLError QueryUnsignedAttribute( const char* name, unsigned int* value ) const	{
				const XMLAttribute* a = FindAttribute( name );
				if ( !a ) {
					return XML_NO_ATTRIBUTE;
				}
				return a->QueryUnsignedValue( value );
			}
			/// See QueryIntAttribute()
			XMLError QueryBoolAttribute( const char* name, bool* value ) const				{
				const XMLAttribute* a = FindAttribute( name );
				if ( !a ) {
					return XML_NO_ATTRIBUTE;
				}
				return a->QueryBoolValue( value );
			}
			/// See QueryIntAttribute()
			XMLError QueryDoubleAttribute( const char* name, double* value ) const			{
				const XMLAttribute* a = FindAttribute( name );
				if ( !a ) {
					return XML_NO_ATTRIBUTE;
				}
				return a->QueryDoubleValue( value );
			}
			/// See QueryIntAttribute()
			XMLError QueryFloatAttribute( const char* name, float* value ) const			{
				const XMLAttribute* a = FindAttribute( name );
				if ( !a ) {
					return XML_NO_ATTRIBUTE;
				}
				return a->QueryFloatValue( value );
			}

	
			/** Given an attribute name, QueryAttribute() returns
    			XML_NO_ERROR, XML_WRONG_ATTRIBUTE_TYPE if the conversion
    			can't be performed, or XML_NO_ATTRIBUTE if the attribute
    			doesn't exist. It is overloaded for the primitive types,
				and is a generally more convenient replacement of
				QueryIntAttribute() and related functions.
		
				If successful, the result of the conversion
    			will be written to 'value'. If not successful, nothing will
    			be written to 'value'. This allows you to provide default
    			value:

    			@verbatim
    			int value = 10;
    			QueryAttribute( "foo", &value );		// if "foo" isn't found, value will still be 10
    			@endverbatim
			*/
			int QueryAttribute( const char* name, int* value ) const {
				return QueryIntAttribute( name, value );
			}

			int QueryAttribute( const char* name, unsigned int* value ) const {
				return QueryUnsignedAttribute( name, value );
			}

			int QueryAttribute( const char* name, bool* value ) const {
				return QueryBoolAttribute( name, value );
			}

			int QueryAttribute( const char* name, double* value ) const {
				return QueryDoubleAttribute( name, value );
			}

			int QueryAttribute( const char* name, float* value ) const {
				return QueryFloatAttribute( name, value );
			}

			/// Sets the named attribute to value.
			void SetAttribute( const char* name, const char* value )	{
				XMLAttribute* a = FindOrCreateAttribute( name );
				a->SetAttribute( value );
			}
			/// Sets the named attribute to value.
			void SetAttribute( const char* name, int value )			{
				XMLAttribute* a = FindOrCreateAttribute( name );
				a->SetAttribute( value );
			}
			/// Sets the named attribute to value.
			void SetAttribute( const char* name, unsigned value )		{
				XMLAttribute* a = FindOrCreateAttribute( name );
				a->SetAttribute( value );
			}
			/// Sets the named attribute to value.
			void SetAttribute( const char* name, bool value )			{
				XMLAttribute* a = FindOrCreateAttribute( name );
				a->SetAttribute( value );
			}
			/// Sets the named attribute to value.
			void SetAttribute( const char* name, double value )		{
				XMLAttribute* a = FindOrCreateAttribute( name );
				a->SetAttribute( value );
			}
			/// Sets the named attribute to value.
			void SetAttribute( const char* name, float value )		{
				XMLAttribute* a = FindOrCreateAttribute( name );
				a->SetAttribute( value );
			}

			/**
    			Delete an attribute.
			*/
			void DeleteAttribute( const char* name );

			/// Return the first attribute in the list.
			const XMLAttribute* FirstAttribute() const {
				return _rootAttribute;
			}
			/// Query a specific attribute in the list.
			const XMLAttribute* FindAttribute( const char* name ) const;

			/** Convenience function for easy access to the text inside an element. Although easy
    			and concise, GetText() is limited compared to getting the XMLText child
    			and accessing it directly.

    			If the first child of 'this' is a XMLText, the GetText()
    			returns the character string of the Text node, else null is returned.

    			This is a convenient method for getting the text of simple contained text:
    			@verbatim
    			<foo>This is text</foo>
    				const char* str = fooElement->GetText();
    			@endverbatim

    			'str' will be a pointer to "This is text".

    			Note that this function can be misleading. If the element foo was created from
    			this XML:
    			@verbatim
    				<foo><b>This is text</b></foo>
    			@endverbatim

    			then the value of str would be null. The first child node isn't a text node, it is
    			another element. From this XML:
    			@verbatim
    				<foo>This is <b>text</b></foo>
    			@endverbatim
    			GetText() will return "This is ".
			*/
			const char* GetText() const;

			/** Convenience function for easy access to the text inside an element. Although easy
    			and concise, SetText() is limited compared to creating an XMLText child
    			and mutating it directly.

    			If the first child of 'this' is a XMLText, SetText() sets its value to
				the given string, otherwise it will create a first child that is an XMLText.

    			This is a convenient method for setting the text of simple contained text:
    			@verbatim
    			<foo>This is text</foo>
    				fooElement->SetText( "Hullaballoo!" );
     			<foo>Hullaballoo!</foo>
				@endverbatim

    			Note that this function can be misleading. If the element foo was created from
    			this XML:
    			@verbatim
    				<foo><b>This is text</b></foo>
    			@endverbatim

    			then it will not change "This is text", but rather prefix it with a text element:
    			@verbatim
    				<foo>Hullaballoo!<b>This is text</b></foo>
    			@endverbatim
		
				For this XML:
    			@verbatim
    				<foo />
    			@endverbatim
    			SetText() will generate
    			@verbatim
    				<foo>Hullaballoo!</foo>
    			@endverbatim
			*/
			void SetText( const char* inText );
			/// Convenince method for setting text inside and element. See SetText() for important limitations.
			void SetText( int value );
			/// Convenince method for setting text inside and element. See SetText() for important limitations.
			void SetText( unsigned value );  
			/// Convenince method for setting text inside and element. See SetText() for important limitations.
			void SetText( bool value );  
			/// Convenince method for setting text inside and element. See SetText() for important limitations.
			void SetText( double value );  
			/// Convenince method for setting text inside and element. See SetText() for important limitations.
			void SetText( float value );  

			/**
    			Convenience method to query the value of a child text node. This is probably best
    			shown by example. Given you have a document is this form:
    			@verbatim
    				<point>
    					<x>1</x>
    					<y>1.4</y>
    				</point>
    			@endverbatim

    			The QueryIntText() and similar functions provide a safe and easier way to get to the
    			"value" of x and y.

    			@verbatim
    				int x = 0;
    				float y = 0;	// types of x and y are contrived for example
    				const XMLElement* xElement = pointElement->FirstChildElement( "x" );
    				const XMLElement* yElement = pointElement->FirstChildElement( "y" );
    				xElement->QueryIntText( &x );
    				yElement->QueryFloatText( &y );
    			@endverbatim

    			@returns XML_SUCCESS (0) on success, XML_CAN_NOT_CONVERT_TEXT if the text cannot be converted
    					 to the requested type, and XML_NO_TEXT_NODE if there is no child text to query.

			*/
			XMLError QueryIntText( int* ival ) const;
			/// See QueryIntText()
			XMLError QueryUnsignedText( unsigned* uval ) const;
			/// See QueryIntText()
			XMLError QueryBoolText( bool* bval ) const;
			/// See QueryIntText()
			XMLError QueryDoubleText( double* dval ) const;
			/// See QueryIntText()
			XMLError QueryFloatText( float* fval ) const;

			// internal:
			enum {
				OPEN,		// <foo>
				CLOSED,		// <foo/>
				CLOSING		// </foo>
			};
			int ClosingType() const {
				return _closingType;
			}
			char* ParseDeep( char* p, StrPair* endTag );
			virtual XMLNode* ShallowClone( XMLDocument* document ) const;
			virtual bool ShallowEqual( const XMLNode* compare ) const;

		private:
			XMLElement( XMLDocument* doc );
			virtual ~XMLElement();
			XMLElement( const XMLElement& );	// not supported
			void operator=( const XMLElement& );	// not supported

			XMLAttribute* FindAttribute( const char* name );
			XMLAttribute* FindOrCreateAttribute( const char* name );
			//void LinkAttribute( XMLAttribute* attrib );
			char* ParseAttributes( char* p );

			enum { BUF_SIZE = 200 };
			int _closingType;
			// The attribute list is ordered; there is no 'lastAttribute'
			// because the list needs to be scanned for dupes before adding
			// a new attribute.
			XMLAttribute* _rootAttribute;
		};
	}
}
