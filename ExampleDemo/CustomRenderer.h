#pragma once
#include <Swoosh/Renderers/Renderer.h>
#include <Swoosh/Shaders.h>
#include <functional>
#include <array>
#include <optional>

/**
  @class Draw3D
  @brief A render event with material data that can render 2D graphics as psuedo-3D
*/
struct Draw3D : RenderSource {
  glsl::deferred::MeshData data; // !< aggregate data for the 2D model

  explicit Draw3D(sf::Sprite* spr, 
    sf::Texture* normal, 
    sf::Texture* emissive = nullptr, 
    float metallic = 0, 
    float specular = 0) :
    RenderSource(spr),
    data({ spr, normal, emissive, metallic, specular })
  {}

  Draw3D WithZ(float z) {
    data.WithZ(z);
    return *this;
  }
};

/**
  @class Light
  @brief A render event with lighting data for rendering lights
  If used in a SimpleRenderer, will draw the light as a circle
*/
struct Light : RenderSource {
  float radius{};
  sf::Vector3f position{};
  sf::Color color{ sf::Color::White };
  sf::CircleShape circle{};
  float specular{};
  float cutoff{};

  explicit Light(float radius, 
    sf::Vector3f position, 
    sf::Color color, 
    float specular = 0.0f, 
    float cutoff = 0.0f) :
    RenderSource(&circle),
    radius(radius),
    position(position),
    color(color),
    specular(specular),
    cutoff(cutoff)
  {
    circle.setPointCount(360);
    circle.setRadius(radius);
    circle.setFillColor(color);
    circle.setPosition({ position.x, position.y });

    sf::FloatRect bounds = circle.getLocalBounds();
    circle.setOrigin(bounds.width/2, bounds.height/2);
  };
};

/**
  @brief Transform 2D vector into a 3D vector
  @param xy a 2D vector
  @param z a float representing the 3rd dimension
  @return sf::Vector3f with a z value
*/
sf::Vector3f WithZ(const sf::Vector2f xy, float z) {
  return sf::Vector3f(xy.x, xy.y, z);
}

/**
  @class CustomRenderer
  @brief A custom renderer example that uses swoosh shaders to compose a 3D scene
  Uses `Draw3D` render event as a tag to draw the sprite multiple times with different G-buffer texture values
  Uses `Light` render event as a tag to calculate the final lighting in the scene
  The end result is a partial implementation of a deferred renderer commonly used in advanced 3D applications
*/
class CustomRenderer : public Renderer<Draw3D, Light> {
  sf::RenderTexture position, diffuse, normal, esm, out;
  glsl::deferred::PositionPass positionShader;
  glsl::deferred::LightPass lightShader;
  glsl::deferred::EmissivePass emissiveShader;
  glsl::deferred::MeshPass meshShader;
  std::list<RenderSource> memForward;
  std::list<Draw3D> mem3D;
  std::list<Light> memLight;

public:
  CustomRenderer(const sf::View view) {
    const unsigned int ux = (unsigned int)view.getSize().x;
    const unsigned int uy = (unsigned int)view.getSize().y;
    const sf::Vector2u size = sf::Vector2u(ux, uy);

    position.create(size.x, size.y);
    diffuse.create(size.x, size.y);
    normal.create(size.x, size.y);
    esm.create(size.x, size.y);
    out.create(size.x, size.y);

    meshShader.configure(&diffuse, &normal, &esm);
    positionShader.configure(-100, 100, view, &position);
    lightShader.configure(view, &position, &diffuse, &normal, &esm);
    emissiveShader.configure(&diffuse, &esm);
  }

  void draw() override {
    mem3D.sort([](Draw3D& a, Draw3D& b) { return a.data.getPosition3D().z < b.data.getPosition3D().z; });
    
    for (Draw3D& source : mem3D) {
      
      // bake the currect normals
      meshShader.setMeshData(source.data);
      meshShader.apply(*this);

      // bake the position data
      positionShader.setMeshData(source.data);
      positionShader.apply(*this);
    }

    lightShader.clearLights();
    bool applyLight = memLight.size() > 0;
    for (Light& source : memLight) {
      lightShader.addLight(source.radius, 
      source.position, 
      source.color, 
      source.specular, 
      source.cutoff);
    }

    // render light geometry over the scene
    if(applyLight) {
      lightShader.apply(*this);
    }

    // draw emissive lighting
    emissiveShader.apply(*this);

    // draw forward rendered content
    for (RenderSource& source : memForward) {
      out.draw(*source.drawable());
    }

    // clear the buffer data
    memForward.clear();
    mem3D.clear();
    memLight.clear();
  }

  void clear(sf::Color color) override {
    // for G-buffers the clear color must always be transparent
    position.clear(sf::Color::Transparent);
    diffuse.clear(sf::Color::Transparent);
    normal.clear(sf::Color::Transparent);
    esm.clear(sf::Color::Transparent);
    out.clear(color);
  }

  void setView(const sf::View& view) override {
    position.setView(view);
    diffuse.setView(view);
    normal.setView(view);
    esm.setView(view);
    out.setView(view);
  }

  sf::RenderTexture& getRenderTextureTarget() override {
    return out;
  }

  void onEvent(const RenderSource& event) override {
    memForward.push_back(event);
  }

  void onEvent(const Draw3D& event) override {
    mem3D.emplace_back(event);
  }

  void onEvent(const Light& event) override {
    memLight.emplace_back(event);
  }
};