#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
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
  // Pre-computed lists of legal placements intersections up front.
  // Used for the optimization to quickly discard the placements of pieces that are becoming illegal
  // as other pieces are being placed.
  std::vector<std::vector<std::vector<std::vector<bool>>>> legal_placements_compatible(
      10, std::vector<std::vector<std::vector<bool>>>(10));
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
    // Now, the optimization: prepare the lists of intersections of legal placements up front.
    {
      for (int t1 = 0; t1 < 10; ++t1) {
        for (int t2 = 0; t2 < 10; ++t2) {
          if (t1 != t2) {
            legal_placements_compatible[t1][t2] = std::vector<std::vector<bool>>(
                legal_placements[t1].size(), std::vector<bool>(legal_placements[t2].size(), false));
          }
        }
      }
      std::vector<std::pair<int, int>> tmp;
      for (int t1 = 0; t1 < 10; ++t1) {
        for (int t2 = t1 + 1; t2 < 10; ++t2) {
          for (int k1 = 0; k1 < static_cast<int>(legal_placements[t1].size()); ++k1) {
            for (int k2 = 0; k2 < static_cast<int>(legal_placements[t2].size()); ++k2) {
              tmp.clear();
              std::set_intersection(std::begin(legal_placements[t1][k1]),
                                    std::end(legal_placements[t1][k1]),
                                    std::begin(legal_placements[t2][k2]),
                                    std::end(legal_placements[t2][k2]),
                                    std::back_inserter(tmp));
              if (tmp.empty()) {
                legal_placements_compatible[t1][t2][k1][k2] = true;
                legal_placements_compatible[t2][t1][k2][k1] = true;
              }
            }
          }
        }
      }
    }
  }
  std::vector<std::string> board(5);
  std::vector<bool> piece_left_to_be_placed(10, true);
  int cells_left_to_cover = 0;
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
          piece_left_to_be_placed[board[i][j] - 'A'] = false;
        } else {
          ++cells_left_to_cover;
        }
      }
    }
    // std::cerr << "Using " << std::count(std::begin(piece_left_to_be_placed), std::end(piece_left_to_be_placed), true) << "
    // pieces.\n";
  }
  // Prepare the data structures to keep track of which positions are still legal for which pieces.
  std::vector<int> legal_placements_active_count(10);
  std::vector<std::vector<int>> legal_placements_indexes(10);
  for (int t = 0; t < 10; ++t) {
    legal_placements_active_count[t] = piece_left_to_be_placed[t] ? static_cast<int>(legal_placements[t].size()) : 0;
    for (int k = 0; k < legal_placements_active_count[t]; ++k) {
      legal_placements_indexes[t].push_back(k);
    }
    // Eliminate certain placements of this piece if they do not fit the original problem setup.
    legal_placements_active_count[t] = std::partition(std::begin(legal_placements_indexes[t]),
                                                      std::end(legal_placements_indexes[t]),
                                                      [&](int k) {
                                                        for (auto const& cell : legal_placements[t][k]) {
                                                          if (board[cell.first][cell.second] != '.') {
                                                            return false;
                                                          }
                                                        }
                                                        return true;
                                                      }) -
                                       std::begin(legal_placements_indexes[t]);
  }
  // See https://en.wikipedia.org/wiki/Knuth%27s_Algorithm_X` and https://en.wikipedia.org/wiki/Dancing_Links for more.
  int total_solutions = 0;
  std::function<void()> const Recursion = [&]() {
    if (!cells_left_to_cover) {
      // The solution is found.
      for (int i = 0; i < 5; ++i) {
        std::cout << board[i] << std::endl;
      }
      std::cout << std::endl;
      ++total_solutions;
    } else {
      // Find the piece, among those left to be placed, with the fewest possible number of ways to place it.
      int next_t = -1;
      for (int t = 0; t < 10; ++t) {
        if (piece_left_to_be_placed[t]) {
          next_t = t;
          break;
        }
      }
      ASSERT(next_t != -1);
      for (int t = next_t + 1; t < 10; ++t) {
        if (piece_left_to_be_placed[t] && legal_placements_active_count[t] < legal_placements_active_count[next_t]) {
          next_t = t;
        }
      }
      // Try all placements of the piece `next_t`.
      char const next_c = 'A' + next_t;
      piece_left_to_be_placed[next_t] = false;
      auto const save_legal_placements_active_count = legal_placements_active_count;
      int const cells_in_next_piece = static_cast<int>(legal_placements[next_t].front().size());
      ASSERT(cells_in_next_piece == 5);  // NOTE(dkorolev): Unnecessary, a sanity check.
      cells_left_to_cover -= cells_in_next_piece;
      for (int next_z = 0; next_z < legal_placements_active_count[next_t]; ++next_z) {
        int const next_k = legal_placements_indexes[next_t][next_z];
        for (auto const& cell : legal_placements[next_t][next_k]) {
          ASSERT(board[cell.first][cell.second] == '.');
          board[cell.first][cell.second] = next_c;
        }
        for (int other_t = 0; other_t < 10; ++other_t) {
          if (piece_left_to_be_placed[other_t]) {
            legal_placements_active_count[other_t] =
                std::partition(std::begin(legal_placements_indexes[other_t]),
                               std::begin(legal_placements_indexes[other_t]) + legal_placements_active_count[other_t],
                               [&](int other_k) { return legal_placements_compatible[next_t][other_t][next_k][other_k]; }) -
                std::begin(legal_placements_indexes[other_t]);
          }
        }
        Recursion();
        for (auto const& cell : legal_placements[next_t][next_k]) {
          board[cell.first][cell.second] = '.';
        }
        for (int other_t = 0; other_t < 10; ++other_t) {
          if (piece_left_to_be_placed[other_t]) {
            legal_placements_active_count[other_t] = save_legal_placements_active_count[other_t];
          }
        }
      }
      piece_left_to_be_placed[next_t] = true;
      cells_left_to_cover += cells_in_next_piece;
    }
  };
  Recursion();
  std::cout << "Total solutions: " << total_solutions << std::endl;
}
