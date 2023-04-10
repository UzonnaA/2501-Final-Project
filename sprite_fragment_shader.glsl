// Source code of fragment shader
#version 130

// Attributes passed from the vertex shader
in vec4 color_interp;
in vec2 uv_interp;

// Texture sampler
uniform sampler2D onetex;
uniform float x = 1.0; //added uniform float variable

void main()
{
    // Sample texture
    

    vec4 color = texture2D(onetex, vec2(uv_interp.x * x, uv_interp.y * x)); // uniform float x can be set from game_object.cpp in my code when calling shader on a game_object

    // Assign color to fragment
    gl_FragColor = vec4(color.r, color.g, color.b, color.a);

    // Check for transparency
    if(color.a < 1.0)
    {
         discard;
    }
}
