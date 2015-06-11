
#pragma once

#include "define.hpp"
	
namespace fyreactor
{
	namespace xml
	{
		/*
			Utility functionality.
		*/
		class XMLUtil
		{
		public:
			// Anything in the high order range of UTF-8 is assumed to not be whitespace. This isn't
			// correct, but simple, and usually works.
			static const char* SkipWhiteSpace( const char* p )	{
				while( !IsUTF8Continuation(*p) && isspace( *reinterpret_cast<const char*>(p) ) ) {
					++p;
				}
				return p;
			}
			static char* SkipWhiteSpace( char* p )				{
				while( !IsUTF8Continuation(*p) && isspace( *reinterpret_cast<char*>(p) ) )		{
					++p;
				}
				return p;
			}
			static bool IsWhiteSpace( char p )					{
				return !IsUTF8Continuation(p) && isspace( static_cast<char>(p) );
			}
    
			inline static bool IsNameStartChar( char ch ) {
                return isalpha(ch)
					   || ch == ':'
					   || ch == '_';
			}
    
			inline static bool IsNameChar( char ch ) {
				return IsNameStartChar( ch )
					   || isdigit( ch )
					   || ch == '.'
					   || ch == '-';
			}

			inline static bool StringEqual( const char* p, const char* q, int nChar=INT_MAX )  {
				int n = 0;
				if ( p == q ) {
					return true;
				}
				while( *p && *q && *p == *q && n<nChar ) {
					++p;
					++q;
					++n;
				}
				if ( (n == nChar) || ( *p == 0 && *q == 0 ) ) {
					return true;
				}
				return false;
			}
    
			inline static int IsUTF8Continuation( const char p ) {
				return p & 0x80;
			}

			static const char* ReadBOM( const char* p, bool* hasBOM );
			// p is the starting location,
			// the UTF-8 value of the entity will be placed in value, and length filled in.
			static const char* GetCharacterRef( const char* p, char* value, int* length );
			static void ConvertUTF32ToUTF8( unsigned long input, char* output, int* length );

			// converts primitive types to strings
			static void ToStr( int v, char* buffer, int bufferSize );
			static void ToStr( unsigned v, char* buffer, int bufferSize );
			static void ToStr( bool v, char* buffer, int bufferSize );
			static void ToStr( float v, char* buffer, int bufferSize );
			static void ToStr( double v, char* buffer, int bufferSize );

			// converts strings to primitive types
			static bool	ToInt( const char* str, int* value );
			static bool ToUnsigned( const char* str, unsigned* value );
			static bool	ToBool( const char* str, bool* value );
			static bool	ToFloat( const char* str, float* value );
			static bool ToDouble( const char* str, double* value );
		};
	}
}
