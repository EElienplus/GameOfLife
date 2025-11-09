#include <iostream>
#include <raylib.h>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <array>
#include <fstream>
#include <sstream>
#include <filesystem>

// Tema: barevne nastaveni pro vykreslovani
struct Theme {
    std::string name;
    Color aliveColor;
    Color deadColor;
    Color fadingColor; // barva pro zhasinajici stopy
    Color gridColor;
    Color uiAccent;
};

static std::vector<Theme> THEMES = {
    {"Neo Plasma", Color{ 0, 255, 200, 255}, Color{10, 10, 18, 255}, Color{120, 50, 255, 255}, Color{30, 50, 70, 80}, Color{0, 255, 180, 255}},
    {"Amber Noir", Color{255, 190, 40, 255}, Color{8, 6, 4, 255}, Color{255, 80, 0, 255}, Color{60, 40, 20, 90}, Color{255, 140, 0, 255}},
    {"Ice Mint", Color{180, 255, 250, 255}, Color{5, 14, 18, 255}, Color{70, 210, 200, 255}, Color{40, 80, 100, 80}, Color{120, 240, 230, 255}},
    {"Sunset", Color{255, 100, 120, 255}, Color{12, 8, 20, 255}, Color{255, 200, 90, 255}, Color{70, 40, 90, 90}, Color{255, 160, 100, 255}},
    {"Matrix", Color{50, 255, 80, 255}, Color{0, 5, 0, 255}, Color{0, 130, 0, 255}, Color{0, 60, 0, 100}, Color{0, 200, 40, 255}}
};

struct Patterns {
    // Kazdy vzor: vektor relativnich (x,y) souradnic
    static std::vector<std::pair<std::string, std::vector<Vector2>>> All() {
        return {
            // 1 - Glider (classic)
            {"Glider", {{0,1},{1,2},{2,0},{2,1},{2,2}}},
            // 2 - Diehard (dlouha sekvence)
            {"Diehard", {{6,0},{0,1},{1,1},{1,2},{5,2},{6,2},{7,2}}},
            // 3 - Pulsar (velky oscilator)
            {"Pulsar", {
                {2,0},{3,0},{4,0},{8,0},{9,0},{10,0},
                {0,2},{5,2},{7,2},{12,2},
                {0,3},{5,3},{7,3},{12,3},
                {0,4},{5,4},{7,4},{12,4},
                {2,5},{3,5},{4,5},{8,5},{9,5},{10,5},
                {2,7},{3,7},{4,7},{8,7},{9,7},{10,7},
                {0,8},{5,8},{7,8},{12,8},
                {0,9},{5,9},{7,9},{12,9},
                {0,10},{5,10},{7,10},{12,10},
                {2,12},{3,12},{4,12},{8,12},{9,12},{10,12}
            }},
            // 4 - Battleship (velky esteticky vzor)
            {"Battleship", {
                {1,2},{2,1},{2,2},{2,3},{3,0},{3,1},{3,2},{3,3},{3,4},
                {4,0},{4,4},{5,0},{5,4},{6,1},{6,2},{6,3},{7,2},
                {8,2},{9,2},{10,2},{11,2}
            }},
            // 5 - R-pentomino (chaoticky maly start)
            {"R-Pentomino", {{0,1},{0,2},{1,0},{1,1},{2,1}}},
            // 6 - LWSS (lehká spaceship)
            {"LWSS", {{0,1},{0,4},{1,0},{2,0},{3,0},{4,0},{4,3},{1,5},{4,4},{2,5},{3,5}}},
            // 7 - Toad (oscilator)
            {"Toad", {{1,0},{2,0},{3,0},{4,0},{0,1},{1,1},{2,1},{3,1}}},
            // 8 - Beacon (dva bloku oscilator)
            {"Beacon", {{0,0},{1,0},{0,1},{1,1},{2,2},{3,2},{2,3},{3,3}}},
            // 9 - Beehive (stabilni obrazec)
            {"Beehive", {{1,0},{2,0},{0,1},{3,1},{1,2},{2,2}}},
            // 0 - Gosper Glider Gun (velky vystrelovac)
            {"Gosper Gun", {
                {0,4},{1,4},{0,5},{1,5},
                {10,4},{10,5},{10,6},{11,3},{11,7},{12,2},{12,8},{13,2},{13,8},{14,5},{15,3},{15,7},{16,4},{16,5},{16,6},{17,5},
                {20,2},{20,3},{20,4},{21,2},{21,3},{21,4},{22,1},{22,5},{24,0},{24,1},{24,5},{24,6},
                {34,2},{34,3},{35,2},{35,3}
            }}
        };
    }
};

