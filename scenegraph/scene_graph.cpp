#include "scene_graph.hpp"

string SceneGraph::getName(){
    return name;
}

void SceneGraph::setName(string new_name){
    name = new_name;
}

Node * SceneGraph::getRoot(){
    return root;
}

void SceneGraph::setRoot(Node * new_root){
    root = new_root;
}


/* int main() {
    Node * root = new Node();
    root->name = "root";
    cout<<"Hello " << root->getName() << " from root\n";

    SceneGraph * scene = new SceneGraph();
    
    scene->setRoot(root);
    cout<<"Hello " << scene->getRoot()->getName() << " from scene\n";
} */