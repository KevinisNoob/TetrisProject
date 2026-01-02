// score.cpp  (C: Score/Level/Lines module)
#include "score.h"
#include <algorithm>

namespace tetris {

    static ScoreState g_state;

    // Standard-ish guideline base scores for single/double/triple/tetris
    // multiplied by (level + 1)
    static constexpr int kBase[5] = { 0, 100, 300, 500, 800 };

    void Score_Reset() {
        g_state = ScoreState{};
    }

    void Score_OnLinesCleared(int n) {
        n = std::clamp(n, 0, 4);

        // add score
        g_state.score += kBase[n] * (g_state.level + 1);

        // update lines/level
        g_state.lines += n;
        g_state.level = g_state.lines / 10;
    }

    void Score_OnSoftDrop(int cells) {
        if (cells <= 0) return;
        // common scoring: +1 per cell
        g_state.score += cells * 1;
    }

    void Score_OnHardDrop(int cells) {
        if (cells <= 0) return;
        // common scoring: +2 per cell
        g_state.score += cells * 2;
    }

    ScoreState Score_GetState() {
        return g_state;
    }

    // B can use this to change SetTimer interval.
    // Keep it simple and safe: faster with higher level, but not too fast.
    int Score_GetDropIntervalMs(int level) {
        if (level < 0) level = 0;

        // Example curve: start 500ms, minus 35ms per level, clamp to >= 60ms
        int ms = 500 - level * 35;
        if (ms < 60) ms = 60;
        return ms;
    }

} // namespace tetris