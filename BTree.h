#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <list>
using namespace std;
#include "Node.h"

class BTree
{
private:
    Node * root;
    int order;
    std::list<Node*> leaf_list;
private:
    void insert_in_node(Node *& node,  int data_insert);
    void print_to_console(Node * tmp);
    void add_to_leaf_list(Node *& node);
public:
    BTree(int _order);
    void insert(int data_to_insert);
    int * search(int data_to_search);
    void remove(int data_to_remove);
    void print_to_console();
    void print_leaf_list();
};

#endif 