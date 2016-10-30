// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include "GlyphExtractor.h"

// Specify that we want the OpenGL core profile before including GLFW headers
#ifdef _WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#else
#include <glad/glad.h>
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#endif

//STB
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace std;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint TCSshader, GLuint TESshader, GLuint fragmentShader);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

enum Font { Lora = 0, SourceSansPro, GreatVibes, AlexBrush, Inconsolata, Amatic };

// Global Variables
static bool needsRedraw_ = true;
static bool isQuadratic_ = true;
static bool isScrolling_ = false;
static Font currNameFont = Lora;
static Font currTextFont = AlexBrush;
static GLfloat offset_ = 1.1f;
static GLfloat minOffset_ = -16.0f;
static GLfloat multiplier_ = 1.0f;
GlyphExtractor extractor_;

// Geometry Buffers
vector<GLfloat> pointVertices_;
vector<GLfloat> pointColours_;
vector<GLfloat> lineVertices_;
vector<GLfloat> lineColours_;
vector<GLfloat> quadraticVertices_;
vector<GLfloat> quadraticColours_;
vector<GLfloat> cubicVertices_;
vector<GLfloat> cubicColours_;

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
   // OpenGL names for vertex and fragment shaders, shader program
   GLuint  vertex;
   GLuint  TCS;
   GLuint  TES;
   GLuint  fragment;
   GLuint  program;

   // initialize shader and program names to zero (OpenGL reserved value)
   MyShader() : vertex(0), TCS(0), TES(0), fragment(0), program(0)
   {}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader, string tcs, string tes)
{
   // load shader source from files
   string vertexSource = LoadSource("vertex.glsl");
   string fragmentSource = LoadSource("fragment.glsl");
   string TCSSource = LoadSource(tcs);
   string TESSource = LoadSource(tes);
   if (vertexSource.empty() || fragmentSource.empty() ||
      TCSSource.empty() || TESSource.empty())
      return false;

   // compile shader source into shader objects
   shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
   shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
   shader->TCS = CompileShader(GL_TESS_CONTROL_SHADER, TCSSource);
   shader->TES = CompileShader(GL_TESS_EVALUATION_SHADER, TESSource);

   // link shader program
   shader->program = LinkProgram(shader->vertex, shader->TCS, shader->TES, shader->fragment);

   // check for OpenGL errors and return false if error occurred
   return !CheckGLErrors();
}

// load, compile, and link shaders, without tessellation
bool InitializeShaders(MyShader *shader)
{
   // load shader source from files
   string vertexSource = LoadSource("vertex.glsl");
   string fragmentSource = LoadSource("fragment.glsl");
   if (vertexSource.empty() || fragmentSource.empty())
      return false;

   // compile shader source into shader objects
   shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
   shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

   // link shader program
   shader->program = LinkProgram(shader->vertex, shader->fragment);

   // check for OpenGL errors and return false if error occurred
   return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
   // unbind any shader programs and destroy shader objects
   glUseProgram(0);
   glDeleteProgram(shader->program);
   glDeleteShader(shader->vertex);
   glDeleteShader(shader->fragment);
   glDeleteShader(shader->TCS);
   glDeleteShader(shader->TES);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
   // OpenGL names for array buffer objects, vertex array object
   GLuint  vertexBuffer;
   GLuint  textureBuffer;
   GLuint  colourBuffer;
   GLuint  vertexArray;
   GLsizei elementCount;

   // initialize object names to zero (OpenGL reserved value)
   MyGeometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
   {}
};

void clearVectors()
{
   // clear vertices
   pointVertices_.clear();
   lineVertices_.clear();
   quadraticVertices_.clear();
   cubicVertices_.clear();

   // clear colours
   pointColours_.clear();
   lineColours_.clear();
   quadraticColours_.clear();
   cubicColours_.clear();
}