class GameOfLife {
public:
    GameOfLife(int w, int h)
        : width(w), height(h), current(width*height, 0), next(width*height, 0), age(width*height, 0) {}

    // Vymaze vsechny bunky a resetuje generaci
    void Clear() {
        std::fill(current.begin(), current.end(), 0);
        std::fill(next.begin(), next.end(), 0);
        std::fill(age.begin(), age.end(), 0);
        generation = 0;
    }

    // Nahodne naplni mrizku s danou pravdepodobnosti
    void Randomize(double aliveProbability = 0.22) {
        std::random_device rd; std::mt19937 gen(rd()); std::bernoulli_distribution d(aliveProbability);
        for (int i = 0; i < width*height; ++i) {
            current[i] = d(gen) ? 1 : 0;
            age[i] = current[i] ? 1 : 0;
        }
        generation = 0;
    }

    // Provede krok(y) simulace; wrap urcuje zda se okraje obalujou
    void Step(bool wrap, bool trails, int steps = 1) {
        for (int s = 0; s < steps; ++s) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int aliveNeighbors = 0;
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = x + dx;
                            int ny = y + dy;
                            if (wrap) {
                                nx = (nx + width) % width;
                                ny = (ny + height) % height;
                            } else {
                                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                            }
                            aliveNeighbors += current[ny*width + nx];
                        }
                    }
                    int idx = y*width + x;
                    next[idx] = current[idx] ? ((aliveNeighbors == 2 || aliveNeighbors == 3) ? 1 : 0)
                                             : ((aliveNeighbors == 3) ? 1 : 0);
                }
            }
            // Aktualizace veku bunek podle stavu pred a po kroku
            for (int i = 0; i < width*height; ++i) {
                if (next[i]) {
                    age[i] = current[i] ? std::min(age[i] + 1, 1000) : 1;
                } else {
                    if (current[i]) {
                        age[i] = trails ? -1 : 0; // pokud zapnute stopy, zacne zhasinani
                    } else if (age[i] < 0) {
                        age[i] = trails ? (age[i] > -15 ? age[i] - 1 : 0) : 0;
                    } else if (!trails) {
                        age[i] = 0;
                    }
                }
            }
            current.swap(next);
            generation++;
        }
    }

    // Prepnuti bunky (zije/nezi)
    void ToggleCell(int x, int y) {
        if (!InBounds(x,y)) return;
        int idx = y*width + x;
        if (current[idx]) {
            current[idx] = 0; age[idx] = -1;
        } else {
            current[idx] = 1; age[idx] = 1;
        }
    }

    // Nastaveni bunky na zivy/ne
    void SetCell(int x, int y, bool alive) {
        if (!InBounds(x,y)) return;
        int idx = y*width + x;
        current[idx] = alive ? 1 : 0;
        age[idx] = alive ? std::max(age[idx]+1,1) : -1;
    }

    // Vlozi vzor na pozici (cx,cy)
    void InsertPattern(int cx, int cy, const std::vector<Vector2>& pattern) {
        for (auto &p : pattern) {
            int x = cx + (int)p.x;
            int y = cy + (int)p.y;
            if (InBounds(x,y)) {
                current[y*width + x] = 1;
                age[y*width + x] = 1;
            }
        }
    }

    int Population() const {
        int sum = 0; for (auto v : current) sum += v; return sum;
    }

    bool InBounds(int x, int y) const { return x >= 0 && x < width && y >= 0 && y < height; }

    // Pristupove funkce
    int Width() const { return width; }
    int Height() const { return height; }
    int Generation() const { return generation; }
    int Cell(int x, int y) const { return current[y*width + x]; }
    int Age(int x, int y) const { return age[y*width + x]; }
private:
    int width, height;
    std::vector<uint8_t> current;
    std::vector<uint8_t> next;
    std::vector<int> age; // kladna = vek zive bunky, zaporna = kroky zhasinani
    int generation = 0;
};

