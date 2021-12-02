#include "point_light_node.hpp"

glm::vec3 point_light_node::getlightColor(){
    return lightColor;
}

void point_light_node::setlightColor(glm::vec3 new_color){
    lightColor = new_color;
}

float point_light_node::getlightIntensity(){
    return lightIntensity;
}

void point_light_node::setlightIntensity(float new_intensity){
    lightIntensity = new_intensity;
}