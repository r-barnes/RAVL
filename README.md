RAVL: (Richard's) AVL Tree Class
================================

An AVL tree is a form of self-balancing binary tree.

Many AVL tree implementations are difficult to understand, or do not include
a method to remove items from the tree. This class is, I think, cleanly written
and includes **insert** and **remove** methods, as well as various methods for
walking the tree.

Usage
-----
Make a new tree with: `AvlTree<int> tree;`

Add a node to the tree with: `tree.insert(3)`

Remove a node from the tree with: `tree.remove(3)`

Return the root of the tree with `tree.top()`

Return and remove the tree's root with: `tree.pop()`

Print a LaTeX qtree representation of the tree with `tree.qtreePrint(cout);`

Print a GraphViz DOT representation of the tree with `tree.print_dot(cout);`

Print the tree breadth-first from the root with `tree.levelPrint(cout);`
