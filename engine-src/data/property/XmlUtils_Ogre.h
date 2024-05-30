/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "XmlUtils.h"

#include <OgreMath.h> // Ogre::Radian
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreColourValue.h>
#include <OgreQuaternion.h>

/**
@page XMLSyntax_BasicElements
@section OgreElementsXML XML Syntax for Ogre Elements
*/


/**
@page XMLSyntax_BasicElements
@subsection XML_Vector2 Vector2

Ogre::Vector2 can be stored in XML as (x,y) or (x,z), as node attributes or subnodes. All examples below are equivalent:
\code{.xml}
<parentNode x="13" y="1.3" />
<parentNode x="13" z="1.3" />
<parentNode><x>13</x><y>1.3</y></parentNode>
<parentNode><x>13</x><z>1.3</z></parentNode>
\endcode
*/
template<> inline Ogre::Vector2 MGE::XMLUtils::getValue<Ogre::Vector2>(const pugi::xml_node& xmlNode) {
	{
		auto x = xmlNode.child("x").text();
		auto y = xmlNode.child("y").text();
		if (!y)  y = xmlNode.child("z").text();
		if (x && y) {
			return Ogre::Vector2( x.as_float(), y.as_float() );
		}
	}
	{
		auto x = xmlNode.attribute("x");
		auto y = xmlNode.attribute("y");
		if (!y)  y = xmlNode.attribute("z");
		if (x && y) {
			return Ogre::Vector2( x.as_float(), y.as_float() );
		}
	}
	throw std::logic_error("wrong Vector2 XML node syntax");
}

namespace pugi {
	inline pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const Ogre::Vector2& val) {
		xmlNode.append_attribute("x") << val.x;
		xmlNode.append_attribute("y") << val.y;
		return xmlNode;
	}
}

/**
@page XMLSyntax_BasicElements
@subsection XML_Vector3 Vector3

Ogre::Vector3 can be stored in XML as node attributes or subnodes. All examples below are equivalent:
\code{.xml}
<parentNode x="13" y="1.3" z="-6.6" />
<parentNode><x>13</x><y>1.3</y><z>-6.6</z></parentNode>
\endcode
*/
template<> inline Ogre::Vector3 MGE::XMLUtils::getValue<Ogre::Vector3>(const pugi::xml_node& xmlNode) {
	{
		auto x = xmlNode.child("x").text();
		auto y = xmlNode.child("y").text();
		auto z = xmlNode.child("z").text();
		if (x && y && z) {
			return Ogre::Vector3( x.as_float(), y.as_float(), z.as_float() );
		}
	}
	{
		auto x = xmlNode.attribute("x");
		auto y = xmlNode.attribute("y");
		auto z = xmlNode.attribute("z");
		if (x && y && z) {
			return Ogre::Vector3( x.as_float(), y.as_float(), z.as_float() );
		}
	}
	throw std::logic_error("wrong Vector3 XML node syntax");
}

namespace pugi {
	inline pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const Ogre::Vector3& val) {
		xmlNode.append_attribute("x") << val.x;
		xmlNode.append_attribute("y") << val.y;
		xmlNode.append_attribute("z") << val.z;
		return xmlNode;
	}
}

/**
@page XMLSyntax_BasicElements
@subsection XML_ColourValue ColourValue

Ogre::ColourValue can be stored in XML as node attributes or subnodes. All examples below are equivalent:
\code{.xml}
<parentNode r="0.5" g="0.3" b="0.1" a="1.0" />
<parentNode r="0.5" g="0.3" b="0.1" />
<parentNode><r>0.5</r><g>0.3</g><b>0.1</b><a>1.0</a></parentNode>
<parentNode><r>0.5</r><g>0.3</g><b>0.1</b></parentNode>
\endcode
attribute / node:
  - @c r red component
  - @c g green component
  - @c b blue component
  - @c a alpha (transparent) setting (optiona, default 1.0 =\> no transparent)
*/
template<> inline Ogre::ColourValue MGE::XMLUtils::getValue<Ogre::ColourValue>(const pugi::xml_node& xmlNode) {
	{
		auto a = xmlNode.child("a").text();
		auto r = xmlNode.child("r").text();
		auto g = xmlNode.child("g").text();
		auto b = xmlNode.child("b").text();
		if (r && g && b) {
			return Ogre::ColourValue( r.as_float(), g.as_float(), b.as_float(), a.as_float(1.0) );
		}
	}
	{
		auto a = xmlNode.attribute("a");
		auto r = xmlNode.attribute("r");
		auto g = xmlNode.attribute("g");
		auto b = xmlNode.attribute("b");
		if (r && g && b) {
			return Ogre::ColourValue( r.as_float(), g.as_float(), b.as_float(), a.as_float(1.0) );
		}
	}
	throw std::logic_error("wrong ColourValue XML node syntax");
}

