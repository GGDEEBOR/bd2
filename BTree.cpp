#include "BTree.h"

BTree::BTree(int order)
{
    this->root = new Node();
    this->order = order;
    this->leaf_list.push_back(this->root);
}

void BTree::add_to_leaf_list(Node *& node)
{
    if (!node) return;
    if (this->leaf_list.empty()) 
    {
        this->leaf_list.push_back(node);
        return;
    }
    int pos_begin = 0;
    int pos_end = this->leaf_list.size() - 1;
  
    auto begin = this->leaf_list.begin();
    auto end = this->leaf_list.end();

    while(pos_end - 1 > pos_begin)
    {
        std::cout << pos_begin << ' ' << pos_end << '\n';
        auto tmp = this->leaf_list.begin();
        std::advance(tmp, (pos_begin + pos_end) / 2);
        if ((*tmp)->data_content.back() < node->data_content.front())
        {
            pos_begin = (pos_begin + pos_end) / 2;
        }
        else
        {
            pos_end = (pos_begin + pos_end) / 2;
        }
        begin = tmp;
    }

    this->leaf_list.insert(begin, node);

}

void BTree::print_leaf_list()
{
    for (const auto& leaf : this->leaf_list)
    {
        for(int i = 0; i < leaf->data_content.size(); i++)
        {
            std::cout << leaf->data_content[i] << '-';
        }
    }
}

void BTree::insert_in_node(Node *& nodex, int data_to_insert)
{
    bool fin_flag = false;

    Node * node = nodex;

    Node * left = nullptr;
    Node * right = nullptr;
    
    while(!fin_flag)
    {
        
        bool flag = false;

        if (node == nullptr)
        {
            this->root = new Node();
            this->root->data_content.push_back(data_to_insert);
            this->root->sons.push_back(right);
            this->root->sons[0] = left;

            if (right) right->parent = this->root;
            if (left) left->parent = this->root;
            return;
        }

        for (int i = 0; i < node->data_content.size(); i++)
        {
            if (node->data_content[i] >= data_to_insert)   
            {
                flag = true;
                node->data_content.insert(node->data_content.begin() + i, data_to_insert);
                node->sons.insert(node->sons.begin() + i, left);
                node->sons[i + 1] = right;
                if (left) left->parent = node; 
                if (right) right->parent = node;
                break;
            }
        }
        if (!flag)
        {
            node->data_content.push_back(data_to_insert);
            node->sons.push_back(right);
            node->sons[node->sons.size() - 2] = left;
            if (left) left->parent = node;
            if (right) right->parent = node;
        }
        if (node->data_content.size() > order)
        {
            int median = node->data_content[(order + 1) / 2]; // Getting median from

            std::vector<Node *> aux_sons = node->sons;
            std::vector<int> aux_buffer = node->data_content;
            
            Node * brother = new Node();
            Node * nod = node;

            brother->data_content.clear();
            brother->sons.clear();
            nod->data_content.clear();
            nod->sons.clear();

            for (int i = 0; i < (order + 1) / 2; i++)
            {
                brother->data_content.push_back(aux_buffer[i]);
                brother->sons.push_back(aux_sons[i]);
                if (aux_sons[i]) brother->sons[i]->parent = brother;
            }
            brother->sons.push_back(aux_sons[(order + 1) / 2]);
            if ( brother->sons[(order + 1) / 2]) brother->sons[(order + 1) / 2]->parent = brother;
            
            if (aux_sons[0] == nullptr)
            {
                nod->data_content.push_back(median);
                nod->sons.push_back(aux_sons[(order + 1) / 2]);
            }

            for (int i = ((order + 1) / 2) + 1; i < aux_buffer.size(); i++)
            {
                nod->data_content.push_back(aux_buffer[i]);
                nod->sons.push_back(aux_sons[i]);
                if (aux_sons[i]) nod->sons[i - (((order + 1) / 2) + 1)]->parent = nod;
            }
            nod->sons.push_back(aux_sons[aux_buffer.size()]);
            if (aux_sons[aux_buffer.size()]) nod->sons[aux_buffer.size() - (((order + 1) / 2) + 1)]->parent = nod;

            //if (brother->data_content.front() > node->data_content.back() && nod && !nod->sons[0]) this->add_to_leaf_list(nod); 
            if (brother && !brother->sons[0]) this->add_to_leaf_list(brother); 

            left = brother;
            right = nod;            

            node = node->parent;
            
            data_to_insert = median;
            
            continue;
        }
        else break;
    }

}

void BTree::insert(int data_to_insert)
{
    Node * tmp = this->root;

    // locating leaf node to insert index data
    while(tmp->sons[0])
    {
        bool flag = false;
        for (int i = 0; i < tmp->data_content.size(); i++)
        {
            if (tmp->data_content[i] >= data_to_insert)
            {
                flag = true;
                tmp = tmp->sons[i];
                break;
            }
        }
        if (!flag) tmp = tmp->sons[tmp->data_content.size()];
    }

    this->insert_in_node(tmp,data_to_insert);
    
}

void BTree::print_to_console()
{
    std::cout << "Printing:\n";
    Node * tmp = this->root;
        for (int i = 0; i < tmp->data_content.size(); i++)
        {
            print_to_console(tmp->sons[i]);
            std::cout << tmp->data_content[i] << ' ';
        }
    print_to_console(tmp->sons[tmp->data_content.size()]);

    std::cout << '\n'; 

}

void BTree::print_to_console(Node * tmp)
{
    if (tmp == nullptr) return;
    for (int i = 0; i < tmp->data_content.size(); i++)
    {
        print_to_console(tmp->sons[i]);
        std::cout << tmp->data_content[i] << ' ';
    }
    print_to_console(tmp->sons[tmp->data_content.size()]);

    std::cout << '\n'; 

}
