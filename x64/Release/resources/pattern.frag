#version 110

uniform sampler2D texture;
uniform sampler2D map;
uniform float progress;

void main()
{
    vec4 pixel = texture2D(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
    vec4 transition = texture2D(map,  vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
    vec4 color = gl_Color * pixel;

    if(progress >= transition.r) { 
      color = vec4(1,1,1,0);
    }

    gl_FragColor = color;
}
