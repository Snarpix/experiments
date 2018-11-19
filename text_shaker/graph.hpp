#include "map_iterator_helper.hpp"

// Graph
class Element
{
public:
    // Connect to next word in sentence
    void add_edge(IdManager::id_t id) {
        assert(!converted);
        edges[id] += 1;
    }

    // Get next word, according to probability
    IdManager::id_t get_next(std::default_random_engine& rd) {
        if(!converted) {
            convert();
        }
        return ids.at(distr(rd));
    }

private:
    // For parse, we need to quickly add edges (id -> count mapping)
    // For generate, we need discrete_distribution id -> word id) mapping
    // Also we need to initialize discrete_distribution with weights
    void convert() {
        // Convert edges to more suitable form for generator
        assert(edges.size() > 0);
        ids.reserve(edges.size());
        ids.insert(ids.begin(), MapKeyIterator(edges.cbegin()), MapKeyIterator(edges.cend()));
        distr = distr_t(MapValueIterator(edges.cbegin()), MapValueIterator(edges.cend()));
        // Clear memory
        edges.clear();
        edges.rehash(0);
        converted = true;
    }

private:
    // Parser view
    using edges_t = std::unordered_map<IdManager::id_t, std::uint64_t>;
    edges_t edges;

    bool converted = false;

    // Converted view for generator
    using ids_t = std::vector<IdManager::id_t>;
    ids_t ids;
    using distr_t = std::discrete_distribution<ids_t::size_type>;
    distr_t distr;
};
using graph_t = std::unordered_map<IdManager::id_t, Element>;
