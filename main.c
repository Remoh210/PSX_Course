#include <stdlib.h>
#include <sys/types.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <inline_n.h>

#include "joypad.h"
#include "globals.h"  
#include "display.h"
#include "camera.h"    

///////////////////////////////////////////////////////////////////////////////
// Vertices and face indices
///////////////////////////////////////////////////////////////////////////////
typedef struct Cube {
  SVECTOR rotation;
  VECTOR position;
  VECTOR scale;
  VECTOR vel;
  VECTOR acc;
  SVECTOR vertices[8];
  short faces[24];
} Cube;

typedef struct Floor {
  SVECTOR rotation;
  VECTOR position;
  VECTOR scale;
  SVECTOR vertices[4];
  short faces[6];
} Floor;

///////////////////////////////////////////////////////////////////////////////
// Declarations and global variables
///////////////////////////////////////////////////////////////////////////////

//DoubleBuff screen;           // Struct to hold the display & draw buffers
//u_short currbuff;            // Holds the current buffer number (0 or 1)

POLY_G4 *polyg4;
POLY_F3 *polyf3;

MATRIX world = {0};
MATRIX viewmat = {0};

Camera camera;

Cube cube = {
  {0, 0, 0},
  {0, -400, 1800},
  {ONE, ONE, ONE},
  {0, 0, 0},
  {0, 1, 0},
  {
    { -128, -128, -128 },
    {  128, -128, -128 },
    {  128, -128,  128 },
    { -128, -128,  128 },
    { -128,  128, -128 },
    {  128,  128, -128 },
    {  128,  128,  128 },
    { -128,  128,  128 },
  },
  {
    3, 2, 0, 1,
    0, 1, 4, 5,
    4, 5, 7, 6,
    1, 2, 5, 6,
    2, 3, 6, 7,
    3, 0, 7, 4,
  }
};

Floor floor = {
  {0, 0, 0},
  {0, 450, 1800},
  {ONE, ONE, ONE},
  {
    {-900, 0, -900},
    {-900, 0,  900},
    { 900, 0, -900},
    { 900, 0,  900},
  },
  {
    0, 1, 2,
    1, 3, 2,
  }
};

///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Setup(void) {
  // Setup the display environment
  ScreenInit();

  // Initialize the joypad
  JoyPadInit();

  // Reset next primitive pointer to the start of the primitive buffer
  ResetNextPrim(GetCurrentBuff());

  camera.position.vx = 500;
  camera.position.vy = -1500; // y grows down
  camera.position.vz = -1500;

  camera.lookat = (MATRIX){0};
}

///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Update(void) {
  int i, nclip;
  long otz, p, flg;

  // Empty the Ordering Table
  EmptyOT(GetCurrentBuff());

  JoyPadUpdate();

  if (JoyPadCheck(PAD1_LEFT)) {
    camera.position.vx -= 50;
  }
  if (JoyPadCheck(PAD1_RIGHT)) {
    camera.position.vx += 50;
  }
  if (JoyPadCheck(PAD1_UP)) {
    camera.position.vy -= 50;
  }
  if (JoyPadCheck(PAD1_DOWN)) {
    camera.position.vy += 50;
  }
  if (JoyPadCheck(PAD1_CROSS)) {
    camera.position.vz += 50;
  }
  if (JoyPadCheck(PAD1_CIRCLE)) {
    camera.position.vz -= 50;
  }

  // Update the cube velocity based on its acceleration
  cube.vel.vx += cube.acc.vx;
  cube.vel.vy += cube.acc.vy;
  cube.vel.vz += cube.acc.vz;

  // Update the cube position based on its velocity
  cube.position.vx += (cube.vel.vx) >> 1;
  cube.position.vy += (cube.vel.vy) >> 1;
  cube.position.vz += (cube.vel.vz) >> 1;

  // Check "collision" with the floor
  if (cube.position.vy + 150 > floor.position.vy) {
    cube.vel.vy *= -1;
  }

  // Compute the LookAt mat for this frame
  LookAt(&camera, &camera.position, &cube.position, &(VECTOR){0,-ONE,0});

  /////////////////////
  // Draw the Cube
  /////////////////////
  RotMatrix(&cube.rotation, &world);
  TransMatrix(&world, &cube.position);
  ScaleMatrix(&world, &cube.scale);

  // Combine View and World mats
  CompMatrixLV(&camera.lookat, &world, &viewmat);

  SetRotMatrix(&viewmat);
  SetTransMatrix(&viewmat);

  for (i = 0; i < 24; i += 4) {
    polyg4 = (POLY_G4*) GetNextPrim();
    setPolyG4(polyg4);
    setRGB0(polyg4, 255, 0, 255);
    setRGB1(polyg4, 255, 255, 0);
    setRGB2(polyg4, 0, 255, 255);
    setRGB3(polyg4, 0, 255, 0);


    // Loading the first 3 vertices (the GTE can only perform a max. of 3 vectors at a time)
    gte_ldv0(&cube.vertices[cube.faces[i + 0]]);
    gte_ldv1(&cube.vertices[cube.faces[i + 1]]);
    gte_ldv2(&cube.vertices[cube.faces[i + 2]]);

    gte_rtpt();

    gte_nclip();
    gte_stopz(&nclip);

    if (nclip >= 0) {
      // Store/save the transformed projected coord of the first vertex
      gte_stsxy0(&polyg4->x0);

      // Load the last 4th vertex
      gte_ldv0(&cube.vertices[cube.faces[i + 3]]);

      // Project & transform the remaining 4th vertex
      gte_rtps();

      // Store the transformed last vertices
      gte_stsxy3(&polyg4->x1, &polyg4->x2, &polyg4->x3);

      gte_avsz4();
      gte_stotz(&otz);

      if ((otz > 0) && (otz < OT_LEN)) 
      {
        addPrim(GetOTAt(GetCurrentBuff(), otz), polyg4);
        IncrementNextPrim(sizeof(POLY_G4));
      }
    }
  }

  /////////////////////
  // Draw the Floor
  /////////////////////
  RotMatrix(&floor.rotation, &world);
  TransMatrix(&world, &floor.position);
  ScaleMatrix(&world, &floor.scale);

    // Combine View and World mats
  CompMatrixLV(&camera.lookat, &world, &viewmat);

  SetRotMatrix(&viewmat);
  SetTransMatrix(&viewmat);

  for (i = 0; i < 6; i += 3) {
    polyf3 = (POLY_F3*) GetNextPrim();
    setPolyF3(polyf3);
    setRGB0(polyf3, 255, 255, 0);

    gte_ldv0(&floor.vertices[floor.faces[i + 0]]);
    gte_ldv1(&floor.vertices[floor.faces[i + 1]]);
    gte_ldv2(&floor.vertices[floor.faces[i + 2]]);

    gte_rtpt();

    gte_nclip();
    gte_stopz(&nclip);

    if (nclip >= 0) {
      gte_stsxy3(&polyf3->x0, &polyf3->x1, &polyf3->x2);

      gte_avsz3();
      gte_stotz(&otz);

      if ((otz > 0) && (otz < OT_LEN)) 
      {
        addPrim(GetOTAt(GetCurrentBuff(), otz), polyf3);
        IncrementNextPrim(sizeof(POLY_F3));
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Render function that invokes the draw of the current frame
///////////////////////////////////////////////////////////////////////////////
void Render(void) {
  DisplayFrame();
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////
int main(void) {
  Setup();
  while (1) {
    Update();
    Render();
  }
  return 0;
}
