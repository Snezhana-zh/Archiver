#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <iterator>
#include <bitset>
#include <algorithm>
#include "Node.h"

using queue_t = std::priority_queue<Node::pointer, std::vector<Node::pointer>, LowestPriority>;

std::ifstream::pos_type get_size(const std::string& filename) {
    std::ifstream ifs(filename, std::ifstream::ate);
    return ifs.tellg();
}

class Exception : public std::exception {
public:
	Exception(const std::string_view& message, const std::string_view& function)
		: errorMessage(message), functionName(function) {}
	const char* what() const noexcept override {
		return errorMessage.c_str();
	}

	const char* where() const noexcept {
		return functionName.c_str();
	}

private:
	std::string errorMessage;
	std::string functionName;
};

void read_file(const std::string& filename, std::vector<int>& frequency) {
    int filesize = get_size(filename);
    if (filesize < 0) {
        throw Exception("Negative filesize", filename);
    }

    std::ifstream ifs(filename, std::ifstream::binary);
    if (!ifs) {
        ifs.exceptions(std::ifstream::failbit);
    }

    std::cout << "Reading file: " << filename << std::endl;
    int index = 0;
    while(true) {
        char ch;
        ifs.read(&ch, 1);
        if (ifs.eof()) {
            break;
        }
        frequency[static_cast<unsigned char>(ch)]++;

        if(index % 20 == 0) {
            int value = (index + filesize%20) * 100.0 / filesize;
            std::cout << "\r" << value << "%" << std::flush;
        }
        ++index;
    }
    std::cout << std::endl;
}

void make_codes(Node::pointer& node, std::string str, std::vector<std::string>& codes) {
    if (node->left != nullptr) {
        make_codes(node->left, str + "0", codes);
    }
    if (node->right != nullptr) {
        make_codes(node->right, str + "1", codes);
    }
    if (node->left == nullptr && node->right == nullptr) {
        node->code(str);
        codes[node->get_byte()] = str;
    }
}

void PrintQueue(queue_t queue) {
    while (!queue.empty()) {
        std::cout << *(queue.top()) << " ";
        queue.pop();
    }
}

void fill_queue(const std::vector<int>& frequency, queue_t& queue) {
    for (size_t i = 0; i != frequency.size(); ++i) {
        if (frequency[i] != 0) {
            Node::pointer node = std::make_shared<Node>(i, frequency[i]);
            queue.push(node);
        }
    }
}

void build_tree(queue_t& queue) {
    while (queue.size() > 1) {
        Node::pointer x = queue.top();
        queue.pop();

        Node::pointer y = queue.top();
        queue.pop();

        Node::pointer z = std::make_shared<Node>('0', x->get_freq() + y->get_freq());
        z->left = x;
        z->right = y;

        queue.push(z);
    }
}

std::string message_to_code(const std::string& filename, std::vector<std::string>& codes) {
    std::string msg = "";
    std::ifstream ifs(filename, std::ifstream::binary);
    if (!ifs) {
        std::cerr << "Error in: [" << __PRETTY_FUNCTION__ << "]\n";
        ifs.exceptions(std::ifstream::failbit);
    }
    while(true) {
        char ch;
        ifs.read(&ch, 1);
        if(ifs.eof()) {
            break;
        }
        msg += codes[static_cast<unsigned char>(ch)];
    }
    return msg;
}

void write_file(const Node::pointer& root, const std::vector<int>& frequency, const std::string& message) {
    std::string new_filename = "output.txt";
    std::ofstream ofs(new_filename, std::ofstream::binary);
    if (!ofs) {
        std::cerr << "Error in: [" << __PRETTY_FUNCTION__ << "]\n";
        ofs.exceptions(std::ofstream::failbit);
    }

    unsigned char count = count_if(frequency.begin(), frequency.end(), [](const int& value) {return (value != 0);});
    ofs.write(reinterpret_cast<char*>(&count), sizeof(count));
    for (size_t i = 0; i != frequency.size(); ++i) {
        unsigned char index = i;
        int value = frequency[i];
        if(value != 0) {
            ofs.write(reinterpret_cast<char*>(&index), sizeof(index));
            ofs.write(reinterpret_cast<char*>(&value), sizeof(value));
        }
    };

    int byte_count = message.size() / CHAR_BIT;
    unsigned char modulo = message.size() % CHAR_BIT;

    ofs.write(reinterpret_cast<char*>(&byte_count), sizeof(byte_count));
    ofs.write(reinterpret_cast<char*>(&modulo), sizeof(modulo));

    int i = 0;
    for (; i < byte_count; ++i) {
        std::bitset<CHAR_BIT> b(message.substr(i * CHAR_BIT, CHAR_BIT).c_str());
        unsigned char value = static_cast<unsigned char>(b.to_ulong());
        ofs.write(reinterpret_cast<char*>(&value), sizeof(value));
    }
    if (modulo > 0) {
        std::bitset<CHAR_BIT> b(message.substr(i * CHAR_BIT, modulo).c_str());
        unsigned char value = static_cast<unsigned char>(b.to_ulong());
        ofs.write(reinterpret_cast<char*>(&value), sizeof(value));
    }
}

