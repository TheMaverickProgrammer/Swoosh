#pragma once
#include <Swoosh/Events/Events.h>
#include <SFML/Graphics.hpp>
#include <list>

using swoosh::events::IDispatcher;
using swoosh::events::ISubscriber;

namespace swoosh {
  class IRenderer;
  struct RendererEntry {
    const char* name;
    IRenderer& renderer;
  };

  using RendererEntries = std::list<RendererEntry>;

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
    Immediate(const sf::Drawable& src, const sf::RenderStates& states = sf::RenderStates()) : RenderSource(src, states) {}
  };

  namespace {
    template<typename T, bool isRenderEvent>
    struct usefulCopier_t {};

    template<typename T>
    using usefulCopier = usefulCopier_t<T, std::is_base_of_v<T, RenderSource>>;
    
    template<typename T>
    struct usefulCopier_t<T, true> {
      static RenderSource* exec(const T& from, sf::Drawable* dptr, const char** tname) { 
        *tname = typeid(T).name(); 
        return new T(from); 
      }
    };
    
    template<typename T>
    struct usefulCopier_t<T, false> {
      static RenderSource* exec(const T& from, sf::Drawable* dptr, const char** tname) { 
        *tname = typeid(RenderSource).name();
        dptr = new T(from);
        return new RenderSource(*dptr);
      }
    };
  }

  template<typename T>
  using is_render_event = std::is_base_of<RenderSource, typename T>;

  template<typename T>
  using is_sfml_primitive = std::is_base_of<sf::Drawable, typename T>;

  struct ClonedSource : RenderSource {
    const char* name {0};
    void* mem { nullptr };
    sf::Drawable* dptr {nullptr};
    ClonedSource(void* memIn, sf::Drawable* dptr, const char* nameIn) : 
      name(nameIn), mem(memIn), dptr(dptr), RenderSource(*dptr) {}
  };

  template<typename T>
  ClonedSource Clone(const T& t) {
    const char* tname {0};
    sf::Drawable* dptr {nullptr};
    RenderSource* ptr = ::usefulCopier<T>::exec(t, dptr, &tname);
    return ClonedSource((void*)ptr, dptr, tname);
  }

  class IRenderer : public IDispatcher<RenderSource> {
    friend class ActivityController;

    // for ease-of-use structures that allocate memory and need to free
    virtual void flushMemory() = 0;

  public:
    virtual ~IRenderer() {}

    // This overload shortcut for SFML primitives wraps a drawable into a render event
    void submit(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates()) {
      IDispatcher::submit(RenderSource(drawable, states));
    }

    // Submit a render event
    template<typename Event, typename use = std::enable_if_t<is_render_event<Event>::value>>
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
  class Renderer : public IRenderer, public ISubscriber<RenderSource, ClonedSource, Ts...> {
  private:
    std::vector<ClonedSource> clonedMem;

    void broadcast(const char* name, void* src) override {
      this->redirect(name, src);
    }

    void onEvent(const ClonedSource& event) override {
      ClonedSource& ref = clonedMem.emplace_back(std::move(event));
      this->redirect(ref.name, ref.mem);
    }

    void flushMemory() override {
      for(ClonedSource& c : clonedMem) {
        delete c.mem;
        delete c.dptr;
      }
      clonedMem.clear();
    }

  public:
    virtual ~Renderer() {}
  };
}