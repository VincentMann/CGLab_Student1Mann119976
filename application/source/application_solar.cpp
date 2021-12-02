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
#include "point_light_node.hpp"
#include "glm/gtx/string_cast.hpp"



// For some reason errors are generated: undefined reference to e.g "Node::getName[abi:cxx11]()«" is triggered,
// if the cpp files aren't included. Maybe Cmake.txt must be extended?
#include "node.cpp"
#include "geometry_node.cpp"
#include "scene_graph.cpp"
#include "point_light_node.cpp"

// ------------------Personal includes------------------------------------------------------------------------

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star_object{}
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})}
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
{
  initializeSceneGraph();
  initializeGeometry();
  initializeStars();
  initializeShaderPrograms();
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);
}

void ApplicationSolar::render() const {
  this->renderPlanetObjects();
  this->renderStarObjects();
}

//Personal Code --------------------

void ApplicationSolar::renderStarObjects() const{
  
  glUseProgram(m_shaders.at("star").handle);

  glBindVertexArray(star_object.vertex_AO);
  
  glDrawArrays(star_object.draw_mode, GLint(0), star_object.num_elements);

}

void ApplicationSolar::renderPlanetObjects() const{
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


  // Render Color of planet
  // glUniform3f
  GLint location = glGetUniformLocation(m_shaders.at("planet").handle, "geo_color");
  glUniform3f(location, planet_geo->geo_color[0], planet_geo->geo_color[1], planet_geo->geo_color[2]);
  

  // Render lightning:
  
  // Light intensity:
  location = glGetUniformLocation(m_shaders.at("planet").handle, "light_intensity");
  glUniform1f(location, light_all->lightIntensity);

  // Light Color:
  location = glGetUniformLocation(m_shaders.at("planet").handle, "light_color");
  glUniform3f(location, light_all->lightColor[0], light_all->lightColor[1], light_all->lightColor[2]);

  // Light position:
  glm::fvec4 light_position = light_all->getWorldTransform() * glm::fvec4{0.f, 0.f, 0.f, 1.f};
  location = glGetUniformLocation(m_shaders.at("planet").handle, "light_position");
  glUniform3f(location, light_position[0], light_position[1], light_position[2]);

  // Camera position:
  glm::fvec4 cam_position = m_view_transform * glm::fvec4{0.f, 0.f, 0.f, 1.f};
  location = glGetUniformLocation(m_shaders.at("planet").handle, "cam_position");
  glUniform3f(location, cam_position[0], cam_position[1], cam_position[2]);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
}

//Personal Code --------------------