void initQuadraticControlPoints()
{
   clearVectors();

   GLfloat scale = 2.5f;

   // first set
   quadraticVertices_.push_back(1.0f / scale);
   quadraticVertices_.push_back(1.0f / scale);
   quadraticVertices_.push_back(2.0f / scale);
   quadraticVertices_.push_back(-1.0f / scale);
   quadraticVertices_.push_back(0.0f / scale);
   quadraticVertices_.push_back(-1.0f / scale);

   // line equivalent
   lineVertices_.push_back(1.0f / scale);
   lineVertices_.push_back(1.0f / scale);

   lineVertices_.push_back(2.0f / scale);
   lineVertices_.push_back(-1.0f / scale);
   lineVertices_.push_back(2.0f / scale);
   lineVertices_.push_back(-1.0f / scale);

   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(-1.0f / scale);

   //second set
   quadraticVertices_.push_back(0.0f / scale);
   quadraticVertices_.push_back(-1.0f / scale);
   quadraticVertices_.push_back(-2.0f / scale);
   quadraticVertices_.push_back(-1.0f / scale);
   quadraticVertices_.push_back(-1.0f / scale);
   quadraticVertices_.push_back(1.0f / scale);

   // line equivalent
   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(-1.0f / scale);

   lineVertices_.push_back(-2.0f / scale);
   lineVertices_.push_back(-1.0f / scale);
   lineVertices_.push_back(-2.0f / scale);
   lineVertices_.push_back(-1.0f / scale);

   lineVertices_.push_back(-1.0f / scale);
   lineVertices_.push_back(1.0f / scale);

   // third set
   quadraticVertices_.push_back(-1.0f / scale);
   quadraticVertices_.push_back(1.0f / scale);
   quadraticVertices_.push_back(0.0f / scale);
   quadraticVertices_.push_back(1.0f / scale);
   quadraticVertices_.push_back(1.0f / scale);
   quadraticVertices_.push_back(1.0f / scale);

   // line equivalent
   lineVertices_.push_back(-1.0f / scale);
   lineVertices_.push_back(1.0f / scale);

   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(1.0f / scale);
   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(1.0f / scale);

   lineVertices_.push_back(1.0f / scale);
   lineVertices_.push_back(1.0f / scale);

   //fourth set
   quadraticVertices_.push_back(1.2f / scale);
   quadraticVertices_.push_back(0.5f / scale);
   quadraticVertices_.push_back(2.5f / scale);
   quadraticVertices_.push_back(1.0f / scale);
   quadraticVertices_.push_back(1.3f / scale);
   quadraticVertices_.push_back(-0.4f / scale);

   // line equivalent
   lineVertices_.push_back(1.2f / scale);
   lineVertices_.push_back(0.5f / scale);

   lineVertices_.push_back(2.5f / scale);
   lineVertices_.push_back(1.0f / scale);
   lineVertices_.push_back(2.5f / scale);
   lineVertices_.push_back(1.0f / scale);

   lineVertices_.push_back(1.3f / scale);
   lineVertices_.push_back(-0.4f / scale);

   // add control points to vector
   pointVertices_ = quadraticVertices_;

   // init quadratic colours
   for (unsigned int i = 0; i < quadraticVertices_.size() / 2; i++)
   {
      quadraticColours_.push_back(0.2f);
      quadraticColours_.push_back(0.2f);
      quadraticColours_.push_back(1.0f);
   }

   // init line colours
   for (unsigned int i = 0; i < lineVertices_.size() / 2; i++)
   {
      lineColours_.push_back(0.0f);
      lineColours_.push_back(0.0f);
      lineColours_.push_back(0.3f);
   }

   // init point colours
   for (unsigned int i = 0; i < pointVertices_.size() / 2; i++)
   {
      if (i % 3 == 0 || i % 3 == 2)
      {
         pointColours_.push_back(1.0f);
         pointColours_.push_back(0.0f);
         pointColours_.push_back(0.0f);
      }
      else
      {
         pointColours_.push_back(1.0f);
         pointColours_.push_back(1.0f);
         pointColours_.push_back(0.0f);
      }
   }
}

