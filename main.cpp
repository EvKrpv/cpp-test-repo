#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

struct Document {
    int id;
    double relevance;
};

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

class SearchServer {
public:
    void SetStopWords(const string& text) {
        stop_words_.clear();
        if (text.empty()) {
            return;
        }
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        if (document.empty()) {
            return;
        }
        const vector<string> words = SplitIntoWordsNoStop(document);
        double TF = 1.0 / static_cast<double>(words.size());
        ++ document_count_;
        for (const auto& word : words) {
            word_to_document_freqs_[word][document_id] += TF;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    size_t document_count_ = 0;

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                string stripped = word.substr(1);
                if (!IsStopWord(stripped)) {
                    query_words.minus_words.insert(stripped);
                }
            } else {
                query_words.plus_words.insert(word);
            }
        }
        return query_words;
    }

    static double CalculateIDF(const size_t document_count, const size_t word_document) {
        return log(static_cast<double>(document_count) / static_cast<double>(word_document));
    } 

    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int, double> document_to_relevance;
        for (const string& plus_word : query_words.plus_words) {
            if (word_to_document_freqs_.find(plus_word) != word_to_document_freqs_.end()) {
                double IDF = CalculateIDF(document_count_, word_to_document_freqs_.at(plus_word).size());
                for (const auto& [document_id, TF] : word_to_document_freqs_.at(plus_word)) {
                    document_to_relevance[document_id] += TF * IDF;
                }
            } 
        }
        for (const string& minus_word : query_words.minus_words) {
            if (word_to_document_freqs_.find(minus_word) != word_to_document_freqs_.end()) {
                for (const auto& [document_id, TF] : word_to_document_freqs_.at(minus_word)) {
                    document_to_relevance.erase(document_id);
                }
            }
        }
        vector<Document> id_relevance;
        for (const auto& [id, relevance] : document_to_relevance) {
            id_relevance.push_back({id, relevance});
        }
        return id_relevance;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}