void ApplicationSolar::uploadView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));


  // upload star matrix to gpu
  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  // upload star matrix to gpu
  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // bind shader to which to upload unforms
  //glUseProgram(m_shaders.at("planet").handle);
  //glUseProgram(m_shaders.at("star").handle);

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


  // Star shader
  // store shader program objects in container
  m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/vao.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});

  // request uniform locations for shader program
  // Content of vao.vert
  m_shaders.at("star").u_locs["ModelViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;

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


// ------------------Personal scenegraph------------------------------------------------------------------------
void ApplicationSolar::initializeStars(){

  // Generate Star Array as Vector: One Star: x,y,z,r,g,b 

  std::vector<float> milky_way;
  int star_count = 10000;
  for(int i = 0; i < star_count; i++){
    float x = (float)(std::rand() % 100) + (-50.f);
    milky_way.push_back(x);
    float y = (float)(std::rand() % 100) + (-50.f);
    milky_way.push_back(y);
    float z = (float)(std::rand() % 100) + (-50.f);
    milky_way.push_back(z);

    float r = (float)(std::rand() % 255) / 255.f;
    milky_way.push_back(r);
    float g = (float)(std::rand() % 255) / 255.f;
    milky_way.push_back(g);
    float b = (float)(std::rand() % 255) / 255.f;
    milky_way.push_back(b);

    if(i == 51 ){
      std::cout<<"STAR POS: "<<x<<", "<<y<<", "<<z<<" \n";
      std::cout<<"STAR COL: "<<r<<", "<<g<<", "<<b<<" \n";
      std::cout<<"TESTING COL: "<<std::rand() % 255 / 255<<" \n";
    }
  }

  glGenVertexArrays(1, &star_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(star_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &star_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * milky_way.size(), milky_way.data(), GL_STATIC_DRAW);

  // glVertexAttribPointer(index, size, dtype, norm, str, ptr)
  // index: 0 for POSITION and 1 for COLOR of star
  // size: POSITION contains x,y,z so 3, COLOR contains r,g,b so 3
  // dtype: GL_FLOAT
  // norm: GL_FALSE
  // str: size of each star content(for iterating the milky way) POSITION + COLOR = 3 +3 = 6
  // ptr: in the milky_way, points to a stars first value of either POSITION or COLOR, so either 0 or 3

  // first attribute is POSITION 0
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(sizeof(float) * 6), (GLvoid*)(sizeof(float) * 0) );
  
  // second attribute is COLOR 1
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, GLsizei(sizeof(float) * 6), (GLvoid*)(sizeof(float) * 3) );


  // store type of primitive to draw
  star_object.draw_mode = GL_POINTS;
  // transfer number of indices to model object 
  star_object.num_elements = GLsizei(milky_way.size()/6);

}
// ------------------Personal StarInit--------------------------------------------------------------------------

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

  // Light Node--------------------------------------------------------
  // Create light node, which contains position of light source

  position_matrix = glm::translate({}, glm::vec3(0.f, 0.f, 0.f));
  light_all = new point_light_node("light", root, position_matrix);
  // lightIntensity and lightColor
  light_all->setlightColor(glm::vec3(1.f, 1.f, 0.f));
  light_all->lightIntensity = 1.f;

  // Sun Node--------------------------------------------------------
  // Create hold, which contains position
  
  //Suns position_matrix
  position_matrix = glm::translate({}, glm::vec3(0.f, 0.f, 0.f));
  Node * sun_hold = new Node("sun_hold", root, position_matrix);
  root->addChildren(sun_hold);

  position_matrix = glm::scale(glm::fmat4{}, glm::fvec3(3.5f, 3.5f, 3.5f));
  geometry_node * sun_geo = new geometry_node("sun_geo", sun_hold, position_matrix);
  sun_hold->addChildren(sun_geo);
  sun_geo->geo_color = glm::fvec3(1.f, 1.f, 0.f);
  geometry_node_Vector.push_back(sun_geo);

  // Planet Nodes
  
  // Planet Node Earth---------------------------------------------------------------------------------------------
  // Create hold, which contains position and rotation speed
  // earths position_matrix:
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(13.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 1.f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  Node * earth_hold = new Node("earth_hold", root, position_matrix);
  root->addChildren(earth_hold);

  geometry_node * earth_geo = new geometry_node("earth_geo", earth_hold, glm::mat4{});
  earth_hold->addChildren(earth_geo);
  earth_geo->geo_color = glm::fvec3(0.2f, 0.8f, 0.1f);
  geometry_node_Vector.push_back(earth_geo);

  // Moon Node Earth-Moon
  position_matrix = glm::translate({}, glm::vec3(2.f, 0.f, 0.f));
  Node * earthmoon_hold = new Node("earthmoon_hold", earth_hold, position_matrix);
  earth_hold->addChildren(earthmoon_hold);
  //moons position_matrix
  // Scale down the moon to highlight from planets
  position_matrix = glm::scale(glm::fmat4{}, glm::fvec3(.4f, .4f, .4f));
  geometry_node * earthmoon_geo = new geometry_node("earthmoon_geo", earthmoon_hold, position_matrix);
  earthmoon_hold->addChildren(earthmoon_geo);
  earthmoon_geo->geo_color = glm::fvec3(0.6f, 0.6f, 0.6f);
  geometry_node_Vector.push_back(earthmoon_geo);


  // Planet Node Mercury---------------------------------------------------------------------------------------------
  // Create hold, which contains position and rotation speed
  // Mercury position_matrix
  position_matrix = glm::translate({}, glm::vec3(5.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 0.7f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  Node * mercury_hold = new Node("mercury_hold", root, position_matrix);
  root->addChildren(mercury_hold);

  geometry_node * mercury_geo = new geometry_node("mercury_geo", mercury_hold, glm::mat4{});
  mercury_hold->addChildren(mercury_geo);
  mercury_geo->geo_color = glm::fvec3(0.7f, 0.4f, 0.f);
  geometry_node_Vector.push_back(mercury_geo);

  // Planet Node Venus---------------------------------------------------------------------------------------------
  // Create hold, which contains position and rotation speed
  // venus position_matrix
  position_matrix = glm::translate({}, glm::vec3(9.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 2.f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  Node * venus_hold = new Node("venus_hold", root, position_matrix);
  root->addChildren(venus_hold);
  
  geometry_node * venus_geo = new geometry_node("venus_geo", venus_hold, glm::mat4{});
  venus_hold->addChildren(venus_geo);
  venus_geo->geo_color = glm::fvec3(0.6f, 0.9f, 0.2f);
  geometry_node_Vector.push_back(venus_geo);

  // Planet Node Mars---------------------------------------------------------------------------------------------
  // Create hold, which contains position and rotation speed
  // Mars position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(16.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 2.9f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  Node * mars_hold = new Node("mars_hold", root, position_matrix);
  root->addChildren(mars_hold);
  
  geometry_node * mars_geo = new geometry_node("mars_geo", mars_hold, glm::mat4{});
  mars_hold->addChildren(mars_geo);
  mars_geo->geo_color = glm::fvec3(0.8f, 0.2f, 0.2f);
  geometry_node_Vector.push_back(mars_geo);

  // Planet Node Jupiter---------------------------------------------------------------------------------------------
  // Create hold, which contains position and rotation speed
  // Jupiter position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(19.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 3.6f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  Node * jup_hold = new Node("jup_hold", root, position_matrix);
  root->addChildren(jup_hold);
  
  geometry_node * jup_geo = new geometry_node("jup_geo", jup_hold, glm::mat4{});
  jup_hold->addChildren(jup_geo);
  jup_geo->geo_color = glm::fvec3(0.9f, 0.5f, 0.f);
  geometry_node_Vector.push_back(jup_geo);

  // Planet Node Saturn---------------------------------------------------------------------------------------------
  // Create hold, which contains position and rotation speed
  // Saturn position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(22.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 4.9f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  Node * sat_hold = new Node("sat_hold", root, position_matrix);
  root->addChildren(sat_hold);

  geometry_node * sat_geo = new geometry_node("sat_geo", sat_hold, glm::mat4{});
  sat_hold->addChildren(sat_geo);
  sat_geo->geo_color = glm::fvec3(0.9f, 0.7f, 0.2f);
  geometry_node_Vector.push_back(sat_geo);

  // Planet Node Uranus---------------------------------------------------------------------------------------------
  // Create hold, which contains position and rotation speed
  // Uranus position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(25.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 5.7f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  Node * ur_hold = new Node("ur_hold", root, position_matrix);
  root->addChildren(ur_hold);
  
  geometry_node * ur_geo = new geometry_node("ur_geo", ur_hold, glm::mat4{});
  ur_hold->addChildren(ur_geo);
  ur_geo->geo_color = glm::fvec3(0.3f, 0.6f, 0.7f);
  geometry_node_Vector.push_back(ur_geo);

  // Planet Node Neptune---------------------------------------------------------------------------------------------
  // Create hold, which contains position and rotation speed
  // Neptune position_matrix
  position_matrix = glm::translate(glm::fmat4{}, glm::vec3(28.f, 0.f, 0.f));
  position_matrix = glm::rotate(glm::mat4{}, 6.f, glm::fvec3{0.0f, 1.0f, 0.0f}) * position_matrix;
  Node * nep_hold = new Node("nep_hold", root, position_matrix);
  root->addChildren(nep_hold);
  
  geometry_node * nep_geo = new geometry_node("nep_geo", nep_hold, glm::mat4{});
  nep_hold->addChildren(nep_geo);
  nep_geo->geo_color = glm::fvec3(0.1f, 0.1f, 1.f);
  geometry_node_Vector.push_back(nep_geo);

  
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
  if(fabs(pos_x) > fabs(pos_y)){
    m_view_transform = glm::rotate(m_view_transform, (float)pos_x/80.f, glm::fvec3{0.0f, 1.0f, 0.0f});
    uploadView();
  }
  else{
    /* m_view_transform = glm::rotate(m_view_transform, (float)pos_y/80.f, glm::fvec3{1.0f, 0.0f, 0.0f});
    uploadView(); */
  }
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
  // recalculate projection matrix for new aspect ration
  m_view_projection = utils::calculate_projection_matrix(float(width) / float(height));
  // upload new projection matrix
  uploadProjection();
}

// exe entry point
int main(int argc, char* argv[]) {
  Application::run<ApplicationSolar>(argc, argv, 3, 2);
}