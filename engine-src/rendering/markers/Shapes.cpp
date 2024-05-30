/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>
Copyright (c) 2008-2013 Ismail TARIM <ismail@royalspor.com> and the Ogitor Team

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

Based on:
	→ MIT licensed code from OGITOR
		- ex http://www.ogitor.org
		- now https://github.com/OGRECave/ogitor
	→ public domain code from Ogre Wiki
		- http://www.ogre3d.org/tikiwiki/ManualSphereMeshes
*/

#include "rendering/markers/Shapes.h"

#include "data/utils/OgreUtils.h"

#include <OgreManualObject2.h>
#include <OgreSceneManager.h>

Ogre::MeshPtr MGE::Shapes::createPlaneMesh(Ogre::SceneManager* manager, const Ogre::String& name, const Ogre::String& group, const Ogre::String& material) {
	Ogre::ManualObject* manualObj = manager->createManualObject();
	
	manualObj->begin(material, Ogre::OT_TRIANGLE_LIST);
	
	const float s = 1.5;
	manualObj->position( 0, s, 0);
	manualObj->position( s, s, 0);
	manualObj->position( s, 0, 0);
	manualObj->position( 0, 0, 0);
	
	manualObj->index(0);
	manualObj->index(1);
	manualObj->index(2);
	manualObj->index(0);
	manualObj->index(2);
	manualObj->index(3);
	
	manualObj->end();
	
	Ogre::MeshPtr mesh = MGE::OgreUtils::convertManualToMesh(manualObj, name, group);
	
	return mesh;
}

Ogre::MeshPtr MGE::Shapes::createSphereMesh(
	Ogre::SceneManager* manager, const Ogre::String& name, const Ogre::String& group, const Ogre::String& material,
	const float r, const int nRings, const int nSegments
) {
	// based on public domain code from http://www.ogre3d.org/tikiwiki/ManualSphereMeshes#Creating_a_sphere_with_ManualObject
	
	Ogre::ManualObject* manualObj = manager->createManualObject();
	
	manualObj->begin(material, Ogre::OT_TRIANGLE_LIST);
	
	float fDeltaRingAngle = (Ogre::Math::PI / nRings);
	float fDeltaSegAngle = (2 * Ogre::Math::PI / nSegments);
	unsigned short wVerticeIndex = 0 ;
	
	// Generate the group of rings for the sphere
	for( int ring = 0; ring <= nRings; ring++ ) {
		float r0 = r * sinf (ring * fDeltaRingAngle);
		float y0 = r * cosf (ring * fDeltaRingAngle);
		
		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= nSegments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);
			
			// Add one vertex to the strip which makes up the sphere
			manualObj->position( x0, y0, z0);
			//manualObj->normal(Ogre::Vector3(x0, y0, z0).normalisedCopy());
			//manualObj->textureCoord((float) seg / (float) nSegments, (float) ring / (float) nRings);
			
			if (ring != nRings) {
				// each vertex (except the last) has six indicies pointing to it
				manualObj->index(wVerticeIndex + nSegments + 1);
				manualObj->index(wVerticeIndex);
				manualObj->index(wVerticeIndex + nSegments);
				manualObj->index(wVerticeIndex + nSegments + 1);
				manualObj->index(wVerticeIndex + 1);
				manualObj->index(wVerticeIndex);
				wVerticeIndex ++;
			}
		}
	}
	
	manualObj->end();
	
	Ogre::MeshPtr mesh = MGE::OgreUtils::convertManualToMesh(manualObj, name, group);
	
	return mesh;
}

Ogre::MeshPtr MGE::Shapes::createConeMesh(
	Ogre::SceneManager* manager, const Ogre::String& name, const Ogre::String& group, const Ogre::String& material,
	const float r, const float h, const int nSegments
) {
	Ogre::ManualObject* manualObj = manager->createManualObject();
	
	manualObj->begin(material, Ogre::OT_TRIANGLE_LIST);
	
	manualObj->position(0, 0, 0);
	manualObj->position(0, 0, h);
	
	float step = 2 * Ogre::Math::PI / nSegments;
	for(float theta = 0; theta < 2 * Ogre::Math::PI; theta += step) {
		manualObj->position(r * cos(theta), r * sin(theta), 0);
	}
	
	for(int inside = 2; inside <= nSegments; ++inside)
	{
		manualObj->index(0);
		manualObj->index(inside);
		manualObj->index(inside + 1);
	}
	manualObj->index(0);
	manualObj->index(nSegments);
	manualObj->index(2);
	
	for(int outside = 2; outside <= nSegments; ++outside) {
		manualObj->index(1);
		manualObj->index(outside);
		manualObj->index(outside + 1);
	}
	manualObj->index(1);
	manualObj->index(nSegments);
	manualObj->index(2);
	
	manualObj->end();
	
	Ogre::MeshPtr mesh = MGE::OgreUtils::convertManualToMesh(manualObj, name, group);
	
	return mesh;
}
