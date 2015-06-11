
#pragma once

#include "define.hpp"

namespace fyreactor
{
	namespace xml
	{
		/**
			A XMLHandle is a class that wraps a node pointer with null checks; this is
			an incredibly useful thing. Note that XMLHandle is not part of the TinyXML-2
			DOM structure. It is a separate utility class.

			Take an example:
			@verbatim
			<Document>
				<Element attributeA = "valueA">
					<Child attributeB = "value1" />
					<Child attributeB = "value2" />
				</Element>
			</Document>
			@endverbatim

			Assuming you want the value of "attributeB" in the 2nd "Child" element, it's very
			easy to write a *lot* of code that looks like:

			@verbatim
			XMLElement* root = document.FirstChildElement( "Document" );
			if ( root )
			{
				XMLElement* element = root->FirstChildElement( "Element" );
				if ( element )
				{
					XMLElement* child = element->FirstChildElement( "Child" );
					if ( child )
					{
						XMLElement* child2 = child->NextSiblingElement( "Child" );
						if ( child2 )
						{
							// Finally do something useful.
			@endverbatim

			And that doesn't even cover "else" cases. XMLHandle addresses the verbosity
			of such code. A XMLHandle checks for null pointers so it is perfectly safe
			and correct to use:

			@verbatim
			XMLHandle docHandle( &document );
			XMLElement* child2 = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).FirstChild().NextSibling().ToElement();
			if ( child2 )
			{
				// do something useful
			@endverbatim

			Which is MUCH more concise and useful.

			It is also safe to copy handles - internally they are nothing more than node pointers.
			@verbatim
			XMLHandle handleCopy = handle;
			@endverbatim

			See also XMLConstHandle, which is the same as XMLHandle, but operates on const objects.
		*/
		class XMLHandle
		{
		public:
			/// Create a handle from any node (at any depth of the tree.) This can be a null pointer.
			XMLHandle( XMLNode* node )												{
				_node = node;
			}
			/// Create a handle from a node.
			XMLHandle( XMLNode& node )												{
				_node = &node;
			}
			/// Copy constructor
			XMLHandle( const XMLHandle& ref )										{
				_node = ref._node;
			}
			/// Assignment
			XMLHandle& operator=( const XMLHandle& ref )							{
				_node = ref._node;
				return *this;
			}

			/// Get the first child of this handle.
			XMLHandle FirstChild() 													{
				return XMLHandle( _node ? _node->FirstChild() : 0 );
			}
			/// Get the first child element of this handle.
			XMLHandle FirstChildElement( const char* value=0 )						{
				return XMLHandle( _node ? _node->FirstChildElement( value ) : 0 );
			}
			/// Get the last child of this handle.
			XMLHandle LastChild()													{
				return XMLHandle( _node ? _node->LastChild() : 0 );
			}
			/// Get the last child element of this handle.
			XMLHandle LastChildElement( const char* _value=0 )						{
				return XMLHandle( _node ? _node->LastChildElement( _value ) : 0 );
			}
			/// Get the previous sibling of this handle.
			XMLHandle PreviousSibling()												{
				return XMLHandle( _node ? _node->PreviousSibling() : 0 );
			}
			/// Get the previous sibling element of this handle.
			XMLHandle PreviousSiblingElement( const char* _value=0 )				{
				return XMLHandle( _node ? _node->PreviousSiblingElement( _value ) : 0 );
			}
			/// Get the next sibling of this handle.
			XMLHandle NextSibling()													{
				return XMLHandle( _node ? _node->NextSibling() : 0 );
			}
			/// Get the next sibling element of this handle.
			XMLHandle NextSiblingElement( const char* _value=0 )					{
				return XMLHandle( _node ? _node->NextSiblingElement( _value ) : 0 );
			}

			/// Safe cast to XMLNode. This can return null.
			XMLNode* ToNode()							{
				return _node;
			}
			/// Safe cast to XMLElement. This can return null.
			XMLElement* ToElement() 					{
				return ( ( _node && _node->ToElement() ) ? _node->ToElement() : 0 );
			}
			/// Safe cast to XMLText. This can return null.
			XMLText* ToText() 							{
				return ( ( _node && _node->ToText() ) ? _node->ToText() : 0 );
			}
			/// Safe cast to XMLUnknown. This can return null.
			XMLUnknown* ToUnknown() 					{
				return ( ( _node && _node->ToUnknown() ) ? _node->ToUnknown() : 0 );
			}
			/// Safe cast to XMLDeclaration. This can return null.
			XMLDeclaration* ToDeclaration() 			{
				return ( ( _node && _node->ToDeclaration() ) ? _node->ToDeclaration() : 0 );
			}

		private:
			XMLNode* _node;
		};


		/**
			A variant of the XMLHandle class for working with const XMLNodes and Documents. It is the
			same in all regards, except for the 'const' qualifiers. See XMLHandle for API.
		*/
		class XMLConstHandle
		{
		public:
			XMLConstHandle( const XMLNode* node )											{
				_node = node;
			}
			XMLConstHandle( const XMLNode& node )											{
				_node = &node;
			}
			XMLConstHandle( const XMLConstHandle& ref )										{
				_node = ref._node;
			}

			XMLConstHandle& operator=( const XMLConstHandle& ref )							{
				_node = ref._node;
				return *this;
			}

			const XMLConstHandle FirstChild() const											{
				return XMLConstHandle( _node ? _node->FirstChild() : 0 );
			}
			const XMLConstHandle FirstChildElement( const char* value=0 ) const				{
				return XMLConstHandle( _node ? _node->FirstChildElement( value ) : 0 );
			}
			const XMLConstHandle LastChild()	const										{
				return XMLConstHandle( _node ? _node->LastChild() : 0 );
			}
			const XMLConstHandle LastChildElement( const char* _value=0 ) const				{
				return XMLConstHandle( _node ? _node->LastChildElement( _value ) : 0 );
			}
			const XMLConstHandle PreviousSibling() const									{
				return XMLConstHandle( _node ? _node->PreviousSibling() : 0 );
			}
			const XMLConstHandle PreviousSiblingElement( const char* _value=0 ) const		{
				return XMLConstHandle( _node ? _node->PreviousSiblingElement( _value ) : 0 );
			}
			const XMLConstHandle NextSibling() const										{
				return XMLConstHandle( _node ? _node->NextSibling() : 0 );
			}
			const XMLConstHandle NextSiblingElement( const char* _value=0 ) const			{
				return XMLConstHandle( _node ? _node->NextSiblingElement( _value ) : 0 );
			}


			const XMLNode* ToNode() const				{
				return _node;
			}
			const XMLElement* ToElement() const			{
				return ( ( _node && _node->ToElement() ) ? _node->ToElement() : 0 );
			}
			const XMLText* ToText() const				{
				return ( ( _node && _node->ToText() ) ? _node->ToText() : 0 );
			}
			const XMLUnknown* ToUnknown() const			{
				return ( ( _node && _node->ToUnknown() ) ? _node->ToUnknown() : 0 );
			}
			const XMLDeclaration* ToDeclaration() const	{
				return ( ( _node && _node->ToDeclaration() ) ? _node->ToDeclaration() : 0 );
			}

		private:
			const XMLNode* _node;
		};
	}
}
