#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>
#include <Swoosh\Game.h>
#include <Swoosh\EmbedGLSL.h>

using namespace swoosh;

/*
This segue was written from the 2004 paper titled

"Deforming Pages of 3D Electronic Books"
By Lichan Hong, Stuart K. Card, and Jindong (JD) ChenPalo Alto Research Center

Original paper:
https://www.scribd.com/document/260579490/Hong-DeformingPages-algorithm
----

The concept is clever. Make an invisible cone with a large radius of size theta.
The y axis is the contact point of the 3D cone and the 2D surface.
Project the 2D point from paper to the other side of the cone (2D -> 3D).
OVer time, shrink the radius of the cone to 0. The projection will be flat and to the side.
This model is perfect for page turning.

With minor adjustment to the invisible cone's size, shape, and projection point;
we can emulate a more realistic page turning segue.
*/

auto TURN_PAGE_VERT_SHADER = GLSL
(
  110, 
  uniform float theta;
  uniform float rho;
  uniform float A;

  void main()
  {
    float Z, r, beta;
    vec3  v1;
    vec4  position = gl_Vertex;

    // Radius of the circle circumscribed by vertex (vi.x, vi.y) around A on the x-y plane
    Z = sqrt(position.x * position.x + pow(position.y - A, 2.0));
    // Now get the radius of the cone cross section intersected by our vertex in 3D space.
    r = Z * sin(theta);
    // Angle subtended by arc |ST| on the cone cross section.
    beta = asin(position.x / Z) / sin(theta);

    //project the vertex onto the cone
    v1.x = r * sin(beta);
    v1.y = Z + A - r * (1.0 - cos(beta)) * sin(theta);
    v1.z = r * (1.0 - cos(beta)) * cos(theta);

    position.x = (v1.x * cos(rho) - v1.z * sin(rho));
    position.y = v1.y;
    position.z = (v1.x * sin(rho) + v1.z * cos(rho));


    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(position.xy, 0.0, 1.0);

    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
  }

);

auto TURN_PAGE_FRAG_SHADER = GLSL
(
  110,
  uniform sampler2D texture;

  void main()
  {
    vec4 pixel = texture2D(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
    gl_FragColor = gl_Color * pixel;
  }
);

class PageTurn : public Segue {
private:
  sf::Texture* temp;
  sf::Texture* pattern;
  sf::Shader shader;
  sf::VertexArray paper;

  // More cells means higher quality effect at the cost of more work for cpu and gpu
  // Bigger cell size = less cells fit
  // Smaller cell size = more cells fit
  void triangleStripulate(int screenWidth, int screenHeight, sf::VertexArray& destination, int cellSize) {
    destination.clear();

    int cols = screenWidth / cellSize;
    int rows = screenHeight / cellSize;

    // each grid has 2 triangles which have 3 points (1 point = 1 vertex)
    int total = cols * rows * 2 * 3;
    destination = sf::VertexArray(sf::PrimitiveType::Triangles, total);

    int index = 0;
    for (int i = 0; i < cols; i++) {
      for (int j = 0; j < rows; j++) {
        sf::Vertex vertex;
        vertex.color = sf::Color::White;

        sf::Vector2f pos[4] = { 
          sf::Vector2f(i*cellSize      , j*cellSize),
          sf::Vector2f(i*cellSize      , (j + 1)*cellSize),
          sf::Vector2f((i + 1)*cellSize, (j + 1)*cellSize),
          sf::Vector2f((i + 1)*cellSize, j*cellSize) 
        };


        sf::Vector2f tex[4] = {
          sf::Vector2f((i*cellSize) /(float)screenWidth, (j*cellSize)/(float)screenHeight),
          sf::Vector2f((i*cellSize) / (float)screenWidth, ((j + 1)*cellSize)/(float)screenHeight),
          sf::Vector2f(((i + 1)*cellSize)/(float)screenWidth, ((j + 1)*cellSize)/(float)screenHeight),
          sf::Vector2f(((i + 1)*cellSize)/(float)screenWidth, (j*cellSize)/(float)screenHeight)
        };

        // ccw
        int order[6] = { 0, 2, 1, 0, 3, 2 };

        for (auto o : order) {
          vertex.position = pos[o];
          vertex.texCoords = tex[o];
          destination[index++] = vertex;
        }
      }
    }

    destination[0].color = sf::Color::Blue;
    destination[(((cols-1) * (rows)) * 6) + 5].color = sf::Color::Green;
    destination[total-1].color = sf::Color::Red;

  }

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    float angle1 = ease::radians(90.0f);
    float angle2 = ease::radians(45.0f);
    float angle3 = ease::radians(4.0f);
    float     A1 = -15.0f;
    float     A2 = -1.5f;
    float     A3 = -50.5f;
    float theta1 = 0.05f;
    float theta2 = 0.5f;
    float theta3 = 10.0f;
    float theta4 = 2.0f;

    float f1, f2, dt;
    double theta = 90.f;
    double A = 0.0f;

    double rho = alpha * ease::pi;

    if (alpha <= 0.15)
    {
      // Start off with a flat page with no deformation at the beginning of a page turn, then begin to curl the page gradually
      // as the hand lifts it off the surface of the book.
      dt = alpha / 0.15;
      f1 = sin(ease::pi * pow(dt, theta1) / 2.0);
      f2 = sin(ease::pi * pow(dt, theta2) / 2.0);
      theta = ease::interpolate(f1, angle1, angle2);
      A = ease::interpolate(f2, A1, A2);
    }
    else if (alpha <= 0.84)
    {
      // Produce the most pronounced curling near the middle of the turn. Here small values of theta and A
      // result in a short, fat cone that distinctly show the curl effect.
      dt = (alpha - 0.15) / 0.25;
      theta = ease::interpolate(dt, angle2, angle3);
      A = ease::interpolate(dt, A2, A3);
    }
    else if (alpha <= 1.0)
    {
      // Near the middle of the turn, the hand has released the page so it can return to its normal form.
      // Ease out the curl until it returns to a flat page at the completion of the turn. More advanced simulations
      // could apply a slight wobble to the page as it falls down like in real life.
      dt = (alpha - 0.4) / 0.6;
      f1 = sin(ease::pi * pow(dt, theta3) / 2.0);
      f2 = sin(ease::pi * pow(dt, theta4) / 2.0);
      theta = ease::interpolate(f1, angle3, angle1);
      A = ease::interpolate(f2, A3, A1);
    }

    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    shader.setUniform("A", (float)A);
    shader.setUniform("theta", (float)theta);
    shader.setUniform("rho", (float)rho);
    shader.setUniform("texture", *temp);

    sf::RenderStates states;
    states.shader = &shader;

    surface.clear(sf::Color::Transparent);
    surface.draw(paper, states);
    
    surface.display();

    sf::Texture* temp2 = new sf::Texture(surface.getTexture()); // Make a copy of the source texture
    sf::Sprite left(*temp2);

    surface.clear(sf::Color::Transparent);

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    getController().getWindow().draw(right);
    getController().getWindow().draw(left);

    delete temp2;
    surface.clear(sf::Color::Transparent);
  }

  PageTurn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(TURN_PAGE_VERT_SHADER, TURN_PAGE_FRAG_SHADER);
    auto size = getController().getWindow().getView().getSize();
    triangleStripulate(size.x, size.y, paper, 10);
  }

  virtual ~PageTurn() { ; }
};
