# SpeedyList

An efficient doubly linked list implementation in pure C.

## Key Features
 - Elements are stored in a single buffer (per list instance) to increase cache-friendliness and prevent memory fragmentation
 - A special 'array mode', where the elements are rearranged to ensure indexed access in O(1)
 - Anti-tampering measures
 - Self-diagnostics
