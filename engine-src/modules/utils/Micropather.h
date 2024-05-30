/*
Copyright (c) 2000-2009 Lee Thomason (www.grinninglizard.com)
Grinning Lizard Utilities.

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and 
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.

Some small changes (class structure, etc) by Robert Paciorek (2013-2022) for Modular Game Engine
*/

#pragma once

//#define DEBUG_PATH
//#define DEBUG_PATH_DEEP
//#define TRACK_COLLISION

#include "pragma.h"
#include <inttypes.h>

#include "physics/utils/HexagonalGrid.h"

#ifndef MICROPATHER_STATE_TYPE
//#define MICROPATHER_STATE_TYPE void*
#define MICROPATHER_STATE_TYPE MGE::HexagonalGridPoint
#endif

#include <vector>
#include <float.h>

#if defined( _DEBUG )
#	ifndef MGE_DEBUG
#		define DEBUG
#	endif
#	if defined( _MSC_VER )
#		define MPASSERT( x )		if ( !(x)) { _asm { int 3 } }
#	else
#		include <assert.h>
#		define MPASSERT assert
#	endif
#else
#	define MPASSERT( x ) {}
#endif

//#define MICROPATHER_STRESS

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/** @brief Search path for moving objects (path finder)
 * 
 * MicroPather is a path finder and A* solver (astar or a-star) written in platform independent 
 * C++ that can be easily integrated into existing code. MicroPather focuses on being a path 
 * finding engine for video games but is a generic A* solver. MicroPather is open source, with 
 * a license suitable for open source or commercial use.
 * 
 * An overview of using MicroPather is in the <A HREF="../readme-micropather.htm">readme</A> or
 * on the Grinning Lizard website: http://grinninglizard.com/micropather/
 */
class MicroPather {
friend class PathNode;

public:
	/**
		Used to pass the cost of states from the cliet application to MicroPather. This
		structure is copied in a vector.
		
		@sa AdjacentCost
	*/
	struct StateCost {
		MICROPATHER_STATE_TYPE state;			///< The state as a MICROPATHER_STATE_TYPE
		float cost;				///< The cost to the state. Use FLT_MAX for infinite cost.
	};
	
	
	/**
		A pure abstract class used to define a set of callbacks. 
		The client application inherits from 
		this class, and the methods will be called when MicroPather::Solve() is invoked.
		
		The notion of a "state" is very important. It must have the following properties:
		- Unique
		- Unchanging (unless MicroPather::Reset() is called)
		
		If the client application represents states as objects, then the state is usually
		just the object cast to a MICROPATHER_STATE_TYPE. If the client application sees states as numerical
		values, (x,y) for example, then state is an encoding of these values. MicroPather
		never interprets or modifies the value of state.
	*/
	class Graph {
	public:
		virtual ~Graph() {}
		
		/**
			Return the least possible cost between 2 states. For example, if your pathfinding 
			is based on distance, this is simply the straight distance between 2 points on the 
			map. If you pathfinding is based on minimum time, it is the minimal travel time 
			between 2 points given the best possible terrain.
		*/
		virtual float LeastCostEstimate( MICROPATHER_STATE_TYPE stateStart, MICROPATHER_STATE_TYPE stateEnd ) = 0;
		
		/** 
			Return the exact cost from the given state to all its neighboring states. This
			may be called multiple times, or cached by the solver. It* must* return the same
			exact values for every call to MicroPather::Solve(). It should generally be a simple,
			fast function with no callbacks into the pather.
		*/	
		virtual void AdjacentCost( MICROPATHER_STATE_TYPE state, std::vector< StateCost >* adjacent ) = 0;
		
		/**
			This function is only used in DEBUG mode - it dumps output to stdout. Since MICROPATHER_STATE_TYPE 
			aren't really human readable, normally you print out some concise info (like "(1,2)") 
			without an ending newline.
		*/
		virtual void  PrintStateInfo( MICROPATHER_STATE_TYPE state ) = 0;
	};
	
private:
	/**
		Every state (MICROPATHER_STATE_TYPE) is represented by a PathNode in MicroPather. There
		can only be one PathNode for a given state.
	*/
	class PathNode {
	public:
		void Init(	unsigned _frame,
					MICROPATHER_STATE_TYPE _state,
					float _costFromStart, 
					float _estToGoal, 
					PathNode* _parent );
		
		void Clear() {
			MGE_GCC_WARNING_IGNORED("-Wclass-memaccess")
			memset( this, 0, sizeof( PathNode ) );
			MGE_GCC_WARNING_POP
			numAdjacent = -1;
			cacheIndex  = -1;
		}
		void InitSentinel() {
			Clear();
			Init( 0, 0, FLT_MAX, FLT_MAX, 0 );
			prev = next = this;
		}
		
