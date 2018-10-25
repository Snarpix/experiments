#include <iostream>
#include <cctype>
#include <cassert>

enum class State {
    START,
    RESTART,
    NUM,
    RANGE,
    RANGE_NUM,
    END,
};

enum class NumberType {
    ALONE,
    RANGE_BEGIN,
    RANGE_END,
};

#if 0
template<typename T, typename It>
It replace(T cont, It rem_begin, It rem_end, It ins_begin, It ins_end)
{
    auto insert_count = std::distance(ins_begin, ins_end);
    auto del_count = std::distance(rem_begin, rem_end);
    auto diff = insert_count - del_count;
    if(diff > 0) {
        cont.resize(cont.size() + diff);
        for(uint64_t i = 0 ; i < diff; ++i) {
            cont[cont.size() - 1 - i] = std::move(cont[cont.size() - 1 - diff + i]);
        }
        for(uint64_t i = 0 ; i < insert_count; ++i) {
            cont[cont.size() - 1 - i] = 
        }
    }

    It current = rem_begin;
    It next = rem_end;
    while(next != cont_end) {
        if(ins_begin != ins_end) {
            *current = *ins_begin++;
        } else if (rem_begin != rem_end){
            *current = *next++;
        }
    }
}
#endif

class Parser {
public:
    Parser(std::string str, uint64_t num):
    str(str),
    new_num(num)
    {}

    bool check_number(State new_st) {
        if(new_st == State::RESTART || new_st == State::RANGE) {
            if(cur_num.value == new_num) {
                return true;
            } else if(last_num.value < new_num && cur_num.value > new_num) {
                if(last_num.value + 1 == new_num) {
                    std::string temp;
                    switch(last_num.type) {
                    case NumberType::ALONE:
                        if(cur_num.value - 1 == new_num) {
                            *last_num.end = '-';
                            switch(cur_num.type) {
                            case NumberType::ALONE:
                                break;

                            case NumberType::RANGE_END: 
                                assert(0);
                                break;

                            case NumberType::RANGE_BEGIN: {
                                str.erase(cur_num.begin, cur_num.end + 1);
                                break;
                            }
                            }
                            return true;
                        } else {
                            temp = "-";
                            temp += std::to_string(new_num);
                            str.insert(last_num.end, temp.begin(), temp.end());
                            return true;
                        }
                    case NumberType::RANGE_END: {
                        if(cur_num.value - 1 == new_num) {
                            auto end_it = last_num.end + 1;
                            switch(cur_num.type) {
                            case NumberType::ALONE:
                                break;

                            case NumberType::RANGE_END: 
                                assert(0);
                                break;

                            case NumberType::RANGE_BEGIN: {
                                end_it = cur_num.end + 1;
                                break;
                            }
                            }
                            str.erase(last_num.begin, end_it);
                            return true;
                        } else {
                            auto new_it = str.erase(last_num.begin, last_num.end);
                            temp = std::to_string(new_num);
                            str.insert(new_it, temp.begin(), temp.end());
                            return true;
                        }
                    }
                    case NumberType::RANGE_BEGIN: {
                        return true;
                    }
                    }
                } else {
                    if(cur_num.value - 1 == new_num) {
                        std::string temp;
                        switch(cur_num.type) {
                        case NumberType::ALONE:
                            temp = std::to_string(new_num);
                            temp += "-";
                            str.insert(cur_num.begin, temp.begin(), temp.end());
                            return true;
                        case NumberType::RANGE_END:
                            return true;
                        case NumberType::RANGE_BEGIN: {
                            auto new_it = str.erase(cur_num.begin, cur_num.end);
                            temp = std::to_string(new_num);
                            str.insert(new_it, temp.begin(), temp.end());
                            return true;
                        }
                        }
                    } else {
                        if(cur_num.type == NumberType::RANGE_END 
                            || last_num.type == NumberType::RANGE_BEGIN) {
                            return true;
                        }
                        std::string temp(",");
                        temp += std::to_string(new_num);
                        str.insert(last_num.end, temp.begin(), temp.end());
                        return true;
                    }
                }
            }
        } else if (new_st == State::END) {
            if(st == State::START) {
                str = std::to_string(new_num);
            } else if (cur_num.value + 1 == new_num) {
                switch(cur_num.type) {
                case NumberType::ALONE:
                    str += '-';
                    str += std::to_string(new_num);
                    return true;
                case NumberType::RANGE_END:
                    str.erase(cur_num.begin, str.end());
                    str += std::to_string(new_num); 
                    break;
                case NumberType::RANGE_BEGIN:
                    assert(0);
                    return true;
                }
            } else {
                str += ',';
                str += std::to_string(new_num);
                return true;
            }
        }

        return false;
    }

