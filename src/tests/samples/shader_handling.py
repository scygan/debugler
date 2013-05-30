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
    
	source = '''
            #version 120
            attribute vec4 position;
            
            void main() {
                gl_Position = position;
            }
         '''
		 
	#simple case: create-delete
	shader = glCreateShader(GL_VERTEX_SHADER);
	#test point: we have one shader
	glDeleteShader(shader);
	#test point: shader deleted
	glFlush(); #flush is only to mark case end
	
	#source cache test
	shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader, source);
	glDeleteShader(shader);
	#test point: shader deleted, has cached source
	glFlush(); #flush is only to mark case end
		
	#lazy deletion test, create-attach-delete-detach
	shader = glCreateShader(GL_VERTEX_SHADER);
	program = glCreateProgram();
	glAttachShader(program, shader);
	#test point: program has shader
	glDeleteShader(shader);
	#test point: shader not deleted
	glDetachShader(program, shader);
	#test point: shader deleted, program has no shader
	glDeleteProgram(program)
	glFlush(); #flush is only to mark case end
		
	#lazy deletion test2: create-attach-delete-deleteprogram
	shader = glCreateShader(GL_VERTEX_SHADER);
	program = glCreateProgram();
	glAttachShader(program, shader);
	glDeleteShader(shader);
	#test point: shader not deleted, program has shader
	glDeleteProgram(program)
	#test point: shader deleted
	glFlush(); #flush is only to mark case end
		
	#caching in lazy deletion test (source after delete by detach) 
	shader = glCreateShader(GL_VERTEX_SHADER);
	program = glCreateProgram();
	glAttachShader(program, shader);
	glDeleteShader(shader);
	glShaderSource(shader, source);
	glDetachShader(program, shader);
	#test point: cached source
	glDeleteProgram(program)
	glFlush(); #flush is only to mark case end
	
	#caching in lazy deletion test 2 (source after delete by deleteprogram) 
	shader = glCreateShader(GL_VERTEX_SHADER);
	program = glCreateProgram();
	glAttachShader(program, shader);
	glDeleteShader(shader);
	glShaderSource(shader, source);
	glDeleteProgram(program)
	#test point: cached source
	glFlush(); #flush is only to mark case end
	
    
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
 
    window = glutCreateWindow("shaderhandling.py")
 
 
    glutDisplayFunc(DrawGLScene)
    glutIdleFunc(DrawGLScene)
    glutReshapeFunc(ReSizeGLScene)
    initializeState(800, 600)
    glutMainLoop()
    
    tidy()
 
# Print message to console, and kick off the main to get it rolling.
 
if __name__ == "__main__":
    main()
