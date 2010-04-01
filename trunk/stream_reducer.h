class ReduceInput {
public:
  bool Done() const;
  void Next();
  const string& key();
  const string& value();
};

class StreamReducer {
public:
  virtual void Start() {};
  virtual void Reduce(ReduceInput* input) = 0;
  virtual void Flush() {}
};
