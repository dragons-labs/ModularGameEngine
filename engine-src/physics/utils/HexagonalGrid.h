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

#pragma   once
#include "config.h"

#include <inttypes.h>
#include <OgreVector3.h>
#include <ostream>

#ifdef MGE_DEBUG_HEXAGONAL_GRID
#include "LogSystem.h"
#endif

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief (a,b) 2*16 bit signed integer ponit representation for A* solver
 */
struct Point16 {
	/// first coordinate of point
	int16_t a;
	/// second coordinate of point
	int16_t b;
	
	/// constructor - from separated coordinates
	inline Point16 (const int16_t _a, const int16_t _b) {
		a = _a;
		b = _b;
	}
	/// constructor - from coordinates coded as single 32bit number
	inline Point16 (const int32_t _x) {
		a = _x & 0xffff;
		b = (_x >> 16) & 0xffff;
	}
	/// constructor
	inline Point16 () {
		a = 0;
		b = 0;
	}
	
	/// equal operator
	inline Point16 operator == ( const Point16& p ) const {
		return b == p.b && a == p.a;
	}
	
	/// less operator
	inline Point16 operator < ( const Point16& p ) const {
		return b < p.b || (b == p.b && a < p.a);
	}
	
	/// substract operator
	inline Point16 operator - ( const Point16& p ) const {
		return Point16( a - p.a, b - p.b );
	}
	
	/// conversion to 32bit number operator
	inline operator int32_t() const {
		int32_t ret = b;
		ret = (ret << 16) | a;
		return ret;
	}
};

/**
 * @brief hexagonal grid representation for A* solver based on 2x16bit point map
 */
struct HexagonalGridPoint : public Point16 {
	/// constructor
	inline HexagonalGridPoint(const int16_t _a, const int16_t _b) : Point16(_a, _b) { }
	/// constructor
	inline HexagonalGridPoint(const int32_t _x) : Point16(_x) { }
	/// constructor
	inline HexagonalGridPoint() : Point16() { }
	/// constructor
	inline HexagonalGridPoint(Ogre::Vector3 ogrePoint) { fromOgre(ogrePoint); }
	
	/**
	 * @brief set hexagonal grid point from Ogre 3D point
	 * 
	 * @param[in] ogrePoint - 3D world point vector
	 */
	inline void fromOgre(Ogre::Vector3 ogrePoint) {
		a = round( ogrePoint.x / distanceX ); // yes, in some cases hit in neighboring hexagen ...
		b = round( (ogrePoint.z + (a & 1) * halfDistanceY) / distanceY );
	}
	
	/**
	 * @brief get Ogre X,Z coordinate from hexagonal grid point
	 */
	inline Ogre::Vector3 toOgre() const {
		return Ogre::Vector3(
			a * distanceX,
			0.0,
			b * distanceY - (a & 1) * halfDistanceY
		);
	}
	
	/**
	 * @brief return direction info betwen this point and @a neighbor point
	 * 
	 * @note direction info:
	 * -# depends on the order of points
	 * -# depends on the distance betwen points (is NOT normalized)
	 * -# not depend on space on the grid
	 * -# (only) for neighboring points is comparable
	 * 
	 * @param[in] neighbor - gridPoint for calculate direction info
	 */
	inline uint16_t getDirection(MGE::Point16 neighbor) const {
		char dx = neighbor.a - a + 0x0c;                // x offset - for neighboring hexagen: -2, -1, 0, 1, 2
		char dy = neighbor.b - b + (a & dx & 1) + 0x0c; // y offset - for neighboring hexagen: -1, 0, 1, 2
		return dx | (dy << 8);
	}
	
	/**
	 * @brief convert Ogre X,Z direction to grid direction
	 */
	static uint16_t getDirection(float x, float z);
	
	/**
	 * @brief convert Ogre X,Z direction to grid direction
	 */
	uint16_t getDirection(const Ogre::Vector3& dir);
	
