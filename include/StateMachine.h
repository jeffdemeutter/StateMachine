#include <functional>

template <typename T>
  requires std::is_enum_v<T>
class StateMachine {

  struct TransitionContext {
    T to;
    std::function<bool()> condition;
  };

  struct StateContext {
    std::function<void()> onStart = [] {};
    std::function<void()> onUpdate = [] {};
    std::function<void()> onStop = [] {};

    std::vector<TransitionContext> transitions{};
  };

public:
  StateMachine(T startState);
  ~StateMachine() = default;
  StateMachine(const StateMachine &) = delete;
  StateMachine(StateMachine &&) noexcept = delete;
  StateMachine &operator=(const StateMachine &) = delete;
  StateMachine &operator=(StateMachine &&) noexcept = delete;

  StateMachine &AddOnStart(T state, std::function<void()> onStart);
  StateMachine &AddOnUpdate(T state, std::function<void()> onUpdate);
  StateMachine &AddOnStop(T state, std::function<void()> onStop);
  StateMachine &AddStateContext(T state, std::function<void()> onStart,
                                std::function<void()> onUpdate,
                                std::function<void()> onStop);
  StateMachine &AddTransition(T from, T to, std::function<bool()> condition);

  void Update();
  T GetCurrentState() const { return mCurrentState; }

private:
  std::unordered_map<T, StateContext> mStateContexts;
  T mCurrentState;

  void SwitchToState(const TransitionContext &stateContext);
  bool TransitionExists(const T &to,
                        const std::vector<TransitionContext> &transitions);
};

template <typename T>
  requires std::is_enum_v<T>
StateMachine<T>::StateMachine(T startState) : mCurrentState(startState) {}

template <typename T>
  requires std::is_enum_v<T>
StateMachine<T> &StateMachine<T>::AddOnStart(T state,
                                             std::function<void()> onStart) {
  mStateContexts[state].onStart = onStart;
  return *this;
}

template <typename T>
  requires std::is_enum_v<T>
StateMachine<T> &StateMachine<T>::AddOnUpdate(T state,
                                              std::function<void()> onUpdate) {
  mStateContexts[state].onUpdate = onUpdate;
  return *this;
}

template <typename T>
  requires std::is_enum_v<T>
StateMachine<T> &StateMachine<T>::AddOnStop(T state,
                                            std::function<void()> onStop) {
  mStateContexts[state].onStop = onStop;
  return *this;
}

template <typename T>
  requires std::is_enum_v<T>
StateMachine<T> &
StateMachine<T>::AddStateContext(T state, std::function<void()> onStart,
                                 std::function<void()> onUpdate,
                                 std::function<void()> onStop) {
  AddOnStart(state, onStart);
  AddOnUpdate(state, onUpdate);
  AddOnStop(state, onStop);
  return *this;
}

template <typename T>
  requires std::is_enum_v<T>
StateMachine<T> &
StateMachine<T>::AddTransition(T from, T to, std::function<bool()> condition) {
  if (from == to)
    return *this;

  if (TransitionExists(to, mStateContexts[from].transitions))
    return *this;

  mStateContexts[from].transitions.emplace_back(
      TransitionContext{to, condition});
  return *this;
}

template <typename T>
  requires std::is_enum_v<T>
void StateMachine<T>::Update() {
  for (const auto &stateContext : mStateContexts[mCurrentState].transitions) {
    if (mCurrentState == stateContext.to)
      continue;

    if (!stateContext.condition())
      continue;

    SwitchToState(stateContext);
    return;
  }

  mStateContexts[mCurrentState].onUpdate();
}

template <typename T>
  requires std::is_enum_v<T>
void StateMachine<T>::SwitchToState(const TransitionContext &stateContext) {
  mStateContexts[mCurrentState].onStop();
  mCurrentState = stateContext.to;
  mStateContexts[mCurrentState].onStart();
}

template <typename T>
  requires std::is_enum_v<T>
bool StateMachine<T>::TransitionExists(
    const T &to, const std::vector<TransitionContext> &transitions) {
  for (const auto &transition : transitions)
    if (transition.to == to)
      return true;

  return false;
}