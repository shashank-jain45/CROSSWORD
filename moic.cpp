#include <mpi.h>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
using namespace std;
// Converts vector<string> to vector<char>.
std::vector<char> string_to_char(const std::vector<std::string> &strings)
{
    std::vector<char> cstrings;
    for (std::string s : strings)
    {
        for (size_t i = 0; i < strlen(s.c_str()); ++i)
        {
            cstrings.push_back(s.c_str()[i]);
        }
        cstrings.push_back('$');
    }

    return cstrings;
}

int main(int argc, char *argv[])
{
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    freopen("Error.txt", "w", stderr);
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n, m, k;
    int len;
    vector<char> board;
    vector<string> words;
    std::vector<char> cstrings;
    if (rank == 0)
    {

        // Initialize `n`, `m`, and `k` and populate `board` and `words` in the root process
        cin >> n >> m;
        cout << " (n x m): " << n << " " << m << endl;

        // Initialize the board with some data
        board.resize(n * m);
        // cout << "Enter the board elements:" << endl;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                cin >> board[i * m + j];

        // cout << "Enter number of words (k): ";
        cin >> k;

        // Initialize words with some data
        words.resize(k);
        // cout << "Enter each word:" << endl;
        for (int i = 0; i < k; ++i)
            cin >> words[i];

        cstrings = string_to_char(words);
        len = cstrings.size();
    }

    // Broadcast the dimensions n, m, and k
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<int> mat_data;
    // Resize board and words in non-root processes after receiving dimensions
    if (rank != 0)
    {
        board.reserve(n * m);
        words.reserve(k);
        cstrings.reserve(len);
    }

    // Broadcast the entire board as a contiguous 1D array
    MPI_Bcast(cstrings.data(), len, MPI_CHAR, 0, MPI_COMM_WORLD);
    int j = 0; 
    for (int i = 0; i < k; i++)
    {
        while (1)
        {
            if (cstrings[j] == '$')
            {
                j++;
                break;
            }
            words[i].push_back(cstrings[j]);
            j++;
        }
    }
    //    MPI_Bcast(&board[0][0], n * m, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Each process can now use `board` and `words` as they have received the same data
    cout << "Process " << rank << " received data:" << endl;
    // cout << "Board:" << endl;
    // for (int i = 0; i < n; ++i)
    // {
    //     for (int j = 0; j < m; ++j)
    //     {
    //         cout << board[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    cout << "Words:" << endl;
    for (auto it : words)
    {
        cout << it << endl;
    }

    MPI_Finalize();
    return 0;
}