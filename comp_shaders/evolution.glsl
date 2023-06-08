#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
layout(rgba32f, binding = 0) uniform image3D initial_grid;
layout(rgba32f, binding = 1) uniform image3D second_grid;
layout(rgba32f, binding = 2) uniform image3D new_first_grid;
layout(rgba32f, binding = 3) uniform image3D new_second_grid;
layout(rgba32f, binding = 4) uniform image3D first_grid_derivatives;
layout(rgba32f, binding = 5) uniform image3D second_grid_derivatives;

#define PI 3.141592653589

//uniform mat4 viewmat;
uniform float time;
float res = 128.0f;

mat4[3] read_derivatives(ivec3 texture_coordinates);

void main()
{
    ivec3 texCoords = ivec3(gl_GlobalInvocationID.xyz);
    vec3 pos = 32.0f*(vec3(texCoords) - res/2)/res;

}

mat4[3] read_derivatives(ivec3 texture_coordinates){
    mat4[3] main_derivatives;
    main_derivatives[0][0] = vec4(1, 1, 1, 1);
    main_derivatives[0][1] = vec4(1, 1, 1, 1);
    main_derivatives[0][2] = vec4(1, 1, 1, 1);
    main_derivatives[0][3] = vec4(1, 1, 1, 1);

    main_derivatives[1][0] = vec4(1, 1, 1, 1);
    main_derivatives[1][1] = vec4(1, 1, 1, 1);
    main_derivatives[1][2] = vec4(1, 1, 1, 1);
    main_derivatives[1][3] = vec4(1, 1, 1, 1);

    main_derivatives[2][0] = vec4(1, 1, 1, 1);
    main_derivatives[2][1] = vec4(1, 1, 1, 1);
    main_derivatives[2][2] = vec4(1, 1, 1, 1);
    main_derivatives[2][3] = vec4(1, 1, 1, 1);

    return main_derivatives; 
}//