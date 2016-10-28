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

enum Font {Lora=0, SourceSansPro, GreatVibes};

// Global Variables
static bool isQuadratic_ = true;
static Font currFont = Lora;
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
    lineVertices_.clear();
    quadraticVertices_.clear();
    cubicVertices_.clear();

    // clear colours
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

    //second set
    quadraticVertices_.push_back(0.0f / scale);
    quadraticVertices_.push_back(-1.0f / scale);
    quadraticVertices_.push_back(-2.0f / scale);
    quadraticVertices_.push_back(-1.0f / scale);
    quadraticVertices_.push_back(-1.0f / scale);
    quadraticVertices_.push_back(1.0f / scale);

    // third set
    quadraticVertices_.push_back(-1.0f / scale);
    quadraticVertices_.push_back(1.0f / scale);
    quadraticVertices_.push_back(0.0f / scale);
    quadraticVertices_.push_back(1.0f / scale);
    quadraticVertices_.push_back(1.0f / scale);
    quadraticVertices_.push_back(1.0f / scale);

    //fourth set
    quadraticVertices_.push_back(1.2f / scale);
    quadraticVertices_.push_back(0.5f / scale);
    quadraticVertices_.push_back(2.5f / scale);
    quadraticVertices_.push_back(1.0f / scale);
    quadraticVertices_.push_back(1.3f / scale);
    quadraticVertices_.push_back(-0.4f / scale);

    for (unsigned int i = 0; i < quadraticVertices_.size() / 2; i++)
    {
        quadraticColours_.push_back(1.0f);
        quadraticColours_.push_back(0.0f);
        quadraticColours_.push_back(0.0f);
    }
}

void initCubicControlPoints()
{
    clearVectors();

    GLfloat scale = 9.0f;

    // first set
    cubicColours_.push_back(1.0f / scale);
    cubicColours_.push_back(1.0f / scale);
    cubicColours_.push_back(4.0f / scale);
    cubicColours_.push_back(0.0f / scale);
    cubicColours_.push_back(6.0f / scale);
    cubicColours_.push_back(2.0f / scale);
    cubicColours_.push_back(9.0f / scale);
    cubicColours_.push_back(1.0f / scale);

    // second set
    cubicColours_.push_back(8.0f / scale);
    cubicColours_.push_back(2.0f / scale);
    cubicColours_.push_back(0.0f / scale);
    cubicColours_.push_back(8.0f / scale);
    cubicColours_.push_back(0.0f / scale);
    cubicColours_.push_back(-2.0f / scale);
    cubicColours_.push_back(8.0f / scale);
    cubicColours_.push_back(4.0f / scale);

    // third set
    cubicColours_.push_back(5.0f / scale);
    cubicColours_.push_back(3.0f / scale);
    cubicColours_.push_back(3.0f / scale);
    cubicColours_.push_back(2.0f / scale);
    cubicColours_.push_back(3.0f / scale);
    cubicColours_.push_back(3.0f / scale);
    cubicColours_.push_back(5.0f / scale);
    cubicColours_.push_back(2.0f / scale);

    // fourth set
    cubicColours_.push_back(3.0f / scale);
    cubicColours_.push_back(2.2f / scale);
    cubicColours_.push_back(3.5f / scale);
    cubicColours_.push_back(2.7f / scale);
    cubicColours_.push_back(3.5f / scale);
    cubicColours_.push_back(3.3f / scale);
    cubicColours_.push_back(3.0f / scale);
    cubicColours_.push_back(3.8f / scale);

    // fifth set
    cubicColours_.push_back(2.8f / scale);
    cubicColours_.push_back(3.5f / scale);
    cubicColours_.push_back(2.4f / scale);
    cubicColours_.push_back(3.8f / scale);
    cubicColours_.push_back(2.4f / scale);
    cubicColours_.push_back(3.2f / scale);
    cubicColours_.push_back(2.8f / scale);
    cubicColours_.push_back(3.5f / scale);

    for (unsigned int i = 0; i < cubicVertices_.size() / 2; i++)
    {
        cubicColours_.push_back(1.0f);
        cubicColours_.push_back(0.0f);
        cubicColours_.push_back(0.0f);
    }
}

void initFont(string filename)
{
    clearVectors();

    //Load a font file and extract a glyph
    GlyphExtractor extractor;
    extractor.LoadFontFile(filename);

    // Load a character
    MyGlyph glyph = extractor.ExtractGlyph('a');
   
    // load the contours
    for each(MyContour contour in glyph.contours)
    {
        for each(MySegment seg in contour)
        {
            if (seg.degree == 1)
            {
                for (int i = 0; i < 2; i++)
                {
                    lineVertices_.push_back(seg.x[i]);
                    lineVertices_.push_back(seg.y[i]);
                }
            }
            else if (seg.degree == 2)
            {
                for (int i = 0; i < 3; i++)
                {
                    quadraticVertices_.push_back(seg.x[i]);
                    quadraticVertices_.push_back(seg.y[i]);
                }
            }
            else if (seg.degree == 3)
            {
                for (int i = 0; i < 4; i++)
                {
                    cubicVertices_.push_back(seg.x[i]);
                    cubicVertices_.push_back(seg.y[i]);
                }
            }
        }
    }
   
    // init line colours
    for (unsigned int i = 0; i < lineVertices_.size() / 2; i++)
    {
        lineColours_.push_back(1.0f);
        lineColours_.push_back(0.0f);
        lineColours_.push_back(0.0f);
    }

    // init quadratic colours
    for (unsigned int i = 0; i < quadraticVertices_.size() / 2; i++)
    {
        quadraticColours_.push_back(0.0f);
        quadraticColours_.push_back(1.0f);
        quadraticColours_.push_back(0.0f);
    }

    // init cubic colours
    for (unsigned int i = 0; i < cubicVertices_.size() / 2; i++)
    {
        cubicColours_.push_back(0.0f);
        cubicColours_.push_back(0.0f);
        cubicColours_.push_back(1.0f);
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
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        if (currFont == Lora)
        {
            initFont("fonts/lora/Lora-Regular.ttf");
        }
        else if (currFont == SourceSansPro)
        {
            initFont("fonts/source-sans-pro/SourceSansPro-Regular.otf");
        }
        else if (currFont == GreatVibes)
        {
            initFont("fonts/great-vibes/GreatVibes-Regular.otf");
        }

        currFont = static_cast<Font>((currFont + 1) % 3);
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

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        // clear screen to a dark grey colour
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

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