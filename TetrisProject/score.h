#pragma once
// score.h  (C: Score/Level/Lines module)

namespace tetris {

    struct ScoreState {
        int score = 0;
        int lines = 0;
        int level = 0;
    };

    // Reset score/lines/level to 0
    void Score_Reset();

    // Call this when lines are cleared (n = 0..4 typically)
    void Score_OnLinesCleared(int n);

    // Optional: scoring for soft/hard drop (cells = how many cells dropped)
    void Score_OnSoftDrop(int cells);
    void Score_OnHardDrop(int cells);

    // Read current score state
    ScoreState Score_GetState();

    // Suggested falling interval (ms) based on level (B can use this to update timer)
    int Score_GetDropIntervalMs(int level);

} // namespace tetris