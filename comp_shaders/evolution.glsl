#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
layout(rgba32f, binding = 0) uniform image3D initial_grid;
layout(rgba32f, binding = 1) uniform image3D second_grid;
layout(rgba32f, binding = 2) uniform image3D new_first_grid;
layout(rgba32f, binding = 3) uniform image3D new_second_grid;
layout(rgba32f, binding = 4) uniform image3D first_grid_derivatives;
layout(rgba32f, binding = 5) uniform image3D second_grid_derivatives;

#define x_step 0.25

#define PI 3.141592653589
#define dx 0
#define dy 1
#define dz 2
/*
    current value key words to make things easier
*/
#define alpha first_mat[3].x
#define X first_mat[3].y
#define K first_mat[3].z
#define beta vec3(first_mat[0].w, first_mat[1].w, first_mat[2].w)
#define gamma mat3(first_mat)
#define Aext mat3(second_mat)

/*
    Derivative Key words to make things easier
*/
#define delAlph vec3(derivatives_one[0][3].x, derivatives_one[1][3].x, derivatives_one[2][3].x)
#define delX vec3(derivatives_one[0][3].y, derivatives_one[1][3].y, derivatives_one[2][3].y)
#define delK vec3(derivatives_one[0][3].z, derivatives_one[1][3].z, derivatives_one[2][3].z)
#define delB mat3(derivatives_one[0][0].w, derivatives_one[0][1].w, derivatives_one[0][2].w, derivatives_one[1][0].w, derivatives_one[1][1].w, derivatives_one[1][2].w, derivatives_one[2][0].w, derivatives_one[2][1].w, derivatives_one[2][2].w)
#define del_gamma(a) mat3(derivatives_one[(a)]) 

/*
    Second Derivatives
*/
#define del_2_alpha(a, b, c, d) ((imageLoad(first_grid_derivatives, (texCoords + ivec3(b, c, d))*ivec3(4, 3, 1) + ivec3(3, a, 0))).x - (imageLoad(first_grid_derivatives, (texCoords - ivec3(b, c, d))*ivec3(4, 3, 1) + ivec3(3, a, 0))).x)/(2*x_step)
#define del_2_alpha_mat mat3(del_2_alpha(0, 1, 0, 0), del_2_alpha(0, 0, 1, 0), del_2_alpha(0, 0, 0, 1), del_2_alpha(1, 1, 0, 0), del_2_alpha(1, 0, 1, 0), del_2_alpha(1, 0, 0, 1), del_2_alpha(2, 1, 0, 0), del_2_alpha(2, 0, 1, 0), del_2_alpha(2, 0, 0, 1))

/*Christoffel symbols*/
//individual symbols
#define christ_sym(i, k, l) 0.5f*(inverse(gamma)[i][0]*(del_gamma(l)[0][k] + del_gamma(k)[0][l] - del_gamma(0)[k][l]) + inverse(gamma)[i][1]*(del_gamma(l)[1][k] + del_gamma(k)[1][l] - del_gamma(1)[k][l]) + inverse(gamma)[i][2]*(del_gamma(l)[2][k] + del_gamma(k)[2][l] - del_gamma(2)[k][l]))
#define christoffel_mat(i) mat3(christ_sym(i, 0, 0), christ_sym(i, 0, 1), christ_sym(i, 0, 2), christ_sym(i, 1, 0), christ_sym(i, 1, 1), christ_sym(i, 1, 2), christ_sym(i, 2, 0), christ_sym(i, 2, 1), christ_sym(i, 2, 2))

/*Covariant derivative of vector*/
//conformal covariant derivative of d-alpha
#define conf_cov_alpha mat3(del_2_alpha_mat - christoffel_mat(0)*delAlph.x - christoffel_mat(1)*delAlph.y - christoffel_mat(2)*delAlph.z)

/*useful function*/
#define outer_product(i, j) mat3((i).x*(j), (i).y*(j), (i).z*(j))

//uniform mat4 viewmat;
uniform float time;
float res = 128.0f;

mat4[3] read_derivatives_main(ivec3 texture_coordinates);
mat4[3] read_derivatives_secondary(ivec3 texture_coordinates);

mat4 readMatrix1(ivec3 texture_coordinates, ivec3 direction);
mat4 readMatrix2(ivec3 texture_coordinates, ivec3 direction);
float trace(mat3 matrix);
float trace(mat4 matrix);

void storeMatrixNew(ivec3 texture_coordinates, mat4 data_matrix);

#define t_step 0.025f

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

    mat3 inv_gamma = inverse(gamma);

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
    float dt_X = dot(delX, beta) + (2.0f/3.0f)*X*(alpha*K - trace(delB));
    mat3 dt_gamma = (beta.x*del_gamma(0) + beta.y*del_gamma(1) + beta.z*del_gamma(2)) + gamma*transpose(delB) * delB*gamma - (2.0f/3.0f)*gamma*trace(delB) - 2*alpha*Aext;

    //intermediary variable
    mat3 physical_cov_alpha = conf_cov_alpha + (0.5f*X)*(outer_product(delX, delAlph) + outer_product(delAlph, delX)) - (0.5f*X)*gamma*trace(inv_gamma*transpose(outer_product(delX, delAlph)));

    float dt_K = dot(delK, beta)*K - X*trace(inv_gamma*transpose(physical_cov_alpha)) + alpha*trace(inv_gamma*inv_gamma*transpose(Aext*Aext)) + (1.0f/3.0f)*alpha*pow(K, 2);
    vec3 dt_beta = vec3(0.0f);
    float dt_alpha = 0.0f;

    mat4 delta_metric = mat4(0.0f);
    delta_metric[0] = vec4(dt_gamma[0], dt_beta.x);
    delta_metric[1] = vec4(dt_gamma[1], dt_beta.y);
    delta_metric[2] = vec4(dt_gamma[2], dt_beta.z);
    delta_metric[3] = vec4(dt_alpha, dt_X, dt_K, 0);

    delta_metric *= t_step;

    storeMatrixNew(texCoords, first_mat + delta_metric);

}

void storeMatrixNew(ivec3 texture_coordinates, mat4 data_matrix){
    imageStore(new_first_grid, texture_coordinates*ivec3(4, 1, 1) + ivec3(0, 0, 0), data_matrix[0]);    
    imageStore(new_first_grid, texture_coordinates*ivec3(4, 1, 1) + ivec3(1, 0, 0), data_matrix[1]);  
    imageStore(new_first_grid, texture_coordinates*ivec3(4, 1, 1) + ivec3(2, 0, 0), data_matrix[2]);    
    imageStore(new_first_grid, texture_coordinates*ivec3(4, 1, 1) + ivec3(3, 0, 0), data_matrix[3]); 
}

float trace(mat3 matrix){
    return matrix[0][0] + matrix[1][1] + matrix[2][2];
}

float trace(mat4 matrix){
    return matrix[0][0] + matrix[1][1] + matrix[2][2] + matrix[3][3];
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