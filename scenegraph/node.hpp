#ifndef NODE_HPP
#define NODE_HPP

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

using namespace std;
class Node {
    public:
        // Values
        string name = "placeholder";
        string path = "";
        int depth = 0;
        glm::mat4 localTransform;
        glm::mat4 globalTransform;

        // Methods
        Node * getParent();
        void setParent(Node * parent);
        Node * getChildren(string child_name);
        vector<Node *> getChildrenList();
        string getName();
        string getPath();
        int getDepth();
        glm::mat4 getLocalTransform();
        void setLocalTransform(glm::mat4 new_local);
        glm::mat4 getWorldTransform();
        void setWorldTransform(glm::mat4 new_global);
        void addChildren(Node * child); //in main(): parent->addChildren(& child)
        Node * removeChildren(string child_name);

    // Links
    Node * parent;
    vector<Node *> children;    // Stores pointers to children (allocated on heap with new) 
                                // Example:  Node * parent = NULL; parent = new Node(); 
                                // Node * child = NULL; child = new Node();
                                // parent->children.push_back(child)

    //Construct
    Node();
    Node(string name, Node * parent, glm::mat4 localTransform);


};

#endif