struct AppState {
    GameOfLife life;
    bool running = false; // start zastaveno pro uzivatele
    bool wrap = false; // bez obalovani, aby vzory mizely na okraji
    bool showGrid = true;
    bool showHelp = false; // minimalni nápoveda
    int themeIndex = 0;
    float simSpeed = 20.0f;
    float accumulator = 0.0f;
    Camera2D camera{};
    float cellPixelSize = 14.0f;
    bool drawingAdd = true;
    bool isDragging = false;
    bool showTrails = true;
    bool enableBloom = true;
    float bloomIntensity = 1.6f; // intenzita bloom
    bool enableGloom = true;
    Shader bloomShader{}; // separabilni blur shader
    RenderTexture2D glowRT{}; // render target pro svetelny efekt
    RenderTexture2D tempRT{}; // pomocny target pro ping-pong
    int locTexelSize = -1;
    int locAxis = -1;
    int locRadius = -1;
    float bloomRadius = 2.5f; // polomer rozmazani v texelech
    // UI prepinace (zadny text na obrazovce neni implicitne)
    bool showKeybinds = false; // F1 zobrazit zkratky
    bool showStats = false;    // F2 zobrazit statistiky
    // Mereni vykonnosti
    float measureTimer = 0.0f;
    int lastMeasuredGeneration = 0;
    float measuredGenRate = 0.0f;
    AppState(int w, int h) : life(w,h) {
        camera.target = {0,0};
        camera.offset = {0,0};
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
    }
};

// Jednoducha funkce pro jemne zmeny intervalu animace
static float EaseOutQuad(float t) { return 1.0f - (1.0f - t)*(1.0f - t); }

// Kresli vignette overlay pro zjemneni okraju
static void DrawVignetteOverlay(int screenWidth, int screenHeight, float strength) {
    int cx = screenWidth/2, cy = screenHeight/2;
    float radius = std::sqrt((float)cx*cx + (float)cy*cy) * 1.1f;
    Color center = Color{0,0,0,(unsigned char)(0)};
    Color edge = Color{0,0,0,(unsigned char)std::clamp((int)(120*strength), 0, 220)};
    DrawCircleGradient(cx, cy, radius, center, edge);
}

// Zajisti spravny render target pro glow (zmena pri resize)
static void EnsureGlowTarget(AppState &state, int screenWidth, int screenHeight) {
    if (state.glowRT.id == 0 || state.glowRT.texture.width != screenWidth || state.glowRT.texture.height != screenHeight) {
        if (state.glowRT.id != 0) UnloadRenderTexture(state.glowRT);
        if (state.tempRT.id != 0) UnloadRenderTexture(state.tempRT);
        state.glowRT = LoadRenderTexture(screenWidth, screenHeight);
        state.tempRT = LoadRenderTexture(screenWidth, screenHeight);
    }
}

// Inicalizace blur shaderu (separabilni)
static void InitBloom(AppState &state) {
    const char* frag = R"(
        #version 330
        in vec2 fragTexCoord;
        in vec4 fragColor;
        out vec4 finalColor;
        uniform sampler2D texture0;
        uniform vec2 texelSize;
        uniform vec2 axis;    // (1,0)=horiz, (0,1)=vert
        uniform float radius; // polomer blur v texelech
        void main() {
            float w[9] = float[](0.05, 0.09, 0.12, 0.15, 0.18, 0.15, 0.12, 0.09, 0.05);
            vec3 col = vec3(0.0);
            for (int i=-4; i<=4; i++) {
                vec2 off = axis * texelSize * radius * float(i);
                col += texture(texture0, fragTexCoord + off).rgb * w[i+4];
            }
            finalColor = vec4(col, 1.0);
        }
    )";
    state.bloomShader = LoadShaderFromMemory(nullptr, frag);
    state.locTexelSize = GetShaderLocation(state.bloomShader, "texelSize");
    state.locAxis = GetShaderLocation(state.bloomShader, "axis");
    state.locRadius = GetShaderLocation(state.bloomShader, "radius");
}