		MICROPATHER_STATE_TYPE state;			// the client state
		float costFromStart;	// exact
		float estToGoal;		// estimated
		float totalCost;		// could be a function, but save some math.
		PathNode* parent;		// the parent is used to reconstruct the path
		unsigned frame;			// unique id for this path, so the solver can distinguish
								// correct from stale values
		
		int numAdjacent;		// -1  is unknown & needs to be queried
		int cacheIndex;			// position in cache
		
		PathNode* child[2];		// Binary search in the hash table. [left, right]
		PathNode* next, *prev;	// used by open queue
		
		bool inOpen;
		bool inClosed;
		
		void Unlink() {
			next->prev = prev;
			prev->next = next;
			next = prev = 0;
		}
		void AddBefore( PathNode* addThis ) {
			addThis->next = this;
			addThis->prev = prev;
			prev->next = addThis;
			prev = addThis;
		}
		#ifdef MGE_DEBUG
		void CheckList()
		{
			MPASSERT( totalCost == FLT_MAX );
			for( PathNode* it = next; it != this; it=it->next ) {
				MPASSERT( it->prev == this || it->totalCost >= it->prev->totalCost );
				MPASSERT( it->totalCost <= it->next->totalCost );
			}
		}
		#endif

		void CalcTotalCost() {
			if ( costFromStart < FLT_MAX && estToGoal < FLT_MAX )
				totalCost = costFromStart + estToGoal;
			else
				totalCost = FLT_MAX;
		}
	
	private:
		void operator=( const PathNode& );
	};
	
	struct NodeCost {
		PathNode* node;
		float cost;
	};
	
	/** Memory manager for the PathNodes. */
	class PathNodePool {
	public:
		PathNodePool( unsigned allocate, unsigned typicalAdjacent );
		~PathNodePool();
		
		// Free all the memory except the first block. Resets all memory.
		void Clear();
		
		// Essentially:
		// pNode = Find();
		// if ( !pNode )
		//		pNode = New();
		//
		// Get the PathNode associated with this state. If the PathNode already
		// exists (allocated and is on the current frame), it will be returned. 
		// Else a new PathNode is allocated and returned. The returned object
		// is always fully initialized.
		//
		// NOTE: if the pathNode exists (and is current) all the initialization
		//       parameters are ignored.
		PathNode* GetPathNode(		unsigned _frame,
									MICROPATHER_STATE_TYPE _state,
									float _costFromStart, 
									float _estToGoal, 
									PathNode* _parent );
		
		// Store stuff in cache
		bool PushCache( const NodeCost* nodes, int nNodes, int* start );
		
		// Get neighbors from the cache
		// Note - always access this with an offset. Can get re-allocated.
		void GetCache( int start, int nNodes, NodeCost* nodes ) {
			MPASSERT( start >= 0 && start < cacheCap );
			MPASSERT( nNodes > 0 );
			MPASSERT( start + nNodes <= cacheCap );
			memcpy( nodes, &cache[start], sizeof(NodeCost)*nNodes );
		}
		
		// Return all the allocated states. Useful for visuallizing what
		// the pather is doing.
		void AllStates( unsigned _frame, std::vector< MICROPATHER_STATE_TYPE >* stateVec, bool getall = true );

	private:
		struct Block
		{
			Block* nextBlock;
			PathNode pathNode[1];
		};
		
		unsigned Hash( MICROPATHER_STATE_TYPE voidval );
		unsigned HashSize() const	{ return 1<<hashShift; }
		unsigned HashMask()	const	{ return ((1<<hashShift)-1); }
		void AddPathNode( unsigned key, PathNode* p );
		Block* NewBlock();
		PathNode* Alloc();
		
		PathNode**	hashTable;
		Block*		firstBlock;
		Block*		blocks;
		
		NodeCost*	cache;
		int			cacheCap;
		int			cacheSize;
		
		PathNode	freeMemSentinel;
		unsigned	allocate;				// how big a block of pathnodes to allocate at once
		unsigned	nAllocated;				// number of pathnodes allocated (from Alloc())
		unsigned	nAvailable;				// number available for allocation
		
		unsigned	hashShift;	
		unsigned	totalCollide;
	};
	
	
public:
	enum {
		SOLVED,
		NO_SOLUTION,
		START_END_SAME,
	};
	
	/**
		Construct the pather, passing a pointer to the object that implements
		the Graph callbacks.

		@param graph		The "map" that implements the Graph callbacks.
		@param allocate		How many states should be internally allocated at a time. This
							can be hard to get correct. The higher the value, the more memory
							MicroPather will use.
							- If you have a small map (a few thousand states?) it may make sense
								to pass in the maximum value. This will cache everything, and MicroPather
								will only need one main memory allocation. For a chess board, allocate 
								would be set to 8x8 (64)
							- If your map is large, something like 1/4 the number of possible
								states is good. For example, Lilith3D normally has about 16,000 
								states, so 'allocate' should be about 4000.
							- If your state space is huge, use a multiple (5-10x) of the normal
								path. "Occasionally" call Reset() to free unused memory.
		@param typicalAdjacent	Used to determine cache size. The typical number of adjacent states
								to a given state. (On a chessboard, 8.) Higher values use a little
								more memory.
	*/
	MicroPather( Graph* graph, unsigned allocate = 250, unsigned typicalAdjacent=6 );
	~MicroPather();