void read_decoding_file(const std::string& filename, std::vector<int>& frequency, std::string& message) {
    std::ifstream ifs(filename, std::ifstream::binary);
    if (!ifs) {
        std::cerr << "Error in: [" << __PRETTY_FUNCTION__ << "]\n";
        ifs.exceptions(std::ifstream::failbit);
    }
    unsigned char count = 0;
    ifs.read(reinterpret_cast<char*>(&count), sizeof(count));

    int i = 0;
    while(i < count) {
        unsigned char ch;
        ifs.read(reinterpret_cast<char*>(&ch), sizeof(ch));

        int f = 0;
        ifs.read(reinterpret_cast<char*>(&f), sizeof(f));
        frequency[ch] = f;
        ++i;
    }

    int byte_count = 0;
    unsigned char modulo = 0;

    ifs.read(reinterpret_cast<char*>(&byte_count), sizeof(byte_count));
    ifs.read(reinterpret_cast<char*>(&modulo), sizeof(modulo));

    for (int i = 0; i < byte_count; ++i) {
        unsigned char byte;
        ifs.read(reinterpret_cast<char*>(&byte), sizeof(byte));

        std::bitset<CHAR_BIT> b(byte);
        message += b.to_string();
    }
    if (modulo > 0) {
        unsigned char byte;
        ifs.read(reinterpret_cast<char*>(&byte), sizeof(byte));

        std::bitset<CHAR_BIT> b(byte);
        message += b.to_string().substr(CHAR_BIT - modulo, CHAR_BIT);
    }
}

void make_char(const Node::pointer& root,const std::string& message, std::string& text) {
    Node::pointer node = root;

    auto found_the_letter = [&text, &root](Node::pointer& n) {
        text += n->get_byte();
        n = root;
    };

    for(size_t i = 0; i < message.size(); ++i) {
        char ch = message[i];
        if (ch == '0') {
            if (node->left != nullptr) {
                node = node->left;
                if (node->is_leaf()) {
                    found_the_letter(node);
                }
            }
        } else if (ch == '1') {
            if (node->right != nullptr) {
                node = node->right;
                if (node->is_leaf()) {
                    found_the_letter(node);
                }
            }
        }
    }
}

void write_decoding_file(const std::string& text) {
    std::string new_filename = "output.txt";
    std::ofstream ofs(new_filename, std::ofstream::binary);
    if (!ofs) {
        std::cerr << "Error in: [" << __PRETTY_FUNCTION__ << "]\n";
        ofs.exceptions(std::ofstream::failbit);
    }
    ofs << text;
}

int main() {
    try {
        std::string filename = "in.txt";
        char action;
        std::cin >> action;
        if(action == 'c') {     
            std::vector<int> frequency(0x100, 0);
            read_file(filename, frequency);

            queue_t queue;

            fill_queue(frequency, queue);

            build_tree(queue);

            Node::pointer root = queue.top();
            std::vector<std::string> codes(0x100, "");
            make_codes(root, "", codes);

            std::string message = message_to_code(filename, codes);

            write_file(root, frequency, message);
        } else if(action == 'd') {
            std::vector<int> frequency(0x100, 0);
            std::string message = "";

            read_decoding_file(filename, frequency, message);
            queue_t queue;
            fill_queue(frequency, queue);
            build_tree(queue);

            Node::pointer root = queue.top();
            std::string text = "";
            make_char(root, message, text);

            write_decoding_file(text);
        } else {
            std::cerr << "You should write symbol \'c\' or \'d\'" << std::endl;
        }
    } catch (const std::istream::failure& ex) {
        std::cerr << "Failed write or read file: " << ex.what() << "\n" ;
        return 1;
    } catch (const Exception& ex) {
        std::cerr << ex.what() << " in " << ex.where() << "\n" ;
        return 1;
    } catch (...) {
        std::cerr << "Exception: " << strerror(errno);
        return 1;
    }
}