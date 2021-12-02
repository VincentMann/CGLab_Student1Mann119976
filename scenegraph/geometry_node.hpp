#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP

#include "node.hpp"
//#include "model.hpp"

class geometry_node : public Node{
    public:
    // Values
    model geometry;
    glm::vec3 geo_color;

    // Methods
    model getGeometry();
    void setGeometry(model new_geometry);
    using Node::Node;

};

#endif