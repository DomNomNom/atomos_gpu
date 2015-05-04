#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <GL/glut.h>
#include <vector>
#include <algorithm>  // min
#include <cassert> // assert

#include "time.hpp"
#include "shader.h"


bool enableTimingPrintouts = false;

GLuint window;
int window_wd = 300;
int window_ht = 300;
uint frameCount = 0;

bool paused = false;

const unsigned simulation_wd = 50;
const unsigned simulation_ht = 50;

Shader shader_tick;
Shader shader_display;

// GLubyte state[]

const unsigned maxCapacity = 128;
const unsigned maxVolumes = 4096;
GLuint framebufferNames[2]; // The frame buffer objects
GLuint renderTextures[2];   // The textures we're going to render to
unsigned int renderSource = 0;
unsigned int renderTarget = 1;

// given a physical location (x<simulation_wd, y<simulation_ht)
// gives a row index of the simulation
GLuint positionMapTexture;
GLuint positionMap[simulation_wd * simulation_ht * 4];

GLuint connectionMapTexture;
GLuint connectionMap[maxCapacity * maxVolumes * 4];

unsigned getVolumeRow(unsigned simX, unsigned simY) {
    return positionMap[(simY*simulation_wd + simX) * 4];
}

// void mouseHandler(int, int state, int, int) {
//     if (state == GLUT_DOWN) {
//         printf("mousepressed\n");
//     }
//     // glutPostRedisplay();
// }



void cleanUp() {

    printf("starting clean up\n");

    glutDestroyWindow(window);

    printf("finished clean up\n");
}

void reshapeHandler(int wd, int ht) {
    window_wd = wd;
    window_ht = ht;
    float scale = std::min(
        (float)window_wd / simulation_wd,
        (float)window_ht / simulation_ht
    );
    glPixelZoom(scale, scale);
}

// Input
void keyHandler(unsigned char key, int, int) {
    switch (key) {
        case 27: // Escape -> exit
        case 'q':
            cleanUp();
            exit(0);
            break; // lol
        case ' ':
            paused = !paused;
    }
}




// prints something identifyable and the time in miliseconds since the last call of this function
void doTiming(const char *description) {
    if (enableTimingPrintouts) {
        printf("[ %s: %5.1fms ]        ", description, time_dt()*1000.0);
    }
}


// draws a rectangle that covers the whole window
void drawRect() {
    glColor4f(1, 0.5, 0, 1);
    static float tv = 1.0;
    glBegin(GL_TRIANGLES);
        // two triangles that cover the screen
        glTexCoord2f(0, 0); glVertex3f(-tv,-tv, 0.0);
        glTexCoord2f(1, 0); glVertex3f( tv,-tv, 0.0);
        glTexCoord2f(0, 1); glVertex3f(-tv, tv, 0.0);

        glTexCoord2f(1, 1); glVertex3f( tv, tv, 0.0);
        glTexCoord2f(0, 1); glVertex3f(-tv, tv, 0.0);
        glTexCoord2f(1, 0); glVertex3f( tv,-tv, 0.0);
    glEnd();
}


// sets uniforms that are used in both shader_tick and shader_display
void setSharedShaderUniforms(Shader &s) {
    glActiveTexture(GL_TEXTURE2);  glBindTexture(GL_TEXTURE_2D, connectionMapTexture);
    glUniform1i( glGetUniformLocation(s.id(), "connectionMap"), 2);

    glUniform1f( glGetUniformLocation(s.id(), "time"), time_sinceInit());
    glUniform1i( glGetUniformLocation(s.id(), "lastTick"), 0);
    glUniform1ui(glGetUniformLocation(s.id(), "maxVolumes"),  maxVolumes);
    glUniform1ui(glGetUniformLocation(s.id(), "maxCapacity"), maxCapacity);
}

