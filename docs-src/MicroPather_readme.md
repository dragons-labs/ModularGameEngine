MicroPather    {#MicroPather_readme}
===========

<p>MicroPather is a path finder and A* solver (astar or a-star) written in platform 
  independent C++ that can be easily integrated into existing code. MicroPather 
  focuses on being a path finding engine for video games but is a generic A* solver. 
  MicroPather is open source, with a license suitable for open source or commercial 
  use.</p>
<p>The goals of MicroPather are:</p>
<ol>
  <li>Easy integration into games and other software</li>
  <li>Easy to use and simple interface</li>
  <li>Fast enough for real time use</li>
</ol>
<p>This document is an overview of MicroPather and how to use it. </p>
<ul>
  <li>The <a href="docs/index.html">API documentation</a> is in the docs subdirectory.</li>
  <li>The project and downloads are at <a href="https://sourceforge.net/projects/micropather/">SourceForge</a></li>
</ul>
<h2>Demo</h2>
<p>MicroPather comes with a demo application - dungeon.cpp - to show off pathing. 
  It's ASCII art dungeon exploring at its finest:</p>
<p><img src="demo0.gif" width="546" height="341"></p>
<p>The demo shows an ASCII art dungeon. You can move around by typing a new location, and it will 
  print the path to that location. In the screen shot above, the path starts in 
  the upper left corner, and steps to the &quot;i&quot; at about the middle of 
  the screen avoiding ASCII walls on the way. The numbers show the path from 0 
  to 9 then back to 0.</p>
<p>You can even open and close the doors to change possible paths. (&quot;d&quot; 
  at the command prompt.) </p>
<p>A Windows Visual C++ 2008 project file, a Linux Makefile, and an XCode project
  are provided. Building it for another environment is trivial: just compile dungeon.cpp, 
  micropather.h, and micropather.cpp to a command line app in your environment.</p>
<blockquote> 
  <p>Linux build &amp; run:</p>
  <blockquote>
    <pre>make

./dungeon</pre>
  </blockquote>
</blockquote>
<h2>Overview &amp; Links</h2>
<h3>About A* </h3>
<p>In video games, the pathfinding problem comes up in many modern games. What 
  is the shortest distance from point A to point B? Humans are good at that problem 
  - you pathfind naturally almost every time you move - but tricky to express 
  as a computer algorithm. A* is the workhorse technique to solve pathing. It 
  is directed, meaning that it is optimized to find a solution quickly rather 
  than by brute force, but it will never fail to find a solution if there is one.</p>
<p>A* is much more universal that just pathfinding. A* and MicroPather could be 
  used to find the solution to general state problems, for example it could be 
  used to solve for a &quot;rubiks cube&quot; puzzle.</p>
<p>The following terminology is used by MicroPather:</p>
<blockquote> 
  <p>The <strong>Graph </strong>is the search space. For the pathfinding problem, 
    this is your Map. It can be any kind of map: squares like the dungeon example, 
    polygonal, 3D, hexagons, etc. </p>
  <p>In pathfinding, a <strong>State</strong> is just a position on the Map. In 
    the demo, the player starts at State (0,0). Adjacent states are very important, 
    as you might image. If something is at state (1,1) in the dungeon example, 
    it has 8 adjacent states (0,1), (2,1) etc. it can move to. Why State instead 
    of location or node? The terminology comes from the more general application. 
    The &quot;states&quot; of a cube puzzle aren't locations. </p>
  <p>States are separated by <strong>Cost. </strong>For simple pathfinding in 
    the dungeon, the <strong>Cost</strong> is simply distance. The cost from state 
    (0,0) to (1,0) is 1.0, and the cost from (0,0) to (1,1) is sqrt(2), about 
    1.4. <strong>Cost</strong> is challenging and interesting because it can be 
    distance, time, or difficulty. In pathfinding:</p>
  <ul>
    <li>using distance as the cost will give the shortest length path</li>
    <li>using traversal time as the cost will give the fastest path</li>
    <li>using difficulty as the cost will give the easiest path</li>
  </ul>
</blockquote>
<p><strong>More info:</strong></p>
<p><a href="http://generation5.org/content/2000/astar.asp">http://generation5.org/content/2000/astar.asp</a></p>
<p><a href="http://www.gamedev.net/reference/programming/features/astar/">http://www.gamedev.net/reference/programming/features/astar/</a></p>
<p>On Gamasutra:</p>
<p><a href="http://www.gamasutra.com/features/19970801/pathfinding.htm">http://www.gamasutra.com/features/19970801/pathfinding.htm</a></p>
<p><a href="http://www.gamasutra.com/features/20010314/pinter_01.htm%20">http://www.gamasutra.com/features/20010314/pinter_01.htm</a></p>
<h2>Integrating MicroPather Into Your Code</h2>
<p>Nothing could by simpler! Or at least that's the goal. More importantly, none 
  of your game data structures need to change to use MicroPather. The steps, in 
  brief, are:</p>
<blockquote>
  <p>1. Include MicroPather files</p>
  <p>2. Implement the Graph interface</p>
  <p>3. Call the Solver</p>
  <p>Bonus: A better way to call the Solver</p>
</blockquote>
<h3>1. Include files </h3>
<p>There are only 2 files for micropather: micropather.cpp and micropather.h. 
  So there's no build, no make, just add the 2 files to your project. That's it. 
  They are standard C++ and don't require exceptions or RTTI. (I know, a bunch 
  of you like exceptions and RTTI. But it does make it less portable and slightly 
  slower to use them.)</p>