namespace pugi {
	inline pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const Ogre::ColourValue& val) {
		xmlNode.append_attribute("a") << val.a;
		xmlNode.append_attribute("r") << val.r;
		xmlNode.append_attribute("g") << val.g;
		xmlNode.append_attribute("b") << val.b;
		return xmlNode;
	}
}

/**
@page XMLSyntax_BasicElements
@subsection XML_Quaternion Quaternion

Ogre::Quaternion can be stored in XML as node attributes or subnodes. All examples below are equivalent:
\code{.xml}
<parentNode  w="0.5"  x="0.7"  y="0.1"  z="0.3" />
<parentNode qw="0.5" qx="0.7" qy="0.1" qz="0.3" />
<parentNode><w>0.5</w><x>0.7</x><y>0.1</y><z>0.3</z></parentNode>
\endcode
*/
template<> inline Ogre::Quaternion MGE::XMLUtils::getValue<Ogre::Quaternion>(const pugi::xml_node& xmlNode) {
	{
		auto w = xmlNode.child("w").text();
		auto x = xmlNode.child("x").text();
		auto y = xmlNode.child("y").text();
		auto z = xmlNode.child("z").text();
		if (w && x && y && z) {
			return Ogre::Quaternion( w.as_float(), x.as_float(), y.as_float(), z.as_float() );
		}
	}
	{
		auto w = xmlNode.attribute("w");
		auto x = xmlNode.attribute("x");
		auto y = xmlNode.attribute("y");
		auto z = xmlNode.attribute("z");
		if (w && x && y && z) {
			return Ogre::Quaternion( w.as_float(), x.as_float(), y.as_float(), z.as_float() );
		}
	}
	{
		auto w = xmlNode.attribute("qw");
		auto x = xmlNode.attribute("qx");
		auto y = xmlNode.attribute("qy");
		auto z = xmlNode.attribute("qz");
		if (w && x && y && z) {
			return Ogre::Quaternion( w.as_float(), x.as_float(), y.as_float(), z.as_float() );
		}
	}
	throw std::logic_error("wrong Vector3 XML node syntax");
}

namespace pugi {
	inline pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const Ogre::Quaternion& val) {
		xmlNode.append_attribute("w") << val.w;
		xmlNode.append_attribute("x") << val.x;
		xmlNode.append_attribute("y") << val.y;
		xmlNode.append_attribute("z") << val.z;
		return xmlNode;
	}
}

/**
@page XMLSyntax_BasicElements
@subsection XML_Radian Radian

Ogre::Radian can be stored in XML as node attributes, value or subnode. Example:
\code{.xml}
<parentNode rad="1.3" />
<parentNode><rad>1.3</rad></parentNode>
<parentNode>1.3</parentNode>
\endcode
*/
template<> inline Ogre::Radian MGE::XMLUtils::getValue<Ogre::Radian>(const pugi::xml_node& xmlNode) {
	{
		auto x = xmlNode.child("rad").text();
		if (x) {
			return Ogre::Radian( x.as_float() );
		}
	}
	{
		auto x = xmlNode.attribute("rad");
		if (x) {
			return Ogre::Radian( x.as_float() );
		}
	}
	{
		auto x = xmlNode.text();
		if (x) {
			return Ogre::Radian( x.as_float() );
		}
	}
	throw std::logic_error("wrong Radian XML node syntax");
}

namespace pugi {
	inline pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const Ogre::Radian& val) {
		xmlNode.append_attribute("rad") << val.valueRadians();
		return xmlNode;
	}
}
