#include "application_solar.hpp"
#include "window_handler.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

// ------------------Personal includes------------------------------------------------------------------------
#include "scene_graph.hpp"
#include "geometry_node.hpp"
#include "camera_node.hpp"
#include "glm/gtx/string_cast.hpp"



// For some reason errors are generated: undefined reference to e.g "Node::getName[abi:cxx11]()«" is triggered,
// if the cpp files aren't included. Maybe Cmake.txt must be extended?
#include "node.cpp"
#include "geometry_node.cpp"
#include "scene_graph.cpp"

// ------------------Personal includes------------------------------------------------------------------------

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})}
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
{
  initializeSceneGraph();
  initializeGeometry();
  initializeShaderPrograms();
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

void ApplicationSolar::render() const {
  this->renderChildrenObjects();
}

//Personal Code --------------------

void ApplicationSolar::renderChildrenObjects() const{
  geometry_node * planet_geo = new geometry_node();
  // Iterating through the geometry_node_Vector and rendering each planets(or moons) position
  for (int i = 0; i < (int)geometry_node_Vector.size(); i++){
    planet_geo = geometry_node_Vector[i];

    // Rendering planet/moon object
    this->renderObject(planet_geo);
  }
}

void ApplicationSolar::renderObject(geometry_node * planet_geo) const{

  // Each holder already contains the planets relative postion in the solar system, set via translate() ininitializeSceneGraph()
  // This postion can be combined with the already written code to create the planets rotation and revolvement around the sun
  // bind shader to upload uniforms

  glUseProgram(m_shaders.at("planet").handle);

  glm::fmat4 model_matrix(1.0f);

  // SEE-1 in initializeSceneGraph():
  // First rotation is applied to the planets holder. So the matrix first rotates and then is translated by the 
  // holders translation matrix. This resultsin the object rotating around the current planets holder position
  // which can either be the center root or another planet in case of the moon

  Node * planet_holder = planet_geo->parent;
  // Rotation matrix:
  // Let moon rotate faster than planet for better visibility
  if (planet_geo->getName().find("moon") != std::string::npos) {
    // Moon speed:
    model_matrix = glm::rotate(glm::mat4{}, 0.005f, glm::fvec3{0.0f, 1.0f, 0.0f});
  }
  else{
    // Planet speed
    model_matrix = glm::rotate(glm::mat4{}, 0.001f, glm::fvec3{0.0f, 1.0f, 0.0f});
  }
  
  // Rotation matrix applied to localT of holder
  planet_holder->setLocalTransform(model_matrix * planet_holder->getLocalTransform());  


  // Let the object around its own axis(applied last in matrix calculation)
  model_matrix = glm::rotate(glm::mat4{}, 0.009f, glm::fvec3{0.0f, 1.0f, 0.0f});
  planet_geo->setLocalTransform(model_matrix*planet_geo->getLocalTransform());

  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                    1, GL_FALSE, glm::value_ptr(planet_geo->getWorldTransform()));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * planet_geo->getWorldTransform());
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                    1, GL_FALSE, glm::value_ptr(normal_matrix));

  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
}

//Personal Code --------------------


void ApplicationSolar::uploadView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // bind shader to which to upload unforms
  glUseProgram(m_shaders.at("planet").handle);
  // upload uniform values to new locations
  uploadView();
  uploadProjection();
}

///////////////////////////// intialisation functions /////////////////////////
// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/simple.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/simple.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
}

// load models
void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);
  
  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());
}

///////////////////////////// callback functions for window events ////////////
// handle key input
void ApplicationSolar::keyCallback(int key, int action, int mods) {
  if (key == GLFW_KEY_W  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -1.f});
    uploadView();
  }
  else if (key == GLFW_KEY_S  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 1.f});
    uploadView();
  }
  else if (key == GLFW_KEY_A  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-1.f, 0.0f, 0.0f});
    uploadView();
  }
  else if (key == GLFW_KEY_D  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{1.f, 0.0f, 0.0f});
    uploadView();
  }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
  // rotate(model/view, angle in rad, rotation axis)
  // mouse input: the slower the lower the number, fastest seems to be approx 200
  // e.g posx: move left: negative number, move right: pos. number
  // Rotate the camera around its own axis for the given x and y values.
  // X input: rotates camera from L to R or R to L
  // Y input: rotates camera from TOP to BOTTOM or BOTTOM to TOP
  pos_y = -pos_y;
  //m_view_transform = glm::rotate(m_view_transform, (float)pos_x/80.f, glm::fvec3{0.0f, 1.0f, 0.0f});
  //m_view_transform = glm::rotate(m_view_transform, (float)pos_y/80.f, glm::fvec3{1.0f, 0.0f, 0.0f});
  // Leaving this commented because for some reason the camera also rotates aroun the z axis(or so it seems)
  uploadView();
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
  // recalculate projection matrix for new aspect ration
  m_view_projection = utils::calculate_projection_matrix(float(width) / float(height));
  // upload new projection matrix
  uploadProjection();
}

