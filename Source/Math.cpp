#include "Math.h"

float calculate_distance(V2 p1, V2 p2) {
    return (float)sqrt(((p2.x - p1.x) * (p2.x - p1.x)) + ((p2.y - p1.y) * (p2.y - p1.y)));
}

int calculate_manhattan_distance(V2 p1, V2 p2) {
    return abs((int)p2.x - (int)p1.x) + abs((int)p2.y - (int)p1.y);
}

// NOTE: COLUMN MAJOR
V3 operator*(const MX3& matrix, const V3& vector) {
    // V3 result;
    // result.x = matrix.e[0] * vector.x + matrix.e[3] * vector.y + matrix.e[6] * vector.z;
    // result.y = matrix.e[1] * vector.x + matrix.e[4] * vector.y + matrix.e[7] * vector.z;
    // result.z = matrix.e[2] * vector.x + matrix.e[5] * vector.y + matrix.e[8] * vector.z;

    V3 row_0 = {matrix.e[0], matrix.e[3], matrix.e[6]};
    V3 row_1 = {matrix.e[1], matrix.e[4], matrix.e[7]};
    V3 row_2 = {matrix.e[2], matrix.e[5], matrix.e[8]};

    V3 result;
    result.x = dot_product(row_0, vector);
    result.y = dot_product(row_1, vector);
    result.z = dot_product(row_2, vector);
    
    return result;
}

V4 operator*(const MX4& matrix, const V4& vector) {
    // V4 result;
    // result.x = matrix.e[0] * vector.x + matrix.e[4] * vector.y + matrix.e[8] * vector.z + matrix.e[12] * vector.w;
    // result.y = matrix.e[1] * vector.x + matrix.e[5] * vector.y + matrix.e[9] * vector.z + matrix.e[13] * vector.w;
    // result.z = matrix.e[2] * vector.x + matrix.e[6] * vector.y + matrix.e[10] * vector.z + matrix.e[14] * vector.w;
    // result.w = matrix.e[3] * vector.x + matrix.e[7] * vector.y + matrix.e[11] * vector.z + matrix.e[15] * vector.w;

    V4 row_0 = {matrix.e[0], matrix.e[4], matrix.e[8],  matrix.e[12]};
    V4 row_1 = {matrix.e[1], matrix.e[5], matrix.e[9],  matrix.e[13]};
    V4 row_2 = {matrix.e[2], matrix.e[6], matrix.e[10], matrix.e[14]};
    V4 row_3 = {matrix.e[3], matrix.e[7], matrix.e[11], matrix.e[15]};

    V4 result;
    result.x = dot_product(row_0, vector);
    result.y = dot_product(row_1, vector);
    result.z = dot_product(row_2, vector);
    result.w = dot_product(row_3, vector);

    return result;
}

V3 operator-(const V3& v) {
    return { -v.x, -v.y, -v.z };
} 

// Matrix * Matrix
MX3 operator*(const MX3& matrix_a, const MX3& matrix_b) {
    MX3 result = {};
    // 2 0 0 * 2 0 0 = 4 0 0 
    // 0 3 0   0 3 0   0 9 0
    // 0 0 4   0 0 4   0 0 16 

    for (int r = 0; r < 3; r++) {
        V3 a_row = get_mx_3_row(matrix_a, r);
        for (int c = 0; c < 3; c++) {
            V3 b_col = get_mx_3_col(matrix_b, c);

            result.e[r + c * 3] = dot_product(a_row, b_col);
        }
    }

    return result;
}

// If we have a 4x4 * 4x4, the resulting matrix will be the 
// outer numbers(4x4), and the inner numbers(4x4) have to match
MX4 operator*(const MX4& matrix_a, const MX4& matrix_b) {
    MX4 result = {};
    for (int r = 0; r < 4; r++) {
        V4 a_row = get_mx_4_row(matrix_a, r);
        for (int c = 0; c < 4; c++) {
            V4 b_col = get_mx_4_col(matrix_b, c);

            result.e[r + c * 4] = dot_product(a_row, b_col);
        }
    }
    return result;
}

float lerp(float left_Point, float right_Point, float percent) {
	// Lerp of T = A * (1 - T) + B * T
	// A is the left side, B is the right side, T is the percentage of the interpolation
	return ((left_Point) * (1 - percent) + (right_Point)*percent);
}

