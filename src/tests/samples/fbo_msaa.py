#! /usr/bin/env python

import OpenGL 
from ctypes import c_void_p
OpenGL.ERROR_ON_COPY = True 
from OpenGL.GL import *
from OpenGL.GLUT import *
 
import time, sys
program = None
vbo = GLuint(0)


def initializeState(Width, Height):
    
	rbo = glGenRenderbuffers(1);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 256, 256);
	
	tex = glGenTextures(2)
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex[0]);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 256, 256, 0);
	
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, tex[1]);
	glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 4, GL_RGBA8, 256, 256, 256, 0);
	
	fbo = glGenFramebuffers(1);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, tex[0], 0);
	glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, tex[1], 0, 0);
	
	
    
def tidy(Width, Height):
    glDeleteBuffers(1, vbo)
    
 
def ReSizeGLScene(Width, Height):
    glViewport(0, 0, Width, Height)
 
# The main drawing function. 
def DrawGLScene():
    glutSwapBuffers()
 
def main():
    global window

    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA)
 
    glutInitWindowSize(640, 480)
 
    glutInitWindowPosition(0, 0)
 
    window = glutCreateWindow("fbo_msaa.py")
 
 
    glutDisplayFunc(DrawGLScene)
    glutIdleFunc(DrawGLScene)
    glutReshapeFunc(ReSizeGLScene)
    initializeState(800, 600)
    glutMainLoop()
    
    tidy()
 
# Print message to console, and kick off the main to get it rolling.
 
if __name__ == "__main__":
    main()