void initCubicControlPoints()
{
   clearVectors();

   GLfloat scale = 9.0f;

   // first set
   cubicVertices_.push_back(1.0f / scale);
   cubicVertices_.push_back(1.0f / scale);
   cubicVertices_.push_back(4.0f / scale);
   cubicVertices_.push_back(0.0f / scale);
   cubicVertices_.push_back(6.0f / scale);
   cubicVertices_.push_back(2.0f / scale);
   cubicVertices_.push_back(9.0f / scale);
   cubicVertices_.push_back(1.0f / scale);

   // line equivalent
   lineVertices_.push_back(1.0f / scale);
   lineVertices_.push_back(1.0f / scale);

   lineVertices_.push_back(4.0f / scale);
   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(4.0f / scale);
   lineVertices_.push_back(0.0f / scale);

   lineVertices_.push_back(6.0f / scale);
   lineVertices_.push_back(2.0f / scale);
   lineVertices_.push_back(6.0f / scale);
   lineVertices_.push_back(2.0f / scale);

   lineVertices_.push_back(9.0f / scale);
   lineVertices_.push_back(1.0f / scale);

   // second set
   cubicVertices_.push_back(8.0f / scale);
   cubicVertices_.push_back(2.0f / scale);
   cubicVertices_.push_back(0.0f / scale);
   cubicVertices_.push_back(8.0f / scale);
   cubicVertices_.push_back(0.0f / scale);
   cubicVertices_.push_back(-2.0f / scale);
   cubicVertices_.push_back(8.0f / scale);
   cubicVertices_.push_back(4.0f / scale);

   // line equivalent
   lineVertices_.push_back(8.0f / scale);
   lineVertices_.push_back(2.0f / scale);

   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(8.0f / scale);
   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(8.0f / scale);

   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(-2.0f / scale);
   lineVertices_.push_back(0.0f / scale);
   lineVertices_.push_back(-2.0f / scale);

   lineVertices_.push_back(8.0f / scale);
   lineVertices_.push_back(4.0f / scale);

   // third set
   cubicVertices_.push_back(5.0f / scale);
   cubicVertices_.push_back(3.0f / scale);
   cubicVertices_.push_back(3.0f / scale);
   cubicVertices_.push_back(2.0f / scale);
   cubicVertices_.push_back(3.0f / scale);
   cubicVertices_.push_back(3.0f / scale);
   cubicVertices_.push_back(5.0f / scale);
   cubicVertices_.push_back(2.0f / scale);

   // line equivalent
   lineVertices_.push_back(5.0f / scale);
   lineVertices_.push_back(3.0f / scale);

   lineVertices_.push_back(3.0f / scale);
   lineVertices_.push_back(2.0f / scale);
   lineVertices_.push_back(3.0f / scale);
   lineVertices_.push_back(2.0f / scale);

   lineVertices_.push_back(3.0f / scale);
   lineVertices_.push_back(3.0f / scale);
   lineVertices_.push_back(3.0f / scale);
   lineVertices_.push_back(3.0f / scale);

   lineVertices_.push_back(5.0f / scale);
   lineVertices_.push_back(2.0f / scale);

   // fourth set
   cubicVertices_.push_back(3.0f / scale);
   cubicVertices_.push_back(2.2f / scale);
   cubicVertices_.push_back(3.5f / scale);
   cubicVertices_.push_back(2.7f / scale);
   cubicVertices_.push_back(3.5f / scale);
   cubicVertices_.push_back(3.3f / scale);
   cubicVertices_.push_back(3.0f / scale);
   cubicVertices_.push_back(3.8f / scale);

   // line equivalent
   lineVertices_.push_back(3.0f / scale);
   lineVertices_.push_back(2.2f / scale);

   lineVertices_.push_back(3.5f / scale);
   lineVertices_.push_back(2.7f / scale);
   lineVertices_.push_back(3.5f / scale);
   lineVertices_.push_back(2.7f / scale);

   lineVertices_.push_back(3.5f / scale);
   lineVertices_.push_back(3.3f / scale);
   lineVertices_.push_back(3.5f / scale);
   lineVertices_.push_back(3.3f / scale);

   lineVertices_.push_back(3.0f / scale);
   lineVertices_.push_back(3.8f / scale);

   // fifth set
   cubicVertices_.push_back(2.8f / scale);
   cubicVertices_.push_back(3.5f / scale);
   cubicVertices_.push_back(2.4f / scale);
   cubicVertices_.push_back(3.8f / scale);
   cubicVertices_.push_back(2.4f / scale);
   cubicVertices_.push_back(3.2f / scale);
   cubicVertices_.push_back(2.8f / scale);
   cubicVertices_.push_back(3.5f / scale);

   // line equivalent
   lineVertices_.push_back(2.8f / scale);
   lineVertices_.push_back(3.5f / scale);

   lineVertices_.push_back(2.4f / scale);
   lineVertices_.push_back(3.8f / scale);
   lineVertices_.push_back(2.4f / scale);
   lineVertices_.push_back(3.8f / scale);

   lineVertices_.push_back(2.4f / scale);
   lineVertices_.push_back(3.2f / scale);
   lineVertices_.push_back(2.4f / scale);
   lineVertices_.push_back(3.2f / scale);

   lineVertices_.push_back(2.8f / scale);
   lineVertices_.push_back(3.5f / scale);

   // add control points to vector
   pointVertices_ = cubicVertices_;

   // init curve colours
   for (unsigned int i = 0; i < cubicVertices_.size() / 2; i++)
   {
      cubicColours_.push_back(0.2f);
      cubicColours_.push_back(0.2f);
      cubicColours_.push_back(1.0f);
   }

   // init line colours
   for (unsigned int i = 0; i < lineVertices_.size() / 2; i++)
   {
      lineColours_.push_back(0.0f);
      lineColours_.push_back(0.0f);
      lineColours_.push_back(0.3f);
   }

   // init point colours
   for (unsigned int i = 0; i < pointVertices_.size() / 2; i++)
   {
      if (i % 4 == 0 || i % 4 == 3)
      {
         pointColours_.push_back(1.0f);
         pointColours_.push_back(0.0f);
         pointColours_.push_back(0.0f);
      }
      else
      {
         pointColours_.push_back(1.0f);
         pointColours_.push_back(1.0f);
         pointColours_.push_back(0.0f);
      }
   }
}

