#ifndef __NODE_H__
#define __NODE_H__

#include <iostream>
#include <vector>

class Node
{
public:
    std::vector<int> data_content;
    std::vector<Node *> sons;
    Node * parent;
public:
    Node();
};

#endif // __NODE_H__