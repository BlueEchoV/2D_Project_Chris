#pragma once

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

struct MX4 {
	float e[16];
};

float dot_product(const V3& a, const V3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float dot_product(const V4& a, const V4& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
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

// COLUMN MAJOR
V3 get_mx_3_row(const MX3& matrix, int row) {
    return {matrix.e[row], matrix.e[row + 3], matrix.e[row + 6]};
}

// COLUMN MAJOR
V3 get_mx_3_col(const MX3& matrix, int col) {
    return {matrix.e[col * 3], matrix.e[col * 3 + 1], matrix.e[col * 3 + 2]};
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

V4 get_mx_4_row(const MX4& matrix, int row) {
    return {matrix.e[row], matrix.e[row + 4], matrix.e[row + 8], matrix.e[row + 12]};
}

V4 get_mx_4_col(const MX4& matrix, int col) {
    return {matrix.e[col * 4], matrix.e[col * 4 + 1], matrix.e[col * 4 + 2], matrix.e[col * 4 + 3]};
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

MX3 identity_mx_3();
MX4 identity_mx_4();
MX4 TranslationMatrixyz(float x, float y, float z);