void checkGLError() {
    GLenum e = glGetError();
    if (e) {
        printf("GL error: ");
        switch (e) {
            case GL_INVALID_ENUM:                  printf("GL_INVALID_ENUM\n"); break;
            case GL_INVALID_VALUE:                 printf("GL_INVALID_VALUE\n"); break;
            case GL_INVALID_OPERATION:             printf("GL_INVALID_OPERATION\n"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: printf("GL_INVALID_FRAMEBUFFER_OPERATION\n"); break;
            case GL_OUT_OF_MEMORY:                 printf("GL_OUT_OF_MEMORY\n"); break;
            default:                               printf("wtf! unknown error\n"); break;
        }
        printf("\n");
        cleanUp();
        exit(1);
    }
    else {
        // printf("no error\n");
    }
}

// the body of the main loop
void tick() {
    doTiming("outside of tick()");

    frameCount += 1;
    // printf("frame %d\n", frameCount);



    // ====== do a tick() ======
    if (!paused) {

        // switch the buffers
        renderTarget = 1 - renderTarget;
        renderSource = 1 - renderSource;

        shader_tick.bind();
        glEnable(GL_TEXTURE_2D);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferNames[renderTarget]);
        glViewport(0, 0, maxCapacity, maxVolumes);
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.9, 0.1, 0.1, 1.0);
        glClearDepth(1000);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set shader data
        glActiveTexture(GL_TEXTURE0);  glBindTexture(GL_TEXTURE_2D, renderTextures[renderSource]);
        setSharedShaderUniforms(shader_tick);
        glBindFragDataLocation(shader_tick.id(), 0, "fragColor");
        drawRect();
        shader_tick.unbind();
    }







    // ====== Render to screen ======
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Render to the screen
    shader_display.bind();
    glViewport(0, 0, window_wd, window_ht);
    glClearColor(0.9, 0.5, 0.1, 1.0);
    glClearDepth(1000);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setSharedShaderUniforms(shader_display);
    // set shader data
    glActiveTexture(GL_TEXTURE1);  glBindTexture(GL_TEXTURE_2D, renderTextures[renderTarget]);
    glActiveTexture(GL_TEXTURE0);  glBindTexture(GL_TEXTURE_2D, positionMapTexture);
    glUniform1i( glGetUniformLocation(shader_display.id(), "lastTick"), 1);
    glUniform1i( glGetUniformLocation(shader_display.id(), "positionMapTexture"), 0);
    drawRect();

    // glActiveTexture(GL_TEXTURE0);  glBindTexture(GL_TEXTURE_2D, positionMapTexture);
    // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, positionMap);
    // for (unsigned i=0; i<simulation_wd * simulation_ht; ++i) {
    //     printf("%d ", positionMap[i*4 + 0]);
    //     printf("%d ", positionMap[i*4 + 1]);
    //     printf("%d ", positionMap[i*4 + 2]);
    //     printf("%d    ", positionMap[i*4 + 3]);
    // }
    // exit(1);

    shader_display.unbind();



    glutSwapBuffers();
    glutPostRedisplay();  // active rendering


    checkGLError(); // just to be safe

    doTiming("draw");
    if (enableTimingPrintouts) {
        printf("\n");
    }
}

// sets up a texture (without width or height)
void createDataTexture(GLuint *holder) {
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, holder);
    glBindTexture(GL_TEXTURE_2D, *holder);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void initPositionMap() {
    for (unsigned i=0; i<simulation_wd * simulation_ht;  ++i) {
        // we add 1 as row zero always contains void
        positionMap[i*4 + 0] = i + 1;
        positionMap[i*4 + 1] = 0;
        positionMap[i*4 + 2] = 0;
        positionMap[i*4 + 3] = 255;
    }

    createDataTexture(&positionMapTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, simulation_wd, simulation_ht); // TODO: less channels
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, simulation_wd, simulation_ht, GL_RGBA_INTEGER, GL_UNSIGNED_INT, positionMap);
    glDisable(GL_TEXTURE_2D);
}


// creates a connection between two volumes
void addConnection(unsigned rowA, unsigned rowB) {
    assert (0 < rowA && rowA < maxVolumes);
    assert (0 < rowB && rowB < maxVolumes);

    // search for available slots
    // this linear search could be optimized out
    unsigned slotA;
    unsigned slotB;
    for (slotA=0; slotA<maxCapacity && connectionMap[(rowA*maxCapacity + slotA)*4 + 1] != 0; ++slotA) {}
    for (slotB=0; slotB<maxCapacity && connectionMap[(rowB*maxCapacity + slotB)*4 + 1] != 0; ++slotB) {}
    assert (slotA < maxCapacity);
    assert (slotB < maxCapacity);

    // make A point to B
    connectionMap[(rowA*maxCapacity + slotA)*4 + 0] = slotB;
    connectionMap[(rowA*maxCapacity + slotA)*4 + 1] = rowB;

    // make B point to A
    connectionMap[(rowB*maxCapacity + slotB)*4 + 0] = slotA;
    connectionMap[(rowB*maxCapacity + slotB)*4 + 1] = rowA;
}