<p>Assuming you build a debug version of your project with _DEBUG or DEBUG (and 
  everyone does) MicroPather will run extra checking in these modes.</p>
<blockquote> 
  <p><em>Note: If you want very deep debugging, enable DEBUG_PATH in micropather.cpp.</em></p>
  <p><em>Note: If you are using other utilities in the grinliz library, putting 
    #include &quot;gldebug.h&quot; before #include &quot;glmicropather.h&quot; 
    will allow for the debugging output to work seamlessly.</em></p>
</blockquote>
<h3>2. Implement Graph Interface</h3>
<p>You have some class called Game, or Map, or World, that organizes and stores 
  your game map. This object (we'll call it Map) needs to inherit from the abstract 
  class Graph:</p>
<blockquote> 
  <p>class Map : public Graph</p>
</blockquote>
<p>Graph is pure abstract, so your map class won't be changed by it (except for 
  possibly gaining a vtable), or have strange conflicts. </p>
<p>Before getting to the methods of Graph, lets think states, as in:</p>
<blockquote> 
  <p>void Foo( void* state )</p>
</blockquote>
<p>The state pointer is provided by you, the game programmer. What it is? It is 
  a unique id for a state. For something like a 3D terrain map, like Lilith3D 
  uses, the states are pointers to a map structure, a &quot;QuadNode&quot; in 
  this case. So the state would simply be:</p>
<blockquote> 
  <p>void* state = (void*) quadNode;</p>
</blockquote>
<p>On the other hand, the Dungeon example doesn't have an object per map location, 
  just an x and y. It then uses:</p>
<blockquote> 
  <p>void* state = (void*)( y * MAPX + x );</p>
</blockquote>
<p>The state can be anything you want, as long as it is unique and you can convert 
  to it and from it.</p>
<p>Now, the methods of Graph.</p>
<blockquote> 
  <p><strong>virtual float LeastCostEstimate( void* stateStart, void* stateEnd 
    ) = 0;</strong></p>
  <p>This should return the least possible cost between 2 states. For example, 
    if your pathfinding is based on distance, this is simply the straight distance 
    between 2 points on the map. If you pathfinding is based on minimum time, 
    it is the minimal travel time between 2 points given the best possible terrain. 
  </p>
  <p>This function should be fast and not make any calls back into MicroPather.</p>
  <p><em>Note: There are performance and quality arguments for using a different 
    heuristic here. The links above discuss some other values you might want to 
    return for this function.</em></p>
  <p><strong>virtual void AdjacentCost( void* state, std::vector&lt; StateCost 
    &gt;* adjacent ) = 0;</strong></p>
  <p>AdjacentCost fills the &quot;adjacent&quot; array with all the states that 
    are next to this one, and the cost to get there. For the Dungeon example, 
    this would be the 8 tiles adjacent to a given tile, excluding ascii walls 
    and such. Your implementation of this method should have the following properties:</p>
  <ol>
    <li>Consistent with every call. This method may be called once for a given 
      method...or many times. The same values should be returned every time. If 
      this method returns different results to the same pather object, paths will 
      not be correct.</li>
    <li>No calls back into MicroPather</li>
    <li>Fast</li>
  </ol>
  <p> <strong>virtual void PrintStateInfo( void* state ) = 0;</strong></p>
  <p>This function is only used in DEBUG mode - it dumps output to stdout. Since 
    void* aren't really human readable, normally you print out some concise info 
    (like &quot;(1,2)&quot;) without an ending newline.</p>
