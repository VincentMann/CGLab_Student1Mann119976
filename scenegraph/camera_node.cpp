#include "camera_node.hpp"

bool camera_node::getPerspective(){
    return isPerspective;
}


bool camera_node::getEnabed(){
    return isEnabled;
}

void camera_node::setEnabled(bool new_value){
    isEnabled = new_value;
}
glm::mat4 camera_node::getProjectionMatrix(){
    return projectionMatrix;
}

void camera_node::setProjectionMatrix(glm::mat4 new_matrix){
    projectionMatrix = new_matrix;
}