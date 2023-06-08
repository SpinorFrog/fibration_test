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

mat4[3] read_derivatives_main(ivec3 texture_coordinates);
mat4[3] read_derivatives_secondary(ivec3 texture_coordinates);

mat4 readMatrix1(ivec3 texture_coordinates, ivec3 direction);
mat4 readMatrix2(ivec3 texture_coordinates, ivec3 direction);

void main()
{
    ivec3 texCoords = ivec3(gl_GlobalInvocationID.xyz);
    vec3 pos = 32.0f*(vec3(texCoords) - res/2)/res;

    //[0] is x derivatives, [1] is y derivatives, [2] is z derivatives
    mat4[3] derivatives_one = read_derivatives_main(texCoords);
    mat4[3] derivatives_two = read_derivatives_secondary(texCoords);

    //gets current values
    mat4 first_mat = readMatrix1(texCoords, ivec3(0));
    mat4 second_mat = readMatrix2(texCoords, ivec3(0));

    /*  TABLE OF VALUES (IMPORTANT TO KEEP TRACK OF)
        (layouts)
        first_mat ->    y_00, y_01, y_02, beta.x
                        y_10, y_11, y_12, beta.y
                        y_20, y_21, y_22, beta.z
                        alph,  X  , K   , 0
                        
        second_mat ->   A_00, A_01, A_02, cChri.x
                        A_10, A_11, A_12, cChri.y
                        A_20, A_21, A_22, cChri.z
                        0   ,  0  , 0   , 0  
    */

    float dtX = dot(vec3(derivatives_one[0][3].y, derivatives_one[1][3].y, derivatives_one[2][3].y), vec3(first_mat[0].w, first_mat[1].w, first_mat[2].w)) + (2.0f/3.0f)*first_mat[3].y*(first_mat[3].x*first_mat[3].z - dot(vec3(derivatives_one[0][0].w , derivatives_one[1][1].w , derivatives_one[2][2].w), vec3(1.0f)));
}

mat4 readMatrix1(ivec3 texture_coordinates, ivec3 direction){
    mat4 new_matrix;
    new_matrix[0] = imageLoad(initial_grid, (texture_coordinates + direction)*ivec3(4, 1, 1) + ivec3(0, 0, 0));
    new_matrix[1] = imageLoad(initial_grid, (texture_coordinates + direction)*ivec3(4, 1, 1) + ivec3(1, 0, 0));
    new_matrix[2] = imageLoad(initial_grid, (texture_coordinates + direction)*ivec3(4, 1, 1) + ivec3(2, 0, 0));
    new_matrix[3] = imageLoad(initial_grid, (texture_coordinates + direction)*ivec3(4, 1, 1) + ivec3(3, 0, 0));

    return new_matrix;
}

mat4 readMatrix2(ivec3 texture_coordinates, ivec3 direction){
    mat4 array = mat4(0.0f);
    array[0] = imageLoad(second_grid, (texture_coordinates + direction)*ivec3(3, 1, 1) + ivec3(0, 0, 0));
    array[1] = imageLoad(second_grid, (texture_coordinates + direction)*ivec3(3, 1, 1) + ivec3(1, 0, 0));
    array[2] = imageLoad(second_grid, (texture_coordinates + direction)*ivec3(3, 1, 1) + ivec3(2, 0, 0));

    return array;
}

//[0] is x derivatives, [1] is y derivatives, [2] is z derivatives
mat4[3] read_derivatives_main(ivec3 texture_coordinates){
    mat4[3] main_derivatives;
    main_derivatives[0][0] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(0, 0, 0));
    main_derivatives[0][1] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(1, 0, 0));
    main_derivatives[0][2] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(2, 0, 0));
    main_derivatives[0][3] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(3, 0, 0));

    main_derivatives[1][0] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(0, 1, 0));
    main_derivatives[1][1] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(1, 1, 0));
    main_derivatives[1][2] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(2, 1, 0));
    main_derivatives[1][3] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(3, 1, 0));

    main_derivatives[2][0] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(0, 2, 0));
    main_derivatives[2][1] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(1, 2, 0));
    main_derivatives[2][2] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(2, 2, 0));
    main_derivatives[2][3] = imageLoad(first_grid_derivatives, texture_coordinates*ivec3(4, 3, 1) + ivec3(3, 2, 0));

    return main_derivatives; 
}

mat4[3] read_derivatives_secondary(ivec3 texture_coordinates){
    mat4[3] secondary_derivatives;
    secondary_derivatives[0][0] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(0, 0, 0));
    secondary_derivatives[0][1] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(1, 0, 0));
    secondary_derivatives[0][2] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(2, 0, 0));

    secondary_derivatives[1][0] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(0, 1, 0));
    secondary_derivatives[1][1] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(1, 1, 0));
    secondary_derivatives[1][2] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(2, 1, 0));

    secondary_derivatives[2][0] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(0, 2, 0));
    secondary_derivatives[2][1] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(1, 2, 0));
    secondary_derivatives[2][2] = imageLoad(second_grid_derivatives, texture_coordinates*ivec3(3, 3, 1) + ivec3(2, 2, 0));

    return secondary_derivatives; 
}//