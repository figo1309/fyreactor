
#pragma once

#include "define.hpp"
#include "xml_node.hpp"

namespace fyreactor
{
	namespace xml
	{
		/** An XML Comment. */
		class XMLComment : public XMLNode
		{
			friend class XMLDocument;
		public:
			virtual XMLComment*	ToComment()					{
				return this;
			}
			virtual const XMLComment* ToComment() const		{
				return this;
			}

			virtual bool Accept( XMLVisitor* visitor ) const;

			char* ParseDeep( char*, StrPair* endTag );
			virtual XMLNode* ShallowClone( XMLDocument* document ) const;
			virtual bool ShallowEqual( const XMLNode* compare ) const;

		protected:
			XMLComment( XMLDocument* doc );
			virtual ~XMLComment();
			XMLComment( const XMLComment& );	// not supported
			XMLComment& operator=( const XMLComment& );	// not supported

		private:
		};
	}
}
