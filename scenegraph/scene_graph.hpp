#ifndef SCENE_GRAPH_HPP
#define SCENE_GRAPH_HPP

#include "node.hpp"

class SceneGraph{
    public:
        // Values
        string name;
        Node * root;

        // Methods
        string getName();
        void setName(string new_name);
        Node * getRoot();
        void setRoot(Node * new_root);
        //string printgraph();

};

#endif