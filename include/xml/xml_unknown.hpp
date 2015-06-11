
#pragma once

#include "define.hpp"

namespace fyreactor
{
	namespace xml
	{
		/** Any tag that TinyXML-2 doesn't recognize is saved as an
			unknown. It is a tag of text, but should not be modified.
			It will be written back to the XML, unchanged, when the file
			is saved.

			DTD tags get thrown into XMLUnknowns.
		*/
		class XMLUnknown : public XMLNode
		{
			friend class XMLDocument;
		public:
			virtual XMLUnknown*	ToUnknown()					{
				return this;
			}
			virtual const XMLUnknown* ToUnknown() const		{
				return this;
			}

			virtual bool Accept( XMLVisitor* visitor ) const;

			char* ParseDeep( char*, StrPair* endTag );
			virtual XMLNode* ShallowClone( XMLDocument* document ) const;
			virtual bool ShallowEqual( const XMLNode* compare ) const;

		protected:
			XMLUnknown( XMLDocument* doc );
			virtual ~XMLUnknown();
			XMLUnknown( const XMLUnknown& );	// not supported
			XMLUnknown& operator=( const XMLUnknown& );	// not supported
		};
	}
}
