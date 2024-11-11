#include <bits/stdc++.h>
#include <mpi.h>
using namespace std;
using namespace std::chrono;

class Solution
{
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
    vector<string> ans;

    // This function is now modified to include MPI-related parallel processing
    void find(int x, int y, TrieNode *curr, vector<vector<char>> &board)
    {
        if (curr->word != nullptr)
        {
            ans.push_back(*(curr->word));
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
        return;
    }

public:
    vector<string> findWords(vector<vector<char>> &board, vector<string> &words, int rank, int size)
    {
        int row = board.size();
        int col = board[0].size();
        TrieNode *node = new TrieNode();
        for (auto &it : words)
        {
            node->addWord(it);
        }

        int start_row = (rank * row) / size;
        int end_row = ((rank + 1) * row) / size;

        for (int i = start_row; i < end_row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                if (node->children[board[i][j] - 'a'])
                    find(i, j, node->children[board[i][j] - 'a'], board);
            }
        }

        if (rank == 0)
        {
            vector<string> global_ans = ans;

            for (int i = 1; i < size; i++)
            {
                int local_size;
                MPI_Recv(&local_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                vector<string> temp(local_size);
                for (int j = 0; j < local_size; j++)
                {
                    int word_len;
                    MPI_Recv(&word_len, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    temp[j].resize(word_len);
                    MPI_Recv(&temp[j][0], word_len, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                global_ans.insert(global_ans.end(), temp.begin(), temp.end());
            }

            for (const auto &word : global_ans)
            {
                cout << word << " ";
            }
            cout << endl;
            return global_ans;
        }
        else
        {
            int local_size = ans.size();

            MPI_Send(&local_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

            for (int i = 0; i < local_size; i++)
            {
                int word_len = ans[i].size();

                MPI_Send(&word_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                MPI_Send(&ans[i][0], word_len, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }

        return ans;
    }
};

int main(int argc, char **argv)
{
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    freopen("Error.txt", "w", stderr);
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n, m, k;
    vector<vector<char>> board;
    vector<string> words;

    if (rank == 0)
    {
        cin >> n >> m;

        board.resize(n, vector<char>(m));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                cin >> board[i][j];

        cin >> k;

        words.resize(k);
        for (int i = 0; i < k; ++i)
            cin >> words[i];
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        board.resize(n, vector<char>(m));
        words.resize(k);
    }

    for (int i = 0; i < n; ++i)
        MPI_Bcast(&board[i][0], m, MPI_CHAR, 0, MPI_COMM_WORLD);

    for (int i = 0; i < k; ++i)
    {
        int word_length = (rank == 0) ? words[i].size() : 0;

        MPI_Bcast(&word_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank != 0)
        {
            words[i].resize(word_length);
        }

        MPI_Bcast(&words[i][0], word_length, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
    /// check output
    // for (int i = 0; i < size; i++)
    // {
    //     if (i == rank)
    //     {
    //         cout << "Process " << rank << " received data:" << endl;
    //         cout << "Board:" << endl;
    //         for (int i = 0; i < n; ++i)
    //         {
    //             for (int j = 0; j < m; ++j)
    //             {
    //                 cout << board[i][j] << " ";
    //             }
    //             cout << endl;
    //         }

    //         cout << "Words:" << endl;
    //         for (const string &word : words)
    //         {
    //             cout << word << endl;
    //         }
    //         cout << "-----------------------------------" << endl;
    //     }

    //     // Use a barrier to ensure each process waits for others
    //     MPI_Barrier(MPI_COMM_WORLD);
    // }
    Solution obj;
    auto start = high_resolution_clock::now();
    vector<string> ans = obj.findWords(board, words, rank, size);
    auto stop = high_resolution_clock::now();

    if (rank == 0)
    {
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

    MPI_Finalize();
    return 0;
}
