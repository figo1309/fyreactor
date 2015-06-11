
#pragma once

#include "define.hpp"

namespace fyreactor
{
	namespace xml
	{
		class XMLDocument;
		class XMLElement;
		class XMLAttribute;
		class XMLComment;
		class XMLText;
		class XMLDeclaration;
		class XMLUnknown;
		class XMLPrinter;
		class XMLVisitor;

		/** XMLNode is a base class for every object that is in the
			XML Document Object Model (DOM), except XMLAttributes.
			Nodes have siblings, a parent, and children which can
			be navigated. A node is always in a XMLDocument.
			The type of a XMLNode can be queried, and it can
			be cast to its more defined type.

			A XMLDocument allocates memory for all its Nodes.
			When the XMLDocument gets deleted, all its Nodes
			will also be deleted.

			@verbatim
			A Document can contain:	Element	(container or leaf)
									Comment (leaf)
									Unknown (leaf)
									Declaration( leaf )

			An Element can contain:	Element (container or leaf)
									Text	(leaf)
									Attributes (not on tree)
									Comment (leaf)
									Unknown (leaf)

			@endverbatim
		*/
		class XMLNode
		{
			friend class XMLDocument;
			friend class XMLElement;
		public:

			/// Get the XMLDocument that owns this XMLNode.
			const XMLDocument* GetDocument() const	{
				return _document;
			}
			/// Get the XMLDocument that owns this XMLNode.
			XMLDocument* GetDocument()				{
				return _document;
			}

			/// Safely cast to an Element, or null.
			virtual XMLElement*		ToElement()		{
				return 0;
			}
			/// Safely cast to Text, or null.
			virtual XMLText*		ToText()		{
				return 0;
			}
			/// Safely cast to a Comment, or null.
			virtual XMLComment*		ToComment()		{
				return 0;
			}
			/// Safely cast to a Document, or null.
			virtual XMLDocument*	ToDocument()	{
				return 0;
			}
			/// Safely cast to a Declaration, or null.
			virtual XMLDeclaration*	ToDeclaration()	{
				return 0;
			}
			/// Safely cast to an Unknown, or null.
			virtual XMLUnknown*		ToUnknown()		{
				return 0;
			}

			virtual const XMLElement*		ToElement() const		{
				return 0;
			}
			virtual const XMLText*			ToText() const			{
				return 0;
			}
			virtual const XMLComment*		ToComment() const		{
				return 0;
			}
			virtual const XMLDocument*		ToDocument() const		{
				return 0;
			}
			virtual const XMLDeclaration*	ToDeclaration() const	{
				return 0;
			}
			virtual const XMLUnknown*		ToUnknown() const		{
				return 0;
			}

			/** The meaning of 'value' changes for the specific type.
    			@verbatim
    			Document:	empty
    			Element:	name of the element
    			Comment:	the comment text
    			Unknown:	the tag contents
    			Text:		the text string
    			@endverbatim
			*/
			const char* Value() const;

			/** Set the Value of an XML node.
    			@sa Value()
			*/
			void SetValue( const char* val, bool staticMem=false );

			/// Get the parent of this node on the DOM.
			const XMLNode*	Parent() const			{
				return _parent;
			}

			XMLNode* Parent()						{
				return _parent;
			}

			/// Returns true if this node has no children.
			bool NoChildren() const					{
				return !_firstChild;
			}

			/// Get the first child node, or null if none exists.
			const XMLNode*  FirstChild() const		{
				return _firstChild;
			}

			XMLNode*		FirstChild()			{
				return _firstChild;
			}

			/** Get the first child element, or optionally the first child
				element with the specified name.
			*/
			const XMLElement* FirstChildElement( const char* value=0 ) const;

			XMLElement* FirstChildElement( const char* value=0 )	{
				return const_cast<XMLElement*>(const_cast<const XMLNode*>(this)->FirstChildElement( value ));
			}

			/// Get the last child node, or null if none exists.
			const XMLNode*	LastChild() const						{
				return _lastChild;
			}

			XMLNode*		LastChild()								{
				return const_cast<XMLNode*>(const_cast<const XMLNode*>(this)->LastChild() );
			}

			/** Get the last child element or optionally the last child
				element with the specified name.
			*/
			const XMLElement* LastChildElement( const char* value=0 ) const;

			XMLElement* LastChildElement( const char* value=0 )	{
				return const_cast<XMLElement*>(const_cast<const XMLNode*>(this)->LastChildElement(value) );
			}

