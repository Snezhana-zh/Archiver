#include <ostream>
#include <memory>

class Node {
private:
    unsigned char ch {0};
    int frequency {0};
    std::string code_string = "";

public:
    using pointer = std::shared_ptr<Node>;
    pointer left = nullptr;
    pointer right =  nullptr;

    Node() = default;
    Node(unsigned char uch, int f) : ch(uch), frequency(f) {}

    int get_freq() const {
        return frequency;
    }
    void set_freq(int f) {
        frequency = f;
    }

    std::string code() const {
        return code_string;
    }
    void code(const std::string& str) {
        code_string = str;
    }

    unsigned char get_byte() const {
        return ch;
    }

    bool is_leaf() const {
        return (left == nullptr && right == nullptr);
    }

    friend std::ostream& operator<<(std::ostream& os, const Node& node);
};

class LowestPriority {
public:
    bool operator()(const Node::pointer& lh, const Node::pointer& rh) const{
        return lh->get_freq() > rh->get_freq();
    }
};

std::ostream& operator<<(std::ostream& os, const Node& node) {
    return os << "[" << node.ch << "]=" << node.frequency;
}