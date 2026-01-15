# Dynamic Behavior (Plugin - Unreal Engine 4.27)

## Description
Dynamic Behavior is a plugin for Unreal Engine (version 4.27) that adds a dynamic runtime connection between the Behavior Tree, the Gameplay Ability System, and the AIController.

- **Connection between the Behavior Tree and the Gameplay Ability System** The following parameters have been added to each Task Node: a boolean variable responsible for the node's inclusion in the change system, and a category variable defining the category to which the specific node belongs. An int variable has been added to each Composite Node, responsible for the usage limit of a player ability tied to the in-game world. A category variable has been added to each instance of the GAS class, defining the category to which the specific ability belongs. The system tracks the number of uses of each ability, and upon reaching the limit of any Composite Node, the priorities of all Task Nodes belonging to that Composite Node are changed. The number of modified nodes is equal.

- **Изменение в классе AIController**