// Kresli jen zive bunky do glow targetu (pro pozdejsi rozmazani)
static void DrawCellsGlow(const AppState &state) {
    const Theme &theme = THEMES[state.themeIndex];
    float size = state.cellPixelSize;
    int w = state.life.Width();
    int h = state.life.Height();
    Vector2 topLeft = GetScreenToWorld2D({0,0}, state.camera);
    Vector2 bottomRight = GetScreenToWorld2D({(float)GetScreenWidth(),(float)GetScreenHeight()}, state.camera);
    int startX = std::max(0, (int)std::floor(topLeft.x / size) - 1);
    int endX = std::min(w, (int)std::ceil(bottomRight.x / size) + 1);
    int startY = std::max(0, (int)std::floor(topLeft.y / size) - 1);
    int endY = std::min(h, (int)std::ceil(bottomRight.y / size) + 1);
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            int a = state.life.Age(x,y);
            if (a <= 0) continue; // pouze zive bunky emitujici
            float boost = 1.4f;
            Color c = Color{
                (unsigned char)std::min(255, (int)(theme.aliveColor.r * boost)),
                (unsigned char)std::min(255, (int)(theme.aliveColor.g * boost)),
                (unsigned char)std::min(255, (int)(theme.aliveColor.b * boost)),
                255
            };
            DrawRectangle(x*size + 1, y*size + 1, size - 2, size - 2, c);
        }
    }
}

// Minimalni UI panel (pusobi ted jen jako fallback, panely jsou v cestine)
static void DrawUI(const AppState& state, int screenWidth, int screenHeight) {
    const Theme &theme = THEMES[state.themeIndex];
    int panelW = 250;
    const int titleSize = 26;
    const int genSize = 20;
    const int popSize = 20;
    const int speedSize = 18;
    const int spacing = 6;           // mezera mezi radky
    const int padTop = 16;           // horní odsazeni
    const int padBottom = 16;        // dolní odsazeni
    int contentH = titleSize + genSize + popSize + speedSize + spacing * 3;
    int panelH = padTop + contentH + padBottom;

    Rectangle panel{16,16,(float)panelW,(float)panelH};
    Color bg{ (unsigned char)(theme.deadColor.r*0.7f), (unsigned char)(theme.deadColor.g*0.7f), (unsigned char)(theme.deadColor.b*0.7f), 210 };
    DrawRectangleRounded(panel, 0.20f, 12, bg);
    DrawRectangleRoundedLines(panel, 0.20f, 12, theme.uiAccent);

    int y = (int)panel.y + padTop;
    auto shadowed = [&](const std::string &s, int size, Color c){
        DrawText(s.c_str(), (int)panel.x + 18 + 1, y + 1, size, Color{0,0,0,140});
        DrawText(s.c_str(), (int)panel.x + 18,     y,     size, c);
        y += size + spacing;
    };
    shadowed("Hra zivota", titleSize, theme.aliveColor);
    shadowed("Generace: " + std::to_string(state.life.Generation()), genSize, WHITE);
    shadowed("Populace: " + std::to_string(state.life.Population()), popSize, WHITE);
    shadowed("Rychlost: " + std::to_string((int)state.measuredGenRate) + "/s", speedSize, theme.uiAccent);

    // Napis se zkratkami (nizky kontrast)
    std::string hint = "Space Spustit/Pozastavit  •  1-0 Vzory  •  T Tema  •  Ctrl/Cmd+Scroll: Rychlost";
    int hintSize = 16;
    int hintWidth = MeasureText(hint.c_str(), hintSize);
    DrawText(hint.c_str(), screenWidth - hintWidth - 20 + 1, screenHeight - 34 + 1, hintSize, Color{0,0,0,150});
    DrawText(hint.c_str(), screenWidth - hintWidth - 20,     screenHeight - 34,     hintSize, Color{255,255,255,200});
}

// Pozadi s jemnym prechodem
static void DrawBackgroundGradient(int screenWidth, int screenHeight, const Theme &theme) {
    Color top = Color{ (unsigned char)(theme.deadColor.r/2), (unsigned char)(theme.deadColor.g/2), (unsigned char)(theme.deadColor.b/2), 255 };
    Color bottom = Color{ (unsigned char)(std::min(255,(int)(theme.aliveColor.r*0.15f + theme.deadColor.r*0.4f))),
                          (unsigned char)(std::min(255,(int)(theme.aliveColor.g*0.15f + theme.deadColor.g*0.4f))),
                          (unsigned char)(std::min(255,(int)(theme.aliveColor.b*0.15f + theme.deadColor.b*0.4f))), 255 };
    int bands = 64;
    for (int i=0;i<bands;i++) {
        float t = (float)i/(bands-1);
        unsigned char r = (unsigned char)(top.r + (bottom.r - top.r)*t);
        unsigned char g = (unsigned char)(top.g + (bottom.g - top.g)*t);
        unsigned char b = (unsigned char)(top.b + (bottom.b - top.b)*t);
        DrawRectangle(0, (int)(t*screenHeight), screenWidth, (int)(screenHeight/bands)+1, Color{r,g,b,255});
    }
}

