#pragma once

#define M_PI 3.1415926535897932384

struct V3 {
	float x;
	float y;
	float z;
};

struct V4 {
	float x;
	float y;
	float z;
	float w;
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

float dot_product(const V3& a, const V3& b);
float dot_product(const V4& a, const V4& b);

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
