#ifndef POINT_LIGHT_NODE_HPP
#define POINT_LIGHT_NODE_HPP

#include "node.hpp"
//#include "model.hpp"

class point_light_node : public Node{
    public:
    // Values
    glm::vec3 lightColor;
    float lightIntensity;

    // Methods
    glm::vec3 getlightColor();
    void setlightColor(glm::vec3 new_color);

    float getlightIntensity();
    void setlightIntensity(float new_intensity);
    
    using Node::Node;

};

#endif