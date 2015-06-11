//
// define.hpp
//
// $Id: //enjoy/1.0/include/util/define.hpp#1 $
//
// Library:	libqing
// Module:	util
// Package:	Xml
//
// Copyright (c) 2013-2014, Shanghai Mjoygames Information Technology Co.
//
// 根据 Tinyxml2 改写，方便融合框架使用...
//

#include <xml/define.hpp>
#include <xml/xml_util.hpp>

namespace fyreactor
{
	namespace xml
	{
		StrPair::~StrPair()
		{
			Reset();
		}

		void StrPair::Reset()
		{
			if ( _flags & NEEDS_DELETE ) {
				delete [] _start;
			}
			_flags = 0;
			_start = 0;
			_end = 0;
		}

		void StrPair::SetStr( const char* str, int flags )
		{
			Reset();
			size_t len = strlen( str );
			_start = new char[ len+1 ];
			memcpy( _start, str, len+1 );
			_end = _start + len;
			_flags = flags | NEEDS_DELETE;
		}

		char* StrPair::ParseText( char* p, const char* endTag, int strFlags )
		{
			TIXMLASSERT( endTag && *endTag );

			char* start = p;	// fixme: hides a member
			char  endChar = *endTag;
			size_t length = strlen( endTag );

			// Inner loop of text parsing.
			while ( *p ) {
				if ( *p == endChar && strncmp( p, endTag, length ) == 0 ) {
					Set( start, p, strFlags );
					return p + length;
				}
				++p;
			}
			return 0;
		}

		char* StrPair::ParseName( char* p )
		{
			char* start = p;

			if ( !start || !(*start) ) {
				return 0;
			}

			while( *p && ( p == start ? XMLUtil::IsNameStartChar( *p ) : XMLUtil::IsNameChar( *p ) )) {
				++p;
			}

			if ( p > start ) {
				Set( start, p, 0 );
				return p;
			}
			return 0;
		}

		void StrPair::CollapseWhitespace()
		{
			// Trim leading space.
			_start = XMLUtil::SkipWhiteSpace( _start );

			if ( _start && *_start ) {
				char* p = _start;	// the read pointer
				char* q = _start;	// the write pointer

				while( *p ) {
					if ( XMLUtil::IsWhiteSpace( *p )) {
						p = XMLUtil::SkipWhiteSpace( p );
						if ( *p == 0 ) {
							break;    // don't write to q; this trims the trailing space.
						}
						*q = ' ';
						++q;
					}
					*q = *p;
					++q;
					++p;
				}
				*q = 0;
			}
		}

		const char* StrPair::GetStr()
		{
			if ( _flags & NEEDS_FLUSH ) {
				*_end = 0;
				_flags ^= NEEDS_FLUSH;

				if ( _flags ) {
					char* p = _start;	// the read pointer
					char* q = _start;	// the write pointer

					while( p < _end ) {
						if ( (_flags & NEEDS_NEWLINE_NORMALIZATION) && *p == CR ) {
							// CR-LF pair becomes LF
							// CR alone becomes LF
							// LF-CR becomes LF
							if ( *(p+1) == LF ) {
								p += 2;
							}
							else {
								++p;
							}
							*q++ = LF;
						}
						else if ( (_flags & NEEDS_NEWLINE_NORMALIZATION) && *p == LF ) {
							if ( *(p+1) == CR ) {
								p += 2;
							}
							else {
								++p;
							}
							*q++ = LF;
						}
						else if ( (_flags & NEEDS_ENTITY_PROCESSING) && *p == '&' ) {
							// Entities handled by tinyXML2:
							// - special entities in the entity table [in/out]
							// - numeric character reference [in]
							//   &#20013; or &#x4e2d;

							if ( *(p+1) == '#' ) {
								char buf[10] = { 0 };
								int len;
								p = const_cast<char*>( XMLUtil::GetCharacterRef( p, buf, &len ) );
								for( int i=0; i<len; ++i ) {
									*q++ = buf[i];
								}
								TIXMLASSERT( q <= p );
							}
							else {
								int i=0;
								for(; i<NUM_ENTITIES; ++i ) {
									if (    strncmp( p+1, entities[i].pattern, entities[i].length ) == 0
											&& *(p+entities[i].length+1) == ';' ) {
										// Found an entity convert;
										*q = entities[i].value;
										++q;
										p += entities[i].length + 2;
										break;
									}
								}
								if ( i == NUM_ENTITIES ) {
									// fixme: treat as error?
									++p;
									++q;
								}
							}
						}
						else {
							*q = *p;
							++p;
							++q;
						}
					}
					*q = 0;
				}
				// The loop below has plenty going on, and this
				// is a less useful mode. Break it out.
				if ( _flags & COLLAPSE_WHITESPACE ) {
					CollapseWhitespace();
				}
				_flags = (_flags & NEEDS_DELETE);
			}
			return _start;
		}
	} // namespace xml
} // namespace fyreactor
