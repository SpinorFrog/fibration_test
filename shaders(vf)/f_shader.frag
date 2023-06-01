#version 460 core
out vec4 FragColor;

//in vec3 ourColor;
in vec2 TexCoord;
in vec4 TexCol;

//uniform sampler2D textures;
//uniform sampler2D tex;

void main()
{
   FragColor = TexCol;
   //FragColor = vec4(0.4f, 0.5f, 0.8f, 1.0f);
   //FragColor = texture(tex, TexCoord);
}//