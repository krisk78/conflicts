# conflicts header-only library
Implements the class Conflicts<br>
The class Conflicts provides a specialized container that lists bidirectional conflict relationship between objects<br>
Rules:
* Create a relationship with the object itself is not allowed
* Doubles are not allowed, each relationship is unique
* Supports 2 distinct immutable modes at instantiation:
  + without cascading: only direct relationships are considered
  + with cascading: evaluation of relationships is done recursively (if A is in conflict with B that is in conflict with C then A is in conflict with C)
 
## Constructor
### Conflicts()
Instantiate a Conflicts object without cascading
### Conflicts(const bool cascading_mode)
Instantiate a Conflicts object using the given cascading mode

## Information

## CRUD methods
