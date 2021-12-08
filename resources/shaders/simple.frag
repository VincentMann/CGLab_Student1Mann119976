#version 150

in vec3 pass_Normal, pass_Position;
in mat4 pass_View, pass_Model;
out vec4 out_Color;

uniform vec3 geo_color;
uniform float light_intensity;
uniform vec3 light_color;
uniform vec3 cam_position;
uniform vec3 light_position;
//uniform vec3 current_position;

vec3 ambient_color = vec3(0.1f, 0.1f, 0.1f);
vec3 diffuse_color = vec3(0.5f, 0.5f, 0.5f);
vec3 ambient_diffuse_color = vec3(0.1f, 0.1f, 0.1f);
vec3 specular_color = vec3(1.f, 1.f, 1.f);


void main() {
  //out_Color = vec4(abs(normalize(pass_Normal)), 1.0);
  // Rewrote formula on page 8, of 4th slide from:
  // I = I_a * k_a * O_d + f_att * I_d * [k_d * O_d * (NL) + k_s * O_s * (RV)]
  // to:
  // I = (I_a * k_a + f_att * I_d * k_d * (NL)) * O_d + f_att * k_s * O_s * (RV)
  // I = (AMB + DIFF) * ambient_diffuse_color + SPEC * specular_color
  // Keep in mind that the PLINN-PHONG model states: (RV) -> (NH), where H = L + V is halfway direct
  // and that one I is calculated for R,G,B each

  // pass_Normal is in viewspace(see simple.vert), so multiply cam,light,frag positins with viewMatrix;

  vec3 L = light_position - pass_Position; // Light direction

  //position norm matrix

  vec3 N = pass_Normal;
  float f_att = 1/pow(length(L), 2.f);

  vec3 AMB =  light_color;
  vec3 DIFF = light_intensity * light_color * f_att * max(dot(normalize(L), normalize(N)), 0.f);


  vec3 V = cam_position - pass_Position; //View direction
  vec3 H = L + V; 
  float spec_pow = 30.f;
  // more power to light color in spec
  vec3 SPEC = light_intensity * light_color * f_att * pow(max(dot(normalize(N), normalize(H)), 0.f), spec_pow);

  vec3 I = AMB * ambient_color + DIFF * diffuse_color + SPEC * specular_color;

  vec3 result_color = I * geo_color;
  out_Color = vec4(result_color, 1.0);
}
