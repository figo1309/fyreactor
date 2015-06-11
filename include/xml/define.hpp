//
// define.hpp
//
// $Id: //enjoy/1.0/include/xml/define.hpp#1 $
//
// Library:	libqing
// Module:	util
// Package:	xml
//
// Copyright (c) 2013-2014, Shanghai Mjoygames Information Technology Co.
//
// 根据 Tinyxml2 改写，方便融合框架使用...
//

#pragma once

#include "../define.hpp"

#if defined(ANDROID_NDK)
#   include <ctype.h>
#   include <limits.h>
#   include <stdio.h>
#   include <stdlib.h>
#   include <string.h>
#   include <stdarg.h>
#else
#   include <cctype>
#   include <climits>
#   include <cstdio>
#   include <cstdlib>
#   include <cstring>
#   include <cstdarg>
#endif

#if defined(DEBUG)
#   if defined(_MSC_VER)
#       define TIXMLASSERT( x )           if ( !(x)) { __debugbreak(); } //if ( !(x)) WinDebugBreak()
#   elif defined (ANDROID_NDK)
#       include <android/log.h>
#       define TIXMLASSERT( x )           if ( !(x)) { __android_log_assert( "assert", "grinliz", "ASSERT in '%s' at %d.", __FILE__, __LINE__ ); }
#   else
#       include <assert.h>
#       define TIXMLASSERT                assert
#   endif
#   else
#       define TIXMLASSERT( x )           {}
#endif


#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
inline int TIXML_SNPRINTF( char* buffer, size_t size, const char* format, ... )
{
    va_list va;
    va_start( va, format );
    int result = vsnprintf_s( buffer, size, _TRUNCATE, format, va );
    va_end( va );
    return result;
}
#define TIXML_SSCANF   sscanf_s
#else
#define TIXML_SNPRINTF snprintf
#define TIXML_SSCANF   sscanf
#endif

static const int TIXML2_MAJOR_VERSION = 1;
static const int TIXML2_MINOR_VERSION = 0;
static const int TIXML2_PATCH_VERSION = 14;

#include <new>		// yes, this one new style header, is in the Android SDK.
#   ifdef ANDROID_NDK
#   include <stddef.h>
#else
#   include <cstddef>
#endif

static const char LINE_FEED				= (char)0x0a;			// all line endings are normalized to LF
static const char LF					= LINE_FEED;
static const char CARRIAGE_RETURN		= (char)0x0d;			// CR gets filtered out
static const char CR					= CARRIAGE_RETURN;
static const char SINGLE_QUOTE			= '\'';
static const char DOUBLE_QUOTE			= '\"';

// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
//	ef bb bf (Microsoft "lead bytes") - designates UTF-8

static const char TIXML_UTF_LEAD_0 = 0xefU;
static const char TIXML_UTF_LEAD_1 = 0xbbU;
static const char TIXML_UTF_LEAD_2 = 0xbfU;


#define DELETE_NODE( node )	{			\
        if ( node ) {						\
            MemPool* pool = node->_memPool;	\
            node->~XMLNode();				\
            pool->Free( node );				\
        }									\
    }
#define DELETE_ATTRIBUTE( attrib ) {		\
        if ( attrib ) {							\
            MemPool* pool = attrib->_memPool;	\
            attrib->~XMLAttribute();			\
            pool->Free( attrib );				\
        }										\
    }
	
namespace fyreactor
{
	namespace xml
	{
		/*
			A class that wraps strings. Normally stores the start and end
			pointers into the XML file itself, and will apply normalization
			and entity translation if actually read. Can also store (and memory
			manage) a traditional char[]
		*/
		class StrPair
		{
		public:
			enum {
				NEEDS_ENTITY_PROCESSING			= 0x01,
				NEEDS_NEWLINE_NORMALIZATION		= 0x02,
				COLLAPSE_WHITESPACE				= 0x04,

