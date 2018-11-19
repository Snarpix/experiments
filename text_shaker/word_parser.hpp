#include "id_manager.hpp"
#include "graph.hpp"
#include "state_machine.hpp"

class WordParser {
public:
    WordParser(std::ifstream& stream):
        stream(stream),
        rd(std::random_device()()),
        statemachine(graph, idmanager)
    {}

    void parse() {
        char c;
        stream >> c;
        while(stream.good()) {
            statemachine.advance_state(c);
            stream >> c;
        }
        statemachine.finish();
    }

    void generate(std::uint64_t count, std::ostream& out) {
        // Begin from dot - sentence start and end.
        auto current_id = IdManager::first_id;
        std::uint64_t generated_words = 0;

        if(count != 0 && graph.size() > 1) {
            // Unroll first cycle, to handle first space properly
            auto& elem = graph.at(current_id);
            current_id = elem.get_next(rd);
            out << idmanager.id2word(current_id);
            generated_words++;

            while(generated_words < count || current_id != IdManager::first_id) {
                auto& elem = graph.at(current_id);
                current_id = elem.get_next(rd);
                // Dot is not a word, it's sentence end marker
                if(current_id != IdManager::first_id) {
                    out << ' ';
                    generated_words++;
                }
                out << idmanager.id2word(current_id);
            }
        }

        out << std::endl;
    }

private:
    std::ifstream& stream;
    std::default_random_engine rd;

    graph_t graph;
    IdManager idmanager;
    StateMachine statemachine;
};
