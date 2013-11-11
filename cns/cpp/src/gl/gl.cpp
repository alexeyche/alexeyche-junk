#include <GL/glut.h>


#define window_width  640
#define window_height 480

// Main loop
void main_loop_function()
{
   // Z angle
   static float angle;
   // Clear color (screen) 
   // And depth (used internally to block obstructed objects)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // Load identity matrix
   glLoadIdentity();
   // Multiply in translation matrix
   glTranslatef(0,0, -10);
   // Multiply in rotation matrix
   glRotatef(angle, 0, 0, 1);
   // Render colored quad
   glBegin(GL_QUADS);
   glColor3ub(255, 000, 000); glVertex2f(-1,  1);
   glColor3ub(000, 255, 000); glVertex2f( 1,  1);
   glColor3ub(000, 000, 255); glVertex2f( 1, -1);
   glColor3ub(255, 255, 000); glVertex2f(-1, -1);
   glEnd();
   // Swap buffers (color buffers, makes previous render visible)
    glutSwapBuffers();
   // Increase angle to rotate
   angle+=0.25;
}

// Initialze OpenGL perspective matrix
void GL_Setup(int width, int height)
{

    glViewport( 0, 0, width, height );
    glMatrixMode( GL_PROJECTION );
    glEnable( GL_DEPTH_TEST );
    gluPerspective( 45, (float)width/height, .1, 100 );
    glMatrixMode( GL_MODELVIEW );
}


// Initialize GLUT and start main loop
int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitWindowSize(window_width, window_height);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

    glutCreateWindow("GLUT Example!!!");

    glutIdleFunc(main_loop_function);

    GL_Setup(window_width, window_height);
   glutMainLoop();

}
