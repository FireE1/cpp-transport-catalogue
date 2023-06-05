#include "json_builder.h"

#include <string>
#include <stdexcept>

namespace json
{

BuilderInputInterface::BuilderInputInterface(Builder& main) : main_(main) {}

Builder::KeyBuilder BuilderInputInterface::Key(std::string str) {return main_.Key(std::move(str));}

Builder& BuilderInputInterface::Value(Node::NodeVariations val) {return main_.Value(std::move(val));}

Builder::DictBuilder BuilderInputInterface::StartDict() {return main_.StartDict();}

Builder& BuilderInputInterface::EndDict() {return main_.EndDict();}

Builder::ArrayBuilder BuilderInputInterface::StartArray() {return main_.StartArray();}

Builder& BuilderInputInterface::EndArray() {return main_.EndArray();}

Builder::KeyBuilder::KeyBuilder(Builder& main) : BuilderInputInterface(main) {}

Builder::DictBuilder Builder::KeyBuilder::Value(Node::NodeVariations val) {return BuilderInputInterface::Value(std::move(val));}

Builder::DictBuilder::DictBuilder(Builder& main) : BuilderInputInterface(main) {}

Builder::ArrayBuilder::ArrayBuilder(Builder& main) : BuilderInputInterface(main) {}

Builder::ArrayBuilder Builder::ArrayBuilder::Value(Node::NodeVariations val) {return BuilderInputInterface::Value(std::move(val));}

void Builder::InsertObject(Node node) {
    if (nodes_stack_.empty())
    {
        if (!root_.IsNull())
        {
            throw std::logic_error("Wrong build");
        }
        root_ = node;
    }
    else
    {
        if (!nodes_stack_.back()->IsString() && !nodes_stack_.back()->IsArray())
        {
            throw std::logic_error("Not a Key");
        }
        if (nodes_stack_.back()->IsArray())
        {
            nodes_stack_.back()->GetArray().push_back(std::move(node));
        }
        else if (nodes_stack_.back()->IsString())
        {
            std::string key_buff = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
            if (nodes_stack_.back()->IsMap())
            {
                nodes_stack_.back()->GetDict().emplace(key_buff, node);
            }
        }
    }
}

Builder::KeyBuilder Builder::Key(std::string str) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap())
    {
        throw std::logic_error("Wrong Key placement");
    }
    nodes_stack_.push_back(std::move(std::make_unique<Node>(str)));
    return KeyBuilder(*this);
}

Builder& Builder::Value(Node::NodeVariations val) {
    InsertObject(Node(val));
    return *this;
}

Builder::DictBuilder Builder::StartDict() {
    nodes_stack_.push_back(std::make_unique<Node>(Dict{}));
    return DictBuilder(*this);
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap())
    {
        throw std::logic_error("Dict ending error");
    }
    Dict buff = nodes_stack_.back()->AsMap();
    nodes_stack_.pop_back();
    InsertObject(Node(buff));
    return *this;
}

Builder::ArrayBuilder Builder::StartArray() {
    nodes_stack_.push_back(std::make_unique<Node>(Array{}));
    return ArrayBuilder(*this);
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
    {
        throw std::logic_error("Array ending error");
    }
    Array buff = nodes_stack_.back()->AsArray();
    nodes_stack_.pop_back();
    InsertObject(Node(buff));
    return *this;
}

Node Builder::Build() {
    if (root_.IsNull() || !nodes_stack_.empty())
    {
        throw std::logic_error("Wrong build");
    }
    return root_;
}

}