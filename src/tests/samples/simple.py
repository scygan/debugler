#! /usr/bin/env python

import OpenGL 
from ctypes import c_void_p
OpenGL.ERROR_ON_COPY = True 
from OpenGL.GL import *
from OpenGL.GLUT import *
 
# PyOpenGL 3.0.1 introduces this convenience module...
from OpenGL.GL.shaders import *
 
import time, sys
program = None
vao = GLuint(0)
vbo = GLuint(0)

def initializeState(Width, Height):
    global vao
    vao = glGenVertexArrays(1)
    glBindVertexArray(vao)
    
    global vbo
    vbo = glGenBuffers(1)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    
    vertexPositions = [
        0.75,  0.75,  0.0,  1.0,
        0.75, -0.75,  0.0,  1.0,
        -0.75, -0.75,  0.0,  1.0,
    ]
    array_type = (GLfloat * len(vertexPositions))
    glBufferData(GL_ARRAY_BUFFER, len(vertexPositions) * 4, array_type(*vertexPositions), GL_STATIC_DRAW)
    glEnableVertexAttribArray(0)
    glVertexAttribPointer(0, 4, GL_FLOAT, False, 0, c_void_p(0))
    
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glClearDepth(1.0)  

    program = compileProgram(
        compileShader('''
            #version 150
            in vec4 position;
            
            void main() {
                gl_Position = position;
            }
 
        ''',GL_VERTEX_SHADER),
        compileShader('''
            #version 150
            out vec4 oColor;
            void main()
            {
                 oColor = vec4(1);
            }
    ''',GL_FRAGMENT_SHADER),
    )
    glUseProgram(program)
    
def tidy(Width, Height):
    glDeleteVertexArrays(1, vao)
    glDeleteBuffers(1, vbo)
    
 
def ReSizeGLScene(Width, Height):
    glViewport(0, 0, Width, Height)
 
# The main drawing function. 
def DrawGLScene():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3)

    glGetError()
 
    glutSwapBuffers()
 
def main():
    global window

    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
 
    glutInitWindowSize(640, 480)
 
    glutInitWindowPosition(0, 0)
 
    window = glutCreateWindow("simple.py")
 
 
    glutDisplayFunc(DrawGLScene)
    glutIdleFunc(DrawGLScene)
    glutReshapeFunc(ReSizeGLScene)
    initializeState(800, 600)
    glutMainLoop()
    
    tidy()
 
# Print message to console, and kick off the main to get it rolling.
 
if __name__ == "__main__":
    main()