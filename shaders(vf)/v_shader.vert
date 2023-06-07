#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexCoord;

layout(rgba32f, binding = 0) uniform image3D data;
layout(rgba32f, binding = 1) uniform image3D second_batch;

#define PI 3.141592653589

out vec3 ourColor;
//out vec3 TexCoord;
//out vec4 TexCol;

//uniform sampler3D tex;
uniform mat4 mvp;
//uniform int fibration;
//uniform float time;

mat4 transform(float parameter);

void main()
{
   //vec4 coordinates = texture(tex, aTexCoord.xyz);
   vec4 coordinates = imageLoad(data, ivec3(aTexCoord) + ivec3(0, 0, 0));

   vec3 modPos = aPos;

   //swizzled for openGL coordinates
   gl_PointSize = 1.0;
   gl_Position = mvp*vec4(modPos, 1);
   //TexCoord = aTexCoord;
   vec3 color = vec3(1, 0.5 , 0);
   
   //ourColor = color.xyz;
   ourColor = coordinates.xyz;
   //TexCol = color.xyzz;
}//