float dot_product(const V3& a, const V3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float dot_product(const V4& a, const V4& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// Computes the cross product of the two vectors. This vector is a 
// third vector that is perpendicular to both of the original 
// vectors.
V3 cross(const V3& a, const V3& b) {
    V3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

// COLUMN MAJOR
V3 get_mx_3_row(const MX3& matrix, int row) {
    return {matrix.e[row], matrix.e[row + 3], matrix.e[row + 6]};
}

// COLUMN MAJOR
V3 get_mx_3_col(const MX3& matrix, int col) {
    return {matrix.e[col * 3], matrix.e[col * 3 + 1], matrix.e[col * 3 + 2]};
}

V4 get_mx_4_row(const MX4& matrix, int row) {
    return {matrix.e[row], matrix.e[row + 4], matrix.e[row + 8], matrix.e[row + 12]};
}

V4 get_mx_4_col(const MX4& matrix, int col) {
    return matrix.col[col];
}
// 1 0 0 
// 0 1 0
// 0 0 1

// COLUMN MAJOR
MX3 identity_mx_3() {
    MX3 result = {};
    result.e[0] = 1.0f;
    result.e[4] = 1.0f;
    result.e[8] = 1.0f;
    return result;
}

// 1 0 0 0
// 0 1 0 0
// 0 0 1 0
// 0 0 0 1 
MX4 identity_mx_4() {
    MX4 result = {};
    result.e[0] = 1.0f;
    result.e[5] = 1.0f;
    result.e[10] = 1.0f;
    result.e[15] = 1.0f;
    return result;
}

MX4 translation_matrix_mx_4(float x, float y, float z) {
    MX4 result = identity_mx_4();
    result.e[12] = x;
    result.e[13] = y;
    result.e[14] = z;
    return result;
}

MX4 scaling_matrix_mx_4(float scale_x, float scale_y, float scale_z) {
    MX4 scale = {0};
    scale.e[0] = scale_x;
    scale.e[5] = scale_y;
    scale.e[10] = scale_z;
    scale.e[15] = 1.0f;
    return scale;
}

Matrix4 mat4_rotate_x(float angle_radians)
{
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    Matrix4 rot = identity_mx_4();
    rot.col[1].y = c;
    rot.col[1].z = s;
    rot.col[2].y = -s;
    rot.col[2].z = c;
    return rot;
}

Matrix4 mat4_rotate_y(float angle_radians)
{
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    Matrix4 rot = identity_mx_4();
    rot.col[0].x = c;
    rot.col[0].z = -s;
    rot.col[2].x = s;
    rot.col[2].z = c;
    return rot;
}

Matrix4 mat4_rotate_z(float angle_radians)
{
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    Matrix4 rot = identity_mx_4();
    rot.col[0].x = c;
    rot.col[0].y = s;
    rot.col[1].x = -s;
    rot.col[1].y = c;
    return rot;
}

// Frustum
MX4 mat4_perspective(float fovy, float aspect)
{
    MX4 conversion = {};
    conversion.col[3].w = 1;

    conversion.col[0].z = -1;
    conversion.col[1].x = 1;
    conversion.col[2].y = 1;

    float z_near = 0.01f;
    float z_far  = 1000.0f;

    float tan_half_fovy = tanf(0.5f * fovy);
    MX4 out = {};
    out.col[0].x = 1.0f / (aspect * tan_half_fovy);
    out.col[1].y = 1.0f / (tan_half_fovy);
    out.col[2].z = -(z_far + z_near) / (z_far - z_near);
    out.col[2].w = -1.0f;
    out.col[3].z = -2.0f * z_far * z_near / (z_far - z_near);

    return out * conversion;
}

MX4 matrix_transpose(MX4 mx) {
    MX4 result;
    for (int row = 0; row < 4; row++) {
        for (int column = 0; column < 4; column++) {
            result.col[column].e[row] = mx.col[row].e[column];
        }
    }
    return result;
}

V3 normalize(const V3& v) {
    float length = (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	V3 result = {};
    if (length != 0.0f) {
        result = {v.x / length, v.y / length, v.z / length};
    }
	return result;
}
