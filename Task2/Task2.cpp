#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <cmath>

struct Pos {
    int row;
    int col;

    bool operator == (const Pos& other) const {
        return row == other.row && col == other.col;
    }

    bool operator != (const Pos& other) const {
        return !(*this == other);
    }
};

struct Room {
    bool trap = false;
};

template <typename T>
class Maze {
private:
    struct Cell {
        T data;
        bool right = false;
        bool down = false;
    };

    int rows;
    int cols;
    std::vector<Cell> cells;

    int index(Pos p) const {
        return p.row * cols + p.col;
    }

    void checkInside(Pos p) const {
        if (!inside(p)) {
            throw std::out_of_range("cell is outside the maze");
        }
    }

    bool adjacent(Pos a, Pos b) const {
        return std::abs(a.row - b.row) + std::abs(a.col - b.col) == 1;
    }

public:
    Maze(int r, int c) : rows(r), cols(c) {
        if (r < 2 || c < 2) {
            throw std::invalid_argument("maze must be at least 2x2");
        }
        cells.resize(r * c);
    }

    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int cellCount() const { return rows * cols; }

    bool inside(Pos p) const {
        return p.row >= 0 && p.row < rows && p.col >= 0 && p.col < cols;
    }

    T& at(Pos p) {
        checkInside(p);
        return cells[index(p)].data;
    }

    const T& at(Pos p) const {
        checkInside(p);
        return cells[index(p)].data;
    }

    std::vector<Pos> neighbors(Pos p) const {
        checkInside(p);
        std::vector<Pos> res;
        Pos all[4] = { { p.row - 1, p.col },{ p.row + 1, p.col },{ p.row, p.col - 1 },{ p.row, p.col + 1 } };
        for (Pos n : all) {
            if (inside(n)) {
                res.push_back(n);
            }
        }
        return res;
    }

    void addPassage(Pos a, Pos b) {
        checkInside(a);
        checkInside(b);
        if (!adjacent(a, b)) {
            throw std::invalid_argument("cells are not adjacent");
        }
        if (b.row < a.row || b.col < a.col) {
            std::swap(a, b);
        }
        if (a.row == b.row) {
            cells[index(a)].right = true;
        }
        else {
            cells[index(a)].down = true;
        }
    }

    bool hasPassage(Pos a, Pos b) const {
        checkInside(a);
        checkInside(b);
        if (!adjacent(a, b)) {
            return false;
        }
        if (b.row < a.row || b.col < a.col) {
            std::swap(a, b);
        }
        if (a.row == b.row) {
            return cells[index(a)].right;
        }
        return cells[index(a)].down;
    }

    std::vector<Pos> openNeighbors(Pos p) const {
        std::vector<Pos> res;
        for (Pos n : neighbors(p)) {
            if (hasPassage(p, n)) {
                res.push_back(n);
            }
        }
        return res;
    }

    int passageCount() const {
        int count = 0;
        for (const Cell& c : cells) {
            if (c.right) count++;
            if (c.down) count++;
        }
        return count;
    }

    void clearPassages() {
        for (Cell& c : cells) {
            c.right = false;
            c.down = false;
        }
    }
};

template <typename T>
void generateMaze(Maze<T>& maze, unsigned seed) {
    maze.clearPassages();
    std::mt19937 rng(seed);
    std::vector<std::vector<bool>> visited(maze.getRows(), std::vector<bool>(maze.getCols(), false));
    std::vector<Pos> stack;

    visited[0][0] = true;
    stack.push_back({ 0, 0 });

    while (!stack.empty()) {
        Pos cur = stack.back();
        std::vector<Pos> free;
        for (Pos n : maze.neighbors(cur)) {
            if (!visited[n.row][n.col]) {
                free.push_back(n);
            }
        }

        if (free.empty()) {
            stack.pop_back();
        }
        else {
            Pos next = free[rng() % free.size()];
            maze.addPassage(cur, next);
            visited[next.row][next.col] = true;
            stack.push_back(next);
        }
    }
}

struct AnyMove {
    bool operator () (Pos, Pos) const {
        return true;
    }
};

