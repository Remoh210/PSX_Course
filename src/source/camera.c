#include "camera.h"

void VectorCross(VECTOR *a, VECTOR *b, VECTOR *out) {
  OuterProduct12(a, b, out);
}

void LookAt(Camera * camera, VECTOR* eye, VECTOR* target, VECTOR* up)
{
    VECTOR xright;      // right x vector
    VECTOR yup;         // up y vector
    VECTOR zforward;    // forward z vector

    VECTOR x, y, z;     // normalized right/up/forward vectors

    VECTOR pos;         // position temp vector
    VECTOR t;           // translation temp vector

    // Compute forward vector and normalized it
    zforward.vx = target->vx - eye->vx;
    zforward.vy = target->vy - eye->vy;
    zforward.vz = target->vz - eye->vz;
    VectorNormal(&zforward, &z);

    // Compute right vector and normalized it
    VectorCross(&z, up, &xright);
    VectorNormal(&xright, &x);

    // Compute up vector and normalized it
    VectorCross(&z, &x, &yup);
    VectorNormal(&yup, &y);

    // Populate lookat mat with rot values
    camera->lookat.m[0][0] = x.vx; camera->lookat.m[0][1] = x.vy; camera->lookat.m[0][2] = x.vz;
    camera->lookat.m[1][0] = y.vx; camera->lookat.m[1][1] = y.vy; camera->lookat.m[1][2] = y.vz;
    camera->lookat.m[2][0] = z.vx; camera->lookat.m[2][1] = z.vy; camera->lookat.m[2][2] = z.vz;

    pos.vx = -eye->vx;
    pos.vy = -eye->vy;
    pos.vz = -eye->vz;

    ApplyMatrixLV(&camera->lookat, &pos, &t); // Mult the vector by the mat
    TransMatrix(&camera->lookat, &t);         // Populate the translation terms
}
