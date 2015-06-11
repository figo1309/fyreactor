
#include <xml/define.hpp>
#include <xml/xml_util.hpp>
#include <xml/xml_document.hpp>
#include <xml/xml_printer.hpp>
#include <xml/xml_declaration.hpp>
#include <xml/xml_unknown.hpp>

namespace fyreactor
{
	namespace xml
	{
		XMLPrinter::XMLPrinter( FILE* file, bool compact, int depth ) :
			_elementJustOpened( false ),
			_firstElement( true ),
			_fp( file ),
			_depth( depth ),
			_textDepth( -1 ),
			_processEntities( true ),
			_compactMode( compact )
		{
			for( int i=0; i<ENTITY_RANGE; ++i ) {
				_entityFlag[i] = false;
				_restrictedEntityFlag[i] = false;
			}
			for( int i=0; i<NUM_ENTITIES; ++i ) {
				TIXMLASSERT( entities[i].value < ENTITY_RANGE );
				if ( entities[i].value < ENTITY_RANGE ) {
					_entityFlag[ (int)entities[i].value ] = true;
				}
			}
			_restrictedEntityFlag[(int)'&'] = true;
			_restrictedEntityFlag[(int)'<'] = true;
			_restrictedEntityFlag[(int)'>'] = true;	// not required, but consistency is nice
			_buffer.Push( 0 );
		}

		void XMLPrinter::Print( const char* format, ... )
		{
			va_list     va;
			va_start( va, format );

			if ( _fp ) {
				vfprintf( _fp, format, va );
			}
			else {
				// This seems brutally complex. Haven't figured out a better
				// way on windows.
		#ifdef _MSC_VER
				int len = -1;
				int expand = 1000;
				while ( len < 0 ) {
					len = vsnprintf_s( _accumulator.Mem(), _accumulator.Capacity(), _TRUNCATE, format, va );
					if ( len < 0 ) {
						expand *= 3/2;
						_accumulator.PushArr( expand );
					}
				}
				char* p = _buffer.PushArr( len ) - 1;
				memcpy( p, _accumulator.Mem(), len+1 );
		#else
				int len = vsnprintf( 0, 0, format, va );
				// Close out and re-start the va-args
				va_end( va );
				va_start( va, format );
				char* p = _buffer.PushArr( len ) - 1;
				vsnprintf( p, len+1, format, va );
		#endif
			}
			va_end( va );
		}

		void XMLPrinter::PrintSpace( int depth )
		{
			for( int i=0; i<depth; ++i ) {
				Print( "    " );
			}
		}

		void XMLPrinter::PrintString( const char* p, bool restricted )
		{
			// Look for runs of bytes between entities to print.
			const char* q = p;
			const bool* flag = restricted ? _restrictedEntityFlag : _entityFlag;

			if ( _processEntities ) {
				while ( *q ) {
					// Remember, char is sometimes signed. (How many times has that bitten me?)
					if ( *q > 0 && *q < ENTITY_RANGE ) {
						// Check for entities. If one is found, flush
						// the stream up until the entity, write the
						// entity, and keep looking.
						if ( flag[(unsigned)(*q)] ) {
							while ( p < q ) {
								Print( "%c", *p );
								++p;
							}
							for( int i=0; i<NUM_ENTITIES; ++i ) {
								if ( entities[i].value == *q ) {
									Print( "&%s;", entities[i].pattern );
									break;
								}
							}
							++p;
						}
					}
					++q;
				}
			}
			// Flush the remaining string. This will be the entire
			// string if an entity wasn't found.
			if ( !_processEntities || (q-p > 0) ) {
				Print( "%s", p );
			}
		}

		void XMLPrinter::PushHeader( bool writeBOM, bool writeDec )
		{
			if ( writeBOM ) {
				static const char bom[] = { TIXML_UTF_LEAD_0, TIXML_UTF_LEAD_1, TIXML_UTF_LEAD_2, 0 };
				Print( "%s", bom );
			}
			if ( writeDec ) {
				PushDeclaration( "xml version=\"1.0\"" );
			}
		}

		void XMLPrinter::OpenElement( const char* name )
		{
			if ( _elementJustOpened ) {
				SealElement();
			}
			_stack.Push( name );

			if ( _textDepth < 0 && !_firstElement && !_compactMode ) {
				Print( "\n" );
			}
			if ( !_compactMode ) {
				PrintSpace( _depth );
			}

			Print( "<%s", name );
			_elementJustOpened = true;
			_firstElement = false;
			++_depth;
		}

