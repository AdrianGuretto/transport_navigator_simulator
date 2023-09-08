#include "headers/json_builder.h"

namespace json{

// ----------- BUILDER -----------
Builder::KeyContext Builder::Key(std::string key_val){
    if (!root_.IsNull() || nodes_stack_.empty() || !nodes_stack_.back()->IsDict()){
        throw std::logic_error("Trying to add key in the wrong JSON context.");
    }
    nodes_stack_.emplace_back(std::make_unique<Node>(std::move(key_val)));
    return *this;
}

Builder& Builder::Value(Node::Value node_val){
    if (!root_.IsNull() || !CanAddPivotNode()){
        throw std::logic_error("Trying to add value in the wrong JSON context.");
    }

    std::visit([this](auto val){ nodes_stack_.emplace_back(std::make_unique<Node>(val)); }, node_val);
    AddNode(std::move(*nodes_stack_.back().release()));
    return *this;
}

Builder::ArrayContext Builder::StartArray(){
    if (!root_.IsNull() || !CanAddPivotNode()){
        throw std::logic_error("Trying to add array in the wrong JSON context.");
    }
    nodes_stack_.emplace_back(std::make_unique<Node>(Array{}));
    return *this;
}

Builder::DictContext Builder::StartDict(){
    if (!root_.IsNull() || !CanAddPivotNode()){
        throw std::logic_error("Trying to add array in the wrong JSON context.");
    }
    nodes_stack_.emplace_back(std::make_unique<Node>(Dict{}));
    return *this;
}

Builder& Builder::EndArray(){
    std::unique_ptr<Node>& back_node = nodes_stack_.back();
    if (nodes_stack_.empty() || !back_node->IsArray()){
        throw std::logic_error("Trying to end array in the wrong JSON context.");
    }
    AddNode(std::move(*back_node.release()));
    return *this;
}

Builder& Builder::EndDict(){
    std::unique_ptr<Node>& back_node = nodes_stack_.back();
    if (nodes_stack_.empty() || !back_node->IsDict()){
        throw std::logic_error("Trying to end dict in the wrong JSON context.");
    }
    AddNode(std::move(*back_node.release()));
    return *this;
}

Node& Builder::Build(){
    if (root_.IsNull() || !nodes_stack_.empty()){
        throw std::logic_error("Cannot build JSON data.");
    }
    return root_;
}

inline bool Builder::CanAddPivotNode() const{
    return nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsString();
}
void Builder::AddNode(Node&& n_node){
    nodes_stack_.pop_back(); // cleaning up the nullptr

    if (nodes_stack_.empty()){
        root_ = std::move(n_node);
        return;
    }

    auto& back_node = nodes_stack_.back();
    if (back_node->IsArray()){
        std::get<Array>(back_node->GetValue()).emplace_back(std::move(n_node));
    }
    else if (back_node->IsString()){
        std::string key = std::get<std::string>(back_node->GetValue());
        nodes_stack_.pop_back();
        std::get<Dict>(nodes_stack_.back()->GetValue()).emplace(std::move(key), std::move(n_node));
    }
    else{ // for Start* functions (containers)
        if (n_node.IsArray()){
            back_node->GetValue() = Array{};
        }
        else if (n_node.IsDict()){
            back_node->GetValue() = Dict{};
        }
    }
}

// ----------- KEYCONTEXT -----------

Builder::KeyContext::KeyContext(Builder& builder) : builder_(builder) {}

Builder::DictContext Builder::KeyContext::Value(Node::Value node_val){
    return builder_.Value(std::move(node_val));
}

Builder::ArrayContext Builder::KeyContext::StartArray(){
    return builder_.StartArray();
}
Builder::DictContext Builder::KeyContext::StartDict(){
    return builder_.StartDict();
}

// ----------- ARRAYCONTEXT -----------
Builder::ArrayContext::ArrayContext(Builder& builder) : builder_(builder) {}

Builder::ArrayContext Builder::ArrayContext::Value(Node::Value node_val){
    return builder_.Value(std::move(node_val));
}

Builder::ArrayContext Builder::ArrayContext::StartArray(){
    return builder_.StartArray();
}
Builder::DictContext Builder::ArrayContext::StartDict(){
    return builder_.StartDict();
}

Builder& Builder::ArrayContext::EndArray(){
    return builder_.EndArray();
}

// ----------- DICTCONTEXT -----------

Builder::DictContext::DictContext(Builder& builder) : builder_(builder) {}

Builder::KeyContext Builder::DictContext::Key(std::string key_value){
    return builder_.Key(std::move(key_value));
}

Builder& Builder::DictContext::EndDict(){
    return builder_.EndDict();
}

} // namespace json