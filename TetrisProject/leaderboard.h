#pragma once
// leaderboard.h  (C: CSV leaderboard storage)

#include <string>
#include <vector>
#include "score.h"

namespace tetris {

    struct LeaderboardEntry {
        std::string name;      // stored as UTF-8 (no commas; commas will be replaced)
        int score = 0;
        int lines = 0;
        int level = 0;
        long long timestamp = 0; // unix epoch seconds
    };

    // Initialize leaderboard storage.
    // path example: "leaderboard.csv"
    // keepN: keep top N records only
    void Leaderboard_Init(const std::string& path = "leaderboard.csv", int keepN = 10);

    // Submit a new record (auto timestamp if timestamp==0)
    // Returns true if saved successfully.
    bool Leaderboard_Submit(LeaderboardEntry e);

    // Convenience: submit using current ScoreState
    bool Leaderboard_Submit(const std::string& name, const ScoreState& s);

    // Load top K records (sorted by score desc, timestamp asc as tie-breaker)
    std::vector<LeaderboardEntry> Leaderboard_LoadTop(int topK = 10);

    // Clear the leaderboard file (dangerous; use carefully)
    bool Leaderboard_Clear();

} // namespace tetris