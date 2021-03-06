#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"
#include "scene_graph.hpp"
#include "geometry_node.hpp"
#include "point_light_node.hpp"
#include "pixel_data.hpp"
#include "texture_loader.hpp"

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // react to key input
  void keyCallback(int key, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);
  //handle resizing
  void resizeCallback(unsigned width, unsigned height);

  // draw all objects
  void render() const;

  // Personal Code, draw single object--------------------
  void renderObject(geometry_node * object, int count) const;
  void renderPlanetObjects() const;
  void renderStarObjects() const;
  void renderOrbitObjects() const;

 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  void initializeSceneGraph();
  void initializeStars();
  void initializeTextures();
  // update uniform values
  void uploadUniforms();
  // upload projection matrix
  void uploadProjection();
  // upload view matrix
  void uploadView();

  // cpu representation of model
  model_object planet_object;
  
  // camera transform matrix
  glm::fmat4 m_view_transform;
  // camera projection matrix
  glm::fmat4 m_view_projection;

  // Personal Code
  SceneGraph scene_graph_all;
  point_light_node * light_all;
  std::vector<geometry_node*> geometry_node_Vector;
  model_object star_object;
  std::vector<float> orbits;
  model_object orbit_object;


};

#endif