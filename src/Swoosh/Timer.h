#pragma once

#include <SFML/System.hpp>
#include <map>
#include <vector>
#include <functional>

namespace swoosh {
  /**
   * @class Timer
   * @brief Creates stopwatch utility objects that can be paused, reset, and started again
   *
   * Useful for timed beaviors in your applications and is used internally for Segue completion
   */
  class Timer {
  public:
    class Trigger; // forward decl

  private:
    sf::Int32 elapsed{ 0 }; //!< Elapsed time in milliseconds
    bool paused{ true }; //!< If true, paused
    bool reversed{ false }; //!< If true, will count down from `elapsed`
    std::map<sf::Int32, Trigger> triggers; //!< List of triggers to perform
  public:
    /**
     * @class Trigger
     * @brief An object with a list of tasks
     *
     * use Timer::at(time) to return a Trigger object to assign tasks to execute when the given time has elapsed
     */
    class Trigger {
    public:
      class Task;
      friend class Timer;

    private:
      std::vector<Task> tasks; //!< List of tasks

    public:
      /**
       * @class Task
       * @brief A task has a duration and a function to poll over
       *
       * Tasks are only polled over when the duration has not yet been met by the elapsed time
       * They are executed by Triggers when the appropriate time has passed to begin them
       */
      class Task {
        friend class Timer;

      private:
        sf::Int32 duration{}; //!< How long the polling lasts in ms
        std::function<void(sf::Time)> func; //!< Behavior to execute

      public:
        /**
        * @brief Constructor
        * @param func The function to execute
        */
        Task(const std::function<void(sf::Time)>& func) :
          func(func) {}

        /**
        * @brief Deonstructor. Defaulted
        */
        ~Task() = default;

        /**
        * @brief Sets the duration of a task
        * @param time The duration
        */
        void withDuration(sf::Time time) {
          duration = time.asMilliseconds();
        }
      }; // class Task


      /**
      * @brief Constructor. Defaulted
      */
      Trigger() = default;

      /**
      * @brief Deonstructor. Defaulted
      */
      ~Trigger() = default;

      /**
      * @brief Adds a task to the list
      * @param task The function to execute
      * @return the task as a modifiable reference
      */
      Task& doTask(const std::function<void(sf::Time)>& task) {
        tasks.push_back(Task{ task });
        return tasks.back();
      }
    }; // class Task

    /**
    * @brief Constructor. Paused set to true and elapsed set to 0
    */
    Timer() = default;

    /**
    * @brief Copy Constructor. Defaulted.
    */
    Timer(const Timer& rhs) = default;

    /**
    * @brief Deconstructor. Defaulted.
    */
    ~Timer() = default;

    /**
    * @brief resets the timer. Does not clear the trigger list and does not start the timer.
    */
    void reset() {
      elapsed = 0;
    }

    /**
    * @brief changes paused to false, tracking the passage of time
    */
    void start() {
      paused = false;
    }

    /**
     @brief Fetch the elapsed time as sf::Time
     @return sf::Time of elapsed time
    */
    void pause() {
      paused = true;
    }

    /**
     @brief query if the timer is paused
     @return bool
    */
    bool isPaused() const {
      return paused;
    }

    /**
     @brief query if the timer is reversed
     @return bool
    */
    bool isReversed() const {
      return reversed;
    }


    /**
     @brief Fetch the elapsed time as sf::Time
     @return sf::Time of elapsed time
   */
    sf::Time getElapsed() const {
      return sf::milliseconds(elapsed);
    }


    /**
     @brief update the timer and trigger any callbacks on the way
     @param span sf::Time elapsed time

     If the timer is not reversed it will count upwards infinitely
     If the timer is reversed, it will count backwards and halt at zero
   */
    void update(const sf::Time& span) {
      if (!paused) {
        auto lastTickElapsed = elapsed;

        if (reversed) {
          elapsed = std::max<sf::Int32>(0, elapsed - span.asMilliseconds());
        }
        else {
          elapsed += span.asMilliseconds();
        }

        if (!reversed) {
          for (auto&& item : triggers) {
            auto startTime = item.first;
            if (startTime <= elapsed) {
              auto trigger = item.second;
              for (auto&& tasks : trigger.tasks) {
                auto progress = elapsed - startTime;

                // account for overlapping the final tick for "perfect" animation transitions
                auto missedProgress = lastTickElapsed - startTime;

                // Check if to update the function or provide the final tick into the function
                if (progress <= tasks.duration) {
                  tasks.func ? tasks.func(sf::milliseconds(progress)) : (void)0;
                }
                else if (progress > tasks.duration && missedProgress <= tasks.duration) {
                  // use final tick for "perfect" animation transitions and endings
                  tasks.func ? tasks.func(sf::milliseconds(tasks.duration)) : (void)0;
                }
              }
            }
          }
        }
        else {
          for (auto&& item : triggers) {
            auto startTime = item.first;
            // If the start time is behind the elapsed time in reverse,
            // There's a chance we have a task that needs to be in progress
            // NOTE: unoptimized. We should change the trigger point to be
            //                    = startTime + duration
            if (startTime < elapsed) {
              auto trigger = item.second;
              for (auto&& tasks : trigger.tasks) {
                auto progress = elapsed - startTime;

                // account for overlapping the final tick for "perfect" animation transitions
                auto missedProgress = lastTickElapsed - (startTime + tasks.duration);

                // Check if to update the function or provide the final tick into the function
                if (progress <= tasks.duration && progress > 0) {
                  tasks.func ? tasks.func(sf::milliseconds(progress)) : (void)0;
                }

              }
            }
            else if (elapsed <= startTime) { // this case catches passed items
              auto trigger = item.second;
              for (auto&& tasks : trigger.tasks) {
                auto missedProgress = lastTickElapsed - startTime;

                if (missedProgress > 0) {
                  // use final tick for "perfect" animation transitions and endings
                  tasks.func ? tasks.func(sf::milliseconds(0)) : (void)0;
                }
              }
            }
          }
        }
      }
    }

    /**
      @brief add a trigger at a specific time
      @param time sf::Time timestamp
      @return a new Trigger object to perform a task or tasks
    */
    Trigger& at(const sf::Time& time) {
      const auto& [tuple, status] = triggers.insert({ time.asMilliseconds(), Trigger{} });
      return tuple->second;
    }

    /**
      @brief clears all of the triggers
    */
    void clear() {
      triggers.clear();
    }

    /**
      @brief sets the timer to reverse counting from `elapsed`
    */
    void reverse(bool state) {
      this->reversed = state;
    }

    /**
      @brief set the current tick point `elapsed` to some value
      @param time sf::Time timestamp

      This is useful if you want to count backwards to 0 from
      some given time
      */
    void set(const sf::Time& time) {
      this->elapsed = time.asMilliseconds();
    }
  };
}