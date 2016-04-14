#pragma once

namespace base {
namespace debug {

// Dummy classes to help generate a pure call violation.

class Derived;
class Base {
 public:
  Base(Derived* derived);
  virtual ~Base();
  virtual void DoSomething() = 0;

 private:
  Derived* derived_;
};
class Derived : public Base {
 public:
  Derived();
  virtual void DoSomething();
};


// Make the optimizer think that var is aliased. This is to prevent it from
// optimizing out variables that that would not otherwise be live at the point
// of a potential crash.
void Alias(const void* var);


// A stacktrace can be helpful in debugging. For example, you can include a
// stacktrace member in a object (probably around #ifndef NDEBUG) so that you
// can later see where the given object was created from.
class StackTrace {
 public:
  // Creates a stacktrace from the current location.
  StackTrace();

  // From http://msdn.microsoft.com/en-us/library/bb204633.aspx,
  // the sum of FramesToSkip and FramesToCapture must be less than 63,
  // so set it to 62. Even if on POSIX it could be a larger value, it usually
  // doesn't give much more information.
  static const int kMaxTraces = 62;

  void* trace_[kMaxTraces];

  // The number of valid frames in |trace_|.
  size_t count_;
};

}  // namespace debug
}  // namespace base
