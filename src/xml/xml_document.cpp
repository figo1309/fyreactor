
#include <xml/define.hpp>
#include <xml/xml_util.hpp>
#include <xml/xml_document.hpp>
#include <xml/xml_declaration.hpp>
#include <xml/xml_unknown.hpp>
#include <xml/xml_visitor.hpp>
#include <xml/xml_printer.hpp>

namespace fyreactor
{
	namespace xml
	{
		char* XMLDocument::Identify( char* p, XMLNode** node )
		{
			XMLNode* returnNode = 0;
			char* start = p;
			p = XMLUtil::SkipWhiteSpace( p );
			if( !p || !*p ) {
				return p;
			}

			// What is this thing?
			// These strings define the matching patters:
			static const char* xmlHeader		= { "<?" };
			static const char* commentHeader	= { "<!--" };
			static const char* dtdHeader		= { "<!" };
			static const char* cdataHeader		= { "<![CDATA[" };
			static const char* elementHeader	= { "<" };	// and a header for everything else; check last.

			static const int xmlHeaderLen		= 2;
			static const int commentHeaderLen	= 4;
			static const int dtdHeaderLen		= 2;
			static const int cdataHeaderLen		= 9;
			static const int elementHeaderLen	= 1;

		#if defined(_MSC_VER)
		#pragma warning ( push )
		#pragma warning ( disable : 4127 )
		#endif
			TIXMLASSERT( sizeof( XMLComment ) == sizeof( XMLUnknown ) );		// use same memory pool
			TIXMLASSERT( sizeof( XMLComment ) == sizeof( XMLDeclaration ) );	// use same memory pool
		#if defined(_MSC_VER)
		#pragma warning (pop)
		#endif
			if ( XMLUtil::StringEqual( p, xmlHeader, xmlHeaderLen ) ) {
				returnNode = new (_commentPool.Alloc()) XMLDeclaration( this );
				returnNode->_memPool = &_commentPool;
				p += xmlHeaderLen;
			}
			else if ( XMLUtil::StringEqual( p, commentHeader, commentHeaderLen ) ) {
				returnNode = new (_commentPool.Alloc()) XMLComment( this );
				returnNode->_memPool = &_commentPool;
				p += commentHeaderLen;
			}
			else if ( XMLUtil::StringEqual( p, cdataHeader, cdataHeaderLen ) ) {
				XMLText* text = new (_textPool.Alloc()) XMLText( this );
				returnNode = text;
				returnNode->_memPool = &_textPool;
				p += cdataHeaderLen;
				text->SetCData( true );
			}
			else if ( XMLUtil::StringEqual( p, dtdHeader, dtdHeaderLen ) ) {
				returnNode = new (_commentPool.Alloc()) XMLUnknown( this );
				returnNode->_memPool = &_commentPool;
				p += dtdHeaderLen;
			}
			else if ( XMLUtil::StringEqual( p, elementHeader, elementHeaderLen ) ) {
				returnNode = new (_elementPool.Alloc()) XMLElement( this );
				returnNode->_memPool = &_elementPool;
				p += elementHeaderLen;
			}
			else {
				returnNode = new (_textPool.Alloc()) XMLText( this );
				returnNode->_memPool = &_textPool;
				p = start;	// Back it up, all the text counts.
			}

			*node = returnNode;
			return p;
		}


		bool XMLDocument::Accept( XMLVisitor* visitor ) const
		{
			if ( visitor->VisitEnter( *this ) ) {
				for ( const XMLNode* node=FirstChild(); node; node=node->NextSibling() ) {
					if ( !node->Accept( visitor ) ) {
						break;
					}
				}
			}
			return visitor->VisitExit( *this );
		}

		XMLDocument::XMLDocument( bool processEntities, Whitespace whitespace ) :
			XMLNode( 0 ),
			_writeBOM( false ),
			_processEntities( processEntities ),
			_errorID( XML_NO_ERROR ),
			_whitespace( whitespace ),
			_errorStr1( 0 ),
			_errorStr2( 0 ),
			_charBuffer( 0 )
		{
			_document = this;	// avoid warning about 'this' in initializer list
		}


		XMLDocument::~XMLDocument()
		{
			DeleteChildren();
			delete [] _charBuffer;

		#if 0
			_textPool.Trace( "text" );
			_elementPool.Trace( "element" );
			_commentPool.Trace( "comment" );
			_attributePool.Trace( "attribute" );
		#endif

		#ifdef DEBUG
			if ( Error() == false ) {
				TIXMLASSERT( _elementPool.CurrentAllocs()   == _elementPool.Untracked() );
				TIXMLASSERT( _attributePool.CurrentAllocs() == _attributePool.Untracked() );
				TIXMLASSERT( _textPool.CurrentAllocs()      == _textPool.Untracked() );
				TIXMLASSERT( _commentPool.CurrentAllocs()   == _commentPool.Untracked() );
			}
		#endif
		}


		void XMLDocument::Clear()
		{
			DeleteChildren();

			_errorID = XML_NO_ERROR;
			_errorStr1 = 0;
			_errorStr2 = 0;

			delete [] _charBuffer;
			_charBuffer = 0;
		}


		XMLElement* XMLDocument::NewElement( const char* name )
		{
			XMLElement* ele = new (_elementPool.Alloc()) XMLElement( this );
			ele->_memPool = &_elementPool;
			ele->SetName( name );
			return ele;
		}

