
#pragma once

#include "define.hpp"

namespace fyreactor
{
	namespace xml
	{
		/** An attribute is a name-value pair. Elements have an arbitrary
			number of attributes, each with a unique name.

			@note The attributes are not XMLNodes. You may only query the
			Next() attribute in a list.
		*/
		class XMLAttribute
		{
			friend class XMLElement;
		public:
			/// The name of the attribute.
			const char* Name() const;

			/// The value of the attribute.
			const char* Value() const;

			/// The next attribute in the list.
			const XMLAttribute* Next() const {
				return _next;
			}

			/** IntValue interprets the attribute as an integer, and returns the value.
				If the value isn't an integer, 0 will be returned. There is no error checking;
    			use QueryIntValue() if you need error checking.
			*/
			int		 IntValue() const				{
				int i=0;
				QueryIntValue( &i );
				return i;
			}
			/// Query as an unsigned integer. See IntValue()
			unsigned UnsignedValue() const			{
				unsigned i=0;
				QueryUnsignedValue( &i );
				return i;
			}
			/// Query as a boolean. See IntValue()
			bool	 BoolValue() const				{
				bool b=false;
				QueryBoolValue( &b );
				return b;
			}
			/// Query as a double. See IntValue()
			double 	 DoubleValue() const			{
				double d=0;
				QueryDoubleValue( &d );
				return d;
			}
			/// Query as a float. See IntValue()
			float	 FloatValue() const				{
				float f=0;
				QueryFloatValue( &f );
				return f;
			}

			/** QueryIntValue interprets the attribute as an integer, and returns the value
    			in the provided parameter. The function will return XML_NO_ERROR on success,
    			and XML_WRONG_ATTRIBUTE_TYPE if the conversion is not successful.
			*/
			XMLError QueryIntValue( int* value ) const;
			/// See QueryIntValue
			XMLError QueryUnsignedValue( unsigned int* value ) const;
			/// See QueryIntValue
			XMLError QueryBoolValue( bool* value ) const;
			/// See QueryIntValue
			XMLError QueryDoubleValue( double* value ) const;
			/// See QueryIntValue
			XMLError QueryFloatValue( float* value ) const;

			/// Set the attribute to a string value.
			void SetAttribute( const char* value );
			/// Set the attribute to value.
			void SetAttribute( int value );
			/// Set the attribute to value.
			void SetAttribute( unsigned value );
			/// Set the attribute to value.
			void SetAttribute( bool value );
			/// Set the attribute to value.
			void SetAttribute( double value );
			/// Set the attribute to value.
			void SetAttribute( float value );

		private:
			enum { BUF_SIZE = 200 };

			XMLAttribute() : _next( 0 ), _memPool( 0 ) {}
			virtual ~XMLAttribute()	{}

			XMLAttribute( const XMLAttribute& );	// not supported
			void operator=( const XMLAttribute& );	// not supported
			void SetName( const char* name );

			char* ParseDeep( char* p, bool processEntities );

			mutable StrPair _name;
			mutable StrPair _value;
			XMLAttribute*   _next;
			MemPool*        _memPool;
		};
	}
}
