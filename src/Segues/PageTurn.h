
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

namespace {
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
}

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
          sf::Vector2f((float)i*cellSize      , (float)j*cellSize),
          sf::Vector2f((float)i*cellSize      , (float)(j + 1)*cellSize),
          sf::Vector2f((float)(i + 1)*cellSize, (float)(j + 1)*cellSize),
          sf::Vector2f((float)(i + 1)*cellSize, (float)j*cellSize)
        };


        sf::Vector2f tex[4] = {
          sf::Vector2f((i*cellSize) / (float)screenWidth, (j*cellSize) / (float)screenHeight),
          sf::Vector2f((i*cellSize) / (float)screenWidth, ((j + 1)*cellSize) / (float)screenHeight),
          sf::Vector2f(((i + 1)*cellSize) / (float)screenWidth, ((j + 1)*cellSize) / (float)screenHeight),
          sf::Vector2f(((i + 1)*cellSize) / (float)screenWidth, (j*cellSize) / (float)screenHeight)
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
    destination[(((cols - 1) * (rows)) * 6) + 5].color = sf::Color::Green;
    destination[total - 1].color = sf::Color::Red;

  }

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    // page turn
    float angle1 = ease::radians(90.0f);
    float angle2 = ease::radians(4.0f);
    float angle3 = ease::radians(4.0f);

    // length of cone
    float     A1 = -15.0f;
    float     A2 = 0.0f;
    float     A3 = 0.5f;

    // curl amount
    float theta1 = 10.5f;
    float theta2 = 15.0f;
    float theta3 = 10.01f;

    float dt;
    double theta = 90.f;
    double A = 0.0f;

    double rho = alpha * (ease::pi*0.5);

    dt = (float)alpha;
    theta = ease::interpolate(dt, angle1, angle2);
    A = ease::interpolate(dt, A1, A2);


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
    shader.loadFromMemory(::TURN_PAGE_VERT_SHADER, ::TURN_PAGE_FRAG_SHADER);
    auto size = getController().getWindow().getView().getSize();
    triangleStripulate((int)size.x, (int)size.y, paper, 10);
  }

  virtual ~PageTurn() { ; }
};