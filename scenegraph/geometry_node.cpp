#include "geometry_node.hpp"

model geometry_node::getGeometry(){
    return geometry;
}

void geometry_node::setGeometry(model new_geometry){
    geometry = new_geometry;
}