void initFont(GlyphExtractor& extractor, string words, GLfloat offset)
{
   clearVectors();

   GLfloat scale = 0.90f;

   // Go through each character
   for (unsigned int i = 0; i < words.size(); i++)
   {
      // Load a character
      MyGlyph glyph = extractor.ExtractGlyph(words[i]);

      // load the contours
      for each(MyContour contour in glyph.contours)
      {
         for each(MySegment seg in contour)
         {
            // linear 
            if (seg.degree == 1)
            {
               for (int j = 0; j < 2; j++)
               {
                  lineVertices_.push_back((seg.x[j] + offset) * scale);
                  lineVertices_.push_back(seg.y[j] * scale);
               }
            }
            // quadratic
            else if (seg.degree == 2)
            {
               for (int j = 0; j < 3; j++)
               {
                  quadraticVertices_.push_back((seg.x[j] + offset) * scale);
                  quadraticVertices_.push_back(seg.y[j] * scale);
               }
            }
            // cubic
            else if (seg.degree == 3)
            {
               for (int j = 0; j < 4; j++)
               {
                  cubicVertices_.push_back((seg.x[j] + offset) * scale);
                  cubicVertices_.push_back(seg.y[j] * scale);
               }
            }
         }
      }
      offset += glyph.advance;
   }

   // init line colours
   for (unsigned int j = 0; j < lineVertices_.size() / 2; j++)
   {
      lineColours_.push_back(1.0f);
      lineColours_.push_back(0.0f);
      lineColours_.push_back(0.0f);
   }

   // init quadratic colours
   for (unsigned int j = 0; j < quadraticVertices_.size() / 2; j++)
   {
      quadraticColours_.push_back(1.0f);
      quadraticColours_.push_back(0.0f);
      quadraticColours_.push_back(0.0f);
   }

   // init cubic colours
   for (unsigned int j = 0; j < cubicVertices_.size() / 2; j++)
   {
      cubicColours_.push_back(1.0f);
      cubicColours_.push_back(0.0f);
      cubicColours_.push_back(0.0f);
   }
}