// ------------------Personal scenegraph------------------------------------------------------------------------

void ApplicationSolar::initializeSceneGraph(){
  
  // Inits for nodes
  // Starting with the scene_graph
  // position_matrix will store the matrices responsible for translating/rotating/scaling our objects
  // The matrices are stored and called as follows:
  // CALLING: see node.cpp getWorldTransform
  // The function getWorldTransform() is applied to geometry Node and calls the parents getWorldTransform() function recursively
  // The resulting matrix calculation looks something like this: root-worldT * ... * parent-localT * geolocalT.
  // SEE-1 renderObject() how this affects renderObject() rotation calculation.

  // As such a HOLDER-NODE will store the position of a planet in the solar system, set with translate()
  // The HOLDER-NODE can also rotate the position of the planet around the center(so the planets dont form a perfect line when rendered)
  // A GEO-NODE stores the planets characterstics. E.g its size set with scale()

  glm::mat4 position_matrix;
  // SceneGraph
  scene_graph_all = SceneGraph();
  scene_graph_all.name = "Scene";

  Node *root = new Node();
  root->name = "root";
  scene_graph_all.root = root;
  root->setWorldTransform(position_matrix);


  // Sun Node--------------------------------------------------------
  //Create hold, which contains position
  Node * sun_hold = new Node();
  sun_hold->name ="sun_hold";
  sun_hold->parent = root;
  root->addChildren(sun_hold);
  //Suns position_matrix
  position_matrix = glm::translate({}, glm::vec3(0.f, 0.f, 0.f));
  sun_hold->setLocalTransform(position_matrix);

  geometry_node * sun_geo = new geometry_node();
  sun_geo->name = "sun_geo";
  sun_geo->parent = sun_hold;
  sun_hold->addChildren(sun_geo);
  position_matrix = glm::scale(glm::fmat4{}, glm::fvec3(3.5f, 3.5f, 3.5f));
  sun_geo->setLocalTransform(position_matrix);
  geometry_node_Vector.push_back(sun_geo);

  // Planet Nodes

  // Planet Node Earth------------------------------------------------
  // Create hold, which contains position and rotation speed
  Node * earth_hold = new Node();
  earth_hold->name ="earth_hold";
  earth_hold->parent = root;
  root->addChildren(earth_hold);
  // earths position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(13.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 1.f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  earth_hold->setLocalTransform(position_matrix);

  geometry_node * earth_geo = new geometry_node();
  earth_geo->name = "earth_geo";
  earth_geo->parent = earth_hold;
  earth_hold->addChildren(earth_geo);
  geometry_node_Vector.push_back(earth_geo);

  // Moon Node Earth Moon
  Node * earthmoon_hold = new Node();
  earthmoon_hold->name ="earthmoon_hold";
  earthmoon_hold->parent = earth_hold;
  earth_hold->addChildren(earthmoon_hold);
  //moons position_matrix
  position_matrix = glm::translate({}, glm::vec3(2.f, 0.f, 0.f));
  earthmoon_hold->setLocalTransform(position_matrix);

  geometry_node * earthmoon_geo = new geometry_node();
  earthmoon_geo->name = "earthmoon_geo";
  earthmoon_geo->parent = earthmoon_hold;
  earthmoon_hold->addChildren(earthmoon_geo);
  // Scale down the moon to highlight from planets
  position_matrix = glm::scale(glm::fmat4{}, glm::fvec3(.4f, .4f, .4f));
  earthmoon_geo->setLocalTransform(position_matrix);
  geometry_node_Vector.push_back(earthmoon_geo);



  // Planet Node Mercury------------------------------------------------
  // Create hold, which contains position and rotation speed
  Node * mercury_hold = new Node();
  mercury_hold->name ="mercury_hold";
  mercury_hold->parent = root;
  root->addChildren(mercury_hold);
  // Mercury position_matrix
  position_matrix = glm::translate({}, glm::vec3(5.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 0.7f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  mercury_hold->setLocalTransform(position_matrix);

  geometry_node * mercury_geo = new geometry_node();
  mercury_geo->name = "venus_geo";
  mercury_geo->parent = mercury_hold;
  mercury_hold->addChildren(mercury_geo);
  geometry_node_Vector.push_back(mercury_geo);

  // Planet Node Venus------------------------------------------------
  // Create hold, which contains position and rotation speed
  Node * venus_hold = new Node();
  venus_hold->name ="venus_hold";
  venus_hold->parent = root;
  root->addChildren(venus_hold);
  // venus position_matrix
  position_matrix = glm::translate({}, glm::vec3(9.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 2.f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  venus_hold->setLocalTransform(position_matrix);

  geometry_node * venus_geo = new geometry_node();
  venus_geo->name = "venus_geo";
  venus_geo->parent = venus_hold;
  venus_hold->addChildren(venus_geo);
  geometry_node_Vector.push_back(venus_geo);


  // Planet Node Mars------------------------------------------------
  // Create hold, which contains position and rotation speed
  Node * mars_hold = new Node();
  mars_hold->name ="mars_hold";
  mars_hold->parent = root;
  root->addChildren(mars_hold);
  // Mars position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(16.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 2.9f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  mars_hold->setLocalTransform(position_matrix);

  geometry_node * mars_geo = new geometry_node();
  mars_geo->name = "mars_geo";
  mars_geo->parent = mars_hold;
  mars_hold->addChildren(mars_geo);
  geometry_node_Vector.push_back(mars_geo);

  // Planet Node Jupiter------------------------------------------------
  // Create hold, which contains position and rotation speed
  Node * jup_hold = new Node();
  jup_hold->name ="jup_hold";
  jup_hold->parent = root;
  root->addChildren(jup_hold);
  // Jupiter position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(19.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 3.6f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  jup_hold->setLocalTransform(position_matrix);

  geometry_node * jup_geo = new geometry_node();
  jup_geo->name = "jup_geo";
  jup_geo->parent = jup_hold;
  jup_hold->addChildren(jup_geo);
  geometry_node_Vector.push_back(jup_geo);

  // Planet Node Saturn------------------------------------------------
  // Create hold, which contains position and rotation speed
  Node * sat_hold = new Node();
  sat_hold->name ="sat_hold";
  sat_hold->parent = root;
  root->addChildren(sat_hold);
  // Saturn position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(22.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 4.9f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  sat_hold->setLocalTransform(position_matrix);

  geometry_node * sat_geo = new geometry_node();
  sat_geo->name = "sat_geo";
  sat_geo->parent = sat_hold;
  sat_hold->addChildren(sat_geo);
  geometry_node_Vector.push_back(sat_geo);

  // Planet Node Uranus------------------------------------------------
  // Create hold, which contains position and rotation speed
  Node * ur_hold = new Node();
  ur_hold->name ="ur_hold";
  ur_hold->parent = root;
  root->addChildren(ur_hold);
  // Uranus position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(25.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 5.7f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  ur_hold->setLocalTransform(position_matrix);

  geometry_node * ur_geo = new geometry_node();
  ur_geo->name = "ur_geo";
  ur_geo->parent = ur_hold;
  ur_hold->addChildren(ur_geo);
  geometry_node_Vector.push_back(ur_geo);

  // Planet Node Neptune------------------------------------------------
  // Create hold, which contains position and rotation speed
  Node * nep_hold = new Node();
  nep_hold->name ="nep_hold";
  nep_hold->parent = root;
  root->addChildren(nep_hold);
  // Neptune position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(28.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 6.f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  nep_hold->setLocalTransform(position_matrix);

  geometry_node * nep_geo = new geometry_node();
  nep_geo->name = "nep_geo";
  nep_geo->parent = nep_hold;
  nep_hold->addChildren(nep_geo);
  geometry_node_Vector.push_back(nep_geo);
}

// ------------------Personal scenegraph------------------------------------------------------------------------


// exe entry point
int main(int argc, char* argv[]) {
  Application::run<ApplicationSolar>(argc, argv, 3, 2);
}