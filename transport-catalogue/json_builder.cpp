#include "json_builder.h"

#include <string>
#include <stdexcept>

namespace json
{

BuilderInputInterface::BuilderInputInterface(Builder& main) : main_(main) {}

KeyBuilder BuilderInputInterface::Key(std::string str) {return main_.Key(str);}

Builder& BuilderInputInterface::Value(Node::NodeVariations val) {return main_.Value(val);}

DictBuilder BuilderInputInterface::StartDict() {return main_.StartDict();}

Builder& BuilderInputInterface::EndDict() {return main_.EndDict();}

ArrayBuilder BuilderInputInterface::StartArray() {return main_.StartArray();}

Builder& BuilderInputInterface::EndArray() {return main_.EndArray();}

KeyBuilder::KeyBuilder(Builder& main) : BuilderInputInterface(main) {}

DictBuilder KeyBuilder::Value(Node::NodeVariations val) {return BuilderInputInterface::Value(val);}

DictBuilder::DictBuilder(Builder& main) : BuilderInputInterface(main) {}

ArrayBuilder::ArrayBuilder(Builder& main) : BuilderInputInterface(main) {}

ArrayBuilder ArrayBuilder::Value(Node::NodeVariations val) {return BuilderInputInterface::Value(val);}


Node ConvertToNode (Node::NodeVariations& raw_val) {
    if (std::holds_alternative<Array>(raw_val))
    {
        Array val = std::get<Array>(raw_val);
        return Node(val);
    }
    else if (std::holds_alternative<Dict>(raw_val))
    {
        Dict val = std::get<Dict>(raw_val);
        return Node(val);
    }
    else if (std::holds_alternative<bool>(raw_val))
    {
        bool val = std::get<bool>(raw_val);
        return Node(val);
    }
    else if (std::holds_alternative<int>(raw_val))
    {
        int val = std::get<int>(raw_val);
        return Node(val);
    }
    else if (std::holds_alternative<double>(raw_val))
    {
        double val = std::get<double>(raw_val);
        return Node(val);
    }
    else if (std::holds_alternative<std::string>(raw_val))
    {
        std::string val = std::get<std::string>(raw_val);
        return Node(val);
    }
    else
    {
        return Node();
    }
}

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
        if (!nodes_stack_.back().IsString() && !nodes_stack_.back().IsArray())
        {
            throw std::logic_error("Not a Key");
        }
        if (nodes_stack_.back().IsArray())
        {
            Array buff = nodes_stack_.back().AsArray();
            buff.push_back(std::move(node));
            nodes_stack_.pop_back();
            nodes_stack_.push_back(Node(buff));
        }
        else if (nodes_stack_.back().IsString())
        {
            std::string key_buff = nodes_stack_.back().AsString();
            nodes_stack_.pop_back();
            if (nodes_stack_.back().IsMap())
            {
                Dict buff = nodes_stack_.back().AsMap();
                buff.emplace(key_buff, node);
                nodes_stack_.pop_back();
                nodes_stack_.push_back(Node(buff));
            }
        }
    }
}

KeyBuilder Builder::Key(std::string str) {
    if (nodes_stack_.empty() || !nodes_stack_.back().IsMap())
    {
        throw std::logic_error("Wrong Key placement");
    }
    nodes_stack_.push_back(Node(str));
    return KeyBuilder(*this);
}

Builder& Builder::Value(Node::NodeVariations val) {
    InsertObject(ConvertToNode(val));
    return *this;
}

DictBuilder Builder::StartDict() {
    nodes_stack_.push_back(Node(Dict{}));
    return DictBuilder(*this);
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back().IsMap())
    {
        throw std::logic_error("Dict ending error");
    }
    Dict buff = nodes_stack_.back().AsMap();
    nodes_stack_.pop_back();
    InsertObject(Node(buff));
    return *this;
}

ArrayBuilder Builder::StartArray() {
    nodes_stack_.push_back(Node(Array{}));
    return ArrayBuilder(*this);
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back().IsArray())
    {
        throw std::logic_error("Array ending error");
    }
    Array buff = nodes_stack_.back().AsArray();
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