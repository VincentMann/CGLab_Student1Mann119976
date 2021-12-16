#version 150

in vec3 pass_Normal, pass_Position;
in vec4 four_pass_position;
in mat4 pass_Model, pass_View;
in vec2 pass_Texture_Coor;

out vec4 out_Color;
uniform vec3 geo_color;
uniform float light_intensity;
uniform vec3 light_color;
uniform vec3 cam_position;
uniform vec3 light_position;
uniform sampler2D current_texture;

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


  vec3 L = light_position - four_pass_position.xyz; // Light direction vector
  vec3 N = pass_Normal; // Normal vector
  float f_att = 1/pow(length(L), 2.f);

  vec3 AMB =  ambient_color;
  vec3 DIFF = 10*light_intensity * light_color * f_att * max(dot(normalize(L), normalize(N)), 0.f);


  vec3 V = cam_position - four_pass_position.xyz; //View direction vector
  vec3 H = normalize(L) + normalize(V); 
  float spec_pow = 60.f;
  // more power to light color in spec
  vec3 SPEC = 10*light_intensity * light_color * f_att * pow(max(dot(normalize(N), normalize(H)), 0.f), spec_pow);

  // Texture: 
  vec4 color_from_tex = texture(current_texture, pass_Texture_Coor);

  vec3 I = AMB * ambient_color + DIFF * diffuse_color + SPEC * specular_color;
  //vec3 I = (AMB + DIFF) * color_from_tex.rgb + SPEC * light_color;;

  vec3 result_color = I * color_from_tex.rgb;
  //result_color = color_from_tex.rgb;
  out_Color = vec4(result_color, 1.0);
}