				TEXT_ELEMENT					= NEEDS_ENTITY_PROCESSING | NEEDS_NEWLINE_NORMALIZATION,
				TEXT_ELEMENT_LEAVE_ENTITIES		= NEEDS_NEWLINE_NORMALIZATION,
				ATTRIBUTE_NAME					= 0,
				ATTRIBUTE_VALUE		           	= NEEDS_ENTITY_PROCESSING | NEEDS_NEWLINE_NORMALIZATION,
				ATTRIBUTE_VALUE_LEAVE_ENTITIES  = NEEDS_NEWLINE_NORMALIZATION,
				COMMENT				      		= NEEDS_NEWLINE_NORMALIZATION
			};

			StrPair() : _flags( 0 ), _start( 0 ), _end( 0 ) {}
			~StrPair();

			void Set( char* start, char* end, int flags ) {
				Reset();
				_start  = start;
				_end    = end;
				_flags  = flags | NEEDS_FLUSH;
			}

			const char* GetStr();

			bool Empty() const {
				return _start == _end;
			}

			void SetInternedStr( const char* str ) {
				Reset();
				_start = const_cast<char*>(str);
			}

			void SetStr( const char* str, int flags=0 );

			char* ParseText( char* in, const char* endTag, int strFlags );
			char* ParseName( char* in );

		private:
			void Reset();
			void CollapseWhitespace();

			enum {
				NEEDS_FLUSH = 0x100,
				NEEDS_DELETE = 0x200
			};

			// After parsing, if *_end != 0, it can be set to zero.
			int     _flags;
			char*   _start;
			char*   _end;
		};


		/*
			A dynamic array of Plain Old Data. Doesn't support constructors, etc.
			Has a small initial memory pool, so that low or no usage will not
			cause a call to new/delete
		*/
		template <class T, int INIT>
		class DynArray
		{
		public:
			DynArray< T, INIT >() {
				_mem = _pool;
				_allocated = INIT;
				_size = 0;
			}

			~DynArray() {
				if ( _mem != _pool ) {
					delete [] _mem;
				}
			}

			void Clear() {
				_size = 0;
			}

			void Push( T t ) {
				EnsureCapacity( _size+1 );
				_mem[_size++] = t;
			}

			T* PushArr( int count ) {
				EnsureCapacity( _size+count );
				T* ret = &_mem[_size];
				_size += count;
				return ret;
			}

			T Pop() {
				return _mem[--_size];
			}

			void PopArr( int count ) {
				TIXMLASSERT( _size >= count );
				_size -= count;
			}

			bool Empty() const					{
				return _size == 0;
			}

			T& operator[](int i)				{
				TIXMLASSERT( i>= 0 && i < _size );
				return _mem[i];
			}

			const T& operator[](int i) const	{
				TIXMLASSERT( i>= 0 && i < _size );
				return _mem[i];
			}

			const T& PeekTop() const                            {
				TIXMLASSERT( _size > 0 );
				return _mem[ _size - 1];
			}

			int Size() const					{
				return _size;
			}

			int Capacity() const				{
				return _allocated;
			}

			const T* Mem() const				{
				return _mem;
			}

			T* Mem()							{
				return _mem;
			}

		private:
			void EnsureCapacity( int cap ) {
				if ( cap > _allocated ) {
					int newAllocated = cap * 2;
					T* newMem = new T[newAllocated];
					memcpy( newMem, _mem, sizeof(T)*_size );	// warning: not using constructors, only works for PODs
					if ( _mem != _pool ) {
						delete [] _mem;
					}
					_mem = newMem;
					_allocated = newAllocated;
				}
			}

			T*  _mem;
			T   _pool[INIT];
			int _allocated;		// objects allocated
			int _size;			// number objects in use
		};


		/*
			Parent virtual class of a pool for fast allocation
			and deallocation of objects.
		*/
		class MemPool
		{
		public:
			MemPool() {}
			virtual ~MemPool() {}

			virtual int ItemSize() const = 0;
			virtual void* Alloc() = 0;
			virtual void Free( void* ) = 0;
			virtual void SetTracked() = 0;
		};


		/*
			Template child class to create pools of the correct type.
		*/
		template< int SIZE >
		class MemPoolT : public MemPool
		{
		public:
			MemPoolT() : _root(0), _currentAllocs(0), _nAllocs(0), _maxAllocs(0), _nUntracked(0)	{}
			~MemPoolT() {
				// Delete the blocks.
				for( int i=0; i<_blockPtrs.Size(); ++i ) {
					delete _blockPtrs[i];
				}
			}

