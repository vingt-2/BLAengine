#include "Maths.h"

blaMat3 matrixCross(blaVec3 v)
{
    return blaMat3
    (
        blaVec3(0, -v[2], v[1]),
        blaVec3(v[2], 0, -v[0]),
        blaVec3(-v[1], v[0], 0)
    );
}

void printVector(blaVec3 vec)
{
    std::cout << vec.x << ", " << vec.y << ", " << vec.z << "\n";
}