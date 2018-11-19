// ID CONVERSION
class IdManager {
public:
    using id_t = std::uint64_t;

    IdManager() {
        // Dot is start and stop marker
        // Graph passes will cycle through the dot
        words2id["."] = first_id;
        id2words[first_id] = ".";
    }

    // Find id for word, or create new
    IdManager::id_t word2id(std::string&& str) {
        auto res = words2id.emplace(std::move(str), next_id);
        if(res.second) {
            if(next_id == last_id) {
                throw std::runtime_error("Id overflow");
            }
            id2words[next_id] = res.first->first;
            return next_id++;
        } else {
            return res.first->second;
        }
    }

    const std::string& id2word(IdManager::id_t id) {
        return id2words.at(id);
    }

public:
    static constexpr IdManager::id_t first_id = std::numeric_limits<IdManager::id_t>::min();

private:
    static constexpr IdManager::id_t last_id = std::numeric_limits<IdManager::id_t>::max();
    IdManager::id_t next_id = first_id + 1;
    //TODO: Refactor using std::shared_ptr<const std::string> to save memory?
    std::unordered_map<std::string, IdManager::id_t> words2id;
    std::unordered_map<IdManager::id_t, std::string> id2words;
};
