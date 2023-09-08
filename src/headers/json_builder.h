#pragma once

#include <memory>

#include "json.h"

namespace json{

// Main JSON building class.
class Builder final{
public: 
    Builder() = default;
public: // --------- CONTEXT CLASSES ---------
    class KeyContext;
    class DictContext;
    class ArrayContext;

public: // --------- METHODS ---------

    // Construct a key with `key_val` string inside.
    KeyContext Key(std::string key_val);

    // Construct a value from `node_val`.
    Builder& Value(Node::Value node_val);

    ArrayContext StartArray();
    DictContext StartDict();

    Builder& EndArray();
    Builder& EndDict();

    // Main building class for `Builder`. Builds JSON data only if all contexts are correct (closed).
    Node& Build();

private: // --------- HELPER METHODS ---------
    // Checks if we can add an independant node (Array, Dict, Val).
    inline bool CanAddPivotNode() const;

    // Adds `n_node` to the node stack.
    void AddNode(Node&& n_node);

private: // --------- FIELDS ---------
    Node root_{nullptr};
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};

// A context class used for building elements for a dict key.
class Builder::KeyContext{
public:
    KeyContext(Builder& builder);

public: // --------- METHODS ---------
    DictContext Value(Node::Value node_val);

    ArrayContext StartArray();
    DictContext StartDict();

private: // --------- FIELDS ---------
    Builder& builder_;
};

// A context class used for building elements inside an array.
class Builder::ArrayContext{
public:
    ArrayContext(Builder& builder);

public: // --------- METHODS ---------
    ArrayContext Value(Node::Value node_val);

    ArrayContext StartArray();
    DictContext StartDict();
    
    Builder& EndArray();

private: // --------- FIELDS ---------
    Builder& builder_;
};

// A context class used for building elemenets inside a dictionary
class Builder::DictContext{
public:
    DictContext(Builder& builder);
public: // --------- METHODS ---------

    KeyContext Key(std::string key_value);

    Builder& EndDict();

private: // --------- FIELDS ---------
    Builder& builder_;
};


} // namespace json