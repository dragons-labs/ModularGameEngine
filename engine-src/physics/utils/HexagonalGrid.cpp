/*
Copyright (c) 2014-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "physics/utils/HexagonalGrid.h"
#include "LogSystem.h"

constexpr char MGE::HexagonalGridPoint::neighborOffset[4][12];

float MGE::HexagonalGridPoint::distanceY;
float MGE::HexagonalGridPoint::halfDistanceY;
float MGE::HexagonalGridPoint::distanceX;
float MGE::HexagonalGridPoint::neighborCost[2];

void MGE::HexagonalGridPoint::init(float size) {
	distanceY       = size;
	halfDistanceY   = distanceY * 0.5;
	distanceX       = distanceY * 0.8660254;
	neighborCost[0] = distanceY;
	neighborCost[1] = 2.0 * distanceX; // distanceX == 3/4 * X, neighborCost[1] == 1.5 * X => neighborCost[1] = 2.0 * distanceX
}

uint16_t MGE::HexagonalGridPoint::getDirection(const Ogre::Vector3& dir) {
	Ogre::Vector3  tmpPoint = toOgre();
	Ogre::Vector3  tmpDir   = dir.normalisedCopy();
	HexagonalGridPoint neighbor( tmpPoint + tmpDir * distanceX * 2 );
	
	LOG_DEBUG("HexagonalGridPoint::getDirection: in 3d: " << tmpPoint << " dir=" << tmpDir << " => " << tmpPoint + tmpDir * distanceX * 2);
	LOG_DEBUG("HexagonalGridPoint::getDirection: a = " << a << " b = " << b << " na = " << neighbor.a << " nb = " << neighbor.b);
	
	char dx = neighbor.a - a;  // x offset - for neighboring hexagen: -2, -1, 0, 1, 2
	char dy = neighbor.b - b;  // y offset - for neighboring hexagen: -1, 0, 1, 2  (or -2, -1, 0, 1, when a%2 != 0)
	dy += (a & dx & 1);        // convert y offsett to -1, 0, 1, 2 space (fix on cols with dx=1 or dx=-1, when point a%2 != 0)
	
	// convert second level neighbors to direct neighbors (only case with a%2 == 0, due to previous conversion of dy)
	if        (dx ==  2  && dy ==  1) {
		dx = 1;
	} else if (dx ==  2  && dy == -1) {
		dx = 1;
		dy = 0;
	} else if (dx ==  0  && dy ==  2) {
		dy = 1;
	} else if (dx ==  0  && dy == -2) {
		dy = -1;
	} else if (dx == -2  && dy == -1) {
		dx = -1;
		dy = 0;
	} else if (dx == -2  && dy ==  1) {
		dx = -1;
	}
	
	// calculate direction
	uint16_t retVal = (dx + 0x0c) | ((dy  + 0x0c) << 8);
	
	LOG_DEBUG("HexagonalGridPoint::getDirection: dx = " << int(dx) << " dy = " << int(dy) << "  =>  direction = " << retVal);
	
	return retVal;
}

uint16_t MGE::HexagonalGridPoint::getDirection(float x, float z) {
	char dx, dy;
	
	if (x == 0) {
		dx = 0;
		dy = (z>0) ? 1 : -1;
	} else {
		float tan = z/std::abs(x);
		
		if (tan < -3.732) {
			// angle((1,0), (x,z)) < -75 deg
			dx = 0;
			dy = -1;
		} else if (tan < -1) {
			// angle((1,0), (x,z)) < -45 deg
			dx = 1;
			dy = -2;
		} else if (tan < -0.2679) {
			// angle((1,0), (x,z)) < -15 deg
			dx = 1;
			dy = -1;
		} else if (tan < 0.2679) {
			// angle((1,0), (x,z)) < 15 deg
			dx = 2;
			dy = 0;
		} else if (tan < 1) {
			// angle((1,0), (x,z)) < 45 deg
			dx = 1;
			dy = 0;
		} else if (tan < 3.732) {
			// angle((1,0), (x,z)) < 75 deg
			dx = 1;
			dy = 1;
		} else {
			// angle((1,0), (x,z)) >= 75 deg
			dx = 0;
			dy = 1;
		}
		
		if (x<0) {
			dx = dx * -1;
		}
	}
	return dx | (dy << 8);
}

namespace MGE {
/// write to output stream operator for Point16
std::ostream & operator<< (std::ostream& output, const MGE::Point16& p) {
	output << "(" << p.a << ", " << p.b << ")";
	return output;
}
}
