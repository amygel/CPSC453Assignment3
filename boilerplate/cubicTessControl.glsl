#version 410

layout(vertices = 4) out; //How long gl_out[] should be

in vec3 Colour[];
out vec3 teColour[];

void main()
{

    // gl_InvocationID tells you what input vertex you are working on
    if (gl_InvocationID == 0) {   // only needs to be set once
        gl_TessLevelOuter[0] = 1; // How many lines to draw
        gl_TessLevelOuter[1] = 30; // how much to subdivide each line
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;	// pass control points to TES
    teColour[gl_InvocationID] = Colour[gl_InvocationID]; 						// pass colours to TES
}