void initConnectionMap() {
    // Initialize with zeroes
    // a zero means it points to itself
    for (unsigned i=0; i<maxCapacity * maxVolumes;  ++i) {
        connectionMap[i*4 + 0] = 0;  // target slot (x)
        connectionMap[i*4 + 1] = 0;  // target row (y)
        connectionMap[i*4 + 2] = 0;
        connectionMap[i*4 + 3] = 0;
    }

    // add connections
    for (unsigned y=0; y<simulation_ht; ++y) {
        for (unsigned x=0; x<simulation_wd; ++x) {
            if (x != 0) {
                addConnection(
                    getVolumeRow(x,   y),
                    getVolumeRow(x-1, y)
                );
            }
            if (y != 0) {
                addConnection(
                    getVolumeRow(x, y),
                    getVolumeRow(x, y-1)
                );
            }
        }
    }

    // repeat connections so we need less ticks
    // find the minimum width that is ok to repeat
    unsigned repeatWidth = 0;
    for (unsigned vol=0; vol<maxVolumes; ++vol) {
        for (unsigned slot=0; slot<maxCapacity; ++slot) {
            if (connectionMap[(vol*maxCapacity + slot)*4 + 1] != 0) {
                repeatWidth = std::max(repeatWidth, slot);
            }
        }
    }
    repeatWidth += 1; // width rather than last non-zero index
    // printf("repeat: %d\n", repeatWidth);

    // trigger warning: long lines
    // copy/move connections in blocks of repeatWidth horizontally
    for (unsigned vol=0; vol<maxVolumes; ++vol) {
        for (unsigned slotShift=repeatWidth; slotShift+repeatWidth-1<maxCapacity; slotShift+=repeatWidth) {
            // printf("repeating: %d\n", slotShift);
            for (unsigned slot=0; slot<repeatWidth; ++slot) {
                int originPixel = (vol*maxCapacity + slot)*4;
                connectionMap[originPixel + slotShift*4 + 0] =  connectionMap[originPixel + 0] + slotShift;
                connectionMap[originPixel + slotShift*4 + 1] =  connectionMap[originPixel + 1];
                connectionMap[originPixel + slotShift*4 + 2] =  connectionMap[originPixel + 2];
                connectionMap[originPixel + slotShift*4 + 3] =  connectionMap[originPixel + 3];
            }
        }
    }

    // create the texture from this data
    createDataTexture(&connectionMapTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, maxCapacity, maxVolumes);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, maxCapacity, maxVolumes, GL_RGBA_INTEGER, GL_UNSIGNED_INT, connectionMap);
    glDisable(GL_TEXTURE_2D);

}

void initFrameBuffers() {
    // create the base volumeData to initialize the renderTextures
    GLubyte volumeData[maxCapacity * maxVolumes * 4];

    // initialize everything with a "unassigned" value (blue)
    for (unsigned i=0; i<maxCapacity*maxVolumes; ++i) {
        volumeData[4*i + 0] = 0;
        volumeData[4*i + 1] = 0;
        volumeData[4*i + 2] = 255;
        volumeData[4*i + 3] = 255;
    }

    // the first row represents space
    for (unsigned col=0; col<maxCapacity; ++col) {
        volumeData[col*4 + 0] = 255;
        volumeData[col*4 + 1] = 0;
        volumeData[col*4 + 2] = 128;
        volumeData[col*4 + 3] = 255;
    }

    // x and y are in simulation space
    // row and col are in the space of the texture we are initializing
    // each row in the texture represents the array (with nulls) of particles in a volume
    for (unsigned y=0; y<simulation_ht; ++y) {
        for (unsigned x=0; x<simulation_wd; ++x) {
            GLubyte particleValue = (
                x >  simulation_wd * 0.25 &&
                x <= simulation_wd * 0.5  &&
                y >  simulation_ht * 0.25 &&
                y <= simulation_ht * 0.5
            ) ? 255 : 0;

            unsigned row = getVolumeRow(x, y);
            for (unsigned col=0; col<maxCapacity; ++col) {
                unsigned pixelIndex = (row * maxCapacity + col) * 4;
                volumeData[pixelIndex + 0] = 0;
                volumeData[pixelIndex + 1] = particleValue; //(x>simulation_ht*0.4 && col > maxCapacity/2)? 0: particleValue;
                volumeData[pixelIndex + 2] = 0;
                volumeData[pixelIndex + 3] = 255; // full alpha

                if (pixelIndex >= maxCapacity * maxVolumes * 4) {
                    printf("wtf\n");
                }
            }
        }
    }

    // create and initialize the renderTextures and framebuffers
    for (unsigned i=0; i<2; ++i) {
        // create the underlying texture
        createDataTexture(&renderTextures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8UI, maxCapacity, maxVolumes);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, maxCapacity, maxVolumes, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, volumeData);
        glDisable(GL_TEXTURE_2D);

        // create the framebuffer and attach the texture to it
        glGenFramebuffers(1, &framebufferNames[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferNames[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextures[i], 0);

        // reset the current framebuffer (why is this needed?)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

int main(int argc, char** argv) {
    printf("hello world\n");

    time_init();

    // set up the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(window_wd, window_ht);
    window = glutCreateWindow("atomos_gpu");
    glutDisplayFunc(tick);
    glutReshapeFunc(reshapeHandler);
    glutKeyboardFunc(keyHandler);
    // glutMouseFunc(mouseHandler);

    assert (maxVolumes > simulation_ht * simulation_ht);
    initPositionMap();
    initConnectionMap();
    initFrameBuffers();


    shader_tick.init(   "shaders/vert.glsl", "shaders/tick.glsl"   );
    shader_display.init("shaders/vert.glsl", "shaders/display.glsl");
    checkGLError();


    doTiming("window setup");
    printf("\n");

    glutMainLoop();

    return 0;
}


