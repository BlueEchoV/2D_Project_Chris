#pragma once

#include <Math.h>

#define M_PI 3.1415926535897932384626

struct V2 {
	float x;
	float y;
};

struct V3 {
	float x;
	float y;
	float z;
};

union V4 {
	struct {
		float x;
		float y;
		float z;
		float w;
	};
	V3 xyz;
	float e[4];
};

struct Color_f {
	float r;
	float g;
	float b;
	float a;
};

struct MX3 {
	float e[9];
};

// Fancy trick
union MX4 {
	float e[16];
    V4 col[4];
};

V3 operator*(const MX3& matrix, const V3& vector);
V4 operator*(const MX4& matrix, const V4& vector);
MX3 operator*(const MX3& matrix_a, const MX3& matrix_b);
MX4 operator*(const MX4& matrix_a, const MX4& matrix_b);
V3 operator-(const V3& v);

float lerp(float left_Point, float right_Point, float percent);

float dot_product(const V3& a, const V3& b);
float dot_product(const V4& a, const V4& b);

V3 cross(const V3& a, const V3& b);

// COLUMN MAJOR
V3 get_mx_3_row(const MX3& matrix, int row);
V3 get_mx_3_col(const MX3& matrix, int col);
V4 get_mx_4_row(const MX4& matrix, int row);
V4 get_mx_4_col(const MX4& matrix, int col);

MX3 identity_mx_3();
MX4 identity_mx_4();
// If we want to change the location of a point, we can use 
// the translation matrix.
MX4 translation_matrix_mx_4(float x, float y, float z);
MX4 scaling_matrix_mx_4(float scale_x, float scale_y, float scale_z);

typedef MX4 Matrix4;
typedef V3 Vector3;
Matrix4 mat4_rotate_x(float angle_radians);
Matrix4 mat4_rotate_y(float angle_radians);
Matrix4 mat4_rotate_z(float angle_radians);

MX4 mat4_perspective(float fovy, float aspect);

MX4 matrix_transpose(MX4 mx);

V3 normalize(const V3& v);