			/// Get the previous (left) sibling node of this node.
			const XMLNode*	PreviousSibling() const					{
				return _prev;
			}

			XMLNode*	PreviousSibling()							{
				return _prev;
			}

			/// Get the previous (left) sibling element of this node, with an optionally supplied name.
			const XMLElement*	PreviousSiblingElement( const char* value=0 ) const ;

			XMLElement*	PreviousSiblingElement( const char* value=0 ) {
				return const_cast<XMLElement*>(const_cast<const XMLNode*>(this)->PreviousSiblingElement( value ) );
			}

			/// Get the next (right) sibling node of this node.
			const XMLNode*	NextSibling() const						{
				return _next;
			}

			XMLNode*	NextSibling()								{
				return _next;
			}

			/// Get the next (right) sibling element of this node, with an optionally supplied name.
			const XMLElement*	NextSiblingElement( const char* value=0 ) const;

			XMLElement*	NextSiblingElement( const char* value=0 )	{
				return const_cast<XMLElement*>(const_cast<const XMLNode*>(this)->NextSiblingElement( value ) );
			}

			/**
    			Add a child node as the last (right) child.
				If the child node is already part of the document,
				it is moved from its old location to the new location.
				Returns the addThis argument or 0 if the node does not
				belong to the same document.
			*/
			XMLNode* InsertEndChild( XMLNode* addThis );

			XMLNode* LinkEndChild( XMLNode* addThis )	{
				return InsertEndChild( addThis );
			}
			/**
    			Add a child node as the first (left) child.
				If the child node is already part of the document,
				it is moved from its old location to the new location.
				Returns the addThis argument or 0 if the node does not
				belong to the same document.
			*/
			XMLNode* InsertFirstChild( XMLNode* addThis );
			/**
    			Add a node after the specified child node.
				If the child node is already part of the document,
				it is moved from its old location to the new location.
				Returns the addThis argument or 0 if the afterThis node
				is not a child of this node, or if the node does not
				belong to the same document.
			*/
			XMLNode* InsertAfterChild( XMLNode* afterThis, XMLNode* addThis );

			/**
    			Delete all the children of this node.
			*/
			void DeleteChildren();

			/**
    			Delete a child of this node.
			*/
			void DeleteChild( XMLNode* node );

			/**
    			Make a copy of this node, but not its children.
    			You may pass in a Document pointer that will be
    			the owner of the new Node. If the 'document' is
    			null, then the node returned will be allocated
    			from the current Document. (this->GetDocument())

    			Note: if called on a XMLDocument, this will return null.
			*/
			virtual XMLNode* ShallowClone( XMLDocument* document ) const = 0;

			/**
    			Test if 2 nodes are the same, but don't test children.
    			The 2 nodes do not need to be in the same Document.

    			Note: if called on a XMLDocument, this will return false.
			*/
			virtual bool ShallowEqual( const XMLNode* compare ) const = 0;

			/** Accept a hierarchical visit of the nodes in the TinyXML-2 DOM. Every node in the
    			XML tree will be conditionally visited and the host will be called back
    			via the XMLVisitor interface.

    			This is essentially a SAX interface for TinyXML-2. (Note however it doesn't re-parse
    			the XML for the callbacks, so the performance of TinyXML-2 is unchanged by using this
    			interface versus any other.)

    			The interface has been based on ideas from:

    			- http://www.saxproject.org/
    			- http://c2.com/cgi/wiki?HierarchicalVisitorPattern

    			Which are both good references for "visiting".

    			An example of using Accept():
    			@verbatim
    			XMLPrinter printer;
    			tinyxmlDoc.Accept( &printer );
    			const char* xmlcstr = printer.CStr();
    			@endverbatim
			*/
			virtual bool Accept( XMLVisitor* visitor ) const = 0;

			// internal
			virtual char* ParseDeep( char*, StrPair* );

		protected:
			XMLNode( XMLDocument* );
			virtual ~XMLNode();
			XMLNode( const XMLNode& );	// not supported
			XMLNode& operator=( const XMLNode& );	// not supported

			XMLDocument*	_document;
			XMLNode*		_parent;
			mutable StrPair	_value;

			XMLNode*		_firstChild;
			XMLNode*		_lastChild;

			XMLNode*		_prev;
			XMLNode*		_next;

		private:
			MemPool*		_memPool;
			void Unlink( XMLNode* child );
		};















	}
}
