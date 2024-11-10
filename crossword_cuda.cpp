#include <bits/stdc++.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
using namespace std;
using namespace std::chrono;

#define MAX_WORD_LEN 50  // Assuming max word length of 50 for simplicity
#define ALPHABET_SIZE 26 // Size of the English alphabet

struct TrieNode
{
    TrieNode *children[ALPHABET_SIZE];
    bool isEndOfWord;

    __device__ TrieNode() : isEndOfWord(false)
    {
        for (int i = 0; i < ALPHABET_SIZE; ++i)
            children[i] = nullptr;
    }

    __device__ void addWord(const char *word)
    {
        TrieNode *curr = this;
        for (int i = 0; word[i] != '\0'; ++i)
        {
            int index = word[i] - 'a';
            if (curr->children[index] == nullptr)
            {
                curr->children[index] = new TrieNode();
            }
            curr = curr->children[index];
        }
        curr->isEndOfWord = true;
    }
};

// Kernel function for performing DFS on the board
__global__ void findWordsKernel(TrieNode *trieRoot, char *board, int rows, int cols, char *words, int *wordLengths, int numWords, int *result, int maxWordLen)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= rows * cols)
        return;

    int x = idx / cols;
    int y = idx % cols;
    int stackSize = maxWordLen * sizeof(int);
    char path[MAX_WORD_LEN];
    bool visited[50][50] = {false}; // Assuming max board size is 50x50
    int top = -1;
    TrieNode *curr = trieRoot;

    // DFS stack-based implementation to avoid recursion on GPU
    // Code for DFS search logic goes here.
    // Use path array to store temporary path found on board.
}

// Host function to prepare data and launch kernel
vector<string> findWordsCUDA(vector<vector<char>> &board, vector<string> &words)
{
    int rows = board.size();
    int cols = board[0].size();

    // Flatten board into 1D array for CUDA
    char *d_board;
    vector<char> h_board;
    for (auto &row : board)
        h_board.insert(h_board.end(), row.begin(), row.end());

    cudaMalloc(&d_board, h_board.size() * sizeof(char));
    cudaMemcpy(d_board, h_board.data(), h_board.size() * sizeof(char), cudaMemcpyHostToDevice);

    // Copy words to device memory as a 1D character array
    int maxWordLen = 0;
    for (auto &word : words)
        maxWordLen = max(maxWordLen, (int)word.size());

    char *d_words;
    int *d_wordLengths;
    vector<int> wordLengths(words.size());
    for (int i = 0; i < words.size(); ++i)
        wordLengths[i] = words[i].size();

    cudaMalloc(&d_words, maxWordLen * words.size() * sizeof(char));
    cudaMalloc(&d_wordLengths, words.size() * sizeof(int));
    cudaMemcpy(d_wordLengths, wordLengths.data(), words.size() * sizeof(int), cudaMemcpyHostToDevice);

    // Flatten Trie on device (alternative approach)
    TrieNode *d_trieRoot;
    cudaMalloc(&d_trieRoot, sizeof(TrieNode));
    TrieNode *h_trieRoot = new TrieNode();
    for (auto &word : words)
        h_trieRoot->addWord(word.c_str());
    cudaMemcpy(d_trieRoot, h_trieRoot, sizeof(TrieNode), cudaMemcpyHostToDevice);

    int *d_result;
    vector<int> h_result(words.size(), 0); // Array to store found word indices
    cudaMalloc(&d_result, words.size() * sizeof(int));
    cudaMemcpy(d_result, h_result.data(), words.size() * sizeof(int), cudaMemcpyHostToDevice);

    // Launch kernel
    int totalThreads = rows * cols;
    int blockSize = 256;
    int numBlocks = (totalThreads + blockSize - 1) / blockSize;
    findWordsKernel<<<numBlocks, blockSize>>>(d_trieRoot, d_board, rows, cols, d_words, d_wordLengths, words.size(), d_result, maxWordLen);

    // Copy result back to host
    cudaMemcpy(h_result.data(), d_result, words.size() * sizeof(int), cudaMemcpyDeviceToHost);

    // Gather found words
    vector<string> resultWords;
    for (int i = 0; i < words.size(); ++i)
    {
        if (h_result[i] == 1)
        {
            resultWords.push_back(words[i]);
        }
    }

    // Cleanup
    cudaFree(d_board);
    cudaFree(d_words);
    cudaFree(d_wordLengths);
    cudaFree(d_result);
    cudaFree(d_trieRoot);
    delete h_trieRoot;

    return resultWords;
}

int main()
{
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    int n, m;
    cin >> n >> m;
    vector<vector<char>> board(n, vector<char>(m));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            cin >> board[i][j];

    int k;
    cin >> k;
    vector<string> words(k);
    for (int i = 0; i < k; ++i)
        cin >> words[i];

    auto start = high_resolution_clock::now();
    vector<string> result = findWordsCUDA(board, words);
    auto stop = high_resolution_clock::now();

    cout << "The words are: ";
    for (auto &word : result)
        cout << word << " ";
    cout << endl;

    cout << "Time taken: " << duration_cast<milliseconds>(stop - start).count() << " ms" << endl;
}
