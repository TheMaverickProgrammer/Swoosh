#pragma once
#include <Swoosh/Renderers/Renderer.h>
#include <Swoosh/Shaders.h>
#include <functional>
#include <array>

struct Fake3D : RenderSource {
  sf::Texture& normal;
  sf::Sprite& sprite;
  float z{};
  explicit Fake3D(sf::Sprite& src, sf::Texture& normal, float z = 0) : 
    RenderSource(src),
    normal(normal),
    sprite(src),
    z(z)
  {}
};

struct Light : RenderSource {
  float radius{};
  sf::Vector2f position{};
  sf::Color color{ sf::Color::White };
  sf::CircleShape circle{};
  explicit Light(float radius, sf::Vector2f position, sf::Color color) :
    RenderSource(circle),
    radius(radius), 
    position(position), 
    color(color)
  {
    circle.setPointCount(360);
    circle.setRadius(radius);
    circle.setFillColor(color);
    circle.setPosition(position);

    sf::FloatRect bounds = circle.getLocalBounds();
    circle.setOrigin(bounds.width/2, bounds.height/2);
  };
};

class CustomRenderer : public Renderer<Immediate, Fake3D, Light> {
  sf::RenderTexture diffuse, normal, light, out;
  swoosh::glsl::Deferred shader;
  std::list<RenderSource*> sources;
  std::list<RenderSource> memForward;
  std::list<Fake3D> mem3D;
  std::list<Light> memLight;

  std::size_t nextLightIdx{ 0 };
  std::array<sf::Glsl::Vec3, 30> lightPos{};
  std::array<sf::Glsl::Vec4, 30> lightColor{};
  std::array<float, 30> lightRadius{};

  sf::Vector2u viewPortSize;
public:
  CustomRenderer(const sf::Vector2u size) {
    viewPortSize = size;
    shader.setCamera(sf::Vector3f(size.x * 0.5f, size.y * 0.5f, 10.f));
    diffuse.create(size.x, size.y);
    normal.create(size.x, size.y);
    light.create(size.x, size.y);
    out.create(size.x, size.y);

    shader.setSurfaces(&diffuse, &normal, &light);
  }

  void draw() override {
    sf::RenderStates lightPass;
    lightPass.blendMode = sf::BlendAdd;

    for (RenderSource* source : sources) {
      if (Fake3D* ptr3D = dynamic_cast<Fake3D*>(source); ptr3D) {
        diffuse.draw(ptr3D->sprite);
        const sf::Texture* prev = ptr3D->sprite.getTexture();
        ptr3D->sprite.setTexture(ptr3D->normal);
        normal.draw(ptr3D->sprite);
        ptr3D->sprite.setTexture(*prev);
        continue;
      }

      if (Light* ptrLight = dynamic_cast<Light*>(source); ptrLight) {
        light.draw(ptrLight->drawable(), lightPass);
        continue;
      }

      out.draw(source->drawable(), source->states());
    }

    // compose the final scene
    shader.setLightCount((int)nextLightIdx);
    shader.setLightsArray(lightPos.data(), lightColor.data(), lightRadius.data(), 30);
    shader.apply(*this);

    // reset the light index
    nextLightIdx = 0;

    // clear the buffers
    sources.clear();
    memForward.clear();
    mem3D.clear();
    memLight.clear();
  }

  void display() override {
    out.display();
  }

  void clear(sf::Color color) override {
    diffuse.clear(sf::Color::Transparent);
    normal.clear(sf::Color::Transparent);
    light.clear(sf::Color::Transparent);
    out.clear(color);
  }

  void setView(const sf::View& view) override {
    diffuse.setView(view);
    normal.setView(view);
    light.setView(view);
    out.setView(view);
  }

  sf::Texture getTexture() override {
    return out.getTexture();
  }

  void onEvent(const RenderSource& event) override {
    memForward.emplace_back(event);
    sources.push_back(&memForward.back());
  }

  void onEvent(const Immediate& event) override {
    out.draw(event.drawable(), event.states());
  }

  void onEvent(const Fake3D& event) override {
    mem3D.emplace_back(event);
    sources.push_back(&mem3D.back());
  }

  void onEvent(const Light& event) override {
    const sf::Vector2f pos = event.position;
    memLight.emplace_back(event.radius, pos, event.color);
    sources.push_back(&memLight.back());

    std::size_t idx = std::min(nextLightIdx++, lightPos.size() - 1);
    lightPos[idx] = sf::Glsl::Vec3(pos.x/viewPortSize.x, pos.y/viewPortSize.y, 0.5f);
    sf::Glsl::Vec4 glsl_vec4 = sf::Glsl::Vec4(event.color);
    lightColor[idx] = glsl_vec4;

    float maxBrightness = std::max(std::max(glsl_vec4.x, glsl_vec4.y), glsl_vec4.z);
    float constant = 1.0;
    float linear = 0.7;
    float quadratic = 1.8;
    float factor = sqrt(linear * linear - 4.0 * quadratic * (constant - (256.0 / 5.0) * maxBrightness));
    float radius = (-linear + factor) / (2.0 * quadratic);
    lightRadius[idx] = radius;
  }
};