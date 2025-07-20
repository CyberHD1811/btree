# B-Tree

**Version *1.0.1***

**License *BSD-3-Clause-Clear***

## ⚠️ Disclaimer ⚠️

Although i tried my best to write an efficient implementation and correct/formal definitions, i cannot guarantee that they are either efficient, well designed and/or correct.
Also i must say that i'm just a computer science student and do not claim my definitions to be 100% correct.

If you have any questions or feedback, feel free to ask via a [Github Issue](https://github.com/CyberHD1811/b-tree/issues) or via [mail](btree.github@contact.cyberhd1811.eu).

## Introduction

During the course [ADS1 - VU 051024](https://ufind.univie.ac.at/de/course.html?lv=051024&semester=2025S), the senior lecturer [Sagar Kale, Ph.D.](https://ufind.univie.ac.at/de/person.html?id=109949) organized a small extracurricular contest, which required to write a rudimentary implementation of a '**b-tree**' as a generalization of the learned concepts of '*2-3 trees*'.

Since i really enjoyed the task of implementing a b-tree and found interest in implementing \[complex\] data structures, i want to share my implementation and thoughts. At the request of Mr. Kale, i made some changes to the method headers and logic to not provide a finished solution for these extracurricular contests \[if they are held in the future\].

I hope everyone can learn something from my thoughts.

## Formal definitions

(This section requires you to have a basic understanding of rooted trees and their terminology.)

There are several ways to define a b-tree. For a specific reason, i will point out later, i chose the following definition:

### Order property

Each node has keys and (if the node is not a leaf) edges to its children. The keys and children are stored alternating (Starting with a child, then a node, then a child and so on). Each key $k_i$ is $\leq$ than it's right neighbor key $k_j$. The keys within the child $c_i$ (and its subtree) left to $k_i$ are all $\leq k_i$, all keys within the child $c_j$ (and its subtree) between $k_i$ and $k_j$ are all $\geq k_i\ \land\leq k_j$ and all keys within the child $c_k$ right to $k_j$ are all $\geq k_j$. This means that each child $c_j$ between any two keys $k_i$ and $k_j$ stores the range $\left[k_i, k_j\right]$.
</br>
The inorder traversal gives the keys in an increasing order.

### Degree of a b-tree

A b-tree has a degree $d$, with $d\geq2$, which has the following characteristics:

- $d$ is the minimum number of children each *inner* node (except the root) must have.
  </br>
  $\rightarrow$ If the root is an *inner* node, it must have at least $2$ children.
- $d-1$ is the minimum number of keys each node (except the root) must have.
  </br>
  $\rightarrow$ The root can have $0$ keys, which would mean that the b-tree is empty.
- $2\cdot d-1$ is the maximum number of children each *inner* node can have.
- $2\cdot d-2$ is the maximum number of keys each node can have. 

*Note*: The first definition is specifically for *inner nodes*. If the root is a leaf, this definition does not apply to it and so the root does not violate it.

### Operations

There are three operations:

- **Search**: Searches a key within the b-tree.
- **Insert**: Inserts a key into a leaf of the b-tree.
- **Remove**: Removes a key from the b-tree. If the key is in an *inner* node, swap the key with the next key $\leq$ to its right and remove the key then in the leaf it was put in.

The operations *insert* and *remove* can violate the definitons of the *degree of a b-tree*. To prevent this, there are two ways (one for each operation):

- **Insert**: In case a node stores exactly $2\cdot d -1$ keys (called overflow) 
  1. Pick the middle key (since $2\cdot d-1$ is odd, there is always a middle key)
  2. Split the keys into two equal parts (excluding the middle key)
  3. \[If the node is not a leaf\] Split the children into two equal parts (possible since the number of children in this node is $2\cdot d$)
  4. Put both key ranges (and children for *inner* nodes) into two '*new*' nodes (without violating the *order property*)
  5. Add into the parent of the node with the overflow (if it does not exist, create a new node as new root) the middle key and the two children (left and right of the middle key respecting the key order)
  6. If the parent is now in an overflow, start over
- **Remove**: In case a node (except the root when it is a leaf) has exactly $d-2$ keys (called underflow)
  1. Check the left and right sibling of the node (in the specified order)
     - \[a\] Left sibling has at least $d$ keys
     - \[b\] Right sibling has at least $d$ keys
     - \[c\] Left sibling has $d-1$ keys
     - \[d\] Right sibling has $d-1$ keys
  2. For case \[a\] (called right rotation):
     1. Move the key seperating the node and left sibling from the parent into the node
     2. Move the last child (if it exists) from the left sibling into the node
     3. Move the last key from the left sibling to the parent
  3. For case \[b\] (called left rotation):
     1. Move the key seperating the node and right sibling from the parent into the node
     2. Move the first child (if it exists) from the right sibling into the node
     3. Move the first key from the right sibling to the parent
  4. For case \[c\] (called right merge):
     1. Move the key seperating the node and left sibling from the parent into the node
     2. Move all children (if they exist) from the left sibling into the node
     3. Move all keys from the left sibling into the node
     4. Delete the left sibling from the parent
  5. For case \[d\] (called left merge):
     1. Move the key seperating the node and right sibling from the parent into the node
     2. Move all children (if they exist) from the right sibling into the node
     3. Move all keys from the right sibling into the node
     4. Delete the right sibling from the parent
  6. If the parent is now in an underflow, start over

### Choice of definition

The main reason, why i chose the previous definition, is that in case of overflows there will always be a middle key and two key ranges with equal length, which perfectly form minimal nodes. In case of allowing $2\cdot d$ children $\Leftrightarrow$ $2\cdot d - 1$ keys; overflows would happen when a node has $2\cdot d$ keys which can't be split into a middle key **and** two parts with equal length.

## Implementation details

Please note that the current implementation requires that the template type ```T``` has a definition for the operators ```<=``` and ```==```. Also the current implementation is not meant to be used with any objects, since all parameters are called by value and not called by reference. This behaviour will likely change in the next few updates.

Currently duplicates are ignored.

A more explaining documentation will follow with version 1.1.0.
