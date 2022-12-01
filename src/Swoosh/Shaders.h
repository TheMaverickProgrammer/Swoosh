#pragma once
#include <Swoosh/Ease.h>
#include <Swoosh/Renderers/Renderer.h>
#include <Swoosh/EmbedGLSL.h>
#include <SFML/Graphics.hpp>
#include <cassert>

/*
All of the pre-defined transition effects use common shaders
that can be re-used elsewhere by Swoosh devs

Custom made or custom implemented by TheMaverickProgrammer 
*/

namespace swoosh {
  namespace glsl {
    
    /**
      @class Shader
      @brief Base class for all swoosh-provided shaders
    */
    class Shader {
    protected:
      sf::Shader shader;

    public:
      const sf::Shader& getShader() const { return shader; }
      virtual ~Shader() { ; }

      virtual void apply(IRenderer& renderer) = 0;
    };
    /**
      @class FastGaussianBlur
      @brief Incredibly fast pass implementation of the Gaussian blur effect
      Results are blockier than other shaders but performance is undeniable
    */
    class FastGaussianBlur final : public Shader {
    private:
      std::string FAST_BLUR_SHADER;
      sf::Texture* texture{ nullptr };
      float power{};
    public:
      void setPower(float power) { this->power = power; shader.setUniform("power", power); }

      void setTexture(sf::Texture* tex) { 
        if (!tex) return;

        this->texture = tex; 

        shader.setUniform("texture", *texture);
        shader.setUniform("textureSizeW", (float)texture->getSize().x);
        shader.setUniform("textureSizeH", (float)texture->getSize().y);
      }

      void apply(IRenderer& renderer) override {
        if (!texture) return;

        sf::RenderStates states;
        states.shader = &shader;

        sf::Sprite sprite;
        sprite.setTexture(*texture);

        renderer.submit(Immediate(&sprite, states));
      }

      FastGaussianBlur(int numOfKernels) {
        texture = nullptr;
        power = 0.0f;

        this->FAST_BLUR_SHADER = GLSL
        (
          110,
          uniform sampler2D texture;
          uniform int kernels;
          uniform float power;
          uniform float textureSizeW;
          uniform float textureSizeH;

          float normpdf(float x, float sigma)
          {
            return 0.39894*exp(-0.5*x*x / (sigma*sigma)) / sigma;
          }

          void main()
          {
            vec3 c = texture2D(texture, gl_TexCoord[0].xy).rgb;

            const int mSize = %kernels%;
            const int kSize = int((float(mSize) - 1.0) / 2.0);
            float kernel[mSize];
            vec3 final_color = vec3(0.0);

            // Create the kernel
            // Increase sigma per 10 multiples of power; this emulates a more powerful blur
            // At no additional cost
            float sigma = 1.0 + power;
            float Z = 0.0;
            for (int j = 0; j <= kSize; ++j)
            {
              kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), sigma);
            }

            //get the normalization factor (as the gaussian has been clamped)
            for (int j = 0; j < mSize; ++j)
            {
              Z += kernel[j];
            }

            //read out the texels
            for (int i = -kSize; i <= kSize; ++i)
            {
              for (int j = -kSize; j <= kSize; ++j)
              {
                final_color += kernel[kSize + j] * kernel[kSize + i] * texture2D(texture, (gl_TexCoord[0].xy + (vec2(float(i), float(j)) / vec2(textureSizeW, textureSizeH)))).rgb;
              }
            }

            gl_FragColor = vec4(final_color / (Z*Z), 1.0);
          }
        );

        std::string from("%kernels%");
        std::string to = std::to_string(numOfKernels);

        size_t start_pos = this->FAST_BLUR_SHADER.find(from);
        if (start_pos != std::string::npos) {
          this->FAST_BLUR_SHADER.replace(start_pos, from.length(), to);
          shader.loadFromMemory(this->FAST_BLUR_SHADER, sf::Shader::Fragment);
        }
        else {
          // should never happen
          assert(true && "could not find string %kernels% in guassian shader string");
        }
      }

