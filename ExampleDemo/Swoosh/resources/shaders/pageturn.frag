#version 110

uniform sampler2D texture;

void main()
 
{ 
    vec4 pixel = texture2D(texture, vec2(gl_TexCoord[0].x, 1.0-gl_TexCoord[0].y));
    gl_FragColor = gl_Color * pixel;
}