// Gloom overlay: zjemneni a jemny fog
static void DrawGloomOverlay(int screenWidth, int screenHeight, float strength) {
    float base = strength;
    Color fog = Color{15, 20, 35, (unsigned char)std::clamp((int)(85*base),0,160)};
    DrawRectangle(0,0,screenWidth,screenHeight,fog);
    DrawVignetteOverlay(screenWidth, screenHeight, 1.2f*base);
    Color edgeDark = Color{0,0,0,(unsigned char)std::clamp((int)(140*base),0,220)};
    int cx = screenWidth/2, cy = screenHeight/2;
    float outerRadius = std::sqrt((float)cx*cx + (float)cy*cy) * 1.05f;
    DrawCircleGradient(cx, cy, outerRadius, Color{0,0,0,0}, edgeDark);
}

// Mrizka: vykresli viditelne ciary
static void DrawGridOverlay(const AppState &state, int screenWidth, int screenHeight) {
    if (!state.showGrid) return;
    const Theme &theme = THEMES[state.themeIndex];
    int w = state.life.Width();
    int h = state.life.Height();
    float size = state.cellPixelSize;
    Vector2 topLeft = GetScreenToWorld2D({0,0}, state.camera);
    Vector2 bottomRight = GetScreenToWorld2D({(float)screenWidth,(float)screenHeight}, state.camera);
    int startX = std::max(0, (int)std::floor(topLeft.x / size) - 1);
    int endX = std::min(w, (int)std::ceil(bottomRight.x / size) + 1);
    int startY = std::max(0, (int)std::floor(topLeft.y / size) - 1);
    int endY = std::min(h, (int)std::ceil(bottomRight.y / size) + 1);
    Color gc = theme.gridColor;
    for (int x = startX; x <= endX; ++x) {
        DrawLineV({x*size, startY*size}, {x*size, endY*size}, gc);
    }
    for (int y = startY; y <= endY; ++y) {
        DrawLineV({startX*size, y*size}, {endX*size, y*size}, gc);
    }
}

// Vykresli bunky do hlavniho vykresleni
static void DrawCells(const AppState &state) {
    const Theme &theme = THEMES[state.themeIndex];
    float size = state.cellPixelSize;
    int w = state.life.Width();
    int h = state.life.Height();

    Vector2 topLeft = GetScreenToWorld2D({0,0}, state.camera);
    Vector2 bottomRight = GetScreenToWorld2D({(float)GetScreenWidth(),(float)GetScreenHeight()}, state.camera);
    int startX = std::max(0, (int)std::floor(topLeft.x / size) - 1);
    int endX = std::min(w, (int)std::ceil(bottomRight.x / size) + 1);
    int startY = std::max(0, (int)std::floor(topLeft.y / size) - 1);
    int endY = std::min(h, (int)std::ceil(bottomRight.y / size) + 1);

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            int age = state.life.Age(x,y);
            if (age == 0) continue; // nikdy nezila a bez stopy
            Color c;
            if (age > 0) {
                float t = std::min(1.0f, std::log2f((float)age + 1.0f)/8.0f);
                c = Color{
                    (unsigned char)(theme.aliveColor.r * (0.6f + 0.4f*t)),
                    (unsigned char)(theme.aliveColor.g * (0.6f + 0.4f*t)),
                    (unsigned char)(theme.aliveColor.b * (0.6f + 0.4f*t)),
                    255
                };
            } else {
                if (!state.showTrails) continue; // skryt stopy kdyz vypnuto
                int fadeFrame = -age; // 1..15
                float t = 1.0f - (fadeFrame / 15.0f);
                t = EaseOutQuad(std::max(t,0.0f));
                Color fade = theme.fadingColor;
                c = Color{
                    (unsigned char)(fade.r * t),
                    (unsigned char)(fade.g * t),
                    (unsigned char)(fade.b * t),
                    (unsigned char)(60 * t) // nizsi nepruhlednost pro stopy
                };
            }
            DrawRectangle(x*size + 1, y*size + 1, size - 2, size - 2, c);
        }
    }
}

