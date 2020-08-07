#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <tuple>
#include <vector>

#define DIFF_THRESHOLD 0.2

// get the edit distance between two strings
int line_diff(std::string s1, std::string s2, int m, int n) {
    if (m == 0) {
        return n;
    } else if (n == 0) {
        return m;
    }

    if (s1[m - 1] == s2[n - 1]) {
        return line_diff(s1, s2, m - 1, n - 1);
    }
    return 1 + std::min(line_diff(s1, s2, m - 1, n - 1),
                        std::min(line_diff(s1, s2, m - 1, n),
                                 line_diff(s1, s2, m, n - 1)));
}

// get the proportional edit distance
double diff_percent(std::string s1, std::string s2) {
    return line_diff(s1, s2, s1.size(), s2.size()) / (double)s1.size();
}

// read a file line-by-line into a vector
void file_to_vector(std::ifstream& file, std::vector<std::string>& v) {
    std::string line;
    while (std::getline(file, line)) {
        v.push_back(line);
    }
}

// calculate the sum of a tuple of three integers
int tuple_sum(std::tuple<int, int, int> t) {
    return std::get<0>(t) + std::get<1>(t) + std::get<2>(t);
}

/*
 * fill the table of tuples with edit distance values
 * the tuple formatting is as follows:
 * 0: number of substitutions
 * 1: number of lines added in the new file
 * 2: number of lines removed from the old file
 */
void populate_table(
    std::vector<std::vector<std::tuple<int, int, int> > >& table,
    const std::vector<std::string>& v1, const std::vector<std::string>& v2) {
    for (int i = 1; i < table.size(); i++) {
        for (int j = 1; j < table[0].size(); j++) {
            // if the lines are the same, there are no edits
            if (v1[i - 1] == v2[j - 1]) {
                table[i].push_back(table[i - 1][j - 1]);
                continue;
            }

            // get the edit distances of the three possible edits
            std::tuple<int, int, int> sub_tuple = table[i - 1][j - 1];
            std::tuple<int, int, int> add_tuple = table[i][j - 1];
            std::tuple<int, int, int> rem_tuple = table[i - 1][j];
            int sub = tuple_sum(sub_tuple);
            int add = tuple_sum(add_tuple);
            int rem = tuple_sum(rem_tuple);

            // get the minimum edit distance
            int min = std::min(sub, std::min(add, rem));

            // handle substitutions
            if (min == sub) {
                int s = std::get<0>(sub_tuple) + 1;
                int a = std::get<1>(sub_tuple);
                int r = std::get<2>(sub_tuple);

                table[i].push_back(std::tuple<int, int, int>(s, a, r));
            }
            // handle additions
            else if (min == add) {
                int s = std::get<0>(add_tuple);
                int a = std::get<1>(add_tuple) + 1;
                int r = std::get<2>(add_tuple);

                table[i].push_back(std::tuple<int, int, int>(s, a, r));
            }
            // handle deletions
            else {
                int s = std::get<0>(rem_tuple);
                int a = std::get<1>(rem_tuple);
                int r = std::get<2>(rem_tuple) + 1;

                table[i].push_back(std::tuple<int, int, int>(s, a, r));
            }
        }
    }
}

/*
 * find all of the edits made to the file and store them in the given list
 * the algorithm for each cell (m, n) is as follows:
 * while m or n still have positive values:
    * if m == 0, then we have reached a point where there are only additions left
    * if n == 0, then we have reached a point where there are only deletions left
    * get the minimum value of the tuples up, left, and diagonally left-up
        * if the cell diagonally left-up is the smallest...
            * if the value of (m-1, n-1) is the same as (m, n), there was no edit
            * if the value of (m-1, n-1) the smaller than (m, n), a substitution was made
        * if the cell left is the smallest, then an addition of v2[n] was made
        * if the cell up is the smallest, then a deletion of v1[m] was made
    * each edit is pushed to the front of the list
 */
void collect_changes(
    std::vector<std::vector<std::tuple<int, int, int> > >& table,
    const std::vector<std::string>& v1, const std::vector<std::string>& v2,
    std::list<std::string>& differences) {
    int m = v1.size(), n = v2.size();

    while (m > 0 || n > 0) {
        // string stream to hold the current edit, if there is one
        std::stringstream str;

        // if we're on the first row, the remaining edits are additions
        if (m == 0) {
            while (n > 0) {
                str << "+ " << v2[n--];
            }
            differences.push_front(str.str());
            break;
        }

        // if we're on the first column, the remaining edits are deletions
        if (n == 0) {
            while (m > 0) {
                str << "- " << v1[m--];
            }
            differences.push_front(str.str());
            break;
        }

        int sub = tuple_sum(table[m - 1][n - 1]);
        int add = tuple_sum(table[m][n - 1]);
        int rem = tuple_sum(table[m - 1][n]);

        int min = std::min(sub, std::min(add, rem));

        // handle possible substitutions
        if (min == sub) {
            // if the values of the tuples are the same, no edit was made
            if (table[m - 1][n - 1] == table[m][n]) {
                m--;
                n--;
                continue;
            }

            // check against the diff threshold to determine whether the line
            // was replaced or edited
            if (diff_percent(v1[m], v2[n]) <= DIFF_THRESHOLD) {
                str << "~ " << v2[n];
            } else {
                str << "/ " << v2[n];
            }
            m--;
            n--;
        }
        // handle additions
        else if (min == add) {
            str << "+ " << v2[n--];
        }
        // handle deletions
        else {
            str << "- " << v1[m--];
        }

        // push the edit for this cell to the list
        differences.push_front(str.str());
    }
}

// temporary driving -- will be removed when added to CoCode
int main() {
    std::vector<std::vector<std::tuple<int, int, int> > > table;

    std::ifstream f1("test1.txt"), f2("test2.txt");
    std::vector<std::string> v1, v2;
    file_to_vector(f1, v1);
    file_to_vector(f2, v2);

    for (int i = 0; i < v1.size() + 1; i++) {
        table.push_back(std::vector<std::tuple<int, int, int> >());
        table[i].push_back(std::tuple<int, int, int>(0, 0, i));
        if (i == 0) {
            for (int j = 1; j < v2.size() + 1; j++) {
                table[0].push_back(std::tuple<int, int, int>(0, j, 0));
            }
        }
    }

    populate_table(table, v1, v2);

    std::cout << "\t\t";
    for (int i = 0; i < v2.size(); i++) {
        std::cout << v2[i] << "\t";
    }
    std::cout << std::endl;
    for (int i = 0; i < table.size(); i++) {
        std::cout << v1[i - 1] << "\t";
        for (int j = 0; j < table[i].size(); j++) {
            std::cout << "<" << std::get<0>(table[i][j]) << " "
                      << std::get<1>(table[i][j]) << " "
                      << std::get<2>(table[i][j]) << ">\t";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;

    std::list<std::string> differences;
    collect_changes(table, v1, v2, differences);

    for (std::string s : differences) {
        std::cout << s << std::endl;
    }
}