std::vector<Pos> restorePath(const std::vector<std::vector<Pos>>& parent, Pos start, Pos finish) {
    std::vector<Pos> path;
    for (Pos p = finish; p != start; p = parent[p.row][p.col]) {
        path.push_back(p);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

template <typename T, typename Rule>
std::vector<Pos> findPath(const Maze<T>& maze, Pos start, Pos finish, Rule canMove) {
    if (!maze.inside(start) || !maze.inside(finish)) {
        throw std::out_of_range("start or finish is outside the maze");
    }

    std::vector<std::vector<bool>> visited(maze.getRows(), std::vector<bool>(maze.getCols(), false));
    std::vector<std::vector<Pos>> parent(maze.getRows(), std::vector<Pos>(maze.getCols(), Pos{ -1, -1 }));
    std::queue<Pos> q;

    visited[start.row][start.col] = true;
    q.push(start);

    while (!q.empty()) {
        Pos cur = q.front();
        q.pop();
        if (cur == finish) {
            break;
        }
        for (Pos n : maze.openNeighbors(cur)) {
            if (!visited[n.row][n.col] && canMove(cur, n)) {
                visited[n.row][n.col] = true;
                parent[n.row][n.col] = cur;
                q.push(n);
            }
        }
    }

    if (!visited[finish.row][finish.col]) {
        return {};
    }
    return restorePath(parent, start, finish);
}

template <typename T>
std::vector<Pos> findPath(const Maze<T>& maze, Pos start, Pos finish) {
    return findPath(maze, start, finish, AnyMove());
}

template <typename T>
int countReachable(const Maze<T>& maze, Pos start) {
    std::vector<std::vector<bool>> visited(maze.getRows(), std::vector<bool>(maze.getCols(), false));
    std::vector<Pos> stack;
    int count = 0;

    visited[start.row][start.col] = true;
    stack.push_back(start);

    while (!stack.empty()) {
        Pos cur = stack.back();
        stack.pop_back();
        count++;
        for (Pos n : maze.openNeighbors(cur)) {
            if (!visited[n.row][n.col]) {
                visited[n.row][n.col] = true;
                stack.push_back(n);
            }
        }
    }
    return count;
}

template <typename T>
bool isConnected(const Maze<T>& maze) {
    return countReachable(maze, { 0, 0 }) == maze.cellCount();
}

template <typename T>
bool isPerfect(const Maze<T>& maze) {
    return isConnected(maze) && maze.passageCount() == maze.cellCount() - 1;
}

template <typename T>
std::string mazeToString(const Maze<T>& maze, Pos start, Pos finish, const std::vector<Pos>& path) {
    std::string s;
    for (int c = 0; c < maze.getCols(); c++) {
        s += "+---";
    }
    s += "+\n";

    for (int r = 0; r < maze.getRows(); r++) {
        s += "|";
        for (int c = 0; c < maze.getCols(); c++) {
            Pos p = { r, c };
            char mark = ' ';
            if (std::find(path.begin(), path.end(), p) != path.end()) mark = '*';
            if (p == start) mark = 'S';
            if (p == finish) mark = 'E';

            s += ' ';
            s += mark;
            s += ' ';
            if (c + 1 < maze.getCols() && maze.hasPassage(p, { r, c + 1 })) {
                s += ' ';
            }
            else {
                s += '|';
            }
        }

        s += "\n+";
        for (int c = 0; c < maze.getCols(); c++) {
            if (r + 1 < maze.getRows() && maze.hasPassage({ r, c }, { r + 1, c })) {
                s += "   +";
            }
            else {
                s += "---+";
            }
        }
        s += "\n";
    }
    return s;
}

class Tester {
private:
    int passed = 0;
    int failed = 0;

public:
    void check(bool condition, const std::string& name) {
        if (condition) {
            passed++;
            std::cout << "[OK]   " << name << "\n";
        }
        else {
            failed++;
            std::cout << "[FAIL] " << name << "\n";
        }
    }

    void printResult() const {
        std::cout << "\nPassed: " << passed << ", failed: " << failed << "\n";
    }
};

template <typename T>
bool isValidPath(const Maze<T>& maze, const std::vector<Pos>& path, Pos start, Pos finish) {
    if (path.empty() || path.front() != start || path.back() != finish) {
        return false;
    }
    for (size_t i = 1; i < path.size(); i++) {
        if (!maze.hasPassage(path[i - 1], path[i])) {
            return false;
        }
    }
    return true;
}

void testCreate(Tester& t) {
    bool thrown = false;
    try {
        Maze<int> m(1, 5);
    }
    catch (const std::invalid_argument&) {
        thrown = true;
    }
    t.check(thrown, "create: 1x5 throws");

    thrown = false;
    try {
        Maze<int> m(3, 0);
    }
    catch (const std::invalid_argument&) {
        thrown = true;
    }
    t.check(thrown, "create: 3x0 throws");

    Maze<int> m(2, 3);
    t.check(m.getRows() == 2 && m.getCols() == 3 && m.cellCount() == 6, "create: 2x3 sizes");
    t.check(m.passageCount() == 0, "create: new maze has no passages");
    t.check(m.inside({ 0, 0 }) && m.inside({ 1, 2 }), "create: corners are inside");
    t.check(!m.inside({ -1, 0 }) && !m.inside({ 2, 0 }) && !m.inside({ 0, 3 }), "create: outside cells");
}

void testNeighbors(Tester& t) {
    Maze<int> m(3, 3);
    t.check(m.neighbors({ 0, 0 }).size() == 2, "neighbors: corner has 2");
    t.check(m.neighbors({ 0, 1 }).size() == 3, "neighbors: edge has 3");
    t.check(m.neighbors({ 1, 1 }).size() == 4, "neighbors: center has 4");

    bool thrown = false;
    try {
        m.neighbors({ 3, 3 });
    }
    catch (const std::out_of_range&) {
        thrown = true;
    }
    t.check(thrown, "neighbors: outside cell throws");
}

void testPassages(Tester& t) {
    Maze<int> m(3, 3);
    t.check(!m.hasPassage({ 0, 0 }, { 0, 1 }), "passage: none at start");

    m.addPassage({ 0, 1 }, { 0, 0 });
    t.check(m.hasPassage({ 0, 0 }, { 0, 1 }) && m.hasPassage({ 0, 1 }, { 0, 0 }), "passage: works in both directions");

    m.addPassage({ 1, 1 }, { 2, 1 });
    t.check(m.hasPassage({ 2, 1 }, { 1, 1 }), "passage: vertical");
    t.check(m.passageCount() == 2, "passage: count");

    m.addPassage({ 0, 0 }, { 0, 1 });
    t.check(m.passageCount() == 2, "passage: adding twice does not duplicate");

    std::vector<Pos> open = m.openNeighbors({ 0, 0 });
    t.check(open.size() == 1 && open[0] == Pos{ 0, 1 }, "passage: open neighbors");

    t.check(!m.hasPassage({ 0, 0 }, { 1, 1 }), "passage: diagonal cells have no passage");

    bool thrown = false;
    try {
        m.addPassage({ 0, 0 }, { 1, 1 });
    }
    catch (const std::invalid_argument&) {
        thrown = true;
    }
    t.check(thrown, "passage: diagonal throws");

    thrown = false;
    try {
        m.addPassage({ 0, 0 }, { 0, 0 });
    }
    catch (const std::invalid_argument&) {
        thrown = true;
    }
    t.check(thrown, "passage: same cell throws");

    thrown = false;
    try {
        m.addPassage({ 0, 2 }, { 0, 3 });
    }
    catch (const std::out_of_range&) {
        thrown = true;
    }
    t.check(thrown, "passage: outside the maze throws");

    m.clearPassages();
    t.check(m.passageCount() == 0, "passage: clear");
}

void testData(Tester& t) {
    Maze<int> m(2, 2);
    m.at({ 1, 0 }) = 5;
    t.check(m.at({ 1, 0 }) == 5 && m.at({ 0, 0 }) == 0, "data: int cells");

    Maze<std::string> s(2, 2);
    s.at({ 0, 1 }) = "key";
    t.check(s.at({ 0, 1 }) == "key", "data: string cells");

    bool thrown = false;
    try {
        m.at({ 2, 2 });
    }
    catch (const std::out_of_range&) {
        thrown = true;
    }
    t.check(thrown, "data: outside cell throws");
}

void testGenerate(Tester& t) {
    Maze<int> m(2, 2);
    generateMaze(m, 1);
    t.check(isConnected(m) && isPerfect(m), "generate: 2x2 is perfect");

    bool allPerfect = true;
    int sizes[4][2] = { { 3, 3 },{ 5, 7 },{ 2, 15 },{ 20, 20 } };
    for (int i = 0; i < 4; i++) {
        for (unsigned seed = 0; seed < 5; seed++) {
            Maze<int> g(sizes[i][0], sizes[i][1]);
            generateMaze(g, seed);
            if (!isPerfect(g)) {
                allPerfect = false;
            }
        }
    }
    t.check(allPerfect, "generate: different sizes and seeds are perfect");

    Maze<std::string> s(4, 4);
    generateMaze(s, 3);
    generateMaze(s, 4);
    t.check(isPerfect(s), "generate: second generation on the same maze is perfect");
}

void testProperties(Tester& t) {
    Maze<int> m(2, 2);
    t.check(!isConnected(m), "props: empty maze is not connected");

    m.addPassage({ 0, 0 }, { 0, 1 });
    m.addPassage({ 0, 1 }, { 1, 1 });
    m.addPassage({ 1, 1 }, { 1, 0 });
    t.check(isConnected(m) && isPerfect(m), "props: 3 passages in 2x2 is perfect");

    m.addPassage({ 1, 0 }, { 0, 0 });
    t.check(isConnected(m) && !isPerfect(m), "props: cycle is connected but not perfect");
}

void testFindPath(Tester& t) {
    Maze<int> m(6, 8);
    generateMaze(m, 10);
    std::vector<Pos> path = findPath(m, { 0, 0 }, { 5, 7 });
    t.check(isValidPath(m, path, { 0, 0 }, { 5, 7 }), "path: valid route in generated maze");

    path = findPath(m, { 2, 3 }, { 2, 3 });
    t.check(path.size() == 1 && path[0] == Pos{ 2, 3 }, "path: start equals finish");

    Maze<int> empty(3, 3);
    t.check(findPath(empty, { 0, 0 }, { 0, 1 }).empty(), "path: no passage, no route");

    Maze<int> cycle(2, 2);
    cycle.addPassage({ 0, 0 }, { 0, 1 });
    cycle.addPassage({ 0, 1 }, { 1, 1 });
    cycle.addPassage({ 1, 1 }, { 1, 0 });
    cycle.addPassage({ 1, 0 }, { 0, 0 });
    path = findPath(cycle, { 0, 0 }, { 1, 1 });
    t.check(path.size() == 3 && isValidPath(cycle, path, { 0, 0 }, { 1, 1 }), "path: shortest route in cycle");

    cycle.at({ 0, 1 }) = 1;
    path = findPath(cycle, { 0, 0 }, { 1, 1 }, [&cycle](Pos, Pos to) { return cycle.at(to) != 1; });
    t.check(path.size() == 3 && path[1] == Pos{ 1, 0 }, "path: rule avoids blocked cell");

    cycle.at({ 1, 0 }) = 1;
    path = findPath(cycle, { 0, 0 }, { 1, 1 }, [&cycle](Pos, Pos to) { return cycle.at(to) != 1; });
    t.check(path.empty(), "path: rule blocks every route");

    bool thrown = false;
    try {
        findPath(m, { 0, 0 }, { 6, 0 });
    }
    catch (const std::out_of_range&) {
        thrown = true;
    }
    t.check(thrown, "path: finish outside throws");
}

void testOutput(Tester& t) {
    Maze<int> m(2, 2);
    m.addPassage({ 0, 0 }, { 0, 1 });
    m.addPassage({ 0, 1 }, { 1, 1 });
    std::vector<Pos> path = findPath(m, { 0, 0 }, { 1, 1 });

    std::string expected =
        "+---+---+\n"
        "| S   * |\n"
        "+---+   +\n"
        "|   | E |\n"
        "+---+---+\n";
    t.check(mazeToString(m, { 0, 0 }, { 1, 1 }, path) == expected, "output: text view");
}

void demo() {
    std::cout << "\n=== Level 6x10 ===\n";
    Maze<Room> level(6, 10);
    generateMaze(level, 42);
    Pos start = { 0, 0 };
    Pos finish = { 5, 9 };

    std::cout << mazeToString(level, start, finish, {});

    std::vector<Pos> path = findPath(level, start, finish);
    std::cout << "\nRoute, " << path.size() << " cells:\n";
    std::cout << mazeToString(level, start, finish, path);

    Pos trap = path[path.size() / 2];
    level.at(trap).trap = true;
    std::vector<Pos> safe = findPath(level, start, finish, [&level](Pos, Pos to) { return !level.at(to).trap; });
    std::cout << "Trap in (" << trap.row << ", " << trap.col << "), route without traps: "
        << (safe.empty() ? "not found" : "found") << "\n";

    std::cout << "\n=== Level 15x15 ===\n";
    Maze<Room> big(15, 15);
    generateMaze(big, 7);
    std::cout << "connected: " << (isConnected(big) ? "yes" : "no")
        << ", perfect: " << (isPerfect(big) ? "yes" : "no")
        << ", route (0,0) -> (14,14): " << findPath(big, { 0, 0 }, { 14, 14 }).size() << " cells\n";
}

int main() {
    Tester t;
    testCreate(t);
    testNeighbors(t);
    testPassages(t);
    testData(t);
    testGenerate(t);
    testProperties(t);
    testFindPath(t);
    testOutput(t);
    t.printResult();

    demo();
    return 0;
}