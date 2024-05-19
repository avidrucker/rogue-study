# Map Setup

1. Initialize map w/ height and width dimensions and an empty tile char matrix, we can call this the "tile map matrix"
2. Define the quadrants and their room placement as a 3x3 int matrix of all 0's, we can call this the "room adjacency matrix"
3. Randomly "place" 5 to 9 rooms into the int matrix by flipping 5 to 9 of the 0's in the room adjacency matrix to 1's.
4. Check to make sure that the rooms (1's) are all fully cardinally adjacent to each other, and that there are no "islands" of 1 or more rooms that cannot be connected to the others
5. Once the rooms are known to be fully adjacent, we can randomly cardinally (vertically or horizontally) connect them to each other until the rooms are fully connected / fully traversable, populating a "room connections matrix" that has 1's where two rooms are connected (for example if rooms 3 and 5 are connected, there would be a 1's at (3,5) and (5,3))
6. Once all the rooms are fully connected/traversable, we can determine where to place the player, the exit, and the treasure
7. Lastly, we can draw/paint to the tile map matrix the rooms, the corridors, the doors (intersections of corridors and room walls), the player, the exit, and the treasure
