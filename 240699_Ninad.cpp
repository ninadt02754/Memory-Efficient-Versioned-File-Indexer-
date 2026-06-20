#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <stdexcept>
#include <queue>

using namespace std;
using namespace std::chrono;

//1. User-Defined Template
// Template function to print results of different types
template <typename T>
void printResult(const string& label, const T& result) {
    cout << label << ": " << result << endl;
}

// Class 1: Buffered File Reading 
class BufferedFileReader {
private:
    ifstream file;
    size_t bufferSize;
    char* buffer;

public:
    BufferedFileReader(const string& filepath, size_t bufferSizeKB) {
        // Enforce buffer size constraints (256 KB to 1024 KB)
        if (bufferSizeKB < 256 || bufferSizeKB > 1024) {
            throw invalid_argument("Error: Buffer size must be between 256 KB and 1024 KB."); // Exception Handling 
        }
        bufferSize = bufferSizeKB * 1024;
        buffer = new char[bufferSize];
        file.open(filepath, ios::binary);//opens file in binary mode
        if (!file.is_open()) {
            throw runtime_error("Error: Could not open file " + filepath); // Exception Handling 
        }
    }

    ~BufferedFileReader() {//destructor 
        if (file.is_open()) file.close();//closes the file
        delete[] buffer;
    }

    size_t readChunk(string& chunkData) {
        file.read(buffer, bufferSize);//read chunk of buffer size
        size_t bytesRead = file.gcount();
        if (bytesRead > 0) {
            chunkData.assign(buffer, bytesRead);
        }
        return bytesRead;
    }
    
    bool isEOF() const { return file.eof(); }
};

// Class 2: Tokenization
class Tokenizer {
private:
    string leftover; // Handles tokens split across buffer boundaries 

public:
    void tokenizeAndIndex(const string& chunk, class VersionedIndex& index, bool isLastChunk);
};

// Class 3: Versioned Indexing
class VersionedIndex {
private:
    string versionName;
    unordered_map<string, int> wordFrequency;

public:
    VersionedIndex(const string& name) : versionName(name) {} // Separate index for each version 

    string getVersionName() const { return versionName; }

    // Function Overloading 
    void addWord(const string& word) {
        wordFrequency[word]++;
    }

    void addWord(const string& word, int count) {
        wordFrequency[word] += count;
    }

    int getFrequency(const string& word) const {
        auto it = wordFrequency.find(word);
        return (it != wordFrequency.end()) ? it->second : 0;
    }

    const unordered_map<string, int>& getAllFrequencies() const {
        return wordFrequency;
    }
};
// function defined outside class to make class look free
void Tokenizer::tokenizeAndIndex(const string& chunk, VersionedIndex& index, bool isLastChunk) {
    string currentWord = leftover; //leftover is assigned to the current word
    leftover = "";

    for (char ch : chunk) {
        // Words are contiguous alphanumeric characters
        if (isalnum(ch)) {
            currentWord += tolower(ch); // Case-insensitive matching 
        } else {
            if (!currentWord.empty()) {
                index.addWord(currentWord);
                currentWord = "";
            }
        }
    }

    if (!currentWord.empty()) {
        if (isLastChunk) {
            index.addWord(currentWord);
        } else {
            leftover = currentWord; // Save partial word across boundary 
        }
    }
}

// Class 4: Query Processing (Abstract Base Class)
class QueryProcessor {
public:
    virtual void execute() = 0; // Pure virtual function enforcing Runtime Polymorphism 
    virtual ~QueryProcessor() {}
};

// Derived Class 1: Word Count Query
class WordQueryProcessor : public QueryProcessor {
private:
    VersionedIndex& index;
    string targetWord;
public:
    WordQueryProcessor(VersionedIndex& idx, const string& word) : index(idx), targetWord(word) {
        transform(targetWord.begin(), targetWord.end(), targetWord.begin(), ::tolower);
    }
    void execute() override {
        cout << "Version: " << index.getVersionName() << "\n"; // Outputs Version name 
        printResult("Frequency of '" + targetWord + "'", index.getFrequency(targetWord)); // Outputs Query result
    }
};

