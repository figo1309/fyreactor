
#pragma once

#include "define.hpp"
#include "xml_node.hpp"

namespace fyreactor
{
	namespace xml
	{
		/** XML text.

			Note that a text node can have child element nodes, for example:
			@verbatim
			<root>This is <b>bold</b></root>
			@endverbatim

			A text node can have 2 ways to output the next. "normal" output
			and CDATA. It will default to the mode it was parsed from the XML file and
			you generally want to leave it alone, but you can change the output mode with
			SetCData() and query it with CData().
		*/
		class XMLText : public XMLNode
		{
			friend class XMLBase;
			friend class XMLDocument;
		public:
			virtual bool Accept( XMLVisitor* visitor ) const;

			virtual XMLText* ToText()			{
				return this;
			}
			virtual const XMLText* ToText() const	{
				return this;
			}

			/// Declare whether this should be CDATA or standard text.
			void SetCData( bool isCData )			{
				_isCData = isCData;
			}
			/// Returns true if this is a CDATA text element.
			bool CData() const						{
				return _isCData;
			}

			char* ParseDeep( char*, StrPair* endTag );
			virtual XMLNode* ShallowClone( XMLDocument* document ) const;
			virtual bool ShallowEqual( const XMLNode* compare ) const;

		protected:
			XMLText( XMLDocument* doc )	: XMLNode( doc ), _isCData( false )	{}
			virtual ~XMLText()												{}
			XMLText( const XMLText& );	// not supported
			XMLText& operator=( const XMLText& );	// not supported

		private:
			bool _isCData;
		};
	}
}
