#include <bits/stdc++.h>
#include <mpi.h>
using namespace std;
using namespace std::chrono;

class Solution
{
public:
    struct TrieNode
    {
        TrieNode *children[26] = {};
        string *word;
        void addWord(string &word)
        {
            TrieNode *curr = this;
            for (auto ch : word)
            {
                ch -= 'a';
                if (curr->children[ch] == nullptr)
                    curr->children[ch] = new TrieNode();
                curr = curr->children[ch];
            }
            curr->word = &word;
        }
    };

    void find(int x, int y, TrieNode *curr, vector<vector<char>> &board, vector<string> &localAns)
    {
        if (curr->word != nullptr)
        {
            localAns.push_back(*(curr->word)); // Collect found words in localAns
            curr->word = nullptr;
        }

        int xm[4] = {0, 0, 1, -1};
        int ym[4] = {1, -1, 0, 0};
        int row = board.size();
        int col = board[0].size();

        for (int i = 0; i < 4; i++)
        {
            int nx = x + xm[i];
            int ny = y + ym[i];
            if (nx >= 0 && nx < row && ny >= 0 && ny < col && board[nx][ny] != '*' &&
                curr->children[board[nx][ny] - 'a'] != nullptr)
            {
                char orig = board[x][y];
                board[x][y] = '*';
                find(nx, ny, curr->children[board[nx][ny] - 'a'], board, localAns);
                board[x][y] = orig;
            }
        }
        return;
    }

    vector<string> findWords(vector<vector<char>> &board, vector<string> &words)
    {
        int rank, size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        int row = board.size();
        int col = board[0].size();
        TrieNode *node = new TrieNode();

        for (auto &it : words)
        {
            node->addWord(it);
        }

        // Divide rows of the board among the processes
        int rows_per_proc = row / size;
        int start_row = rank * rows_per_proc;
        int end_row = (rank == size - 1) ? row : start_row + rows_per_proc;

        vector<string> localAns; // Store found words locally in each process

        // Each process searches for words in its assigned part of the board
        for (int i = start_row; i < end_row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                if (node->children[board[i][j] - 'a'])
                {
                    find(i, j, node->children[board[i][j] - 'a'], board, localAns);
                }
            }
        }

        // Gather results from all processes to the root process
        int localSize = localAns.size();
        vector<int> sizes(size);
        MPI_Gather(&localSize, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

        vector<string> globalAns;
        if (rank == 0)
        {
            int totalSize = 0;
            for (int s : sizes)
                totalSize += s;

            globalAns.resize(totalSize);

            vector<int> displs(size);
            displs[0] = 0;
            for (int i = 1; i < size; i++)
            {
                displs[i] = displs[i - 1] + sizes[i - 1];
            }

            MPI_Gatherv(localAns.data(), localSize, MPI_CHAR,
                        globalAns.data(), sizes.data(), displs.data(), MPI_CHAR, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Gatherv(localAns.data(), localSize, MPI_CHAR,
                        NULL, NULL, NULL, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        return globalAns;
    }
};

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    freopen("Error.txt", "w", stderr);

    int n, m;
    cin >> n >> m;
    vector<vector<char>> board(n, vector<char>(m));
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            cin >> board[i][j];
        }
    }

    vector<string> words;
    int k;
    cin >> k;
    for (int i = 0; i < k; i++)
    {
        string s;
        cin >> s;
        words.push_back(s);
    }

    Solution obj;
    auto start = high_resolution_clock::now();
    vector<string> ans = obj.findWords(board, words);

    auto stop = high_resolution_clock::now();

    cout << "The words are: ";
    for (auto it : ans)
    {
        cout << it << " ";
    }
    cout << endl;

    cout << "Length: " << ans.size() << endl;

    auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Time taken by function: " << duration.count() << " milliseconds" << endl;
    cout << endl;
    MPI_Finalize();
}
