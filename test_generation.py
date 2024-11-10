import random
import string

def generate_random_board(n, m):
    """Generates a random n x m board of lowercase letters."""
    board = []
    for _ in range(n):
        row = [random.choice(string.ascii_lowercase) for _ in range(m)]
        board.append(row)
    return board

def generate_random_words(k, max_word_len, board):
    """Generates a list of k random words, optionally from the board letters."""
    words = []
    for _ in range(k):
        word_length = random.randint(5, max_word_len)
        word = ''.join(random.choice(string.ascii_lowercase) for _ in range(word_length))
        words.append(word)
    return words

def save_test_case(filename, n, m, board, k, words):
    """Saves the test case to the input file."""
    with open(filename, 'w') as f:
        # Write board dimensions
        f.write(f"{n} {m}\n")
        
        # Write the board
        for row in board:
            f.write(" ".join(row) + "\n")
        
        # Write the number of words
        f.write(f"{k}\n")
        
        # Write the list of words
        for word in words:
            f.write(f"{word}\n")

def main():
    # Parameters for the test case generation
    n = 300  # Number of rows in the board
    m = 300  # Number of columns in the board
    k = 1000   # Number of words to generate
    max_word_len = 100  # Maximum word length

    # Generate random board and words
    board = generate_random_board(n, m)
    words = generate_random_words(k, max_word_len, board)

    # Save to input.txt
    save_test_case('input.txt', n, m, board, k, words)

    print("Test case generated and saved to input.txt")

if __name__ == "__main__":
    main()
