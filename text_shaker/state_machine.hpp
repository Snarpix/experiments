class StateMachine {
public:
    StateMachine(graph_t& graph, IdManager& idmanager):
        graph(graph),
        idmanager(idmanager)
    {}

    void advance_state(char c);
    void finish() {
        if(!str.empty()){
            // Fix files that don't end with dot
            bool add_missing_dot = str != ".";
            commit_word(str);
            if(add_missing_dot) {
                str = ".";
                commit_word(str);
            }
        }
        state = state_t::END;
    }

public:
    // Will be comma part of word, or will be separated?
    static constexpr bool punct_same_as_words = true;
    // Convert words to lower case, so there will be no difference
    // has word first or non-first position in sentence.
    static constexpr bool convert_to_lower_case = false;

    enum class state_t {
        BEGIN,
        WORD,
        PUNCT,
        SPACE,
        END,
    };

    enum class char_type_t {
        WORD,
        PUNCT,
        SPACE,
    };

private:
    void append_char(char c) {
        if constexpr (convert_to_lower_case) {
            str += tolower(c);
        } else {
            str += c;
        }
    }

    void commit_word(std::string& str) {
        bool previous_is_first = previous_id == IdManager::first_id;
        auto& prev_element = graph[previous_id];
        previous_id = idmanager.word2id(std::move(str));
        str = std::string();
        // Prevent dot-dot cycle
        if(previous_is_first && previous_id == IdManager::first_id)
            return;
        // Connect previous word to next word in graph
        prev_element.add_edge(previous_id);
    }

    static char_type_t get_char_type(char c) {
        if(isspace(c)) {
            return char_type_t::SPACE;
        }
        if constexpr (punct_same_as_words) {
            if(c == '.') {
                return char_type_t::PUNCT;
            } else {
                return char_type_t::WORD;
            }
        } else {
            if(isalnum(c)) {
                return char_type_t::WORD;
            } else if(ispunct(c)) {
                return char_type_t::PUNCT;
            }
        }
        throw std::runtime_error("Invalid character");
    }

private:
    graph_t& graph;
    IdManager& idmanager;

    state_t state = state_t::BEGIN;
    std::string str;
    IdManager::id_t previous_id = IdManager::first_id;
};


static inline
bool operator==(StateMachine::state_t l, StateMachine::char_type_t r)
{
    switch(l) {
        case StateMachine::state_t::WORD:
            return r == StateMachine::char_type_t::WORD;
        case StateMachine::state_t::PUNCT:
            return r == StateMachine::char_type_t::PUNCT;
        case StateMachine::state_t::SPACE:
            return r == StateMachine::char_type_t::SPACE;
        default:
            throw std::runtime_error("Invalid comparison");
    }
}

static inline
StateMachine::state_t to_state(StateMachine::char_type_t c) noexcept {
    switch(c) {
        case StateMachine::char_type_t::WORD:
            return StateMachine::state_t::WORD;
        case StateMachine::char_type_t::PUNCT:
            return StateMachine::state_t::PUNCT;
        case StateMachine::char_type_t::SPACE:
            return StateMachine::state_t::SPACE;
    }
}

void StateMachine::advance_state(char c) {
    char_type_t type = get_char_type(c);

    switch(state) {
    case state_t::BEGIN:
        if(type == char_type_t::SPACE) {
            state = state_t::SPACE;
        }
        [[fallthrough]];

    case state_t::SPACE:
        switch(type) {
        case char_type_t::SPACE:
            break;
        case char_type_t::WORD:
        case char_type_t::PUNCT:
            append_char(c);
            state = to_state(type);
            break;
        }
        break;

    case state_t::WORD:
    case state_t::PUNCT:
        if(state == type) {
            append_char(c);
        } else {
            state = to_state(type);
            commit_word(str);
            if(type != char_type_t::SPACE) {
                append_char(c);
            }
        }
        break;

    case state_t::END:
        throw std::runtime_error("Invalid state");
        break;
    }
}
