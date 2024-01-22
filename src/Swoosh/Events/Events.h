#pragma once
#include <map>
#include <typeinfo>
#include <type_traits>

//
// Event dispatcher interfaces introduced in Swoosh v2.0.0
//

namespace swoosh {
  namespace events {
    /**
      @class IDispatcher<E>
      @brief am abstract class that broadcasts submitted events with base type `E`
    */
    template<typename E>
    struct IDispatcher {
      virtual ~IDispatcher() {}

      /**
        @brief Implementation defined
      */
      virtual void broadcast(const char* name, void* src, bool is_base) = 0;

      /**
        @brief Sends the event to the correct handling function
        @param event Event type const reference
      */
      template<typename Event>
      void submit(const Event& event) {
        broadcast(typeid(Event).name(), (void*)&event, std::is_base_of<typename std::decay<E>::type, Event>::value);
      }
    };

    /**
      @class IReceiver<T>
      @brief an abstract class that recieves type-erased data for the given type `T`
    */
    template<typename T>
    struct IReceiver {
      virtual ~IReceiver() {}

      /**
        @brief Implementation defined
      */
      virtual void onEvent(const T& src) = 0;

      /**
        @brief recasts the void* src type into type `T`
      */
      void trampoline(void* src) {
        onEvent(*((const T*)src));
      }
    };

    /**
      @class ISubscriber<E, ...Ts>
      @brief an abstract class that redrects typename data to the correct reciever 
    */
    template<typename E, typename... Ts>
    struct ISubscriber : public IReceiver<Ts>... {
      typedef void(*funptr)(ISubscriber* ptr, void* src);
      using typestable = std::map<const char*, funptr>;
      typestable types;

      ISubscriber() {
        (types.emplace(typeid(Ts).name(), +[](ISubscriber* ptr, void* src) {
          ptr->IReceiver<Ts>::trampoline(src);
          }), ...);
      }
      virtual ~ISubscriber() {}

      /**
        @brief Given a type name, find the registered reciever type and invoke its `onEvent()` function
      */
      void redirect(const char* name, void* src, bool is_base) {
        if (types.count(name)) {
          types[name](this, src);
          return;
        }
        
        // Try to fall-back on a default handler for base types
        if (!is_base) return;
        onEvent(*((const E*)src));
      }

    /**
      @brief Base event handler. Implementation defined.
    */
      virtual void onEvent(const E& other) = 0;
    };
  }
}