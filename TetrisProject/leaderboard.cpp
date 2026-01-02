// leaderboard.cpp  (C: CSV leaderboard storage)
#include "leaderboard.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace tetris {

    static std::string g_path = "leaderboard.csv";
    static int g_keepN = 10;

    static long long NowEpochSeconds() {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

    static std::string SanitizeName(std::string s) {
        // CSV-safe minimal sanitize: replace commas and newlines
        for (char& c : s) {
            if (c == ',' || c == '\n' || c == '\r' || c == '\t') c = ' ';
        }
        // trim leading/trailing spaces (lightweight)
        while (!s.empty() && s.front() == ' ') s.erase(s.begin());
        while (!s.empty() && s.back() == ' ') s.pop_back();
        if (s.empty()) s = "Player";
        return s;
    }

    static bool ParseLine(const std::string& line, LeaderboardEntry& out) {
        // expected: name,score,lines,level,timestamp
        std::stringstream ss(line);
        std::string token;

        if (!std::getline(ss, token, ',')) return false;
        out.name = token;

        if (!std::getline(ss, token, ',')) return false;
        out.score = std::atoi(token.c_str());

        if (!std::getline(ss, token, ',')) return false;
        out.lines = std::atoi(token.c_str());

        if (!std::getline(ss, token, ',')) return false;
        out.level = std::atoi(token.c_str());

        if (!std::getline(ss, token, ',')) return false;
        out.timestamp = std::atoll(token.c_str());

        return true;
    }

    static std::string ToLine(const LeaderboardEntry& e) {
        std::ostringstream os;
        os << e.name << ","
            << e.score << ","
            << e.lines << ","
            << e.level << ","
            << e.timestamp;
        return os.str();
    }

    void Leaderboard_Init(const std::string& path, int keepN) {
        g_path = path.empty() ? "leaderboard.csv" : path;
        g_keepN = (keepN <= 0) ? 10 : keepN;
    }

    std::vector<LeaderboardEntry> Leaderboard_LoadTop(int topK) {
        std::vector<LeaderboardEntry> all;
        std::ifstream ifs(g_path);
        if (!ifs.is_open()) {
            return all; // file missing is OK => empty leaderboard
        }

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            LeaderboardEntry e;
            if (ParseLine(line, e)) {
                // minimal sanity
                e.name = SanitizeName(e.name);
                all.push_back(e);
            }
        }

        // sort by score desc, tie-break by timestamp asc (older first)
        std::sort(all.begin(), all.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            if (a.score != b.score) return a.score > b.score;
            return a.timestamp < b.timestamp;
            });

        if (topK <= 0) topK = 10;
        if ((int)all.size() > topK) all.resize((size_t)topK);

        return all;
    }

    static bool SaveAll(const std::vector<LeaderboardEntry>& all) {
        std::ofstream ofs(g_path, std::ios::trunc);
        if (!ofs.is_open()) return false;

        for (const auto& e : all) {
            ofs << ToLine(e) << "\n";
        }
        return true;
    }

    bool Leaderboard_Submit(LeaderboardEntry e) {
        e.name = SanitizeName(e.name);
        if (e.timestamp == 0) e.timestamp = NowEpochSeconds();

        // load existing
        auto all = Leaderboard_LoadTop(999999);

        // append new
        all.push_back(e);

        // sort & keep top N
        std::sort(all.begin(), all.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            if (a.score != b.score) return a.score > b.score;
            return a.timestamp < b.timestamp;
            });

        if ((int)all.size() > g_keepN) all.resize((size_t)g_keepN);

        return SaveAll(all);
    }

    bool Leaderboard_Submit(const std::string& name, const ScoreState& s) {
        LeaderboardEntry e;
        e.name = name;
        e.score = s.score;
        e.lines = s.lines;
        e.level = s.level;
        e.timestamp = 0; // auto now
        return Leaderboard_Submit(e);
    }

    bool Leaderboard_Clear() {
        std::ofstream ofs(g_path, std::ios::trunc);
        return ofs.is_open();
    }

} // namespace tetris