// Export aktualniho vzoru do souboru
static void ExportPattern(const AppState &state, const std::string &filename) {
    int w = state.life.Width();
    int h = state.life.Height();
    int minX = w, minY = h, maxX = -1, maxY = -1;
    std::vector<Vector2> cells;
    cells.reserve(state.life.Population());
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (state.life.Cell(x,y)) {
                cells.push_back({(float)x,(float)y});
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }
    std::ofstream out(filename);
    if (!out) return;
    out << "# Exported Game of Life pattern\n";
    out << "# Alive cells: " << cells.size() << "\n";
    if (cells.empty()) {
        out << "# (No live cells)\n";
        return;
    }
    out << "# Bounding box: (" << minX << "," << minY << ") - (" << maxX << "," << maxY << ")\n";
    out << "# Format: relative_x relative_y from top-left of bounding box\n";
    for (auto &c : cells) {
        out << (int)(c.x - minX) << ' ' << (int)(c.y - minY) << '\n';
    }
}

// Import vzoru ze souboru (relativni souradnice)
static std::vector<Vector2> ImportPattern(const std::string &filename) {
    std::ifstream in(filename);
    std::vector<Vector2> pattern;
    if (!in) return pattern;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        int rx, ry; if (!(iss >> rx >> ry)) continue;
        pattern.push_back({(float)rx,(float)ry});
    }
    return pattern;
}

// Kresli panel se zkratkami (F1)
static void DrawKeybindsPanel(int screenWidth, int screenHeight) {
    std::vector<std::string> lines = {
        "Zkratky (F1 pro zavreni)",
        "Space: Spustit/Pozastavit",
        "S: Krok 1",
        "R: Nahodne",
        "C: Vymazat",
        "1-0: Vlozit vzor",
        "F: Prepnout stopy",
        "B: Prepnout bloom",
        "V: Prepnout gloom",
        "Ctrl/Cmd + Scroll: Zmena rychlosti",
        "+/- nebo =: Zmena velikosti bunek",
        "P: Screenshot",
        "E: Export, I: Import",
    };
    int pad = 18;
    int lineH = 20;
    int w = 520;
    int h = (int)lines.size() * (lineH + 6) + pad*2;
    Rectangle panel = {(float)(screenWidth/2 - w/2), (float)(screenHeight/2 - h/2), (float)w, (float)h};
    Color bg = Color{20,20,28,220};
    DrawRectangleRounded(panel, 0.18f, 10, bg);
    DrawRectangleRoundedLines(panel, 0.18f, 10, Color{200,200,220,90});
    int y = (int)panel.y + pad;
    for (size_t i=0;i<lines.size();++i) {
        const std::string &s = lines[i];
        int size = (i==0 ? 22 : 16);
        DrawText(s.c_str(), (int)panel.x + pad + 1, y + 1, size, Color{0,0,0,160});
        DrawText(s.c_str(), (int)panel.x + pad,     y,     size, Color{230,230,240,220});
        y += size + 6;
    }
}

// Kompaktny panel se statistikami (F2)
static void DrawStatsPanel(const AppState &state, int screenWidth, int screenHeight) {
    int panelW = 260;
    int pad = 12;
    Rectangle panel{16,16,(float)panelW, 120};
    Color bg = Color{15,15,20,200};
    DrawRectangleRounded(panel, 0.18f, 10, bg);
    DrawRectangleRoundedLines(panel, 0.18f, 10, Color{150,150,180,90});
    int y = (int)panel.y + pad;
    auto drawLine = [&](const std::string &label, const std::string &value) {
        std::string s = label + ": " + value;
        DrawText(s.c_str(), (int)panel.x + pad + 1, y + 1, 16, Color{0,0,0,150});
        DrawText(s.c_str(), (int)panel.x + pad,     y,     16, Color{245,245,245,220});
        y += 22;
    };
    drawLine("Generace", std::to_string(state.life.Generation()));
    drawLine("Populace", std::to_string(state.life.Population()));
    drawLine("Cilova rychlost", std::to_string((int)state.simSpeed) + " gen/s");
    drawLine("Zmereno", std::to_string((int)state.measuredGenRate) + " gen/s");
}