	/**
	 * @brief get bIndex value for neighborOffset
	 * 
	 * @return
	 * - 1 if node is in upper sub row (q%2 = 0)
	 * - 2 if node is in lower sub row (q%2 = 1)
	 */
	inline int getBIndex() const {
		return 1 + (a & 1);
	}
	
	
	/**
	 * @brief return neighbor gridPoint
	 * 
	 * @param[in] neighbor - number of neighbor to return
	 * @param[in] bIndex   - index for getting row offset from neighborOffset (1 or 2 - depending of q%2 == 0 for current node)
	 * 
	 * Example:
	 	\code{.cpp}
	 	HexagonalGridPoint startNode(...);
	 	
	 	int bIndex = startNode.getBIndex();
	 	for (int i=0; i<startNode.getNeighborCount(); ++i) {
	 		HexagonalGridPoint dstNode = startNode.getNeighbor(i, bIndex)
	 		int   dstMode = startNode.getNeighborMode(i);
	 		float dstCost = startNode.getNeighborCost(dstMode);
	 		...
	 	}
	 	\endcode
	 */
	HexagonalGridPoint getNeighbor(int neighbor, int bIndex) const {
		#ifdef MGE_DEBUG_HEXAGONAL_GRID
		LOG_DEBUG("getNeighbor: " << int(neighborOffset[0][neighbor]) << "x" << int(neighborOffset[bIndex][neighbor]) << " => " << int(a + neighborOffset[0][neighbor]) << "x" << int(b + neighborOffset[bIndex][neighbor]));
		#endif
		return HexagonalGridPoint(a + neighborOffset[0][neighbor], b + neighborOffset[bIndex][neighbor]);
	}
	
	/**
	 * @brief return neighbor gridPoint
	 * 
	 * @param[in] neighbor - number of neighbor to return
	 */
	HexagonalGridPoint getNeighbor(int neighbor) const {
		return getNeighbor(neighbor, getBIndex());
	}
	
	/**
	 * @brief return neighbor mode (0 for direct neighbors, 1 for "diagonals" neighbors)
	 */
	inline int getNeighborMode(int neighbor) const {
		return neighborOffset[3][neighbor];
	}
	
	/**
	 * @brief return distance (cost) move to neighbor, base on neighbor mode
	 */
	inline float getNeighborCost(int mode) const {
		return neighborCost[mode];
	}
	
	/**
	 * @brief return neighbor count (max + 1 value of @a neighbor in getNeighbor() getNeighborCost()
	 */
	inline static constexpr int getNeighborCount() {
		return 12;
	}
	
	
	/// Y size of hexagon
	static float distanceY;
	
	/// distance in x between adjacent hexagon == 3/4 X size of hexagon:
	///  3/4 * (distanceY * 2/sqrt(3)) == distanceY * 1.5/sqrt(3)
	static float distanceX;
	
	/// half of @ref distanceY == distanceY/2.0
	static float halfDistanceY;
	
	/// distances to neighbors (0 for direct neighbors, 1 for "diagonals" neighbors)
	static float neighborCost[2];
	
	/**
	 * neighbor offset values:
	 * 
	 * - neighborOffset[0] - column (q) offset
	 * - neighborOffset[1] - row (r) offset for node with q%2 = 0 (upper sub row)
	 * - neighborOffset[2] - row (r) offset for node with q%2 = 1 (lower sub row)
	 * - neighborOffset[3] - 0 for direct neighbors, 1 for "diagonals" neighbors
	 */
	static constexpr char  neighborOffset[4][12] = {
	                                                  { -2,  0,  2,  0,   -1, -1, -1, -1,   1,  1,  1,  1 },
	                                                  {  0,  1,  0, -1,   -1,  0,  2,  1,  -1,  0,  2,  1 },
	                                                  {  0,  1,  0, -1,   -2, -1,  1,  0,  -2, -1,  1,  0 },
	                                                  {  1,  0,  1,  0,    1,  0,  1,  0,   1,  0,  1,  0 }
	                                               };
	
	/**
	 * @brief initialize static grid sizing information
	 * 
	 * @note this function is not parallel thread safe
	 */
	static void init(float size);
};

std::ostream & operator<< (std::ostream& output, const MGE::Point16& p);

/// @}

}
