#pragma once

#include "json.h"

#include <memory>
#include <string>

namespace json {

class Builder {
public:
  class KeyBuilder;
  class DictBuilder;
  class ArrayBuilder;

  KeyBuilder Key(std::string str);

  Builder &Value(Node::NodeVariations val);

  //------------------------------

  DictBuilder StartDict();

  Builder &EndDict();

  //------------------------------

  ArrayBuilder StartArray();

  Builder &EndArray();

  //------------------------------

  Node Build();

private:
  void InsertObject(Node node);

private:
  Node root_;
  std::vector<std::unique_ptr<Node>> nodes_stack_;
};

class BuilderInputInterface {
public:
  BuilderInputInterface(Builder &main);

  Builder::KeyBuilder Key(std::string str);

  Builder &Value(Node::NodeVariations val);

  Builder::DictBuilder StartDict();

  Builder &EndDict();

  Builder::ArrayBuilder StartArray();

  Builder &EndArray();

protected:
  Builder &main_;
};

class Builder::KeyBuilder : public BuilderInputInterface {
public:
  KeyBuilder(Builder &main);

  KeyBuilder Key(std::string str) = delete;
  BuilderInputInterface EndDict() = delete;
  BuilderInputInterface EndArray() = delete;

  DictBuilder Value(Node::NodeVariations val);
};

class Builder::DictBuilder : public BuilderInputInterface {
public:
  DictBuilder(Builder &main);

  Builder &Value(Node::NodeVariations val) = delete;
  DictBuilder StartDict() = delete;
  ArrayBuilder StartArray() = delete;
  Builder &EndArray() = delete;
};

class Builder::ArrayBuilder : public BuilderInputInterface {
public:
  ArrayBuilder(Builder &main);

  KeyBuilder Key(std::string str) = delete;
  Builder &EndDict() = delete;

  ArrayBuilder Value(Node::NodeVariations val);
};

} // namespace json
