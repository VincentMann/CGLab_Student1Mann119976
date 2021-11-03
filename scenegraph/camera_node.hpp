#ifndef CAMERA_NODE_HPP
#define CAMERA_NODE_HPP

#include "node.hpp"

class camera_node : public Node{
    public:
    // Values
    bool isPerspective;
    bool isEnabled;
    glm::mat4 projectionMatrix;

    // Methods
    bool getPerspective();
    bool getEnabed();
    void setEnabled(bool new_value);
    glm::mat4 getProjectionMatrix();
    void setProjectionMatrix(glm::mat4 new_matrix);
};

#endif