// Derived Class 2: Difference Query 
class DiffQueryProcessor : public QueryProcessor {
private:
    VersionedIndex& index1;
    VersionedIndex& index2;
    string targetWord;
public:
    DiffQueryProcessor(VersionedIndex& idx1, VersionedIndex& idx2, const string& word) 
        : index1(idx1), index2(idx2), targetWord(word) {
        transform(targetWord.begin(), targetWord.end(), targetWord.begin(), ::tolower);
    }
    void execute() override {
        cout << "Versions: " << index1.getVersionName() << ", " << index2.getVersionName() << "\n"; // Outputs Version names
        int diff = abs(index1.getFrequency(targetWord) - index2.getFrequency(targetWord));
        printResult("Difference in frequency for '" + targetWord + "'", diff); // Outputs Query result 
    }
};

// Derived Class 3: Top-K Query
class TopKQueryProcessor : public QueryProcessor {
private:
    VersionedIndex& index;
    int k;
public:
    TopKQueryProcessor(VersionedIndex& idx, int k_val) : index(idx), k(k_val) {}
    void execute() override {
        cout << "Version: " << index.getVersionName() << "\n"; // Outputs Version name
        
        auto freqs = index.getAllFrequencies();
        vector<pair<string, int>> sortedWords(freqs.begin(), freqs.end());
        
        // Sort in descending order of frequency [cite: 49]
        sort(sortedWords.begin(), sortedWords.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first < b.first; 
        });

        cout << "Top " << k << " Words:\n";
        for (int i = 0; i < k && i < sortedWords.size(); ++i) {
            cout << i + 1 << ". " << sortedWords[i].first << " -> " << sortedWords[i].second << "\n"; // Outputs Query result 
        }
    }
};

// Main Helper Function to Build Index
void buildIndex(const string& filepath, size_t bufferSize, VersionedIndex& index) {
    BufferedFileReader reader(filepath, bufferSize);
    Tokenizer tokenizer;
    string chunk;
    
    while (!reader.isEOF()) {
        size_t bytesRead = reader.readChunk(chunk);
        if (bytesRead > 0) {
            tokenizer.tokenizeAndIndex(chunk, index, reader.isEOF());
        }
    }
}

// Main Execution
int main(int argc, char* argv[]) {
    try {
        auto start_time = high_resolution_clock::now();

        // Command line parsing mapping [cite: 79, 80]
        string file1, file2, version1, version2, queryType, word;
        int bufferKB = 0, topK = 0;

        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];
            if (arg == "--file" || arg == "--file1") file1 = argv[++i];
            else if (arg == "--file2") file2 = argv[++i];
            else if (arg == "--version" || arg == "--version1") version1 = argv[++i];
            else if (arg == "--version2") version2 = argv[++i];
            else if (arg == "--buffer") bufferKB = stoi(argv[++i]);
            else if (arg == "--query") queryType = argv[++i];
            else if (arg == "--word") word = argv[++i];
            else if (arg == "--top") topK = stoi(argv[++i]);
        }

        if (bufferKB < 256 || bufferKB > 1024) throw invalid_argument("Buffer must be 256-1024 KB.");

        QueryProcessor* processor = nullptr;

        if (queryType == "word" || queryType == "top") { // Single-version queries 
            VersionedIndex v1(version1);
            buildIndex(file1, bufferKB, v1);

            if (queryType == "word") processor = new WordQueryProcessor(v1, word); // Word query requirement 
            else if (queryType == "top") processor = new TopKQueryProcessor(v1, topK); // Top-K query requirement 
            
            processor->execute();
            delete processor;
        } 
        else if (queryType == "diff") { // Difference query requires two files 
            VersionedIndex v1(version1);
            VersionedIndex v2(version2);
            
            buildIndex(file1, bufferKB, v1);
            buildIndex(file2, bufferKB, v2);

            processor = new DiffQueryProcessor(v1, v2, word); // Computes difference in frequency 
            processor->execute();
            delete processor;
        } else {
            throw invalid_argument("Invalid query type.");
        }

        auto end_time = high_resolution_clock::now();
        double exec_time = duration_cast<duration<double>>(end_time - start_time).count();

        cout << "Allocated buffer size: " << bufferKB << " KB\n"; // Output requirement 
        cout << "Total execution time: " << exec_time << " seconds\n"; // Output requirement 

    } catch (const exception& e) { // Exception Handling 
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}