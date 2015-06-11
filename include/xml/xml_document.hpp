
#pragma once

#include "define.hpp"
#include "xml_element.hpp"
#include "xml_attribute.hpp"
#include "xml_text.hpp"
#include "xml_comment.hpp"

namespace fyreactor
{
	namespace xml
	{
		enum Whitespace {
			PRESERVE_WHITESPACE,
			COLLAPSE_WHITESPACE
		};

		/** A Document binds together all the functionality.
			It can be saved, loaded, and printed to the screen.
			All Nodes are connected and allocated to a Document.
			If the Document is deleted, all its Nodes are also deleted.
		*/
		class XMLDocument : public XMLNode
		{
			friend class XMLElement;
		public:
			/// constructor
			XMLDocument( bool processEntities = true, Whitespace = PRESERVE_WHITESPACE );
			~XMLDocument();

			virtual XMLDocument* ToDocument()				{
				return this;
			}
			virtual const XMLDocument* ToDocument() const	{
				return this;
			}

			/**
    			Parse an XML file from a character string.
    			Returns XML_NO_ERROR (0) on success, or
    			an errorID.

    			You may optionally pass in the 'nBytes', which is
    			the number of bytes which will be parsed. If not
    			specified, TinyXML-2 will assume 'xml' points to a
    			null terminated string.
			*/
			XMLError Parse( const char* xml, size_t nBytes=(size_t)(-1) );

			/**
    			Load an XML file from disk.
    			Returns XML_NO_ERROR (0) on success, or
    			an errorID.
			*/
			XMLError LoadFile( const char* filename );

			/**
    			Load an XML file from disk. You are responsible
    			for providing and closing the FILE*.

    			Returns XML_NO_ERROR (0) on success, or
    			an errorID.
			*/
			XMLError LoadFile( FILE* );

			/**
    			Save the XML file to disk.
    			Returns XML_NO_ERROR (0) on success, or
    			an errorID.
			*/
			XMLError SaveFile( const char* filename, bool compact = false );

			/**
    			Save the XML file to disk. You are responsible
    			for providing and closing the FILE*.

    			Returns XML_NO_ERROR (0) on success, or
    			an errorID.
			*/
			XMLError SaveFile( FILE* fp, bool compact = false );

			bool ProcessEntities() const		{
				return _processEntities;
			}
			Whitespace WhitespaceMode() const	{
				return _whitespace;
			}

			/**
    			Returns true if this document has a leading Byte Order Mark of UTF8.
			*/
			bool HasBOM() const {
				return _writeBOM;
			}
			/** Sets whether to write the BOM when writing the file.
			*/
			void SetBOM( bool useBOM ) {
				_writeBOM = useBOM;
			}

			/** Return the root element of DOM. Equivalent to FirstChildElement().
				To get the first node, use FirstChild().
			*/
			XMLElement* RootElement()				{
				return FirstChildElement();
			}
			const XMLElement* RootElement() const	{
				return FirstChildElement();
			}

			/** Print the Document. If the Printer is not provided, it will
				print to stdout. If you provide Printer, this can print to a file:
    			@verbatim
    			XMLPrinter printer( fp );
    			doc.Print( &printer );
    			@endverbatim

    			Or you can use a printer to print to memory:
    			@verbatim
    			XMLPrinter printer;
    			doc.Print( &printer );
    			// printer.CStr() has a const char* to the XML
    			@endverbatim
			*/
			void Print( XMLPrinter* streamer=0 ) const;
			virtual bool Accept( XMLVisitor* visitor ) const;

			/**
    			Create a new Element associated with
    			this Document. The memory for the Element
    			is managed by the Document.
			*/
			XMLElement* NewElement( const char* name );
			/**
    			Create a new Comment associated with
    			this Document. The memory for the Comment
    			is managed by the Document.
			*/
			XMLComment* NewComment( const char* comment );
			/**
    			Create a new Text associated with
    			this Document. The memory for the Text
    			is managed by the Document.
			*/
			XMLText* NewText( const char* text );
			/**
    			Create a new Declaration associated with
    			this Document. The memory for the object
    			is managed by the Document.

    			If the 'text' param is null, the standard
    			declaration is used.:
    			@verbatim
    				<?xml version="1.0" encoding="UTF-8"?>
    			@endverbatim
			*/
			XMLDeclaration* NewDeclaration( const char* text=0 );
			/**
    			Create a new Unknown associated with
    			this Document. The memory for the object
    			is managed by the Document.
			*/
			XMLUnknown* NewUnknown( const char* text );

			/**
    			Delete a node associated with this document.
    			It will be unlinked from the DOM.
			*/
			void DeleteNode( XMLNode* node )	{
				node->_parent->DeleteChild( node );
			}

			void SetError( XMLError error, const char* str1, const char* str2 );

			/// Return true if there was an error parsing the document.
			bool Error() const {
				return _errorID != XML_NO_ERROR;
			}
			/// Return the errorID.
			XMLError  ErrorID() const {
				return _errorID;
			}
			/// Return a possibly helpful diagnostic location or string.
			const char* GetErrorStr1() const {
				return _errorStr1;
			}
			/// Return a possibly helpful secondary diagnostic location or string.
			const char* GetErrorStr2() const {
				return _errorStr2;
			}
			/// If there is an error, print it to stdout.
			void PrintError() const;
    
			/// Clear the document, resetting it to the initial state.
			void Clear();

			// internal
			char* Identify( char* p, XMLNode** node );

			virtual XMLNode* ShallowClone( XMLDocument* /*document*/ ) const	{
				return 0;
			}
			virtual bool ShallowEqual( const XMLNode* /*compare*/ ) const	{
				return false;
			}

		private:
			XMLDocument( const XMLDocument& );	// not supported
			void operator=( const XMLDocument& );	// not supported

			bool        _writeBOM;
			bool        _processEntities;
			XMLError    _errorID;
			Whitespace  _whitespace;
			const char* _errorStr1;
			const char* _errorStr2;
			char*       _charBuffer;

			MemPoolT< sizeof(XMLElement) >	 _elementPool;
			MemPoolT< sizeof(XMLAttribute) > _attributePool;
			MemPoolT< sizeof(XMLText) >		 _textPool;
			MemPoolT< sizeof(XMLComment) >	 _commentPool;
		};
	}
}