			virtual int ItemSize() const	{
				return SIZE;
			}
			int CurrentAllocs() const		{
				return _currentAllocs;
			}

			virtual void* Alloc() {
				if ( !_root ) {
					// Need a new block.
					Block* block = new Block();
					_blockPtrs.Push( block );

					for( int i=0; i<COUNT-1; ++i ) {
						block->chunk[i].next = &block->chunk[i+1];
					}
					block->chunk[COUNT-1].next = 0;
					_root = block->chunk;
				}
				void* result = _root;
				_root = _root->next;

				++_currentAllocs;
				if ( _currentAllocs > _maxAllocs ) {
					_maxAllocs = _currentAllocs;
				}
				_nAllocs++;
				_nUntracked++;
				return result;
			}
			virtual void Free( void* mem ) {
				if ( !mem ) {
					return;
				}
				--_currentAllocs;
				Chunk* chunk = (Chunk*)mem;
		#ifdef DEBUG
				memset( chunk, 0xfe, sizeof(Chunk) );
		#endif
				chunk->next = _root;
				_root = chunk;
			}
			void Trace( const char* name ) {
				printf( "Mempool %s watermark=%d [%dk] current=%d size=%d nAlloc=%d blocks=%d\n",
						name, _maxAllocs, _maxAllocs*SIZE/1024, _currentAllocs, SIZE, _nAllocs, _blockPtrs.Size() );
			}

			void SetTracked() {
				_nUntracked--;
			}

			int Untracked() const {
				return _nUntracked;
			}

			// This number is perf sensitive. 4k seems like a good tradeoff on my machine.
			// The test file is large, 170k.
			// Release:		VS2010 gcc(no opt)
			//		1k:		4000
			//		2k:		4000
			//		4k:		3900	21000
			//		16k:	5200
			//		32k:	4300
			//		64k:	4000	21000
			enum { COUNT = (4*1024)/SIZE }; // Some compilers do not accept to use COUNT in private part if COUNT is private

		private:
			union Chunk {
				Chunk*  next;
				char    mem[SIZE];
			};
			struct Block {
				Chunk chunk[COUNT];
			};
			DynArray< Block*, 10 > _blockPtrs;
			Chunk* _root;

			int _currentAllocs;
			int _nAllocs;
			int _maxAllocs;
			int _nUntracked;
		};
		
		enum XMLError {
			XML_NO_ERROR = 0,
			XML_SUCCESS = 0,

			XML_NO_ATTRIBUTE,
			XML_WRONG_ATTRIBUTE_TYPE,

			XML_ERROR_FILE_NOT_FOUND,
			XML_ERROR_FILE_COULD_NOT_BE_OPENED,
			XML_ERROR_FILE_READ_ERROR,
			XML_ERROR_ELEMENT_MISMATCH,
			XML_ERROR_PARSING_ELEMENT,
			XML_ERROR_PARSING_ATTRIBUTE,
			XML_ERROR_IDENTIFYING_TAG,
			XML_ERROR_PARSING_TEXT,
			XML_ERROR_PARSING_CDATA,
			XML_ERROR_PARSING_COMMENT,
			XML_ERROR_PARSING_DECLARATION,
			XML_ERROR_PARSING_UNKNOWN,
			XML_ERROR_EMPTY_DOCUMENT,
			XML_ERROR_MISMATCHED_ELEMENT,
			XML_ERROR_PARSING,

			XML_CAN_NOT_CONVERT_TEXT,
			XML_NO_TEXT_NODE
		};
		
		struct Entity {
			const char* pattern;
			int length;
			char value;
		};

		static const int NUM_ENTITIES = 5;
		static const Entity entities[NUM_ENTITIES] = {
			{ "quot", 4,	DOUBLE_QUOTE },
			{ "amp", 3,		'&'  },
			{ "apos", 4,	SINGLE_QUOTE },
			{ "lt",	2, 		'<'	 },
			{ "gt",	2,		'>'	 }
		};
	}
}
