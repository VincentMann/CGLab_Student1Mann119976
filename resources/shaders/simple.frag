#version 150

in  vec3 pass_Normal, pass_Position;
out vec4 out_Color;

uniform vec3 geo_color;
uniform float light_intensity;
uniform vec3 light_color;
uniform vec3 cam_position;
uniform vec3 light_position;
//uniform vec4 current_position;

vec3 ambient_diffuse_color = vec3(0.5f, 0.5f, 0.5f);
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
  vec3 frag_position = pass_Position;
  vec3 L = normalize(light_position - frag_position); // Light direction
  vec3 N = pass_Normal;
  float f_att = 1/pow(length(L), 5.f);

  vec3 AMB = light_intensity * light_color;
  vec3 DIFF = light_intensity * light_color * f_att * dot(L, N);

  vec3 V = normalize(cam_position - frag_position); //View direction
  vec3 H = normalize(L + V);
  vec3 SPEC = light_intensity * light_color * f_att * dot(N, H);

  float I_R = (AMB[0] + DIFF[0]) * ambient_diffuse_color[0] + SPEC[0] * specular_color[0];
  float I_G = (AMB[1] + DIFF[2]) * ambient_diffuse_color[1] + SPEC[1] * specular_color[1];
  float I_B = (AMB[2] + DIFF[1]) * ambient_diffuse_color[2] + SPEC[2] * specular_color[2];

  vec3 result_color = vec3(geo_color[0] * I_R, geo_color[1] * I_G, geo_color[2] * I_B);
  //vec3 DIFF = 
  out_Color = vec4(result_color, 1.0);
}
