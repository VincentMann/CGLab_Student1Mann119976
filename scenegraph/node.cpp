#include "node.hpp"
#include "glm/gtx/string_cast.hpp"

// Simple return function
Node * Node::getParent(){
    return parent;
}

// Sets the parent ofthe current node and updates its depth and path, depending on predecessor
void Node::setParent(Node * newparent){
    depth = newparent->depth + 1;
    // If the father is the root, his path will be empty, "", in this case the nodes path will be based on the fathers name instead
    if(newparent->path == ""){
        path = newparent->name + ", " + name;
    }
    else{
        path = newparent->path + ", " + name;
    }
    parent = newparent;
}

// Iterates through the vector storing the child pointers of the node and returns the child node pointer if the names match
Node * Node::getChildren(string get_name){
    for (int i=0; i < (int)children.size(); i++){
        if (children[i]->name == get_name){
            return children[i];
        }
    }
    return NULL; 
}

// Returns the vector containing child node pointers
vector<Node *> Node::getChildrenList(){
    return children;
}

// Simple return function
string Node::getName(){
    return name;
}

// Simple return function
string Node::getPath(){
    return path;
}

// Simple return function
int Node::getDepth(){
    return depth;
}

// Returns localT of an object
glm::mat4 Node::getLocalTransform(){
    return localTransform;
}

// Updates localT of object
void Node::setLocalTransform(glm::mat4 new_local){
    localTransform = new_local;
}

// Returns the WorldTransform of an object: Its localT combined with its parents WorldT(called recursively)
glm::mat4 Node::getWorldTransform(){
    if(parent != NULL){
        return parent->getWorldTransform()*localTransform;
    }
    else{
        return globalTransform;
    }
}

// Similar process to setLocalTransform 
void Node::setWorldTransform(glm::mat4 new_global){
    globalTransform = new_global;
}

// Adds a child pointer to the vector containing the nodes children
void Node::addChildren(Node * newchild){ //Should work if child is already declar via "new" in a main funcion
    children.push_back(newchild);
}

// Iterates through the vector and removes the child node(s) with the requested name, the pointer to the node is returned afterwards
Node * Node::removeChildren(string child_name){
    Node * return_child = NULL;
    for (int i = 0; i < (int)children.size(); i++){
        if (children[i]->name == child_name){
            return_child = children[i];
            children.erase(children.begin() + i);
            return return_child;
        }
    }
    return return_child;
}


// Testing the .cpp
/* int main() {
    // Testing simple node creation and name getter
    Node * parent = new Node();
    parent->name = "dad";
    cout<<"Hello " << parent->getName() << "\n";

    // Testing child node creation, addition to parent node and respective getter/setter
    Node * child = new Node();
    child->name = "son";
    child->setParent(parent);
    parent->addChildren(child);

    cout<<"Hello " << parent->getChildren("son")->getName() <<"\n";
    cout<<"Hello " << child->getParent()->getName() << "\n";

    // Testing path and depth creation
    cout<<"Parent depth: " << parent->getDepth() << ", child depth: " << child->getDepth() << "\n";
    cout<<"Child path: " << child->getPath() << "\n";


} */