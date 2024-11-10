#include <mpi.h>
#include <bits/stdc++.h>
using namespace std;
int main(int argc, char *argv[])
{
    int size, myrank;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    cout << "Process of, Hello World" << myrank << " " << size << endl;

    MPI_Finalize();
    return 0;
}