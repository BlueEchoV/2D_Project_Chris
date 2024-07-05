#include "Math.h"

MX3 identity_mx_3() {
    MX3 result = {};
    result.e[0] = 1.0f;
    result.e[4] = 1.0f;
    result.e[8] = 1.0f;
    return result;
}

MX4 identity_mx_4() {
    MX4 result = {};
    result.e[0] = 1.0f;
    result.e[5] = 1.0f;
    result.e[10] = 1.0f;
    result.e[15] = 1.0f;
    return result;
}

MX4 TranslationMatrix(float x, float y, float z) {
    MX4 result = identity_mx_4();
    result.e[12] = x;
    result.e[13] = y;
    result.e[14] = z;
    return result;
}
