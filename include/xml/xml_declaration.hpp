
#pragma once

#include "define.hpp"
#include "xml_node.hpp"

namespace fyreactor
{
	namespace xml
	{
		class XMLDocument;

		/** In correct XML the declaration is the first entry in the file.
			@verbatim
				<?xml version="1.0" standalone="yes"?>
			@endverbatim

			TinyXML-2 will happily read or write files without a declaration,
			however.

			The text of the declaration isn't interpreted. It is parsed
			and written as a string.
		*/
		class XMLDeclaration : public XMLNode
		{
			friend class XMLDocument;
		public:
			virtual XMLDeclaration*	ToDeclaration()					{
				return this;
			}
			virtual const XMLDeclaration* ToDeclaration() const		{
				return this;
			}

			virtual bool Accept( XMLVisitor* visitor ) const;

			char* ParseDeep( char*, StrPair* endTag );
			virtual XMLNode* ShallowClone( XMLDocument* document ) const;
			virtual bool ShallowEqual( const XMLNode* compare ) const;

		protected:
			XMLDeclaration( XMLDocument* doc );
			virtual ~XMLDeclaration();
			XMLDeclaration( const XMLDeclaration& );	// not supported
			XMLDeclaration& operator=( const XMLDeclaration& );	// not supported
		};
	}
}
