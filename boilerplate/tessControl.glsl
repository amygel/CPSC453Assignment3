#version 410

uniform int uOuter0, uOuter1;

layout( vertices = 4 ) out;

void main( )
{
	gl_out[ gl_InvocationID ].gl_Position = gl_in[ gl_InvocationID ].gl_Position;
	gl_TessLevelOuter[0] = float( uOuter0 );
	gl_TessLevelOuter[1] = float( uOuter1 );
}