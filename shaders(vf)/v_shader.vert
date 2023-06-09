#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
#define PI 3.141592653589

out vec3 ourColor;
out vec2 TexCoord;
out vec4 TexCol;

uniform sampler2D tex;
uniform mat4 mvp;
uniform int fibration;
uniform float time;

mat4 transform(float parameter);

void main()
{
   vec4 hopf = texture(tex, aTexCoord);

   float w = time/5;
   mat4 sTrans = transform(w);

   hopf = sTrans*hopf;
   vec3 modPos;

   vec3 base_sphere;

   /*hopf map*/
   base_sphere = vec3(2*(hopf.x*hopf.z + hopf.y*hopf.w), 2*(hopf.y*hopf.z-hopf.x*hopf.w), dot(hopf.xy,hopf.xy)-dot(hopf.zw,hopf.zw));

   float scale = 2.5;

   if(fibration == 1){
      modPos = hopf.yzw/(1-hopf.x);
      modPos = modPos.zyx;
      float new_length; 
      modPos /= (1 + length(modPos)/scale);
      gl_PointSize = 0.8;
   }
   if(fibration == 0){
      modPos = normalize(base_sphere)*scale;
      //modPos = vec3(0, length(hopf), 0);
      gl_PointSize = 5;
   }

   //swizzled for openGL coordinates
   gl_Position = mvp*vec4(modPos.xzy, 1);
   TexCoord = aTexCoord;
   vec3 color = vec3(0.5) + 0.4*base_sphere;
   

   TexCol = color.xyzz;
}

mat4 transform(float parameter){
   float p = parameter;
   mat4 suTrans = mat4(1.0f);
   /*lorentz boost in z direction*/
   mat4 suTrans0 = mat4(1.0f);

   /*z axis rotation*/
   mat4 suTrans1 = mat4(1.0f);

   /*y axis rotation*/
   mat4 suTrans2 = mat4(1.0f);

   /*x axis rotation*/
   mat4 suTrans3 = mat4(1.0f);  
     
   /*
   rot = 0 <= lorentz boost
   rot = 1 <= z axis rotation
   rot = 2 <= y axis rotation
   rot = 3 <= x axis rotation
   */
   int rot = 1;
   float w = exp(1.5*sin(parameter/1.5));

   suTrans0[0] = vec4(sqrt(w), 0, 0, 0);
   suTrans0[1] = vec4(0, sqrt(w), 0, 0);
   suTrans0[2] = vec4(0, 0, inversesqrt(w), 0);
   suTrans0[3] = vec4(0, 0, 0, inversesqrt(w));

   p = parameter;

   suTrans1[0] = vec4(cos(p/2), -sin(p/2), 0, 0);
   suTrans1[1] = vec4(sin(p/2), cos(p/2), 0, 0);
   suTrans1[2] = vec4(0, 0, cos(p/2), sin(p/2));
   suTrans1[3] = vec4(0, 0, -sin(p/2), cos(p/2));

   p = parameter;

   suTrans2[0] = vec4(cos(p/2), 0, -sin(p/2), 0);
   suTrans2[1] = vec4(0, cos(p/2), 0, -sin(p/2));
   suTrans2[2] = vec4(sin(p/2), 0, cos(p/2), 0);
   suTrans2[3] = vec4(0, sin(p/2), 0, cos(p/2));

   suTrans3[0] = vec4(cos(p/2), 0, 0, -sin(p/2));
   suTrans3[1] = vec4(0, cos(p/2), sin(p/2), 0);
   suTrans3[2] = vec4(0, -sin(p/2), cos(p/2), 0);
   suTrans3[3] = vec4(sin(p/2), 0, 0, cos(p/2));

   if(rot == 0)
      suTrans = transpose(suTrans0);
   if(rot == 1)
      suTrans = transpose(suTrans1);
   if(rot == 2)
      suTrans = transpose(suTrans2);
   if(rot == 3)
      suTrans = transpose(suTrans3);
   if(rot == 4)
      suTrans = transpose(suTrans1*suTrans0);

   //suTrans = transpose(suTrans);
   suTrans = transpose(suTrans3*suTrans2*suTrans1);
   //suTrans = transpose(suTrans1*suTrans0);

   return suTrans;
}//