		XMLComment* XMLDocument::NewComment( const char* str )
		{
			XMLComment* comment = new (_commentPool.Alloc()) XMLComment( this );
			comment->_memPool = &_commentPool;
			comment->SetValue( str );
			return comment;
		}

		XMLText* XMLDocument::NewText( const char* str )
		{
			XMLText* text = new (_textPool.Alloc()) XMLText( this );
			text->_memPool = &_textPool;
			text->SetValue( str );
			return text;
		}

		XMLDeclaration* XMLDocument::NewDeclaration( const char* str )
		{
			XMLDeclaration* dec = new (_commentPool.Alloc()) XMLDeclaration( this );
			dec->_memPool = &_commentPool;
			dec->SetValue( str ? str : "xml version=\"1.0\" encoding=\"UTF-8\"" );
			return dec;
		}

		XMLUnknown* XMLDocument::NewUnknown( const char* str )
		{
			XMLUnknown* unk = new (_commentPool.Alloc()) XMLUnknown( this );
			unk->_memPool = &_commentPool;
			unk->SetValue( str );
			return unk;
		}

		XMLError XMLDocument::LoadFile( const char* filename )
		{
			Clear();
			FILE* fp = 0;

		#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
			errno_t err = fopen_s(&fp, filename, "rb" );
			if ( !fp || err) {
		#else
			fp = fopen( filename, "rb" );
			if ( !fp) {
		#endif
				SetError( XML_ERROR_FILE_NOT_FOUND, filename, 0 );
				return _errorID;
			}
			LoadFile( fp );
			fclose( fp );
			return _errorID;
		}

		XMLError XMLDocument::LoadFile( FILE* fp )
		{
			Clear();

			fseek( fp, 0, SEEK_SET );
			fgetc( fp );
			if ( ferror( fp ) != 0 ) {
				SetError( XML_ERROR_FILE_READ_ERROR, 0, 0 );
				return _errorID;
			}

			fseek( fp, 0, SEEK_END );
			size_t size = ftell( fp );
			fseek( fp, 0, SEEK_SET );

			if ( size == 0 ) {
				SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
				return _errorID;
			}

			_charBuffer = new char[size+1];
			size_t read = fread( _charBuffer, 1, size, fp );
			if ( read != size ) {
				SetError( XML_ERROR_FILE_READ_ERROR, 0, 0 );
				return _errorID;
			}

			_charBuffer[size] = 0;

			const char* p = _charBuffer;
			p = XMLUtil::SkipWhiteSpace( p );
			p = XMLUtil::ReadBOM( p, &_writeBOM );
			if ( !p || !*p ) {
				SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
				return _errorID;
			}

			ParseDeep( _charBuffer + (p-_charBuffer), 0 );
			return _errorID;
		}

		XMLError XMLDocument::SaveFile( const char* filename, bool compact )
		{
			FILE* fp = 0;
		#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
			errno_t err = fopen_s(&fp, filename, "w" );
			if ( !fp || err) {
		#else
			fp = fopen( filename, "w" );
			if ( !fp) {
		#endif
				SetError( XML_ERROR_FILE_COULD_NOT_BE_OPENED, filename, 0 );
				return _errorID;
			}
			SaveFile(fp, compact);
			fclose( fp );
			return _errorID;
		}

		XMLError XMLDocument::SaveFile( FILE* fp, bool compact )
		{
			XMLPrinter stream( fp, compact );
			Print( &stream );
			return _errorID;
		}

		XMLError XMLDocument::Parse( const char* p, size_t len )
		{
			const char* start = p;
			Clear();

			if ( len == 0 ) {
				SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
				return _errorID;
			}

			if ( !p || !*p ) {
				SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
				return _errorID;
			}
			if ( len == (size_t)(-1) ) {
				len = strlen( p );
			}
			_charBuffer = new char[ len+1 ];
			memcpy( _charBuffer, p, len );
			_charBuffer[len] = 0;

			p = XMLUtil::SkipWhiteSpace( p );
			p = XMLUtil::ReadBOM( p, &_writeBOM );
			if ( !p || !*p ) {
				SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
				return _errorID;
			}

			ptrdiff_t delta = p - start;	// skip initial whitespace, BOM, etc.
			ParseDeep( _charBuffer+delta, 0 );
			return _errorID;
		}

		void XMLDocument::Print( XMLPrinter* streamer ) const
		{
			XMLPrinter stdStreamer( stdout );
			if ( !streamer ) {
				streamer = &stdStreamer;
			}
			Accept( streamer );
		}

		void XMLDocument::SetError( XMLError error, const char* str1, const char* str2 )
		{
			_errorID = error;
			_errorStr1 = str1;
			_errorStr2 = str2;
		}

		void XMLDocument::PrintError() const
		{
			if ( _errorID ) {
				static const int LEN = 20;
				char buf1[LEN] = { 0 };
				char buf2[LEN] = { 0 };

				if ( _errorStr1 ) {
					TIXML_SNPRINTF( buf1, LEN, "%s", _errorStr1 );
				}
				if ( _errorStr2 ) {
					TIXML_SNPRINTF( buf2, LEN, "%s", _errorStr2 );
				}

				printf( "XMLDocument error id=%d str1=%s str2=%s\n",
						_errorID, buf1, buf2 );
			}
		}
	} // namespace xml
} // namespace fyreactor