	/**
		Solve for the path from start to end.
		
		@param startState	Input, the starting state for the path.
		@param endState		Input, the ending state for the path.
		@param path			Output, a vector of states that define the path. Empty if not found.
		@param totalCost	Output, the cost of the path, if found.
		@return				Success or failure, expressed as SOLVED, NO_SOLUTION, or START_END_SAME.
	*/
	int Solve( MICROPATHER_STATE_TYPE startState, MICROPATHER_STATE_TYPE endState, std::vector< MICROPATHER_STATE_TYPE >* path, float* totalCost );

	/**
		Find all the states within a given cost from startState.
		
		@param startState	Input, the starting state for the path.
		@param near			All the states within 'maxCost' of 'startState', and cost to that state.
		@param maxCost		Input, the maximum cost that will be returned. (Higher values return
							larger 'near' sets and take more time to compute.)
		@return				Success or failure, expressed as SOLVED or NO_SOLUTION.
	*/
	int SolveForNearStates( MICROPATHER_STATE_TYPE startState, std::vector< StateCost >* near, float maxCost );

	/** Should be called whenever the cost between states or the connection between states changes.
		Also frees overhead memory used by MicroPather, and calling will free excess memory.
	*/
	void Reset();

	/**
		Return the "checksum" of the last path returned by Solve(). Useful for debugging,
		and a quick way to see if 2 paths are the same.
	*/
	size_t Checksum()	{ return checksum; }

	/// Debugging function to return all states that were used by the last "solve" 
	void StatesInPool( std::vector< MICROPATHER_STATE_TYPE >* stateVec, bool getall = true );

private:
	MicroPather( const MicroPather& );	// undefined and unsupported
	void operator=( const MicroPather ); // undefined and unsupported
	
	void GoalReached( PathNode* node, MICROPATHER_STATE_TYPE start, MICROPATHER_STATE_TYPE end, std::vector< MICROPATHER_STATE_TYPE >* path );
	
	void GetNodeNeighbors(	PathNode* node, std::vector< NodeCost >* neighborNode );
	
	#ifdef MGE_DEBUG
	//void DumpStats();
	#endif
	
	PathNodePool				pathNodePool;
	std::vector< StateCost >	stateCostVec;	// local to Solve, but put here to reduce memory allocation
	std::vector< NodeCost >		nodeCostVec;	// local to Solve, but put here to reduce memory allocation
	
	Graph* graph;
	unsigned frame;						// incremented with every solve, used to determine if cached data needs to be refreshed
	size_t checksum;						// the checksum of the last successful "Solve".
	
	class OpenQueue {
	public:
		OpenQueue( Graph* _graph )
		{ 
			graph = _graph; 
			sentinel = reinterpret_cast<PathNode*>(sentinelMem);
			sentinel->InitSentinel();
			#ifdef MGE_DEBUG
				sentinel->CheckList();
			#endif
		}
		~OpenQueue()	{}

		void Push( PathNode* pNode );
		PathNode* Pop();
		void Update( PathNode* pNode );
		
		bool Empty()	{ return sentinel->next == sentinel; }

	private:
		OpenQueue( const OpenQueue& );	// undefined and unsupported
		void operator=( const OpenQueue& );
	
		PathNode* sentinel;
		int sentinelMem[ ( sizeof( PathNode ) + sizeof( int ) ) / sizeof( int ) ];
		Graph* graph;	// for debugging
	};
	
	class ClosedSet {
	public:
		ClosedSet( Graph* _graph )		{ this->graph = _graph; }
		~ClosedSet()	{}

		void Add( PathNode* pNode )
		{
			#ifdef MGE_DEBUG_PATH_DEEP
				printf( "Closed add: " );
				graph->PrintStateInfo( pNode->state );
				printf( " total=%.1f\n", pNode->totalCost );
			#endif
			#ifdef MGE_DEBUG
			MPASSERT( pNode->inClosed == 0 );
			MPASSERT( pNode->inOpen == 0 );
			#endif
			pNode->inClosed = 1;
		}

		void Remove( PathNode* pNode )
		{
			#ifdef MGE_DEBUG_PATH_DEEP
				printf( "Closed remove: " );
				graph->PrintStateInfo( pNode->state );
				printf( " total=%.1f\n", pNode->totalCost );
			#endif
			MPASSERT( pNode->inClosed == 1 );
			MPASSERT( pNode->inOpen == 0 );

			pNode->inClosed = 0;
		}

	private:
		ClosedSet( const ClosedSet& );
		void operator=( const ClosedSet& );
		Graph* graph;
	};
};

/// @}

}
