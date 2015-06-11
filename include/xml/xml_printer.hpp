
#pragma once

#include "define.hpp"
#include "xml_visitor.hpp"

namespace fyreactor
{
	namespace xml
	{
		/**
			Printing functionality. The XMLPrinter gives you more
			options than the XMLDocument::Print() method.

			It can:
			-# Print to memory.
			-# Print to a file you provide.
			-# Print XML without a XMLDocument.

			Print to Memory

			@verbatim
			XMLPrinter printer;
			doc.Print( &printer );
			SomeFunction( printer.CStr() );
			@endverbatim

			Print to a File

			You provide the file pointer.
			@verbatim
			XMLPrinter printer( fp );
			doc.Print( &printer );
			@endverbatim

			Print without a XMLDocument

			When loading, an XML parser is very useful. However, sometimes
			when saving, it just gets in the way. The code is often set up
			for streaming, and constructing the DOM is just overhead.

			The Printer supports the streaming case. The following code
			prints out a trivially simple XML file without ever creating
			an XML document.

			@verbatim
			XMLPrinter printer( fp );
			printer.OpenElement( "foo" );
			printer.PushAttribute( "foo", "bar" );
			printer.CloseElement();
			@endverbatim
		*/
		class XMLPrinter : public XMLVisitor
		{
		public:
			/** Construct the printer. If the FILE* is specified,
    			this will print to the FILE. Else it will print
    			to memory, and the result is available in CStr().
    			If 'compact' is set to true, then output is created
    			with only required whitespace and newlines.
			*/
			XMLPrinter( FILE* file=0, bool compact = false, int depth = 0 );
			virtual ~XMLPrinter()	{}

			/** If streaming, write the BOM and declaration. */
			void PushHeader( bool writeBOM, bool writeDeclaration );
			/** If streaming, start writing an element.
				The element must be closed with CloseElement()
			*/
			void OpenElement( const char* name );
			/// If streaming, add an attribute to an open element.
			void PushAttribute( const char* name, const char* value );
			void PushAttribute( const char* name, int value );
			void PushAttribute( const char* name, unsigned value );
			void PushAttribute( const char* name, bool value );
			void PushAttribute( const char* name, double value );
			/// If streaming, close the Element.
			virtual void CloseElement();

			/// Add a text node.
			void PushText( const char* text, bool cdata=false );
			/// Add a text node from an integer.
			void PushText( int value );
			/// Add a text node from an unsigned.
			void PushText( unsigned value );
			/// Add a text node from a bool.
			void PushText( bool value );
			/// Add a text node from a float.
			void PushText( float value );
			/// Add a text node from a double.
			void PushText( double value );

			/// Add a comment
			void PushComment( const char* comment );

			void PushDeclaration( const char* value );
			void PushUnknown( const char* value );

			virtual bool VisitEnter( const XMLDocument& /*doc*/ );
			virtual bool VisitExit( const XMLDocument& /*doc*/ )			{
				return true;
			}

			virtual bool VisitEnter( const XMLElement& element, const XMLAttribute* attribute );
			virtual bool VisitExit( const XMLElement& element );

			virtual bool Visit( const XMLText& text );
			virtual bool Visit( const XMLComment& comment );
			virtual bool Visit( const XMLDeclaration& declaration );
			virtual bool Visit( const XMLUnknown& unknown );

			/**
    			If in print to memory mode, return a pointer to
    			the XML file in memory.
			*/
			const char* CStr() const {
				return _buffer.Mem();
			}
			/**
    			If in print to memory mode, return the size
    			of the XML file in memory. (Note the size returned
    			includes the terminating null.)
			*/
			int CStrSize() const {
				return _buffer.Size();
			}
			/**
    			If in print to memory mode, reset the buffer to the
    			beginning.
			*/
			void ClearBuffer() {
				_buffer.Clear();
				_buffer.Push(0);
			}

		protected:
			void SealElement();
			bool _elementJustOpened;
			DynArray< const char*, 10 > _stack;

		private:
			void PrintSpace( int depth );
			void PrintString( const char*, bool restrictedEntitySet );	// prints out, after detecting entities.
			void Print( const char* format, ... );

			bool _firstElement;
			FILE* _fp;
			int _depth;
			int _textDepth;
			bool _processEntities;
			bool _compactMode;

			enum {
				ENTITY_RANGE = 64,
				BUF_SIZE = 200
			};
			bool _entityFlag[ENTITY_RANGE];
			bool _restrictedEntityFlag[ENTITY_RANGE];

			DynArray< char, 20 > _buffer;
#ifdef _MSC_VER
			DynArray< char, 20 > _accumulator;
#endif
		};
	}
}
