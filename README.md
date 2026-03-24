 Ghost Hunt Simulation - COMP2401 Final Project
 Name: Ahmerd Marhnouj
 Studnet Nmber: 101342709
 

Compilation Instructions
make

 Execution Instructions
./ghost_sim

Enter hunter names and IDs when prompted. Type "done" to start the simulation.

 File Descriptions

- main.c: Main control flow and thread management
- hunter.c: Hunter behavior logic
- ghost.c: Ghost behavior logic
- room.c: Room management and stack operations
- evidence.c: Bitwise operations for evidence
- house.c: House initialization and cleanup
- helpers.c: Provided logging functions
- defs.h: Structure definitions
- helpers.h: Helper function declarations
- Makefile: Build configuration

Collaboration
Working independently - no collaboration
Sources
- Course lecture slides on semaphores
- Assignment 3 for linked list implementation (modified for Room pointers)
- Project specification Appendix on deadlock prevention

 Assumptions
- Hunters can point to van during initialization without being in occupancy array (per R-16.4)
- When returning to van, hunters follow exact reverse path
- Ghost picks one random evidence type from its three types when leaving evidence