		void XMLPrinter::PushAttribute( const char* name, const char* value )
		{
			TIXMLASSERT( _elementJustOpened );
			Print( " %s=\"", name );
			PrintString( value, false );
			Print( "\"" );
		}

		void XMLPrinter::PushAttribute( const char* name, int v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			PushAttribute( name, buf );
		}

		void XMLPrinter::PushAttribute( const char* name, unsigned v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			PushAttribute( name, buf );
		}

		void XMLPrinter::PushAttribute( const char* name, bool v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			PushAttribute( name, buf );
		}

		void XMLPrinter::PushAttribute( const char* name, double v )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( v, buf, BUF_SIZE );
			PushAttribute( name, buf );
		}

		void XMLPrinter::CloseElement()
		{
			--_depth;
			const char* name = _stack.Pop();

			if ( _elementJustOpened ) {
				Print( "/>" );
			}
			else {
				if ( _textDepth < 0 && !_compactMode) {
					Print( "\n" );
					PrintSpace( _depth );
				}
				Print( "</%s>", name );
			}

			if ( _textDepth == _depth ) {
				_textDepth = -1;
			}
			if ( _depth == 0 && !_compactMode) {
				Print( "\n" );
			}
			_elementJustOpened = false;
		}

		void XMLPrinter::SealElement()
		{
			_elementJustOpened = false;
			Print( ">" );
		}

		void XMLPrinter::PushText( const char* text, bool cdata )
		{
			_textDepth = _depth-1;

			if ( _elementJustOpened ) {
				SealElement();
			}
			if ( cdata ) {
				Print( "<![CDATA[" );
				Print( "%s", text );
				Print( "]]>" );
			}
			else {
				PrintString( text, true );
			}
		}
		
		void XMLPrinter::PushText( int value )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( value, buf, BUF_SIZE );
			PushText( buf, false );
		}

		void XMLPrinter::PushText( unsigned value )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( value, buf, BUF_SIZE );
			PushText( buf, false );
		}

		void XMLPrinter::PushText( bool value )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( value, buf, BUF_SIZE );
			PushText( buf, false );
		}

		void XMLPrinter::PushText( float value )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( value, buf, BUF_SIZE );
			PushText( buf, false );
		}

		void XMLPrinter::PushText( double value )
		{
			char buf[BUF_SIZE];
			XMLUtil::ToStr( value, buf, BUF_SIZE );
			PushText( buf, false );
		}

		void XMLPrinter::PushComment( const char* comment )
		{
			if ( _elementJustOpened ) {
				SealElement();
			}
			if ( _textDepth < 0 && !_firstElement && !_compactMode) {
				Print( "\n" );
				PrintSpace( _depth );
			}
			_firstElement = false;
			Print( "<!--%s-->", comment );
		}

		void XMLPrinter::PushDeclaration( const char* value )
		{
			if ( _elementJustOpened ) {
				SealElement();
			}
			if ( _textDepth < 0 && !_firstElement && !_compactMode) {
				Print( "\n" );
				PrintSpace( _depth );
			}
			_firstElement = false;
			Print( "<?%s?>", value );
		}

		void XMLPrinter::PushUnknown( const char* value )
		{
			if ( _elementJustOpened ) {
				SealElement();
			}
			if ( _textDepth < 0 && !_firstElement && !_compactMode) {
				Print( "\n" );
				PrintSpace( _depth );
			}
			_firstElement = false;
			Print( "<!%s>", value );
		}

		bool XMLPrinter::VisitEnter( const XMLDocument& doc )
		{
			_processEntities = doc.ProcessEntities();
			if ( doc.HasBOM() ) {
				PushHeader( true, false );
			}
			return true;
		}

		bool XMLPrinter::VisitEnter( const XMLElement& element, const XMLAttribute* attribute )
		{
			OpenElement( element.Name() );
			while ( attribute ) {
				PushAttribute( attribute->Name(), attribute->Value() );
				attribute = attribute->Next();
			}
			return true;
		}

		bool XMLPrinter::VisitExit( const XMLElement& )
		{
			CloseElement();
			return true;
		}

		bool XMLPrinter::Visit( const XMLText& text )
		{
			PushText( text.Value(), text.CData() );
			return true;
		}

		bool XMLPrinter::Visit( const XMLComment& comment )
		{
			PushComment( comment.Value() );
			return true;
		}

		bool XMLPrinter::Visit( const XMLDeclaration& declaration )
		{
			PushDeclaration( declaration.Value() );
			return true;
		}

		bool XMLPrinter::Visit( const XMLUnknown& unknown )
		{
			PushUnknown( unknown.Value() );
			return true;
		}

	} // namespace xml
} // namespace fyreactor
