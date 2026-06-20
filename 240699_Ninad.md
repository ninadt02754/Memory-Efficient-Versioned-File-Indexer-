# CS253 Assignment 1: Memory-Efficient Versioned File Indexer

**Student Name:**  Ninad Tagade  
**Roll Number:** 240699

---

## Project Overview

This project is a C++ implementation of a memory-efficient versioned file indexer. It is designed to process large text-based log files incrementally using a fixed-size buffer. This ensures that the entire file—or large portions of it—are never loaded fully into memory at once, allowing the system's memory footprint to remain independent of the overall file size.

---

## Architecture and Class Design

The solution adheres strictly to object-oriented design principles and uses four primary classes to handle distinct responsibilities:

* **`BufferedFileReader`**: Responsible for buffered file reading. It enforces the minimum (256 KB) and maximum (1024 KB) buffer size constraints and handles incremental chunk reading.
* **`Tokenizer`**: Responsible for tokenization. It extracts words (defined as contiguous sequences of alphanumeric characters) and converts them to lowercase for case-insensitive matching. It also handles edge cases where tokens are split across buffer boundaries.
* **`VersionedIndex`**: Responsible for versioned indexing. It maintains a word-level index that maps each unique word to its frequency for a specific, user-identified version.
* **`QueryProcessor`**: Responsible for query processing. It interprets the constructed index and provides analytical results.

---

## C++ Features Demonstrated

The codebase implements several mandatory C++ features:

* **Inheritance \& Runtime Polymorphism**  
  The `QueryProcessor` class acts as an abstract base class. Three derived classes (`WordQueryProcessor`, `DiffQueryProcessor`, `TopKQueryProcessor`) override the pure virtual `execute()` function, demonstrating dynamic dispatch.
* **Function Overloading**  
  The `VersionedIndex` class implements overloaded versions of the `addWord()` function with different parameter lists.
* **Exception Handling**  
  Standard `try`, `catch`, and `throw` blocks are used to safely handle runtime errors such as invalid buffer sizes or inaccessible files.
* **Templates**  
  A user-defined template function (`printResult`) is used to format output dynamically for different data types.

---

## Compilation Instructions

The program should be compiled using a **C++** standard environment.

g++ -O3 240699_Ninad.cpp -o analyzer



---

## Command-Line Usage and Supported Queries

The program accepts configuration strictly through command-line arguments.

### 1\. Word Count Query

Returns the frequency of a given word in a specified version.

./analyzer --file dataset\_v1.txt --version v1 --buffer 512 --query word --word error



---

### 2\. Difference Query

Computes the frequency difference of a specific word between two versions using two input files.

./analyzer --file1 dataset\_v1.txt --version1 v1 --file2 dataset\_v2.txt --version2 v2 --buffer 512 --query diff --word error



---

### 3\. Top-K Query

Displays the **top-K most frequent words** in a specified version, sorted in descending order of frequency.

./analyzer --file dataset\_v1.txt --version v1 --buffer 512 --query top --top 10



---

## Output Format

Upon successful execution, the console outputs:

1. Version name(s)
2. Query result
3. Allocated buffer size (in KB)
4. Total execution time (in seconds)