int main() {
    const int screenWidth = 1400;
    const int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "Hra zivota - Portfolio Verze");
    SetTargetFPS(165); // vysoke obnovovani pro plynuly input

    const int GRID_W = 256;
    const int GRID_H = 180;
    AppState state(GRID_W, GRID_H);

    InitBloom(state);
    EnsureGlowTarget(state, GetScreenWidth(), GetScreenHeight());

    auto patterns = Patterns::All();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        EnsureGlowTarget(state, GetScreenWidth(), GetScreenHeight());
        state.measureTimer += dt;
        if (state.measureTimer >= 1.0f) {
            int genNow = state.life.Generation();
            state.measuredGenRate = (genNow - state.lastMeasuredGeneration) / state.measureTimer;
            state.lastMeasuredGeneration = genNow;
            state.measureTimer = 0.0f;
        }

        // Ovládani klavesami a prepinace
        if (IsKeyPressed(KEY_SPACE)) state.running = !state.running;
        if (IsKeyPressed(KEY_S)) state.life.Step(state.wrap, state.showTrails, 1);
        if (IsKeyPressed(KEY_R)) state.life.Randomize();
        if (IsKeyPressed(KEY_C)) state.life.Clear();
        if (IsKeyPressed(KEY_G)) state.showGrid = !state.showGrid;
        if (IsKeyPressed(KEY_T)) state.themeIndex = (state.themeIndex + 1) % (int)THEMES.size();
        if (IsKeyPressed(KEY_LEFT_BRACKET)) state.simSpeed = std::max(1.0f, state.simSpeed - 5.0f);
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) state.simSpeed = std::min(200.0f, state.simSpeed + 5.0f);
        if (IsKeyPressed(KEY_F)) state.showTrails = !state.showTrails;
        if (IsKeyPressed(KEY_B)) state.enableBloom = !state.enableBloom;
        if (IsKeyPressed(KEY_V)) state.enableGloom = !state.enableGloom;
        if (IsKeyPressed(KEY_K)) state.bloomIntensity = std::max(0.0f, state.bloomIntensity - 0.1f);
        if (IsKeyPressed(KEY_L)) state.bloomIntensity = std::min(3.0f, state.bloomIntensity + 0.1f);
        if (IsKeyPressed(KEY_MINUS)) state.cellPixelSize = std::max(4.0f, state.cellPixelSize - 1.0f);
        if (IsKeyPressed(KEY_EQUAL)) state.cellPixelSize = std::min(40.0f, state.cellPixelSize + 1.0f);
        if (IsKeyPressed(KEY_P)) {
            auto ts = std::chrono::system_clock::now().time_since_epoch();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ts).count();
            std::string fname = "gol_screenshot_" + std::to_string(ms) + ".png";
            TakeScreenshot(fname.c_str());
        }
        if (IsKeyPressed(KEY_E)) {
            ExportPattern(state, "pattern_export.txt");
        }
        if (IsKeyPressed(KEY_I)) {
            auto patternImport = ImportPattern("pattern_export.txt");
            if (!patternImport.empty()) {
                Vector2 worldCenter = GetScreenToWorld2D({(float)GetScreenWidth()/2,(float)GetScreenHeight()/2}, state.camera);
                int cellX = (int)(worldCenter.x / state.cellPixelSize);
                int cellY = (int)(worldCenter.y / state.cellPixelSize);
                state.life.InsertPattern(cellX, cellY, patternImport);
            }
        }

        if (IsKeyPressed(KEY_F1)) state.showKeybinds = !state.showKeybinds;
        if (IsKeyPressed(KEY_F2)) state.showStats = !state.showStats;

        // Vlozeni vzoru pomoci klicu 1..0
        int keyMap[10] = { KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE, KEY_ZERO };
        int avail = std::min((int)patterns.size(), 10);
        for (int i = 0; i < avail; ++i) {
            if (IsKeyPressed(keyMap[i])) {
                Vector2 worldCenter = GetScreenToWorld2D({(float)GetScreenWidth()/2,(float)GetScreenHeight()/2}, state.camera);
                int cellX = (int)(worldCenter.x / state.cellPixelSize);
                int cellY = (int)(worldCenter.y / state.cellPixelSize);
                state.life.InsertPattern(cellX, cellY, patterns[i].second);
            }
        }

        // Kamera a zoom (kola mysi)
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            bool speedMod = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
            if (speedMod) {
                float factor = powf(1.3f, wheel);
                state.simSpeed = std::clamp(state.simSpeed * factor, 0.5f, 2000.0f);
            } else {
                Vector2 mouseWorldBefore = GetScreenToWorld2D(GetMousePosition(), state.camera);
                state.camera.zoom *= (1.0f + wheel * 0.15f);
                state.camera.zoom = std::clamp(state.camera.zoom, 0.2f, 6.0f);
                Vector2 mouseWorldAfter = GetScreenToWorld2D(GetMousePosition(), state.camera);
                state.camera.target.x += (mouseWorldBefore.x - mouseWorldAfter.x);
                state.camera.target.y += (mouseWorldBefore.y - mouseWorldAfter.y);
            }
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            Vector2 delta = GetMouseDelta();
            state.camera.target.x -= delta.x / state.camera.zoom;
            state.camera.target.y -= delta.y / state.camera.zoom;
        }

        // Interaktivni kresleni mysi
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), state.camera);
        int cellX = (int)(mouseWorld.x / state.cellPixelSize);
        int cellY = (int)(mouseWorld.y / state.cellPixelSize);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { state.isDragging = true; state.drawingAdd = true; }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) { state.isDragging = true; state.drawingAdd = false; }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) { state.isDragging = false; }
        if (state.isDragging && state.life.InBounds(cellX, cellY)) {
            state.life.SetCell(cellX, cellY, state.drawingAdd);
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && state.life.InBounds(cellX, cellY)) {
            state.life.ToggleCell(cellX, cellY);
        }

        // Krokovani simulace podle akumulatoru
        if (state.running) {
            state.accumulator += dt * state.simSpeed;
            int steps = (int)state.accumulator;
            if (steps > 0) {
                steps = std::min(steps, 10);
                state.life.Step(state.wrap, state.showTrails, steps);
                state.accumulator -= steps;
            }
        }

        BeginDrawing();
        DrawBackgroundGradient(GetScreenWidth(), GetScreenHeight(), THEMES[state.themeIndex]);
        if (state.enableBloom) {
            BeginTextureMode(state.glowRT);
            ClearBackground(BLANK);
            BeginMode2D(state.camera);
            DrawCellsGlow(state);
            EndMode2D();
            EndTextureMode();
            float ts[2] = { 1.0f/(float)state.glowRT.texture.width, 1.0f/(float)state.glowRT.texture.height };
            SetShaderValue(state.bloomShader, state.locTexelSize, ts, SHADER_UNIFORM_VEC2);
            Vector2 axisH = {1.0f, 0.0f};
            SetShaderValue(state.bloomShader, state.locAxis, &axisH, SHADER_UNIFORM_VEC2);
            SetShaderValue(state.bloomShader, state.locRadius, &state.bloomRadius, SHADER_UNIFORM_FLOAT);
            BeginTextureMode(state.tempRT);
            ClearBackground(BLANK);
            BeginShaderMode(state.bloomShader);
            Rectangle srcH = {0,0,(float)state.glowRT.texture.width, -(float)state.glowRT.texture.height};
            DrawTextureRec(state.glowRT.texture, srcH, {0,0}, WHITE);
            EndShaderMode();
            EndTextureMode();
            Vector2 axisV = {0.0f, 1.0f};
            SetShaderValue(state.bloomShader, state.locAxis, &axisV, SHADER_UNIFORM_VEC2);
            BeginTextureMode(state.glowRT);
            ClearBackground(BLANK);
            BeginShaderMode(state.bloomShader);
            Rectangle srcV = {0,0,(float)state.tempRT.texture.width, -(float)state.tempRT.texture.height};
            DrawTextureRec(state.tempRT.texture, srcV, {0,0}, WHITE);
            EndShaderMode();
            EndTextureMode();
        }
        BeginMode2D(state.camera);
        DrawCells(state);
        DrawGridOverlay(state, GetScreenWidth(), GetScreenHeight());
        EndMode2D();
        if (state.enableBloom) {
            unsigned char a = (unsigned char)std::clamp((int)(state.bloomIntensity * 170), 0, 255);
            BeginBlendMode(BLEND_ADDITIVE);
            Rectangle src = {0,0,(float)state.glowRT.texture.width, -(float)state.glowRT.texture.height};
            DrawTextureRec(state.glowRT.texture, src, {0,0}, Color{255,255,255,a});
            EndBlendMode();
        }
        if (state.enableGloom) DrawGloomOverlay(GetScreenWidth(), GetScreenHeight(), 1.0f);
        if (state.showKeybinds) DrawKeybindsPanel(GetScreenWidth(), GetScreenHeight());
        if (state.showStats) DrawStatsPanel(state, GetScreenWidth(), GetScreenHeight());
         EndDrawing();
     }

     if (state.glowRT.id != 0) UnloadRenderTexture(state.glowRT);
     if (state.tempRT.id != 0) UnloadRenderTexture(state.tempRT);
     if (state.bloomShader.id != 0) UnloadShader(state.bloomShader);
     CloseWindow();
     return 0;
}