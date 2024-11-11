#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

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
    vector<vector<char>> board;
    vector<string> words;

    if (rank == 0)
    {
        // Initialize `n`, `m`, and `k` and populate `board` and `words` in the root process
        cout << "Enter dimensions of the board (n x m): ";
        cin >> n >> m;

        // Initialize the board with some data
        board.resize(n, vector<char>(m));
        cout << "Enter the board elements:" << endl;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                cin >> board[i][j];

        cout << "Enter number of words (k): ";
        cin >> k;

        // Initialize words with some data
        words.resize(k);
        cout << "Enter each word:" << endl;
        for (int i = 0; i < k; ++i)
            cin >> words[i];
    }

    // Broadcast the dimensions n, m, and k
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Resize board and words in non-root processes after receiving dimensions
    if (rank != 0)
    {
        board.resize(n, vector<char>(m));
        words.resize(k);
    }

    // Broadcast the entire board as a contiguous 1D array

    for (int i = 0; i < n; ++i)
        MPI_Bcast(&board[i][0], m, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Broadcast each word as a contiguous string

    for (int i = 0; i < k; ++i)
    {
        int word_length = (rank == 0) ? words[i].size() : 0;

        // Broadcast the length of each word first
        MPI_Bcast(&word_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Resize each word in non-root processes to match the length
        if (rank != 0)
        {
            words[i].resize(word_length);
        }

        // Broadcast the word itself
        MPI_Bcast(&words[i][0], word_length, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    // Each process can now use `board` and `words` as they have received the same data
    for (int i = 0; i < size; i++)
    {
        if (i == rank)
        {
            cout << "Process " << rank << " received data:" << endl;
            cout << "Board:" << endl;
            for (int i = 0; i < n; ++i)
            {
                for (int j = 0; j < m; ++j)
                {
                    cout << board[i][j] << " ";
                }
                cout << endl;
            }

            cout << "Words:" << endl;
            for (const string &word : words)
            {
                cout << word << endl;
            }
            cout << "-----------------------------------" << endl;
        }

        // Use a barrier to ensure each process waits for others
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
