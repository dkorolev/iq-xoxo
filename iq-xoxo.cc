#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define ASSERT(x) if (!(x)) { std::cerr << __FILE__ << ':' << __LINE__ << ": `" << #x << "` failed.\n"; std::exit(-1); }

constexpr static const char* const kPiecesFile = "pieces.txt";
constexpr static const char* const kProblemFile = "problem.txt";

int main(int argc, char** argv) {
  // Read and parse the pieces file.
  {
    std::ifstream fi_pieces(kPiecesFile);
    ASSERT(fi_pieces.good());
    std::vector<std::string> pieces(5);
    std::string row;
    for (int i = 0; i < 5; ++i) {
      ASSERT(fi_pieces >> pieces[i]);
      ASSERT(pieces[i].length() == 10);
    }
    std::vector<int> counts(10);
    for (int i = 0; i < 5; ++i) {
      for (int j = 0; j < 10; ++j) {
        ASSERT(pieces[i][j] >= 'A' && pieces[i][j] <= 'J');
        ++counts[pieces[i][j] - 'A'];
      }
    }
    ASSERT(counts == std::vector<int>(10, 5));  // All pieces are of five cells.
    // NOTE(dkorolev): This does not check whether each piece is continuous, trust the contents of `kPiecesFile`.
  }
  std::vector<std::string> board(5);
  std::vector<bool> allowed_pieces(10, true);
  // Read and parse the problem file.
  {
    // NOTE(dkorolev): This does not check whether the right letters were used for the pieces that form the problem.
    std::ifstream fi_problem(argc >= 2 ? argv[1] : kProblemFile);
    ASSERT(fi_problem.good());
    std::string row;
    for (int i = 0; i < 5; ++i) {
      ASSERT(fi_problem >> board[i]);
      ASSERT(board[i].length() == 10);
    }
    for (int i = 0; i < 5; ++i) {
      for (int j = 0; j < 10; ++j) {
        ASSERT(board[i][j] == '.' || board[i][j] >= 'A' && board[i][j] <= 'J');
        if (board[i][j] != '.') {
          allowed_pieces[board[i][j] - 'A'] = false;
        }
      }
    }
    // std::cerr << "Using " << std::count(std::begin(allowed_pieces), std::end(allowed_pieces), true) << " pieces.\n";
  }
}
