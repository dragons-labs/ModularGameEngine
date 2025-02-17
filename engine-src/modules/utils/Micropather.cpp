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
*/

#ifdef _MSC_VER
#pragma warning( disable : 4786 )	// Debugger truncating names.
#pragma warning( disable : 4530 )	// Exception handler isn't used
#endif

#include <vector>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#include "modules/utils/Micropather.h"

namespace MGE {

void MicroPather::OpenQueue::Push( PathNode* pNode )
{
	
	MPASSERT( pNode->inOpen == 0 );
	MPASSERT( pNode->inClosed == 0 );
	
#ifdef MGE_DEBUG_MICROPATHER2
	printf( "Open Push: " );
	graph->PrintStateInfo( pNode->state );
	printf( " total=%.3f estToGoal=%.3f costFromStart=%.3f\n", pNode->totalCost, pNode->estToGoal, pNode->costFromStart );
#endif
	
	// Add sorted. Lowest to highest cost path. Note that the sentinel has
	// a value of FLT_MAX, so it should always be sorted in.
	MPASSERT( pNode->totalCost < FLT_MAX );
	PathNode* iter = sentinel->next;
	while ( true )
	{
		if ( (pNode->totalCost < iter->totalCost)
			 || (pNode->totalCost == iter->totalCost && pNode->estToGoal < iter->estToGoal)
		) {
			iter->AddBefore( pNode );
			pNode->inOpen = 1;
			break;
		}
		iter = iter->next;
	}
	MPASSERT( pNode->inOpen );	// make sure this was actually added.
#ifdef MGE_DEBUG
	sentinel->CheckList();
#endif
}

MicroPather::PathNode* MicroPather::OpenQueue::Pop()
{
#ifdef MGE_DEBUG_MICROPATHER2
	PathNode* iter = sentinel->next;
	while ( true )
	{
		printf(" -- ");
		graph->PrintStateInfo( iter->state );
		printf(" %.3f  %.3f %.3f\n", iter->totalCost , iter->estToGoal , iter->costFromStart);
		iter = iter->next;
		if (iter->totalCost == FLT_MAX) break;
	}
#endif
	
	MPASSERT( sentinel->next != sentinel );
	PathNode* pNode = sentinel->next;
	pNode->Unlink();
#ifdef MGE_DEBUG
	sentinel->CheckList();
#endif
	
	MPASSERT( pNode->inClosed == 0 );
	MPASSERT( pNode->inOpen == 1 );
	pNode->inOpen = 0;
	
#ifdef MGE_DEBUG_MICROPATHER2
	printf( "Open Pop: " );
	graph->PrintStateInfo( pNode->state );
	printf( " total=%.3f estToGoal=%.3f\n", pNode->totalCost, pNode->estToGoal );
#endif
	
	return pNode;
}

void MicroPather::OpenQueue::Update( PathNode* pNode )
{
#ifdef MGE_DEBUG_MICROPATHER2
	printf( "Open Update: " );		
	graph->PrintStateInfo( pNode->state );
	printf( " total=%.1f\n", pNode->totalCost );		
#endif
	
	MPASSERT( pNode->inOpen );
	
	// If the node now cost less than the one before it,
	// move it to the front of the list.
	if ( pNode->prev != sentinel && pNode->totalCost < pNode->prev->totalCost ) {
		pNode->Unlink();
		sentinel->next->AddBefore( pNode );
	}
	
	// If the node is too high, move to the right.
	if ( pNode->totalCost > pNode->next->totalCost ) {
		PathNode* it = pNode->next;
		pNode->Unlink();
		
		while ( pNode->totalCost > it->totalCost )
			it = it->next;
		
		it->AddBefore( pNode );
#ifdef MGE_DEBUG
		sentinel->CheckList();
#endif
	}
}

MicroPather::PathNodePool::PathNodePool( unsigned _allocate, unsigned _typicalAdjacent ) 
	: firstBlock( 0 ),
	  blocks( 0 ),
#if defined( MICROPATHER_STRESS )
	  allocate( 32 ),
#else
	  allocate( _allocate ),
#endif
	  nAllocated( 0 ),
	  nAvailable( 0 )
{
	freeMemSentinel.InitSentinel();

	cacheCap = allocate * _typicalAdjacent;
	cacheSize = 0;
	cache = static_cast<NodeCost*>( malloc(cacheCap * sizeof(NodeCost)) );

	// Want the behavior that if the actual number of states is specified, the cache 
	// will be at least that big.
	hashShift = 3;	// 8 (only useful for stress testing) 
#if !defined( MICROPATHER_STRESS )
	while( HashSize() < allocate )
		++hashShift;
#endif
	hashTable =  static_cast<PathNode**>( calloc( HashSize(), sizeof(PathNode*)) );

	blocks = firstBlock = NewBlock();
//	printf( "HashSize=%d allocate=%d\n", HashSize(), allocate );
	totalCollide = 0;
}


MicroPather::PathNodePool::~PathNodePool()
{
	Clear();
	free( firstBlock );
	free( cache );
	free( hashTable );
#ifdef TRACK_COLLISION
	printf( "Total collide=%d HashSize=%d HashShift=%d\n", totalCollide, HashSize(), hashShift );
#endif
}


bool MicroPather::PathNodePool::PushCache( const NodeCost* nodes, int nNodes, int* start ) {
	*start = -1;
	if ( nNodes+cacheSize <= cacheCap ) {
		for( int i=0; i<nNodes; ++i ) {
			cache[i+cacheSize] = nodes[i];
		}
		*start = cacheSize;
		cacheSize += nNodes;
		return true;
	}
	return false;
}


void MicroPather::PathNodePool::Clear()
{
#ifdef TRACK_COLLISION
	// Collision tracking code.
	int collide=0;
	for( unsigned i=0; i<HashSize(); ++i ) {
		if ( hashTable[i] && (hashTable[i]->child[0] || hashTable[i]->child[1]) )
			++collide;
	}
	//printf( "PathNodePool %d/%d collision=%d %.1f%%\n", nAllocated, HashSize(), collide, 100.0f*(float)collide/(float)HashSize() );
	totalCollide += collide;
#endif

	Block* b = blocks;
	while( b ) {
		Block* temp = b->nextBlock;
		if ( b != firstBlock ) {
			free( b );
		}
		b = temp;
	}
	blocks = firstBlock;	// Don't delete the first block (we always need at least that much memory.)

	// Set up for new allocations (but don't do work we don't need to. Reset/Clear can be called frequently.)
	if ( nAllocated > 0 ) {
		freeMemSentinel.next = &freeMemSentinel;
		freeMemSentinel.prev = &freeMemSentinel;
	
		memset( hashTable, 0, sizeof(PathNode*)*HashSize() );
		for( unsigned i=0; i<allocate; ++i ) {
			freeMemSentinel.AddBefore( &firstBlock->pathNode[i] );
		}
	}
	nAvailable = allocate;
	nAllocated = 0;
	cacheSize = 0;
}


MicroPather::PathNodePool::Block* MicroPather::PathNodePool::NewBlock()
{
	Block* block = static_cast<Block*>( calloc(1, sizeof(Block) + sizeof(PathNode)*(allocate-1)) );
	block->nextBlock = 0;

	nAvailable += allocate;
	for( unsigned i=0; i<allocate; ++i ) {
		freeMemSentinel.AddBefore( &block->pathNode[i] );
	}
	return block;
}


unsigned MicroPather::PathNodePool::Hash( MICROPATHER_STATE_TYPE voidval ) 
{
	/*
		Spent quite some time on this, and the result isn't quite satifactory. The
		input set is the size of a MICROPATHER_STATE_TYPE, and is generally (x,y) pairs or memory pointers.

		FNV resulting in about 45k collisions in a (large) test and some other approaches
		about the same.

		Simple folding reduces collisions to about 38k - big improvement. However, that may
		be an artifact of the (x,y) pairs being well distributed. And for either the x,y case 
		or the pointer case, there are probably very poor hash table sizes that cause "overlaps"
		and grouping. (An x,y encoding with a hashShift of 8 is begging for trouble.)

		The best tested results are simple folding, but that seems to beg for a pathelogical case.
		FNV-1a was the next best choice, without obvious pathelogical holes.

		Finally settled on h%HashMask(). Simple, but doesn't have the obvious collision cases of folding.
	*/
	/*
	// Time: 567
	// FNV-1a
	// http://isthe.com/chongo/tech/comp/fnv/
	// public domain.
	size_t val = (size_t)(voidval);
	const unsigned char* p = (unsigned char *)(&val);
	unsigned int h = 2166136261;

	for( size_t i=0; i<sizeof(size_t); ++i, ++p ) {
		h ^= *p;
		h *= 16777619;
	}
	// Fold the high bits to the low bits. Doesn't (generally) use all
	// the bits since the shift is usually < 16, but better than not
	// using the high bits at all.
	return ( h ^ (h>>hashShift) ^ (h>>(hashShift*2)) ^ (h>>(hashShift*3)) ) & HashMask();
	*/
	/*
	// Time: 526
	size_t h = (size_t)(voidval);
	return ( h ^ (h>>hashShift) ^ (h>>(hashShift*2)) ^ (h>>(hashShift*3)) ) & HashMask();
	*/

	// Time: 512
	// The HashMask() is used as the divisor. h%1024 has lots of common
	// repetitions, but h%1023 will move things out more.
	size_t h = static_cast<size_t>(voidval);
	return h % HashMask();	
}



MicroPather::PathNode* MicroPather::PathNodePool::Alloc()
{
	if ( freeMemSentinel.next == &freeMemSentinel ) {
		MPASSERT( nAvailable == 0 );

		Block* b = NewBlock();
		b->nextBlock = blocks;
		blocks = b;
		MPASSERT( freeMemSentinel.next != &freeMemSentinel );
	}
	PathNode* pathNode = freeMemSentinel.next;
	pathNode->Unlink();

	++nAllocated;
	MPASSERT( nAvailable > 0 );
	--nAvailable;
	return pathNode;
}


void MicroPather::PathNodePool::AddPathNode( unsigned key, PathNode* root )
{
	if ( hashTable[key] ) {
		PathNode* p = hashTable[key];
		while( true ) {
			int dir = (root->state < p->state) ? 0 : 1;
			if ( p->child[dir] ) {
				p = p->child[dir];
			}
			else {
				p->child[dir] = root;
				break;
			}
		}
	}
	else {
		hashTable[key] = root;
	}
}


MicroPather::PathNode* MicroPather::PathNodePool::GetPathNode( unsigned _frame, MICROPATHER_STATE_TYPE _state, float _costFromStart, float _estToGoal, PathNode* _parent )
{
	unsigned key = Hash( _state );

	PathNode* root = hashTable[key];
	while( root ) {
		if ( root->state == _state ) {
			if ( root->frame == _frame )		// This is the correct state and correct frame.
				break;
			// Correct state, wrong frame.
			root->Init( _frame, _state, _costFromStart, _estToGoal, _parent );
			break;
		}
		root = ( _state < root->state ) ? root->child[0] : root->child[1];
	}
	if ( !root ) {
		// allocate new one
		root = Alloc();
		root->Clear();
		root->Init( _frame, _state, _costFromStart, _estToGoal, _parent );
		AddPathNode( key, root );
	}
	return root;
}


void MicroPather::PathNode::Init(	unsigned _frame,
						MICROPATHER_STATE_TYPE _state,
						float _costFromStart, 
						float _estToGoal, 
						PathNode* _parent )
{
	state = _state;
	costFromStart = _costFromStart;
	estToGoal = _estToGoal;
	CalcTotalCost();
	parent = _parent;
	frame = _frame;
	inOpen = 0;
	inClosed = 0;
}

MicroPather::MicroPather( Graph* _graph, unsigned allocate, unsigned typicalAdjacent )
	:	pathNodePool( allocate, typicalAdjacent ),
		graph( _graph ),
		frame( 0 ),
		checksum( 0 )
{}


MicroPather::~MicroPather()
{
}

	      
void MicroPather::Reset()
{
	pathNodePool.Clear();
	frame = 0;
	checksum = 0;
}


void MicroPather::GoalReached( PathNode* node, MICROPATHER_STATE_TYPE start, MICROPATHER_STATE_TYPE end, vector< MICROPATHER_STATE_TYPE > *_path )
{
	std::vector< MICROPATHER_STATE_TYPE >& path = *_path;
	path.clear();

	// We have reached the goal.
	// How long is the path? Used to allocate the vector which is returned.
	int count = 1;
	PathNode* it = node;
	while( it->parent )
	{
		++count;
		it = it->parent;
	}

	// Now that the path has a known length, allocate
	// and fill the vector that will be returned.
	if ( count < 3 )
	{
		// Handle the short, special case.
		path.resize(2);
		path[0] = start;
		path[1] = end;
	}
	else
	{
		path.resize(count);

		path[0] = start;
		path[count-1] = end;
		count-=2;
		it = node->parent;

		while ( it->parent )
		{
			path[count] = it->state;
			it = it->parent;
			--count;
		}
	}

	checksum = 0;
	#ifdef MGE_DEBUG_MICROPATHER1
	printf( "Path: " );
	int counter=0;
	#endif
	for ( unsigned k=0; k<path.size(); ++k )
	{
		checksum += (static_cast<size_t>(path[k])) << (k%8);

		#ifdef MGE_DEBUG_MICROPATHER1
		graph->PrintStateInfo( path[k] );
		printf( " " );
		++counter;
		if ( counter == 8 )
		{
			printf( "\n" );
			counter = 0;
		}
		#endif
	}
	#ifdef MGE_DEBUG_MICROPATHER1
	printf( "Cost=%.1f Checksum %d\n", node->costFromStart, (unsigned int)checksum );
	#endif
}


void MicroPather::GetNodeNeighbors( PathNode* node, std::vector< NodeCost >* pNodeCost )
{
	if ( node->numAdjacent == 0 ) {
		// it has no neighbors.
		pNodeCost->resize( 0 );
	}
	else if ( node->cacheIndex < 0 )
	{
		// Not in the cache. Either the first time or just didn't fit. We don't know
		// the number of neighbors and need to call back to the client.
		stateCostVec.resize( 0 );
		graph->AdjacentCost( node->state, &stateCostVec );

		#ifdef MGE_DEBUG
		{
			// If this assert fires, you have passed a state
			// as its own neighbor state. This is impossible --
			// bad things will happen.
			for ( unsigned i=0; i<stateCostVec.size(); ++i )
				MPASSERT( stateCostVec[i].state != node->state );
		}
		#endif

		pNodeCost->resize( stateCostVec.size() );
		node->numAdjacent = stateCostVec.size();

		if ( node->numAdjacent > 0 ) {
			// Now convert to pathNodes.
			// Note that the microsoft std library is actually pretty slow.
			// Move things to temp vars to help.
			const unsigned stateCostVecSize = stateCostVec.size();
			const StateCost* stateCostVecPtr = &stateCostVec.at(0);
			NodeCost* pNodeCostPtr = &pNodeCost->at(0);

			for( unsigned i=0; i<stateCostVecSize; ++i ) {
				MICROPATHER_STATE_TYPE state = stateCostVecPtr[i].state;
				pNodeCostPtr[i].cost = stateCostVecPtr[i].cost;
				pNodeCostPtr[i].node = pathNodePool.GetPathNode( frame, state, FLT_MAX, FLT_MAX, 0 );
			}

			// Can this be cached?
			int start = 0;
			if ( pNodeCost->size() > 0 && pathNodePool.PushCache( pNodeCostPtr, pNodeCost->size(), &start ) ) {
				node->cacheIndex = start;
			}
		}
	}
	else {
		// In the cache!
		pNodeCost->resize( node->numAdjacent );
		NodeCost* pNodeCostPtr = &pNodeCost->at(0);
		pathNodePool.GetCache( node->cacheIndex, node->numAdjacent, pNodeCostPtr );

		// A node is uninitialized (even if memory is allocated) if it is from a previous frame.
		// Check for that, and Init() as necessary.
		for( int i=0; i<node->numAdjacent; ++i ) {
			PathNode* pNode = pNodeCostPtr[i].node;
			if ( pNode->frame != frame ) {
				pNode->Init( frame, pNode->state, FLT_MAX, FLT_MAX, 0 );
			}
		}
	}
}


#ifdef MGE_DEBUG
/*
void MicroPather::DumpStats()
{
	int hashTableEntries = 0;
	for( int i=0; i<HASH_SIZE; ++i )
		if ( hashTable[i] )
			++hashTableEntries;
	
	int pathNodeBlocks = 0;
	for( PathNode* node = pathNodeMem; node; node = node[ALLOCATE-1].left )
		++pathNodeBlocks;
	printf( "HashTableEntries=%d/%d PathNodeBlocks=%d [%dk] PathNodes=%d SolverCalled=%d\n",
			  hashTableEntries, HASH_SIZE, pathNodeBlocks, 
			  pathNodeBlocks*ALLOCATE*sizeof(PathNode)/1024,
			  pathNodeCount,
			  frame );
}
*/
#endif


void MicroPather::StatesInPool( std::vector< MICROPATHER_STATE_TYPE >* stateVec, bool getall )
{
 	stateVec->clear();
	pathNodePool.AllStates( frame, stateVec, getall );
}


void MicroPather::PathNodePool::AllStates( unsigned _frame, std::vector< MICROPATHER_STATE_TYPE >* stateVec, bool getall )
{	
    for ( Block* b=blocks; b; b=b->nextBlock )
    {
    	for( unsigned i=0; i<allocate; ++i )
    	{
    	    if ( b->pathNode[i].frame == _frame && (getall || b->pathNode[i].inClosed))
	    	    stateVec->push_back( b->pathNode[i].state );
    	}    
	}           
}   


int MicroPather::Solve( MICROPATHER_STATE_TYPE startNode, MICROPATHER_STATE_TYPE endNode, vector< MICROPATHER_STATE_TYPE >* path, float* cost )
{

	#ifdef MGE_DEBUG_MICROPATHER1
	printf( "Path: " );
	graph->PrintStateInfo( startNode );
	printf( " --> " );
	graph->PrintStateInfo( endNode );
	printf( " min cost=%f\n", graph->LeastCostEstimate( startNode, endNode ) );
	#endif

	*cost = 0.0f;

	if ( startNode == endNode )
		return START_END_SAME;

	++frame;

	OpenQueue open( graph );
	ClosedSet closed( graph );
	
	PathNode* newPathNode = pathNodePool.GetPathNode(	frame, 
														startNode, 
														0, 
														graph->LeastCostEstimate( startNode, endNode ), 
														0 );

	#ifdef MGE_DEBUG_MICROPATHER2
	int pop_cnt = 0;
	printf("Put start state to OpenQueue: ");
	#endif
	open.Push( newPathNode );
	stateCostVec.resize(0);
	nodeCostVec.resize(0);

	while ( !open.Empty() )
	{
		#ifdef MGE_DEBUG_MICROPATHER2
		printf("(%d) ", ++pop_cnt);
		if(pop_cnt > 1000) {
			printf("To many iteration - break\n");
			break;
		}
		#endif
		PathNode* node = open.Pop();
		
		if ( node->state == endNode )
		{
			GoalReached( node, startNode, endNode, path );
			*cost = node->costFromStart;
			#ifdef MGE_DEBUG_MICROPATHER1
			//DumpStats();
			#endif
			return SOLVED;
		}
		else
		{
			#ifdef MGE_DEBUG_MICROPATHER2
			printf("  -> ADD it to ClosedSet: ");
			#endif
			closed.Add( node );

			// We have not reached the goal - add the neighbors.
			GetNodeNeighbors( node, &nodeCostVec );

			for( int i=0; i<node->numAdjacent; ++i )
			{
				// Not actually a neighbor, but useful. Filter out infinite cost.
				if ( nodeCostVec[i].cost == FLT_MAX ) {
					continue;
				}
				PathNode* child = nodeCostVec[i].node;
				float newCost = node->costFromStart + nodeCostVec[i].cost;

				PathNode* inOpen   = child->inOpen ? child : 0;
				PathNode* inClosed = child->inClosed ? child : 0;
				PathNode* inEither = reinterpret_cast<PathNode*>( reinterpret_cast<size_t>(inOpen) | reinterpret_cast<size_t>(inClosed) );

				MPASSERT( inEither != node );
				MPASSERT( !( inOpen && inClosed ) );

				if ( inEither ) {
					if ( newCost < child->costFromStart ) {
						child->parent = node;
						child->costFromStart = newCost;
						child->estToGoal = graph->LeastCostEstimate( child->state, endNode );
						child->CalcTotalCost();
						if ( inOpen ) {
							#ifdef MGE_DEBUG_MICROPATHER2
							printf("  -> Put neighbors of ");
							graph->PrintStateInfo( node->state );
							printf(" in OpenQueue: ");
							#endif
							open.Update( child );
						}
					}
				}
				else {
					child->parent = node;
					child->costFromStart = newCost;
					child->estToGoal = graph->LeastCostEstimate( child->state, endNode );
					child->CalcTotalCost();
					
					MPASSERT( !child->inOpen && !child->inClosed );
					#ifdef MGE_DEBUG_MICROPATHER2
					printf("  -> Put neighbors of ");
					graph->PrintStateInfo( node->state );
					printf(" to OpenQueue: ");
					#endif
					open.Push( child );
				}
			}
		}
	}
	#ifdef MGE_DEBUG_MICROPATHER1
	//DumpStats();
	#endif
	return NO_SOLUTION;
}	

#if 0
int MicroPather::SolveForNearStates( MICROPATHER_STATE_TYPE startState, std::vector< StateCost >* near, float maxCost )
{
	/*	 http://en.wikipedia.org/wiki/Dijkstra%27s_algorithm

		 1  function Dijkstra(Graph, source):
		 2      for each vertex v in Graph:           // Initializations
		 3          dist[v] := infinity               // Unknown distance function from source to v
		 4          previous[v] := undefined          // Previous node in optimal path from source
		 5      dist[source] := 0                     // Distance from source to source
		 6      Q := the set of all nodes in Graph
				// All nodes in the graph are unoptimized - thus are in Q
		 7      while Q is not empty:                 // The main loop
		 8          u := vertex in Q with smallest dist[]
		 9          if dist[u] = infinity:
		10              break                         // all remaining vertices are inaccessible from source
		11          remove u from Q
		12          for each neighbor v of u:         // where v has not yet been removed from Q.
		13              alt := dist[u] + dist_between(u, v) 
		14              if alt < dist[v]:             // Relax (u,v,a)
		15                  dist[v] := alt
		16                  previous[v] := u
		17      return dist[]
	*/

	++frame;

	OpenQueue open( graph );			// nodes to look at
	ClosedSet closed( graph );

	nodeCostVec.resize(0);
	stateCostVec.resize(0);

	PathNode closedSentinel;
	closedSentinel.Clear();
	closedSentinel.Init( frame, 0, FLT_MAX, FLT_MAX, 0 );
	closedSentinel.next = closedSentinel.prev = &closedSentinel;

	PathNode* newPathNode = pathNodePool.GetPathNode( frame, startState, 0, 0, 0 );
	open.Push( newPathNode );
	
	while ( !open.Empty() )
	{
		PathNode* node = open.Pop();	// smallest dist
		closed.Add( node );				// add to the things we've looked at
		closedSentinel.AddBefore( node );
			
		if ( node->totalCost > maxCost )
			continue;		// Too far away to ever get here.

		GetNodeNeighbors( node, &nodeCostVec );

		for( int i=0; i<node->numAdjacent; ++i )
		{
			MPASSERT( node->costFromStart < FLT_MAX );
			float newCost = node->costFromStart + nodeCostVec[i].cost;

			PathNode* inOpen   = nodeCostVec[i].node->inOpen ? nodeCostVec[i].node : 0;
			PathNode* inClosed = nodeCostVec[i].node->inClosed ? nodeCostVec[i].node : 0;
			MPASSERT( !( inOpen && inClosed ) );
			PathNode* inEither = inOpen ? inOpen : inClosed;
			MPASSERT( inEither != node );

			if ( inEither && inEither->costFromStart <= newCost ) {
				continue;	// Do nothing. This path is not better than existing.
			}
			// Groovy. We have new information or improved information.
			PathNode* child = nodeCostVec[i].node;
			MPASSERT( child->state != newPathNode->state );	// should never re-process the parent.

			child->parent = node;
			child->costFromStart = newCost;
			child->estToGoal = 0;
			child->totalCost = child->costFromStart;

			if ( inOpen ) {
				open.Update( inOpen );
			}
			else if ( !inClosed ) {
				open.Push( child );
			}
		}
	}	
	near->clear();

	for( PathNode* pNode=closedSentinel.next; pNode != &closedSentinel; pNode=pNode->next ) {
		if ( pNode->totalCost <= maxCost ) {
			StateCost sc;
			sc.cost = pNode->totalCost;
			sc.state = pNode->state;

			near->push_back( sc );
		}
	}
#ifdef MGE_DEBUG
	for( unsigned i=0; i<near->size(); ++i ) {
		for( unsigned k=i+1; k<near->size(); ++k ) {
			MPASSERT( near->at(i).state != near->at(k).state );
		}
	}
#endif

	return SOLVED;
}
#endif
}