</blockquote>
<h3>3. Call the Solver</h3>
<p>The easiest way to call the solver:</p>
<blockquote>
  <pre>{
	MicroPather pather( myGraph );

	std::vector&lt; void* &gt; path;
	float totalCost;


	int result = pather.Solve( startState, endState, &amp;path, &amp;totalCost );


	// do something with the path

}</pre>
</blockquote>
<p>That's it. Given the start state and the end state, the sequence of states 
  from start to end will be written to the std::vector.</p>
<h3>Bonus: A better way to call the Solver</h3>
<p>You may be thinking &quot;Performance is important&quot; and that creating 
  a MicroPather object for every path is not a good use of resources...you would 
  be correct. To get better performance out of MicroPather, you should create 
  the MicroPather object and keep it around.</p>
<blockquote> 
  <pre>pather = new MicroPather( myGraph );
</pre>
</blockquote>
<p>It will cache lots of information about your graph, and get faster as it is 
  called. However, for caching to work, the connections between states and the 
  costs of those connections must stay the same. (Else the cache information will 
  be invalid.) If costs between connections does change, be sure to call Reset()</p>
<blockquote> 
  <pre>pather-&gt;Reset()</pre>
</blockquote>
<p>The easiest way to good performance is to create a pather object and keep it 
  around. Every time something about the connections changes (a door open, terrain 
  changes, travel costs change, a new road is built) just call Reset(). Reset() 
  is a fast call, and is a good way to manage the pather.</p>
<h2>Performance Optimization</h2>
<ol>
  <li>Keep the MicroPather object around and use Reset()</li>
  <li>Set the &quot;allocate&quot; parameter in the MicroPather constructor. (See 
    the API docs)</li>
  <li>Set the MAX_CACHE appropriately in glmicropather.h (See the API docs)</li>
</ol>
<h2>Improvements &amp; Bugs</h2>
<h3>Removed Functionality</h3>
<p>MicroPather 1.1 removed StateCostChange(). The function allowed terrain costs 
  to change without calling Reset(). The performance enabled by the function was 
  small, especially compared to its complexity and ease of mis-use.</p>
<h3>Bugs</h3>
<p>First off, thanks. Thanks for taking the time to use MicroPather and file a 
  bug.</p>
<p>Please file bugs on the sourceforge.net site. While I appreciate email communication 
  about a bug, it is very hard for me to track bugs sent in via email.</p>
<p>The most important parts of a bug:</p>
<ol>
  <li>Simple description</li>
  <li>Small test case</li>
</ol>
<h3>Improvements</h3>
<p>I really like getting patches, improvements, and performance enhancements. 
  And I'm happy to credit the author, of course. Here are some guidelines for 
  patches that I'd like to take, versus changes that aren't appropriate to put 
  into source control.</p>
<h4>Guidelines for changes</h4>
<ol>
  <li>The goals of the project - listed at the top of this document - are what 
    they are. For example, there are easy ways to speed up the program by increasing 
    the requirements on the client applications, but this is against the goals 
    of the package. If in doubt, post a message in the forums.</li>
  <li>It can't break the pathfinder. (Of course :) Although detecting this can 
    be subtle.) dungeon.cpp prints out basic validation when it starts to verify 
    everything is still working.</li>
  <li>Performance improvements that help in some cases, shouldn't cause damage 
    in the general case. &quot;speed.cpp&quot; is a performance testing application 
    to validate performance of the system.</li>
</ol>
<p>Thanks! I've enjoyed making MicroPather, and would appreciate hearing about 
  where you use it and how it works for you. Drop me a line at leethomason at 
  users.sourceforge.net. </p>
<p>lee</p>