// create buffers and fill with geometry data, returning true if successful
bool InitializeGeometry(MyGeometry *geometry, vector<GLfloat>& vertices, vector<GLfloat>& colours)
{
   geometry->elementCount = vertices.size() / 2;

   // these vertex attribute indices correspond to those specified for the
   // input variables in the vertex shader
   const GLuint VERTEX_INDEX = 0;
   const GLuint COLOUR_INDEX = 1;

   // create an array buffer object for storing our vertices
   glGenBuffers(1, &geometry->vertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

   // create another one for storing our colours
   glGenBuffers(1, &geometry->colourBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
   glBufferData(GL_ARRAY_BUFFER, colours.size()*sizeof(GLfloat), colours.data(), GL_STATIC_DRAW);

   // create a vertex array object encapsulating all our vertex attributes
   glGenVertexArrays(1, &geometry->vertexArray);
   glBindVertexArray(geometry->vertexArray);

   // associate the position array with the vertex array object
   glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
   glVertexAttribPointer(VERTEX_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(VERTEX_INDEX);

   // associate the colour array with the vertex array object
   glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
   glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(COLOUR_INDEX);

   // unbind our buffers, resetting to default state
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   // check for OpenGL errors and return false if error occurred
   return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
   // unbind and destroy our vertex array object and associated buffers
   glBindVertexArray(0);
   glDeleteVertexArrays(1, &geometry->vertexArray);
   glDeleteBuffers(1, &geometry->vertexBuffer);
   glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(MyGeometry *geometry, MyShader *shader, int renderMode)
{
   // bind our shader program and the vertex array object containing our
   // scene geometry, then tell OpenGL to draw our geometry
   glUseProgram(shader->program);
   glBindVertexArray(geometry->vertexArray);
   glDrawArrays(renderMode, 0, geometry->elementCount);

   // reset state to default (no shader or geometry bound)
   glBindVertexArray(0);
   glUseProgram(0);

   // check for an report any OpenGL errors
   CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
   cout << "GLFW ERROR " << error << ":" << endl;
   cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
   else if (key == GLFW_KEY_B && action == GLFW_PRESS)
   {
      needsRedraw_ = true;
      isScrolling_ = false;

      isQuadratic_ = !isQuadratic_;

      if (isQuadratic_)
      {
         initQuadraticControlPoints();
      }
      else
      {
         initCubicControlPoints();
      }
   }
   else if (key == GLFW_KEY_N && action == GLFW_PRESS)
   {
      needsRedraw_ = true;
      isScrolling_ = false;

      if (currNameFont == Lora)
      {
         //Load a font file
         extractor_.LoadFontFile("fonts/lora/Lora-Regular.ttf");
         initFont(extractor_, "Amy", -.99f);
      }
      else if (currNameFont == SourceSansPro)
      {
         //Load a font file
         extractor_.LoadFontFile("fonts/source-sans-pro/SourceSansPro-Regular.otf");
         initFont(extractor_, "Amy", -0.91f);
      }
      else if (currNameFont == GreatVibes)
      {
         //Load a font file
         extractor_.LoadFontFile("fonts/great-vibes/GreatVibes-Regular.otf");
         initFont(extractor_, "Amy", -0.85f);
      }

      currNameFont = static_cast<Font>((currNameFont + 1) % 3);
   }
   else if (key == GLFW_KEY_T && action == GLFW_PRESS)
   {
      needsRedraw_ = true;
      isScrolling_ = true;

      if (currTextFont == AlexBrush)
      {
         //Load a font file
         minOffset_ = -16.0f;
         extractor_.LoadFontFile("fonts/alex-brush/AlexBrush-Regular.ttf");
      }
      else if (currTextFont == Inconsolata)
      {
         //Load a font file
         minOffset_ = -23.0f;
         extractor_.LoadFontFile("fonts/inconsolata/Inconsolata.otf");
      }
      else if (currTextFont == Amatic)
      {
         //Load a font file
         minOffset_ = -13.0f;
         extractor_.LoadFontFile("fonts/amatic/AmaticSC-Regular.ttf");
      }

      currTextFont = static_cast<Font>(currTextFont + 1);
      if (currTextFont == 6)
      {
         currTextFont = static_cast<Font>(3);
      }
   }
   else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
   {
      multiplier_ -= 0.2f;
      if (multiplier_ <= 0)
      {
         multiplier_ = 0.1f;
      }
   }
   else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
   {
      multiplier_ += 0.2f;
   }
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
   // initialize the GLFW windowing system
   if (!glfwInit()) {
      cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
      return -1;
   }
   glfwSetErrorCallback(ErrorCallback);

   // attempt to create a window with an OpenGL 4.1 core profile context
   GLFWwindow *window = 0;
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_SAMPLES, 4);
   window = glfwCreateWindow(512, 512, "CPSC 453 OpenGL Assignment 3", 0, 0);
   if (!window) {
      cout << "Program failed to create GLFW window, TERMINATING" << endl;
      glfwTerminate();
      return -1;
   }

   // set keyboard callback function and make our context current (active)
   glfwSetKeyCallback(window, KeyCallback);
   glfwMakeContextCurrent(window);

   //Initialize GLAD
   if (!gladLoadGL())
   {
      cout << "GLAD init failed" << endl;
      return -1;
   }

   // query and print out information about our OpenGL environment
   QueryGLVersion();

   // call function to load and compile shader programs
   MyShader lineShader;
   if (!InitializeShaders(&lineShader)) {
      cout << "Program could not initialize shaders, TERMINATING" << endl;
      return -1;
   }

   MyShader quadraticShader;
   if (!InitializeShaders(&quadraticShader, "quadraticTessControl.glsl", "quadraticTessEval.glsl")) {
      cout << "Program could not initialize shaders, TERMINATING" << endl;
      return -1;
   }

   MyShader cubicShader;
   if (!InitializeShaders(&cubicShader, "cubicTessControl.glsl", "cubicTessEval.glsl")) {
      cout << "Program could not initialize shaders, TERMINATING" << endl;
      return -1;
   }

   // call function to create and fill buffers with geometry data
   MyGeometry lineGeometry;
   MyGeometry quadraticGeometry;
   MyGeometry cubicGeometry;

   // Start with quadratic bezier
   initQuadraticControlPoints();

   glPointSize(5.0f);

   // run an event-triggered main loop
   while (!glfwWindowShouldClose(window))
   {
      // only draw if something changed
      if (needsRedraw_)
      {
         // clear screen to a dark grey colour
         glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT);

         // render lines
         if (pointVertices_.size())
         {
            if (!InitializeGeometry(&lineGeometry, pointVertices_, pointColours_))
               cout << "Program failed to initialize geometry!" << endl;

            RenderScene(&lineGeometry, &lineShader, GL_POINTS);
         }

         // render lines
         if (lineVertices_.size())
         {
            if (!InitializeGeometry(&lineGeometry, lineVertices_, lineColours_))
               cout << "Program failed to initialize geometry!" << endl;

            RenderScene(&lineGeometry, &lineShader, GL_LINES);
         }

         // render quadratic bezier curves
         if (quadraticVertices_.size())
         {
            if (!InitializeGeometry(&quadraticGeometry, quadraticVertices_, quadraticColours_))
               cout << "Program failed to initialize geometry!" << endl;

            glPatchParameteri(GL_PATCH_VERTICES, 3);
            RenderScene(&quadraticGeometry, &quadraticShader, GL_PATCHES);
         }

         // render cubic bezier curves
         if (cubicVertices_.size())
         {
            if (!InitializeGeometry(&cubicGeometry, cubicVertices_, cubicColours_))
               cout << "Program failed to initialize geometry!" << endl;

            glPatchParameteri(GL_PATCH_VERTICES, 4);
            RenderScene(&cubicGeometry, &cubicShader, GL_PATCHES);
         }

         needsRedraw_ = false;
      }

      if (isScrolling_ == true)
      {
         offset_ -= (0.03f * multiplier_);
         if (offset_ <= minOffset_)
         {
            offset_ = 1.1f;
         }

         initFont(extractor_, "The quick brown fox jumps over the lazy dog.", offset_);

         needsRedraw_ = true;
      }

      glfwSwapBuffers(window);

      glfwPollEvents();

      DestroyGeometry(&lineGeometry);
      DestroyGeometry(&quadraticGeometry);
      DestroyGeometry(&cubicGeometry);
   }

   // clean up allocated resources before exit
   DestroyGeometry(&lineGeometry);
   DestroyGeometry(&quadraticGeometry);
   DestroyGeometry(&cubicGeometry);
   DestroyShaders(&lineShader);
   DestroyShaders(&quadraticShader);
   DestroyShaders(&cubicShader);
   glfwDestroyWindow(window);
   glfwTerminate();

   cout << "Goodbye!" << endl;
   return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
   // query opengl version and renderer information
   string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
   string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
   string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

   cout << "OpenGL [ " << version << " ] "
      << "with GLSL [ " << glslver << " ] "
      << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
   bool error = false;
   for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
   {
      cout << "OpenGL ERROR:  ";
      switch (flag) {
      case GL_INVALID_ENUM:
         cout << "GL_INVALID_ENUM" << endl; break;
      case GL_INVALID_VALUE:
         cout << "GL_INVALID_VALUE" << endl; break;
      case GL_INVALID_OPERATION:
         cout << "GL_INVALID_OPERATION" << endl; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
         cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
      case GL_OUT_OF_MEMORY:
         cout << "GL_OUT_OF_MEMORY" << endl; break;
      default:
         cout << "[unknown error code]" << endl;
      }
      error = true;
   }
   return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
   string source;

   ifstream input(filename.c_str());
   if (input) {
      copy(istreambuf_iterator<char>(input),
         istreambuf_iterator<char>(),
         back_inserter(source));
      input.close();
   }
   else {
      cout << "ERROR: Could not load shader source from file "
         << filename << endl;
   }

   return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
   // allocate shader object name
   GLuint shaderObject = glCreateShader(shaderType);

   // try compiling the source as a shader of the given type
   const GLchar *source_ptr = source.c_str();
   glShaderSource(shaderObject, 1, &source_ptr, 0);
   glCompileShader(shaderObject);

   // retrieve compile status
   GLint status;
   glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
   if (status == GL_FALSE)
   {
      GLint length;
      glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
      string info(length, ' ');
      glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
      cout << "ERROR compiling shader:" << endl << endl;
      cout << source << endl;
      cout << info << endl;
   }

   return shaderObject;
}

// creates and returns a program object linked from vertex, tcs, tes and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint TCSshader, GLuint TESshader, GLuint fragmentShader)
{
   // allocate program object name
   GLuint programObject = glCreateProgram();

   // attach provided shader objects to this program
   if (vertexShader)   glAttachShader(programObject, vertexShader);
   if (TCSshader) glAttachShader(programObject, TCSshader);
   if (TESshader) glAttachShader(programObject, TESshader);
   if (fragmentShader) glAttachShader(programObject, fragmentShader);

   // try linking the program with given attachments
   glLinkProgram(programObject);

   // retrieve link status
   GLint status;
   glGetProgramiv(programObject, GL_LINK_STATUS, &status);
   if (status == GL_FALSE)
   {
      GLint length;
      glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
      string info(length, ' ');
      glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
      cout << "ERROR linking shader program:" << endl;
      cout << info << endl;
   }

   return programObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
   // allocate program object name
   GLuint programObject = glCreateProgram();

   // attach provided shader objects to this program
   if (vertexShader)   glAttachShader(programObject, vertexShader);
   if (fragmentShader) glAttachShader(programObject, fragmentShader);

   // try linking the program with given attachments
   glLinkProgram(programObject);

   // retrieve link status
   GLint status;
   glGetProgramiv(programObject, GL_LINK_STATUS, &status);
   if (status == GL_FALSE)
   {
      GLint length;
      glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
      string info(length, ' ');
      glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
      cout << "ERROR linking shader program:" << endl;
      cout << info << endl;
   }

   return programObject;
}