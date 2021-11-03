#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP

#include "node.hpp"
//#include "model.hpp"

class geometry_node : public Node{
    public:
    // Values
    model geometry;

    // Methods
    model getGeometry();
    void setGeometry(model new_geometry);

};

#endif