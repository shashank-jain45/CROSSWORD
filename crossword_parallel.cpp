    #include <bits/stdc++.h>
    #include <omp.h>
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
        omp_lock_t lock;

        void find(int x, int y, TrieNode *curr, vector<vector<char>> &board)
        {
            if (curr->word != nullptr)
            {
                omp_set_lock(&lock); 
                ans.push_back(*(curr->word));
                omp_unset_lock(&lock); 
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
        vector<string> findWords(vector<vector<char>> &board, vector<string> &words)
        {
            int row = board.size();
            int col = board[0].size();
            TrieNode *node = new TrieNode();

            for (auto &it : words)
            {
                node->addWord(it);
            }

            omp_init_lock(&lock);

    #pragma omp parallel shared(node)
            {
                vector<vector<char>> localBoard = board;

    #pragma omp for collapse(2) nowait
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < col; j++)
                    {
                        if (node->children[board[i][j] - 'a'])
                        {
                            find(i, j, node->children[board[i][j] - 'a'], localBoard);
                        }
                    }
                }
            }

            omp_destroy_lock(&lock);
            return ans;
        }
    };

    int main()
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