    void switch_state(std::string::iterator it, State new_st) {
        if(new_st == st)
            return;

        if(new_st == State::NUM || new_st == State::RANGE_NUM) {
            num_begin = it;
        }

        if(new_st == State::RESTART || new_st == State::RANGE) {
            NumberType type;
            if(st == State::RANGE_NUM) {
                if(new_st == State::RESTART) {
                    type = NumberType::RANGE_END;
                } else {
                    assert(0);
                }
            } else if (st == State::NUM) {
                if(new_st == State::RESTART) {
                    type = NumberType::ALONE;
                } else {
                    type = NumberType::RANGE_BEGIN;
                } 
            } else {
                assert(0);
            }

            num_end = it;
            last_num = cur_num;
            cur_num = {num_begin, num_end, num, type};
        }

        if(check_number(new_st)) {
            new_st = State::END;
        }

        st = new_st;
    }

    void parse_digit(char c) {
        num *= 10;
        num += c - '0';
    }

    void parse_char(std::string::iterator it) {
        switch(st) {
        case State::START:
            if(it == str.end()) {
                switch_state(it, State::END);
            }
            [[fallthrough]];
        case State::RESTART:
            if(it == str.end()) {
                switch_state(it, State::END);
            } else if(isdigit(*it)) {
                num = *it - '0';
                switch_state(it, State::NUM);
            } else {
                assert(0);
            }
            break;
        case State::NUM:
            if(it == str.end()) {
                switch_state(it, State::RESTART);
                switch_state(it, State::END);
            } else if(isdigit(*it)) {
                parse_digit(*it);
            } else if(*it == ',') {
                switch_state(it, State::RESTART);
            } else if(*it == '-') {
                switch_state(it, State::RANGE);
            } else {
                assert(0);
            }
            break;
        case State::RANGE:
            if(it == str.end()) {
                assert(0);
            } else if(isdigit(*it)) {
                num = *it - '0';
                switch_state(it, State::RANGE_NUM);
            } else if(*it == ',') {
                assert(0);
            } else if(*it == '-') {
                assert(0);
            } else {
                assert(0);
            }
            break;
        case State::RANGE_NUM:
            if(it == str.end()) {
                switch_state(it, State::RESTART);
                switch_state(it, State::END);
            } else if(isdigit(*it)) {
                parse_digit(*it);
            } else if(*it == ',') {
                switch_state(it, State::RESTART);
            } else if(*it == '-') {
                assert(0);
            } else {
                assert(0);
            }
            break;
        case State::END:
            break;
        }
    }

    bool parse() {
        for(auto it = str.begin(); it != str.end(); ++it) {
            parse_char(it);
            if(st == State::END) {
                break;
            }
        }
        parse_char(str.end());
        return true;
    }

    std::string get_string() {
        return str;
    }
    
private:
    State st = State::START;
    std::string str;
    std::int64_t new_num;
    std::int64_t num = 0;
    using string_it_t = std::string::iterator;
    string_it_t num_begin;
    string_it_t num_end;
    struct Number_t {
        string_it_t begin;
        string_it_t end;
        std::int64_t value;
        NumberType type;
    };
    Number_t cur_num = {string_it_t(), string_it_t(), -10, NumberType::ALONE};
    Number_t last_num = cur_num;
};

std::string get_value(std::string str, uint64_t num) {
    Parser p(str, num);
    p.parse();
    return p.get_string();
}

void test_value(std::string& str, uint64_t num) {
    str = get_value(str, num);
    std::cout << "Testing: " << num << ' ' << str << '\n';
}

int main() {
    std::string s = "";
    test_value(s, 1);
    test_value(s, 2);
    test_value(s, 3);
    test_value(s, 5);
    test_value(s, 6);
    test_value(s, 8);
    test_value(s, 15);
    test_value(s, 13);
    test_value(s, 20);
    test_value(s, 19);
    test_value(s, 18);
    test_value(s, 17);
    test_value(s, 19);
    test_value(s, 20);
    test_value(s, 18);
    test_value(s, 17);
    test_value(s, 21);
    test_value(s, 19);
    test_value(s, 16);
    test_value(s, 19);
    test_value(s, 23);
    test_value(s, 25);
    test_value(s, 24);
    test_value(s, 26);
    test_value(s, 24);
    test_value(s, 22);
    test_value(s, 14);
    test_value(s, 4);

    std::string s2 = "";
    test_value(s2, 1);
    test_value(s2, 0);
    return 0;
}