      ~FastGaussianBlur() { }
    };

    /**
      @class Checkerboard
      @brief Blocks out texture1 with pieces of texture2 defined by the cols x rows grid over time.

      Smoothness factor can make the effect look softer and less blocky
    */
    class Checkerboard final : public Shader {
    private:
      std::string CHECKERBOARD_SHADER;
      float alpha{};
      int cols{}, rows{};
      float smoothness{};
      sf::Texture* texture1{ nullptr }, * texture2{ nullptr };

    public:
      void setAlpha(float alpha) { this->alpha = alpha; shader.setUniform("progress", (float)alpha); }
      void setCols(int cols) { this->cols = cols;       shader.setUniform("cols", cols); }
      void setRows(int rows) { this->rows = rows;       shader.setUniform("rows", rows); }
      void setSmoothness(float smoothness) { this->smoothness = smoothness;         shader.setUniform("smoothness", smoothness);  }
      void setTexture1(sf::Texture* tex) { if (!tex) return;  this->texture1 = tex; shader.setUniform("texture",  *texture1); }
      void setTexture2(sf::Texture* tex) { if (!tex) return;  this->texture2 = tex; shader.setUniform("texture2", *texture2); }

      void apply(IRenderer& renderer) override {
        if (!(texture1 && texture2)) return;

        sf::RenderStates states;
        states.shader = &shader;

        sf::Sprite sprite;
        sprite.setTexture(*texture1);

        renderer.submit(Immediate(&sprite, states));
      }

      Checkerboard(int cols = 10, int rows = 10) {
        this->cols = cols;
        this->rows = rows;
        this->alpha = 0;
        this->smoothness = 0.0f;
        this->texture1 = this->texture2 = nullptr;

        this->CHECKERBOARD_SHADER = GLSL(
          110,
          uniform sampler2D texture;
          uniform sampler2D texture2;
          uniform float progress;
          uniform float smoothness;
          uniform int cols;
          uniform int rows;

          float rand(vec2 co) {
            return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
          }

          void main() {
            vec2 p = gl_TexCoord[0].xy;
            vec2 size = vec2(cols, rows);
            float r = rand(floor(vec2(size) * p));
            float m = smoothstep(0.0, -smoothness, r - (progress * (1.0 + smoothness)));
            gl_FragColor = mix(texture2D(texture, p.xy), texture2D(texture2, p.xy), m);
          }
        );

        shader.loadFromMemory(this->CHECKERBOARD_SHADER, sf::Shader::Fragment);
      }

      ~Checkerboard() { ; }
    };

    /**
      @class CircleMask
      @brief Hides the input texture that exceeds the circle's radius. Radius increases as alpha approaches 1.0
    */
    class CircleMask final : public Shader {
    private:
      std::string CIRCLE_MASK_SHADER;
      sf::Texture* texture{ nullptr };
      float alpha{};
      float aspectRatio{};

    public:
      void setAlpha(float alpha) { this->alpha = alpha; shader.setUniform("time", (float)alpha); }
      void setAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio;  shader.setUniform("ratio", aspectRatio); }
      void setTexture(sf::Texture* tex) { if (!tex) return; this->texture = tex; shader.setUniform("texture", *texture); }

      void apply(IRenderer& renderer) override {
        if (!texture) return;

        sf::RenderStates states;
        states.shader = &shader;

        sf::Sprite sprite;
        sprite.setTexture(*texture);

        renderer.submit(Immediate(&sprite, states));
      }

      CircleMask() {
        this->CIRCLE_MASK_SHADER = GLSL(
          110,
          uniform sampler2D texture;
          uniform float ratio;
          uniform float time;

          void main() {
            vec2 pos = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);

            float size = time;

            vec4 outcol = vec4(0.0);

            float x = pos.x - 0.5;
            float y = pos.y - 0.5;

            if (ratio >= 1.0) {
              x *= ratio;
            }
            else {
              y *= 1.0 / ratio;
            }

            if (x*x + y * y < size*size) {
              outcol = texture2D(texture, gl_TexCoord[0].xy);
            }

            gl_FragColor = outcol;
          }
        );

        texture = nullptr;
        alpha = 0;
        shader.loadFromMemory(this->CIRCLE_MASK_SHADER, sf::Shader::Fragment);
      }

      ~CircleMask() { ; }
    };

    /**
      @class RetroBlit
      @brief Strips color away as layers as alpha approaches 1.0
    */
    class RetroBlit final : public Shader {
    private:
      std::string RETRO_BLIT_SHADER;
      float alpha{};
      sf::Texture* texture{ nullptr };

    public:
      void setTexture(sf::Texture* tex) { if (!tex) return; texture = tex; shader.setUniform("texture", *texture); }
      void setAlpha(float alpha) { this->alpha = alpha; shader.setUniform("progress", alpha); }

      void apply(IRenderer& renderer) override {
        if (!texture) return;

        sf::RenderStates states;
        states.shader = &shader;

        sf::Sprite sprite;
        sprite.setTexture(*texture);

        renderer.submit(Immediate(&sprite, states));
      }

      RetroBlit() {
        this->RETRO_BLIT_SHADER = GLSL(
          110,
          uniform sampler2D texture;
          uniform float progress;

          vec4 channelBitrate(vec4 inColor, int bit) { 
            float bitDepth = pow(2.0, abs(float(bit))); 
            vec4 outColor = floor(inColor * bitDepth) / bitDepth; 
            return outColor; 
          }

          void main() {
            vec2 p = gl_TexCoord[0].xy;
            vec4 color = texture2D(texture, p.xy);
            gl_FragColor = channelBitrate(color,int(8.0*progress));
          }
        );

        shader.loadFromMemory(this->RETRO_BLIT_SHADER, sf::Shader::Fragment);

        texture = nullptr;
        alpha = 0;
      }

      ~RetroBlit() { ; }
    };

    /**
      @class CrossZoom 
      @brief Mimics the blinding light-shearing effect from texture1 to texture2
      @warning CPU intensive process with SFML at this time
    */
    class CrossZoom final : public Shader {
    private:
      std::string CROSS_ZOOM_SHADER;
      sf::Texture* texture1{ nullptr }, * texture2{ nullptr };
      float power{};
      float alpha{};

    public:
      void setTexture1(sf::Texture* tex) { if (!tex) return; texture1 = tex; shader.setUniform("texture", *texture1); }
      void setTexture2(sf::Texture* tex) { if (!tex) return; texture2 = tex; shader.setUniform("texture2", *texture2); }
      void setAlpha(float alpha) { this->alpha = alpha; shader.setUniform("progress", (float)alpha); }
      void setPower(float power) { this->power = power; shader.setUniform("strength", power); }

      void apply(IRenderer& renderer) override {
        if (!(texture1 && texture2)) return;

        sf::RenderStates states;
        states.shader = &shader;

        sf::Sprite sprite;
        sprite.setTexture(*texture1);

        renderer.submit(Immediate(&sprite, states));
      }

      CrossZoom() {
        texture1 = texture2 = nullptr;
        alpha = 0;

        // Modified by TheMaverickProgrammer slightly to support GLSL 1.10
        this->CROSS_ZOOM_SHADER = GLSL(
            110,
            uniform sampler2D texture;
            uniform sampler2D texture2;
            uniform float progress;

            // License: MIT
            // Author: rectalogic
            // ported by gre from https://gist.github.com/rectalogic/b86b90161503a0023231

            // Converted from https://github.com/rectalogic/rendermix-basic-effects/blob/master/assets/com/rendermix/CrossZoom/CrossZoom.frag
            // Which is based on https://github.com/evanw/glfx.js/blob/master/src/filters/blur/zoomblur.js
            // With additional easing functions from https://github.com/rectalogic/rendermix-basic-effects/blob/master/assets/com/rendermix/Easing/Easing.glsllib

            uniform float strength;

            const float PI = 3.141592653589793;

            float Linear_ease(in float begin, in float change, in float duration, in float time) {
              return change * time / duration + begin;
            }

            float Exponential_easeInOut(in float begin, in float change, in float duration, in float time) {
              if (time == 0.0)
                return begin;
              else if (time == duration)
                return begin + change;
              time = time / (duration / 2.0);
              if (time < 1.0)
                return change / 2.0 * pow(2.0, 10.0 * (time - 1.0)) + begin;
              return change / 2.0 * (-pow(2.0, -10.0 * (time - 1.0)) + 2.0) + begin;
            }

            float Sinusoidal_easeInOut(in float begin, in float change, in float duration, in float time) {
              return -change / 2.0 * (cos(PI * time / duration) - 1.0) + begin;
            }

            float rand(vec2 co) {
              return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
            }

            vec3 crossFade(in vec2 uv, in float dissolve) {
              return mix(texture2D(texture, uv).rgb, texture2D(texture2, uv).rgb, dissolve);
            }

            void main() {
              vec2 uv = gl_TexCoord[0].xy;

              vec2 texCoord = uv.xy / vec2(1.0).xy;

              // Linear interpolate center across center half of the image
              vec2 center = vec2(Linear_ease(0.25, 0.5, 1.0, progress), 0.5);
              float dissolve = Exponential_easeInOut(0.0, 1.0, 1.0, progress);

              // Mirrored sinusoidal loop. 0->strength then strength->0
              float strength = Sinusoidal_easeInOut(0.0, strength, 0.5, progress);

              vec3 color = vec3(0.0);
              float total = 0.0;
              vec2 toCenter = center - texCoord;

              /* randomize the lookup values to hide the fixed number of samples */
              float offset = rand(uv);

              for (float t = 0.0; t <= 40.0; t++) {
                float percent = (t + offset) / 40.0;
                float weight = 4.0 * (percent - percent * percent);
                color += crossFade(texCoord + toCenter * percent * strength, dissolve) * weight;
                total += weight;
              }

              gl_FragColor = vec4(color / total, 1.0);
            }
        );

        shader.loadFromMemory(this->CROSS_ZOOM_SHADER, sf::Shader::Fragment);
      }

      ~CrossZoom() { }
    };

    /**
      @class Morph
      @brief Transforms texture1 to texture2 by some alpha value. Strength intensifies the morph effect.
    */
    class Morph final : public Shader {
    private:
      std::string MORPH_SHADER;
      sf::Texture* texture1{ nullptr }, * texture2{ nullptr };
      float strength{};
      float alpha{};
    public:

      void setTexture1(sf::Texture* tex) { if (!tex) return; texture1 = tex; shader.setUniform("texture", *texture1); }
      void setTexture2(sf::Texture* tex) { if (!tex) return; texture2 = tex; shader.setUniform("texture2", *texture2); }
      void setAlpha(float alpha) { this->alpha = alpha; shader.setUniform("alpha", (float)alpha); }
      void setStrength(float strength) { this->strength = strength; shader.setUniform("strength", strength); }

      void apply(IRenderer& renderer) override {
        if (!(texture1 && texture2)) return;

        sf::RenderStates states;
        states.shader = &shader;

        sf::Sprite sprite;
        sprite.setTexture(*texture1);

        renderer.submit(Immediate(&sprite, states));
      }

      Morph() {
        this->MORPH_SHADER = GLSL(
          110,
          uniform sampler2D texture;
          uniform sampler2D texture2;
          uniform float strength;
          uniform float alpha;

          void main() {
            vec2 pos = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);

            vec4 ca = texture2D(texture, pos.xy);
            vec4 cb = texture2D(texture2, pos.xy);

            vec2 oa = (((ca.rg + ca.b)*0.5)*2.0 - 1.0);
            vec2 ob = (((cb.rg + cb.b)*0.5)*2.0 - 1.0);
            vec2 oc = mix(oa, ob, 0.5)*strength;

            float w0 = alpha;
            float w1 = 1.0 - w0;

            gl_FragColor = mix(texture2D(texture, pos + oc * w0), texture2D(texture2, pos - oc * w1), alpha);
          }
        );

        shader.loadFromMemory(this->MORPH_SHADER, sf::Shader::Fragment);

        texture1 = texture2 = nullptr;
        alpha = strength = 0;
      }

      ~Morph() { ; }
    };

    /*
    This shader effect was written from the 2004 paper titled
    "Deforming Pages of 3D Electronic Books"
    By Lichan Hong, Stuart K. Card, and Jindong (JD) ChenPalo Alto Research Center
    Implemented by Maverick Peppers for Swoosh

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
    class PageTurn final : public Shader {
    private:
      sf::Texture* texture{ nullptr };
      sf::Vector2u size;
      float alpha{};

      std::string TURN_PAGE_VERT_SHADER, TURN_PAGE_FRAG_SHADER;
      sf::VertexArray buffer;

      // More cells means higher quality effect at the cost of more work for cpu and gpu
      // Bigger cell size = less cells fit, less smooth, higher performance
      // Smaller cell size = more cells fit, smooth, slower performance
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

      void setTexture(sf::Texture* tex) { if (!tex) return;  this->texture = tex; shader.setUniform("texture", *texture); }

      void setAlpha(float alpha) {
        this->alpha = alpha; 

        /*
        these are hard-coded values that make the effect look natural
        */

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

        shader.setUniform("A", (float)A);
        shader.setUniform("theta", (float)theta);
        shader.setUniform("rho", (float)rho);
      }

      void apply(IRenderer& renderer) override {
        if (!(this->texture)) return;

        sf::RenderStates states;
        states.shader = &shader;

        renderer.clear(sf::Color::Transparent);
        renderer.submit(Immediate(&buffer, states));
      }

      PageTurn(sf::Vector2u size, const int cellSize = 10) {
        alpha = 0;
        texture = nullptr;
        this->size = size;

        this->TURN_PAGE_VERT_SHADER = GLSL
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

        this->TURN_PAGE_FRAG_SHADER = GLSL
        (
          110,
          uniform sampler2D texture;

          void main()
          {
            vec4 pixel = texture2D(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
            gl_FragColor = gl_Color * pixel;
          }
        );

        shader.loadFromMemory(this->TURN_PAGE_VERT_SHADER, this->TURN_PAGE_FRAG_SHADER);
        triangleStripulate((int)size.x, (int)size.y, buffer, cellSize);
      }

      ~PageTurn() {}
    };

    /**
      @class Pixelate
      @brief Adds a retro blur effect that reduces the input texture into blocky pixels
    */
    class Pixelate final : public Shader {
    private:
      std::string PIXELATE_SHADER;
      sf::Texture* texture{ nullptr };
      float threshold{};

    public:
      void apply(IRenderer& renderer) override {
        if (!this->texture) return;
        
        sf::RenderStates states;
        states.shader = &shader;

        sf::Sprite sprite;
        sprite.setTexture(*this->texture);

        renderer.submit(Immediate(&sprite, states));
      }

      void setTexture(sf::Texture* tex) { if (!tex) return; this->texture = tex; shader.setUniform("texture", *this->texture); }
      void setThreshold(float t) { this->threshold = t; shader.setUniform("pixel_threshold", threshold); }

      Pixelate() {
        threshold = 0;
        texture = nullptr;
        this->PIXELATE_SHADER = GLSL
        (
          110,
          uniform sampler2D texture;
          uniform float pixel_threshold;

          void main()
          {
            float factor = 1.0 / (pixel_threshold + 0.001);
            vec2 pos = floor(gl_TexCoord[0].xy * factor + 0.5) / factor;
            gl_FragColor = texture2D(texture, pos) * gl_Color;
          }
        );

        shader.loadFromMemory(this->PIXELATE_SHADER, sf::Shader::Fragment);
      }

      ~Pixelate() {}
    };

    /**
      @class RadialCCW
      @brief Masks the input texture1 with input texture2 in a clock-wise fashion
    */
    class RadialCCW final : public Shader {
    private:
      std::string RADIAL_CCW_SHADER;
      sf::Texture* texture1{ nullptr };
      sf::Texture* texture2{ nullptr };
      float alpha{};

    public:
      void apply(IRenderer& renderer) override {
        if (!(this->texture1 && this->texture2)) return;

        sf::RenderStates states;
        states.shader = &shader;

        sf::Sprite sprite;
        sprite.setTexture(*texture1);

        renderer.submit(Immediate(&sprite, states));
      }

      void setTexture1(sf::Texture* tex) { if (!tex) return; this->texture1 = tex; shader.setUniform("texture", *texture1); }
      void setTexture2(sf::Texture* tex) { if (!tex) return; this->texture2 = tex; shader.setUniform("texture2", *texture2);}
      void setAlpha(float alpha) { this->alpha = alpha; shader.setUniform("time", (float)alpha); }

      RadialCCW() {
        alpha = 0;
        texture1 = texture2 = nullptr;
        
        RADIAL_CCW_SHADER = GLSL(
          110,
          uniform sampler2D texture;
          uniform sampler2D texture2;
          uniform float time;

          void main() {
            vec2 pos = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);

            const float PI = 3.141592653589;

            vec2 rp = pos * 2.0 - 1.0;
            gl_FragColor = mix(
              texture2D(texture, pos),
              texture2D(texture2, pos),
              smoothstep(0.0, 0.0, atan(rp.y, rp.x) - (1.0 - time - .5) * PI * 2.5)
            );
          }
        );

        shader.loadFromMemory(this->RADIAL_CCW_SHADER, sf::Shader::Fragment);
      }

      ~RadialCCW() { ; }
    };

    /**
      @namespace deferred
      @brief Using extra input textures, light parameters, and destination textures: draws a scene with 3D lighting
    */
    namespace deferred {
      struct MeshData {
        sf::Sprite* sprite{ nullptr };
        sf::Texture* normal{ nullptr };
        sf::Texture* esm{ nullptr }; // Emissive (R), Specular (G), Metallic (B)
        float metallic{};
        float specular{};
        float z{};

        MeshData WithZ(float z) {
          this->z = z;
          return *this;
        }

        sf::Glsl::Vec3 getPosition3D() {
          sf::Vector2f pos2D = sprite->getPosition();
          return sf::Glsl::Vec3(pos2D.x, pos2D.y, z);
        }
      };

      class MeshPass final : public Shader {
      private:
        std::string SHADER_FRAG;
        sf::RenderTexture* diffuse{ nullptr }, * normal{ nullptr }, * esm{ nullptr };
        MeshData meshData;

      public:
        void apply(IRenderer& renderer) override {
          if (!(diffuse && normal && esm && meshData.sprite)) return;

          sf::RenderStates states;
          states.shader = &shader;

          sf::Sprite& sprite = *meshData.sprite;
          shader.setUniform("normal", sf::Shader::CurrentTexture);
          shader.setUniform("rotation", sprite.getRotation());

          // draw the albedo/diffuse pass (normal sprite)
          diffuse->draw(sprite);

          // next, draw the normals

          // store the original texture in temp
          const sf::Texture* temp = sprite.getTexture();
          // use the normal texture and draw
          sprite.setTexture(*meshData.normal);

          normal->draw(sprite, states);

          // next, draw the esm
          if (meshData.esm) {
            // use the emissive texture and draw
            sprite.setTexture(*meshData.esm);
            esm->draw(sprite);
          }

          // restore original texture
          sprite.setTexture(*temp);
        }

        void configure(sf::RenderTexture* diffuseIn, sf::RenderTexture* normalIn, sf::RenderTexture* esmIn) {
          diffuse = diffuseIn;
          normal = normalIn;
          esm = esmIn;
        }

        void setMeshData(const MeshData& data) {
          meshData = data;
        }

        MeshPass() {
          /**
            fragment shader
          **/
          SHADER_FRAG = GLSL(
            110,
            uniform sampler2D normal;
            uniform float rotation;

            // rotates in 2D only
            vec4 rotate(vec4 v, float angle) {
              float r = radians(angle);
              return vec4(cos(r) * v.x, sin(r) * v.y, v.zw);
            }

            void main()
            {
              vec4 pxNormal = texture2D(normal, gl_TexCoord[0].xy);
              vec4 n = rotate(pxNormal * 2.0 - 1.0, rotation);
              gl_FragColor = (n + 1.0) / 2.0;
            }
          );

          shader.loadFromMemory(SHADER_FRAG, sf::Shader::Fragment);
        }

        ~MeshPass() { ; }
      };

      class LightPass final : public Shader {
      private:
        std::string SHADER_FRAG;
        sf::RenderTexture* position{ nullptr }, * diffuse{ nullptr }, * normal{ nullptr }, * esm{ nullptr };

        struct light_t {
          float radius{};
          sf::Glsl::Vec3 position{};
          sf::Glsl::Vec4 color{ sf::Color::White };
          float specular{};
          float cutoff{};
        };

        std::list<light_t> lights;

      public:
        void apply(IRenderer& renderer) override {
          if (!(position && diffuse && normal && esm)) return;

          diffuse->display();
          normal->display();
          esm->display();
          position->display();
          const sf::Texture texDiffuse = diffuse->getTexture();
          const sf::Texture texNormal = normal->getTexture();
          const sf::Texture texESM = esm->getTexture();
          const sf::Texture texPosition = position->getTexture();

          // prepare the renderer for drawing
          renderer.clear();

          shader.setUniform("accumulation", sf::Shader::CurrentTexture);
          shader.setUniform("screenpos", texPosition);
          shader.setUniform("diffuse", texDiffuse);
          shader.setUniform("normal", texNormal);
          shader.setUniform("esm", texESM);
          sf::RenderStates states;
          states.shader = &shader;

          for (auto& light : lights) {
            shader.setUniform("lightPos", light.position);
            shader.setUniform("lightColor", light.color);
            shader.setUniform("lightRadius", light.radius);
            shader.setUniform("lightSpecular", light.specular);
            shader.setUniform("lightCutoff", light.cutoff);

            renderer.display();
            const sf::Texture out = renderer.getTexture();

            sf::Sprite temp(out);
            renderer.submit(Immediate(&temp, states));
          }
        }

        void clearLights() {
          lights.clear();
        }

        void addLight(float radius, sf::Glsl::Vec3 pos, sf::Glsl::Vec4 color, float specular, float cutoff) {
          lights.push_back({ radius, pos, color, specular, cutoff });
        }

        void configure(sf::View view, 
        sf::RenderTexture* positionIn, 
        sf::RenderTexture* diffuseIn, 
        sf::RenderTexture* normalIn, 
        sf::RenderTexture* esmIn) {
          shader.setUniform("InvProj", sf::Glsl::Mat4(view.getInverseTransform().getMatrix()));
          position = positionIn;
          diffuse = diffuseIn;
          normal = normalIn;
          esm = esmIn;
        }

        LightPass() {
          /**
            fragment shader
          **/
          SHADER_FRAG = GLSL(
            110,
            uniform vec3 lightPos; // TODO: use structs (UBO) in SFML?
            uniform vec4 lightColor;
            uniform float lightRadius;
            uniform float lightSpecular;
            uniform float lightCutoff;
            uniform mat4 InvProj;
            uniform sampler2D accumulation;
            uniform sampler2D screenpos;
            uniform sampler2D diffuse;
            uniform sampler2D normal;
            uniform sampler2D esm;

            void main()
            {
              vec2 xy = gl_TexCoord[0].xy;
              vec4 pxOut = texture2D(accumulation, xy);
              vec4 pxDiffuse = texture2D(diffuse, xy);
              vec4 pxNormal = texture2D(normal, xy);
              vec4 pxESM = texture2D(esm, xy);
              vec4 pxScreenPos = texture2D(screenpos, xy);

              float depth = pxScreenPos.z;
              depth = depth * 2.0 - 1.0;

              float emissive = pxESM.r; // emissive surface component
              float specular = pxESM.g; // specular surface component
              float metallic = pxESM.b; // reflective metal surface component

              vec2 flippedXY = vec2(xy.x, 1.0 - xy.y); // NOTE: why am I having to do this? SFML quirk? 11/12/2022
              vec4 position = vec4(flippedXY * 2.0 - 1.0, depth, 1.0);
              position = InvProj * position;
              position /= position.w;

              vec3 Normal = normalize(pxNormal.rgb * 2.0 - 1.0);
              
              vec3 Light = lightPos - position.xyz;
              vec3 LightDir = normalize(Light);

              float distance = length(Light);
              float d = max(distance - lightRadius, 0.0);
              Light /= d;
              float denom = d/lightRadius + 1.0;
              float attenuation = 1.0 / (denom*denom);

              // apply cutoff
              attenuation = (attenuation - lightCutoff) / (1.0 - lightCutoff);
              attenuation = max(attenuation, 0.0);

              // calculate bump + diffuse
              float NdotLD = clamp(dot(Normal, LightDir), 0.0, 1.0);

              vec3 metal_i = texture2D(diffuse, xy + NdotLD).rgb * metallic;
              vec3 diffuse_i = NdotLD * lightColor.rgb * pxDiffuse.rgb;

              // combine with diffuse
              diffuse_i = (metallic * metal_i) + ((1.0 - metallic) * diffuse_i);

              // calculate specular
              vec3 halfwayDir = normalize(LightDir + normalize(vec3(0.5, 0.5, 1.0) - vec3(flippedXY, depth)));
              float specularIntensity = pow(max(dot(Normal, halfwayDir), 0.0), 16.0) * specular;
              vec3 specular_i = lightColor.rgb * specularIntensity * lightSpecular;

              // combine all lights
              gl_FragColor = vec4(pxOut.rgb + (attenuation * (diffuse_i + specular_i)) * lightColor.a, 1.0);
            }
          );

          shader.loadFromMemory(SHADER_FRAG, sf::Shader::Fragment);
        }

        ~LightPass() { ; }
      };

      class EmissivePass final : public Shader {
      private:
        std::string SHADER_FRAG;
        sf::RenderTexture* diffuse{ nullptr }, * esm{ nullptr };

      public:
        void apply(IRenderer& renderer) override {
          if (!(diffuse && esm)) return;

          sf::RenderStates states;
          states.shader = &shader;

          shader.setUniform("esm", esm);
          shader.setUniform("diffuse", diffuse);

          renderer.display();
          const sf::Texture out = renderer.getTexture();

          sf::Sprite temp(out);
          renderer.submit(Immediate(&temp, states));
        }

        void configure(sf::RenderTexture* diffuseIn, sf::RenderTexture* esmIn) {
          diffuse = diffuseIn;
          esm = esmIn;
        }

        EmissivePass() {
          /**
            fragment shader
          **/
          SHADER_FRAG = GLSL(
            110,
            uniform sampler2D diffuse;
            uniform sampler2D esm;

            void main()
            {
              vec2 px = gl_TexCoord[0].xy;
              vec4 pxESM = texture2D(esm, px);
              gl_FragColor = gl_FragColor.rgba + (pxESM.r*texture2D(diffuse, px));
            }
          );

          shader.loadFromMemory(SHADER_FRAG, sf::Shader::Fragment);
        }

        ~EmissivePass() { ; }
      };

      class PositionPass final : public Shader {
      private:
        std::string SHADER_FRAG;
        sf::RenderTexture* position{ nullptr };
        MeshData meshData;
        float nearZ{-1}, farZ{1};
        sf::View view;
      public:
        void setMeshData(const MeshData& data) {
          meshData = data;
        }

        void apply(IRenderer& renderer) override {
          if (!position) return;

          sf::RenderStates states;
          states.shader = &shader;

          sf::Sprite& sprite = *meshData.sprite;
          sf::Glsl::Vec3 screenPos = meshData.getPosition3D();
          screenPos.x /= view.getSize().x;
          screenPos.y /= view.getSize().y;
          screenPos.z = (screenPos.z-nearZ)/(farZ-nearZ);

          // clamp values for shader programs
          screenPos.x = std::clamp(screenPos.x, 0.0f, 1.0f);
          screenPos.y = std::clamp(screenPos.y, 0.0f, 1.0f);
          screenPos.z = std::clamp(screenPos.z, 0.0f, 1.0f);

          shader.setUniform("position", screenPos);
          shader.setUniform("diffuse", sf::Shader::CurrentTexture);

          position->draw(sprite, states);
        }

        void configure(float nearZIn, 
        float farZIn, 
        sf::View viewIn, 
        sf::RenderTexture* positionIn) {
          position = positionIn;
          farZ = farZIn;
          nearZ = nearZIn;
          view = viewIn;
        }

        PositionPass() {
          /**
            fragment shader
          **/
          SHADER_FRAG = GLSL(
            110,
            uniform sampler2D diffuse;
            uniform vec3 position;

            void main()
            {
              vec2 px = gl_TexCoord[0].xy;
              if(texture2D(diffuse, px).a == 0.0) discard;
              if(gl_FragColor.z > position.z) discard;

              gl_FragColor = vec4(position, 1.0);
            }
          );

          shader.loadFromMemory(SHADER_FRAG, sf::Shader::Fragment);
        }

        ~PositionPass() { ; }
      };
    }
  }
}
