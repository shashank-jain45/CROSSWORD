
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

    vector<string> ans_local;

    void find(int x, int y, TrieNode *curr, vector<vector<char>> &board)
    {
        if (curr->word != nullptr)
        {
            ans_local.push_back(*(curr->word));
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
                find(nx, ny, curr->children[board[nx][ny] - 'a'], board);
                board[x][y] = orig;
            }
        }
    }

    vector<string> findWords(vector<vector<char>> &board, vector<string> &words)
    {
        int row = board.size();
        int col = board[0].size();
        TrieNode *node = new TrieNode();

        // Build Trie
        for (auto &it : words)
        {
            node->addWord(it);
        }

        // Initialize MPI
        int rank, size;
        MPI_Init(nullptr, nullptr);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        cout << "SIZE:" << size << endl;
        cout << "RANK" << rank << endl;
        // Determine work distribution
        int rows_per_process = row / size;
        int start_row = rank * rows_per_process;
        int end_row = (rank == size - 1) ? row : start_row + rows_per_process;

        // Each process works on its own rows
        for (int i = start_row; i < end_row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                if (node->children[board[i][j] - 'a'])
                {
                    find(i, j, node->children[board[i][j] - 'a'], board);
                }
            }
        }

        // Gather results at the root process
        vector<string> ans_global;
        if (rank == 0)
        {
            // Insert local results from rank 0
            ans_global.insert(ans_global.end(), ans_local.begin(), ans_local.end());

            // Receive data from other processes
            for (int i = 1; i < size; i++)
            {
                int recv_size;
                MPI_Recv(&recv_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Calculate the total number of bytes needed to receive all strings
                vector<char> buffer;
                for (int j = 0; j < recv_size; j++)
                {
                    int str_len;
                    MPI_Recv(&str_len, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // Now allocate a buffer for the string content
                    vector<char> str_buffer(str_len);
                    MPI_Recv(str_buffer.data(), str_len, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    ans_global.push_back(string(str_buffer.begin(), str_buffer.end()));
                }
            }
        }
        else
        {
            // Send the local results from other processes
            int send_size = ans_local.size();
            MPI_Send(&send_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

            // Send each string length followed by its content
            for (const string &str : ans_local)
            {
                int str_len = str.size();
                MPI_Send(&str_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                MPI_Send(str.data(), str_len, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }

        // Finalize MPI
        MPI_Finalize();

        // Only root process returns the result
        if (rank == 0)
        {
            cout << rank << endl;
            return ans_global;
        }
        else
        {
            cout << rank << endl;
            return {}; // Other processes return an empty vector
        }
    }
};
int main(int argc, char **argv)
{

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
}