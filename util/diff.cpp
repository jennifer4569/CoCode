#include <iostream>
#include <list>
#include <fstream>

int edit_distance(std::string s1, std::string s2, int m, int n) {
    if (m == 0) {
        return n;
    } else if (n == 0) {
        return m;
    }

    if (s1[m - 1] == s2[n - 1]) {
        return edit_distance(s1, s2, m - 1, n - 1);
    }
    return 1 + std::min(edit_distance(s1, s2, m - 1, n - 1),
                        edit_distance(s1, s2, m - 1, n),
                        edit_distance(s1, s2, m, n - 1));
}

double diff_percent(std::string s1, std::string s2) {
    return edit_distance(s1, s2, s1.size(), s2.size()) / (double)s1.size();
}

void file_to_list(std::ifstream file, std::list<std::string>& l) {
    std::string line;
    while (file >> line) {
        l.push_back(line);
    }
}