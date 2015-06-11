
#include <xml/define.hpp>
#include <xml/xml_util.hpp>

namespace fyreactor
{
	namespace xml
	{
		const char* XMLUtil::ReadBOM( const char* p, bool* bom )
		{
			*bom = false;
			const char* pu = reinterpret_cast<const char*>(p);
			// Check for BOM:
			if (    *(pu+0) == TIXML_UTF_LEAD_0
					&& *(pu+1) == TIXML_UTF_LEAD_1
					&& *(pu+2) == TIXML_UTF_LEAD_2 ) {
				*bom = true;
				p += 3;
			}
			return p;
		}

		void XMLUtil::ConvertUTF32ToUTF8( unsigned long input, char* output, int* length )
		{
			const unsigned long BYTE_MASK = 0xBF;
			const unsigned long BYTE_MARK = 0x80;
			const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

			if (input < 0x80) {
				*length = 1;
			}
			else if ( input < 0x800 ) {
				*length = 2;
			}
			else if ( input < 0x10000 ) {
				*length = 3;
			}
			else if ( input < 0x200000 ) {
				*length = 4;
			}
			else {
				*length = 0;    // This code won't covert this correctly anyway.
				return;
			}

			output += *length;

			// Scary scary fall throughs.
			switch (*length) {
				case 4:
					--output;
					*output = (char)((input | BYTE_MARK) & BYTE_MASK);
					input >>= 6;
				case 3:
					--output;
					*output = (char)((input | BYTE_MARK) & BYTE_MASK);
					input >>= 6;
				case 2:
					--output;
					*output = (char)((input | BYTE_MARK) & BYTE_MASK);
					input >>= 6;
				case 1:
					--output;
					*output = (char)(input | FIRST_BYTE_MARK[*length]);
				default:
					break;
			}
		}

		const char* XMLUtil::GetCharacterRef( const char* p, char* value, int* length )
		{
			// Presume an entity, and pull it out.
			*length = 0;

			if ( *(p+1) == '#' && *(p+2) ) {
				unsigned long ucs = 0;
				ptrdiff_t delta = 0;
				unsigned mult = 1;

				if ( *(p+2) == 'x' ) {
					// Hexadecimal.
					if ( !*(p+3) ) {
						return 0;
					}

					const char* q = p+3;
					q = strchr( q, ';' );

					if ( !q || !*q ) {
						return 0;
					}

					delta = q-p;
					--q;

					while ( *q != 'x' ) {
						if ( *q >= '0' && *q <= '9' ) {
							ucs += mult * (*q - '0');
						}
						else if ( *q >= 'a' && *q <= 'f' ) {
							ucs += mult * (*q - 'a' + 10);
						}
						else if ( *q >= 'A' && *q <= 'F' ) {
							ucs += mult * (*q - 'A' + 10 );
						}
						else {
							return 0;
						}
						mult *= 16;
						--q;
					}
				}
				else {
					// Decimal.
					if ( !*(p+2) ) {
						return 0;
					}

					const char* q = p+2;
					q = strchr( q, ';' );

					if ( !q || !*q ) {
						return 0;
					}

					delta = q-p;
					--q;

					while ( *q != '#' ) {
						if ( *q >= '0' && *q <= '9' ) {
							ucs += mult * (*q - '0');
						}
						else {
							return 0;
						}
						mult *= 10;
						--q;
					}
				}
				// convert the UCS to UTF-8
				ConvertUTF32ToUTF8( ucs, value, length );
				return p + delta + 1;
			}
			return p+1;
		}

		void XMLUtil::ToStr( int v, char* buffer, int bufferSize )
		{
			TIXML_SNPRINTF( buffer, bufferSize, "%d", v );
		}


		void XMLUtil::ToStr( unsigned v, char* buffer, int bufferSize )
		{
			TIXML_SNPRINTF( buffer, bufferSize, "%u", v );
		}


		void XMLUtil::ToStr( bool v, char* buffer, int bufferSize )
		{
			TIXML_SNPRINTF( buffer, bufferSize, "%d", v ? 1 : 0 );
		}

		/*
			ToStr() of a number is a very tricky topic.
			https://github.com/leethomason/tinyxml2/issues/106
		*/
		void XMLUtil::ToStr( float v, char* buffer, int bufferSize )
		{
			TIXML_SNPRINTF( buffer, bufferSize, "%.8g", v );
		}


		void XMLUtil::ToStr( double v, char* buffer, int bufferSize )
		{
			TIXML_SNPRINTF( buffer, bufferSize, "%.17g", v );
		}


		bool XMLUtil::ToInt( const char* str, int* value )
		{
			if ( TIXML_SSCANF( str, "%d", value ) == 1 ) {
				return true;
			}
			return false;
		}

		bool XMLUtil::ToUnsigned( const char* str, unsigned *value )
		{
			if ( TIXML_SSCANF( str, "%u", value ) == 1 ) {
				return true;
			}
			return false;
		}

		bool XMLUtil::ToBool( const char* str, bool* value )
		{
			int ival = 0;
			if ( ToInt( str, &ival )) {
				*value = (ival==0) ? false : true;
				return true;
			}
			if ( StringEqual( str, "true" ) ) {
				*value = true;
				return true;
			}
			else if ( StringEqual( str, "false" ) ) {
				*value = false;
				return true;
			}
			return false;
		}

		bool XMLUtil::ToFloat( const char* str, float* value )
		{
			if ( TIXML_SSCANF( str, "%f", value ) == 1 ) {
				return true;
			}
			return false;
		}

		bool XMLUtil::ToDouble( const char* str, double* value )
		{
			if ( TIXML_SSCANF( str, "%lf", value ) == 1 ) {
				return true;
			}
			return false;
		}
	} // namespace xml
} // namespace fyreactor
