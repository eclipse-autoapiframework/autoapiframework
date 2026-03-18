#ifndef VAF_FUTURE_H_
#define VAF_FUTURE_H_

#include "vaf/logging.h"
#include "vaf/result.h"
#include <future>


namespace vaf {

    template<typename T, typename E = vaf::Error>
    class Future {
    public:
      Future() : future_{} {}
      Future(std::future<vaf::Result<T, E>>&& future) : future_{std::move(future)} {}

      Future(Future&& future) : future_{std::move(future.future_)}{}
      Future(const Future& other ) = delete;

      Future& operator=( Future&& other ) noexcept {
        if (this != &other)
          future_ = std::move(other.future_);
        return *this;
      }
      Future& operator=( const Future& other ) = delete;

      bool valid() const noexcept {
        return future_.valid();
      }

      void wait() const {
        future_.wait();
      }

      template< class Rep, class Period >
      std::future_status wait_for( const std::chrono::duration<Rep,Period>& timeout_duration ) const {
        return future_.wait_for(timeout_duration);
      }

      template< class Clock, class Duration >
      std::future_status wait_until( const std::chrono::time_point<Clock,Duration>& timeout_time ) const {
        return wait_until(timeout_time);
      }

      vaf::Result<T, E> GetResult() {
        return future_.get();
      }

      T get() {
        vaf::Result<T, E> res{GetResult()};
        if (!res.HasValue()) {
          vaf::LoggerSingleton::getInstance()->default_logger_.LogFatal() << "Future result has no value!";
          std::abort();
        }
        return std::move(res).Value();
      }

      bool is_ready() const noexcept {
        if (this->valid())
          return future_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        return false;
      }

    private:
      std::future<vaf::Result<T, E>> future_;
    };

    template<typename T, typename E = vaf::Error>
    class Promise : protected std::promise<vaf::Result<T, E>> {
    public:
      void SetError(E error_code) {
        std::promise<vaf::Result<T, E>>::set_value(vaf::Result<T, E>{error_code});
      }
      template <class U = T, class Y = E,
                std::enable_if_t<!std::is_void<U>::value, int> = 0>
      void set_value(U value) {
        std::promise<vaf::Result<T, E>>::set_value(vaf::Result<U, Y>{value});
      }
      template <class U = T, class Y = E,
                std::enable_if_t<std::is_void<U>::value, int> = 0>
      void set_value() {
        std::promise<vaf::Result<T, E>>::set_value(vaf::Result<U, Y>{});
      }

      vaf::Future<T,E> get_future() {
        return std::move(vaf::Future<T,E>(std::promise<vaf::Result<T, E>>::get_future()));
      }
    };


    template<typename T>
    bool is_future_ready(vaf::Future<T> const &f, uint32_t timeout_ms = 0) {
        if (f.valid())
          return f.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::ready;
        return false;
    }
}  // namespace vaf

#endif  // VAF_FUTURE_H_


