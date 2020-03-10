#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#define ASSERT(x)                                                             \
  if (!(x)) {                                                                 \
    std::cerr << __FILE__ << ':' << __LINE__ << ": `" << #x << "` failed.\n"; \
    std::exit(-1);                                                            \
  }

constexpr static const char* const kPiecesFile = "pieces.txt";
constexpr static const char* const kProblemFile = "problem.txt";

int main(int argc, char** argv) {
  // `legal_placements[t]` lists all possible legal ways to place piece `t` onto the board.
  std::vector<std::vector<std::vector<std::pair<int, int>>>> legal_placements(10);
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
    // Fill the `legal_placements` data structure.
    for (int t = 0; t < 10; ++t) {
      std::set<std::set<std::pair<int, int>>> placements_set;  // To exclude duplicate placements.

      char const c = 'A' + t;
      std::set<std::pair<int, int>> cells;
      for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 10; ++j) {
          if (pieces[i][j] == c) {
            cells.insert({i, j});
          }
        }
      }
      auto const Mark = [&]() {
        // Mark all possible legal placements of the piece in its current orientation.
        // Assume `X`-s and `O`-x are in the right places.
        for (int di = -22; di <= +22; ++di) {
          for (int dj = -22; dj <= +22; ++dj) {
            if (((di + dj) & 1) == 0) {  // Respect parity for `X`-s and `O`-s.
              bool ok = true;
              std::set<std::pair<int, int>> placement;
              for (auto const& cell : cells) {
                int const i = cell.first + di;
                int const j = cell.second + dj;
                if (i >= 0 && i < 5 && j >= 0 && j < 10) {
                  placement.insert({i, j});
                } else {
                  ok = false;
                }
              }
              if (ok) {
                if (!placements_set.count(placement)) {
                  placements_set.insert(placement);
                  legal_placements[t].push_back(
                      std::vector<std::pair<int, int>>(std::begin(placement), std::end(placement)));
#if 0
                  // NOTE(dkorolev): Visualize all possible placements for this piece.
                  if (c == 'E') {
                    std::vector<std::string> tmp(5, std::string(10, '.'));
                    for (auto const& cell : placement) {
                      tmp[cell.first][cell.second] = c;
                    }
                    for (int i = 0; i < 5; ++i) {
                      std::cerr << tmp[i] << std::endl;
                    }
                    std::cerr << std::endl;
                  }
#endif
                }
              }
            }
          }
        }
      };
      auto const Rotate90 = [&]() {
        std::set<std::pair<int, int>> new_cells;
        for (auto const& cell : cells) {
          new_cells.insert({-cell.second, cell.first});
        }
        cells = std::move(new_cells);
      };
      auto const FlipOver = [&]() {
        std::set<std::pair<int, int>> new_cells;
        for (auto const& cell : cells) {
          // `+1` to swap `X`-s and `O`-s when flipping the piece over.
          new_cells.insert({cell.second, cell.first + 1});
        }
        cells = std::move(new_cells);
      };
      // Mark all rotations and flips.
      Mark();
      Rotate90();
      Mark();
      Rotate90();
      Mark();
      Rotate90();
      Mark();
      FlipOver();
      Mark();
      Rotate90();
      Mark();
      Rotate90();
      Mark();
      Rotate90();
      Mark();
      // std::cerr << "Piece `" << c << "`: " << legal_placements[t].size() << " legal placements.\n";
    }
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
