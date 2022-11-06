#pragma once
#include <Swoosh/Events/Events.h>
#include <SFML/Graphics.hpp>

using swoosh::events::IDispatcher;
using swoosh::events::ISubscriber;

namespace swoosh {
    class RenderSource {
    private:
      const sf::Drawable& ref;
      const sf::RenderStates statesIn;

    public:
      explicit RenderSource(const sf::Drawable& src, const sf::RenderStates& states = sf::RenderStates())
        : ref(src), statesIn(states) {}
      virtual ~RenderSource() {}

      const sf::Drawable& drawable() const { return ref; }
      const sf::RenderStates& states() const { return statesIn; }
    };

    class Immediate : public RenderSource {
    public:
        explicit Immediate(const sf::Drawable& src, const sf::RenderStates& states = sf::RenderStates()) : RenderSource(src, states) {}
    };

    class IRenderer : public IDispatcher<RenderSource> {
    public:
        virtual ~IRenderer(){}

        // shortcut for SFML primitives
        void submit(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates()) {
          IDispatcher::submit(RenderSource(drawable, states));
        }

        // enable only if SFML shortcut is not applicable
        template<typename Event, typename use = std::enable_if_t<!std::is_base_of_v<sf::Drawable, Event>, void>>
        void submit(const Event& event) {
          IDispatcher::submit(event);
        }

        virtual void draw() = 0;
        virtual void display() = 0;
        virtual void clear(sf::Color color = sf::Color::Transparent) = 0;
        virtual void setView(const sf::View& view) = 0;
        virtual sf::Texture getTexture() = 0;
    };

    template<typename... Ts>
    class Renderer : public IRenderer, public ISubscriber<RenderSource, Ts...> {
    private:
        void broadcast(const char* name, void* src, bool ofBaseEvent) override {
            this->redirect(name, src, ofBaseEvent);
        }

    public:
        virtual ~Renderer(){}
    };
}