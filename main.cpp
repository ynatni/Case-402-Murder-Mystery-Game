#include "raylib.h"
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <cstring>

// =============================================================================
// NAMESPACE for game logic
// =============================================================================
namespace Case402 {

// =============================================================================
// STRUCT definitions (Academic Req #1)
// =============================================================================

struct Evidence {
    int id;
    std::string name;
    std::string description;
    std::string imagePath;
    bool found;
    Evidence* next; 

    Evidence(int i = 0, const std::string& n = "", const std::string& d = "", const std::string& img = "")
        : id(i), name(n), description(d), imagePath(img), found(false), next(nullptr) {}
};

struct Suspect {
    int id;
    std::string name;
    int age;
    float height;
    std::string job;
    std::string relation;
    std::string motive;
    std::string alibi;
    std::string cctv_time;
    std::vector<std::string> checklistItems;
    std::vector<bool> checked;
    bool isGuilty;

    Suspect() : id(0), age(0), height(0.0f), isGuilty(false) {}
};

struct InvestigationNote {
    std::string category;
    std::string content;
    bool isChecked;

    InvestigationNote(const std::string& cat = "", const std::string& cont = "")
        : category(cat), content(cont), isChecked(false) {}
};

// Node for various linked structures
template<typename T>
struct Node {
    T data;
    Node* next;
    Node* prev; // for doubly linked list
    Node(const T& d) : data(d), next(nullptr), prev(nullptr) {}
};

// =============================================================================
// LINKED LIST (Data Structure #1) - Evidence storage
// =============================================================================
struct EvidenceLinkedList {
    Evidence* head;
    int count;

    EvidenceLinkedList() : head(nullptr), count(0) {}

    // References (&) in parameters (Academic Req #2)
    void insert(Evidence* &ev) {
        if (!head) {
            head = ev;
        } else {
            Evidence* cur = head;
            while (cur->next) cur = cur->next;
            cur->next = ev;
        }
        count++;
    }

    Evidence* findById(int id) {
        Evidence* cur = head;
        while (cur) {
            if (cur->id == id) return cur;
            cur = cur->next;
        }
        return nullptr;
    }

    ~EvidenceLinkedList() {
        Evidence* cur = head;
        while (cur) {
            Evidence* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
    }
};

// =============================================================================
// CIRCULAR LINKED LIST (Data Structure #2) - Room navigation cycling
// =============================================================================
template<typename T>
struct CircularList {
    Node<T>* head;
    Node<T>* current;
    int size;

    CircularList() : head(nullptr), current(nullptr), size(0) {}

    void push(const T& val) {
        Node<T>* n = new Node<T>(val);
        if (!head) {
            head = n;
            n->next = head;
            current = head;
        } else {
            Node<T>* tmp = head;
            while (tmp->next != head) tmp = tmp->next;
            tmp->next = n;
            n->next = head;
        }
        size++;
    }

    // Pointer (*) usage (Academic Req #3)
    T* next() {
        if (!current) return nullptr;
        current = current->next;
        return &current->data;
    }

    T* prev() {
        if (!current) return nullptr;
        Node<T>* tmp = head;
        while (tmp->next != current) tmp = tmp->next;
        current = tmp;
        return &current->data;
    }

    T* get() {
        return current ? &current->data : nullptr;
    }

    ~CircularList() {
        if (!head) return;
        Node<T>* cur = head->next;
        while (cur != head) {
            Node<T>* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        delete head;
    }
};

// =============================================================================
// STACK (Data Structure #3) - Navigation history / undo
// =============================================================================
struct GameStateStack {
    std::stack<int> history; // stores scene IDs

    void push(int sceneId) { history.push(sceneId); }

    int pop() {
        if (history.empty()) return -1;
        int top = history.top();
        history.pop();
        return top;
    }

    int peek() {
        return history.empty() ? -1 : history.top();
    }

    bool empty() { return history.empty(); }
};

// =============================================================================
// QUEUE (Data Structure #4) - Typewriter text queue
// =============================================================================
struct TextQueue {
    std::queue<std::string> lines;

    void enqueue(const std::string& text) { lines.push(text); }

    std::string dequeue() {
        if (lines.empty()) return "";
        std::string front = lines.front();
        lines.pop();
        return front;
    }

    bool empty() { return lines.empty(); }
    std::string front() { return lines.empty() ? "" : lines.front(); }
};

// =============================================================================
// BINARY TREE / AVL TREE (Data Structure #5) - Evidence categorization
// =============================================================================
struct EvidenceTreeNode {
    int key;
    std::string category;
    int height;
    EvidenceTreeNode* left;
    EvidenceTreeNode* right;

    EvidenceTreeNode(int k, const std::string& cat)
        : key(k), category(cat), height(1), left(nullptr), right(nullptr) {}
};

struct AVLTree {
    EvidenceTreeNode* root;
    AVLTree() : root(nullptr) {}

    int getHeight(EvidenceTreeNode* n) { return n ? n->height : 0; }

    int getBalance(EvidenceTreeNode* n) {
        return n ? getHeight(n->left) - getHeight(n->right) : 0;
    }

    EvidenceTreeNode* rotateRight(EvidenceTreeNode* y) {
        EvidenceTreeNode* x = y->left;
        EvidenceTreeNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
        return x;
    }

    EvidenceTreeNode* rotateLeft(EvidenceTreeNode* x) {
        EvidenceTreeNode* y = x->right;
        EvidenceTreeNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
        return y;
    }

    EvidenceTreeNode* insert(EvidenceTreeNode* node, int key, const std::string& cat) {
        if (!node) return new EvidenceTreeNode(key, cat);
        if (key < node->key) node->left = insert(node->left, key, cat);
        else if (key > node->key) node->right = insert(node->right, key, cat);
        else return node;

        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        int balance = getBalance(node);

        if (balance > 1 && key < node->left->key) return rotateRight(node);
        if (balance < -1 && key > node->right->key) return rotateLeft(node);
        if (balance > 1 && key > node->left->key) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (balance < -1 && key < node->right->key) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    void insertEvidence(int key, const std::string& cat) {
        root = insert(root, key, cat);
    }
};

// =============================================================================
// GRAPH with BFS/DFS (Data Structure #6) - Suspect relationship graph
// =============================================================================
struct SuspectGraph {
    std::unordered_map<int, std::vector<int>> adjList;
    std::unordered_map<int, std::string> nodeNames;

    void addNode(int id, const std::string& name) {
        nodeNames[id] = name;
        adjList[id] = {};
    }

    void addEdge(int from, int to) {
        adjList[from].push_back(to);
        adjList[to].push_back(from);
    }

    // BFS from source
    std::vector<int> bfs(int start) {
        std::vector<int> visited;
        std::unordered_map<int, bool> seen;
        std::queue<int> q;
        q.push(start);
        seen[start] = true;
        while (!q.empty()) {
            int cur = q.front(); q.pop();
            visited.push_back(cur);
            for (int neighbor : adjList[cur]) {
                if (!seen[neighbor]) {
                    seen[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        return visited;
    }

    // DFS from source
    void dfsUtil(int node, std::unordered_map<int, bool>& seen, std::vector<int>& result) {
        seen[node] = true;
        result.push_back(node);
        for (int nb : adjList[node]) {
            if (!seen[nb]) dfsUtil(nb, seen, result);
        }
    }

    std::vector<int> dfs(int start) {
        std::unordered_map<int, bool> seen;
        std::vector<int> result;
        dfsUtil(start, seen, result);
        return result;
    }
};

// =============================================================================
// HASH TABLE (Data Structure #7) - Fast evidence lookup
// =============================================================================
struct EvidenceHashTable {
    std::unordered_map<std::string, Evidence*> table;

    void insert(const std::string& key, Evidence* ev) {
        table[key] = ev;
    }

    Evidence* find(const std::string& key) {
        auto it = table.find(key);
        return (it != table.end()) ? it->second : nullptr;
    }
};

// =============================================================================
// SORTING (Data Structure #8) - Sort evidence by ID
// =============================================================================
void sortEvidenceVector(std::vector<Evidence*>& vec) {
    // Lambda expression (Academic Req #11)
    std::sort(vec.begin(), vec.end(), [](Evidence* a, Evidence* b) {
        return a->id < b->id;
    });
}

} // namespace Case402

// =============================================================================
// GAME STATE ENUMS
// =============================================================================
enum GameScene {
    SCENE_MAIN_MENU = 0,
    SCENE_CREDITS,
    SCENE_PROLOG,
    SCENE_NAME_INPUT,
    SCENE_CHAPTER1_INTRO,
    SCENE_CHAPTER1_GAMEPLAY,
    SCENE_CHAPTER1_CLOSE_UP,
    SCENE_CHAPTER2_INTRO,
    SCENE_CHAPTER2_BOARD,
    SCENE_CHAPTER3_INTRO,
    SCENE_CHAPTER3_PUZZLE,
    SCENE_CHAPTER3_ACCUSATION,
    SCENE_ENDING_TRUE,
    SCENE_ENDING_BAD,
    SCENE_PAUSE,
    SCENE_COUNT
};

enum RoomType {
    ROOM_LIVING = 0,
    ROOM_KITCHEN,
    ROOM_COUNT
};

// =============================================================================
// FUNCTION OVERLOADING (Academic Req #7)
// =============================================================================
// Inline functions (Academic Req #6)
inline Color FadeColor(Color c, float alpha) {
    c.a = (unsigned char)(alpha * 255);
    return c;
}

inline Rectangle MakeRect(float x, float y, float w, float h) {
    return {x, y, w, h};
}

// Draw text with default font size (default argument = Academic Req #6)
void DrawGameText(const std::string& text, int x, int y, int fontSize = 20, Color color = WHITE) {
    DrawText(text.c_str(), x, y, fontSize, color);
}

void DrawGameText(const std::string& text, Vector2 pos, int fontSize, Color color) {
    DrawText(text.c_str(), (int)pos.x, (int)pos.y, fontSize, color);
}

// =============================================================================
// CALLBACK FUNCTION (Academic Req #5)
// =============================================================================
using SceneCallback = std::function<void()>;

struct Button {
    Rectangle rect;
    std::string label;
    Color bgColor;
    Color textColor;
    SceneCallback onClick;

    Button(float x, float y, float w, float h,
           const std::string& lbl,
           Color bg = {40, 40, 60, 255},
           Color tc = WHITE)
        : rect{x, y, w, h}, label(lbl), bgColor(bg), textColor(tc) {}

    void draw() const {
        bool hover = CheckCollisionPointRec(GetMousePosition(), rect);
        Color bg = hover ? Color{80, 80, 120, 255} : bgColor;
        DrawRectangleRec(rect, bg);
        DrawRectangleLinesEx(rect, 2, {180, 140, 80, 255});
        int tw = MeasureText(label.c_str(), 20);
        DrawText(label.c_str(),
                 (int)(rect.x + (rect.width - tw) / 2),
                 (int)(rect.y + (rect.height - 20) / 2),
                 20, textColor);
    }

    bool isClicked() const {
        return CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    }
};

// =============================================================================
// TYPEWRITER SYSTEM
// =============================================================================
struct Typewriter {
    std::string fullText;
    std::string displayed;
    float timer;
    float speed; // chars per second
    bool done;
    bool waitingClick;

    Typewriter(float spd = 30.0f) : timer(0), speed(spd), done(false), waitingClick(false) {}

    void setText(const std::string& text) {
        fullText = text;
        displayed = "";
        done = false;
        waitingClick = false;
        timer = 0;
    }

    void update(float dt) {
        if (done) return;
        timer += dt;
        int charsToShow = (int)(timer * speed);
        if (charsToShow >= (int)fullText.size()) {
            displayed = fullText;
            done = true;
            waitingClick = true;
        } else {
            displayed = fullText.substr(0, charsToShow);
        }
    }

    void skip() {
        displayed = fullText;
        done = true;
        waitingClick = true;
    }

void draw(int x, int y, int fontSize = 22, Color c = WHITE) const {
    int maxW = 900;
    std::string line;
    int ly = y;
    
    for (char ch : displayed) {
        line += ch;
        if (MeasureText(line.c_str(), fontSize) > maxW || ch == '\n') {
            // UBAH DI SINI: Hitung titik tengah berdasarkan panjang baris teks
            int centerX = x + (maxW - MeasureText(line.c_str(), fontSize)) / 2;
            
            DrawText(line.c_str(), centerX, ly, fontSize, c);
            line = "";
            ly += fontSize + 5;
        }
    }
    if (!line.empty()) {
        int centerX = x + (maxW - MeasureText(line.c_str(), fontSize)) / 2;
        DrawText(line.c_str(), centerX, ly, fontSize, c);
    }

    // Kursor berkedip (Blink cursor) juga disesuaikan posisinya ke tengah setelah teks
    if (done && (int)(GetTime() * 2) % 2 == 0) {
        int centerX = x + (maxW - MeasureText(displayed.c_str(), fontSize)) / 2;
        DrawText("_", centerX + MeasureText(displayed.c_str(), fontSize), ly, fontSize, c);
    }
}
};

// =============================================================================
// EVIDENCE POPUP
// =============================================================================
struct EvidencePopup {
    bool active;
    std::string title;
    std::string description;
    float fadeAlpha;
    bool fadingIn;
    Texture2D itemImage;     // gambar item sesuai bukti yang sedang ditampilkan
    bool hasImage;

    EvidencePopup() : active(false), fadeAlpha(0), fadingIn(false), hasImage(false) {}

    void show(const std::string& t, const std::string& d, Texture2D img) {
        active = true;
        title = t;
        description = d;
        fadeAlpha = 0;
        fadingIn = true;
        itemImage = img;
        hasImage = true;
    }

    void update(float dt) {
        if (!active) return;
        if (fadingIn && fadeAlpha < 1.0f) fadeAlpha = std::min(1.0f, fadeAlpha + dt * 3);
    }

    void draw(int sw, int sh) const {
        if (!active) return;
        // Darken background — dipertahankan supaya fokus ke popup
        DrawRectangle(0, 0, sw, sh, {0, 0, 0, (unsigned char)(180 * fadeAlpha)});

        const Color darkText = {51, 51, 51, 255};
        const int TEXT_BLOCK_W = (int)(sw * 0.42f); // lebar blok teks (rata kanan)
        const int MAX_DESC_LINES = 3; // jumlah baris deskripsi tetap (tidak berubah-ubah)

        // ----- WADAH POPUP: satu panel besar, gambar + teks sama-sama di dalamnya -----
        Rectangle popupArea = {(float)sw * 0.04f, (float)sh * 0.06f,
                                (float)sw * 0.92f, (float)sh * 0.88f};

        // ----- Gambar item: fit ke dalam wadah popup -----
        if (hasImage) {
            // Fit gambar ke dalam popupArea sambil menjaga aspect ratio (letterbox)
            float scale = std::min(popupArea.width / itemImage.width, popupArea.height / itemImage.height);
            float drawW = itemImage.width * scale;
            float drawH = itemImage.height * scale;
            Rectangle dest = {popupArea.x + (popupArea.width - drawW) / 2.0f,
                               popupArea.y + (popupArea.height - drawH) / 2.0f,
                               drawW, drawH};
            DrawTexturePro(itemImage,
                {0, 0, (float)itemImage.width, (float)itemImage.height},
                dest, {0, 0}, 0, FadeColor(WHITE, fadeAlpha));
        }

        // ----- Judul + deskripsi: OVERLAY di kanan-TENGAH wadah (menumpuk di atas gambar) -----
        const int TEXT_INSET_X = 20; // jarak teks dari tepi kanan wadah
        int titleFs = 22;
        int descFs  = 16;
        int lineGap = descFs + 6;

        // Word-wrap description dulu (rata kanan / right-aligned), supaya tinggi
        // total blok teks (judul + deskripsi) bisa dihitung sebelum menentukan
        // posisi Y, agar blok teks bisa benar-benar center secara vertikal.
        std::vector<std::string> descLines;
        {
            std::string word, line, desc = description;
            for (size_t i = 0; i <= desc.size(); i++) {
                if (i == desc.size() || desc[i] == ' ') {
                    std::string testLine = line.empty() ? word : line + " " + word;
                    if (MeasureText(testLine.c_str(), descFs) > TEXT_BLOCK_W && !line.empty()) {
                        descLines.push_back(line);
                        line = word;
                    } else {
                        line = testLine;
                    }
                    word = "";
                } else {
                    word += desc[i];
                }
            }
            if (!line.empty()) descLines.push_back(line);
        }
        int descLineCount = std::min((int)descLines.size(), MAX_DESC_LINES);

        // Tinggi total blok = judul + gap + N baris deskripsi
        int titleToDescGap = 10;
        int blockH = titleFs + titleToDescGap + (descLineCount > 0 ? descLineCount * lineGap : 0);

        // Center-kan blok teks terhadap tinggi popupArea
        int titleY = (int)(popupArea.y + (popupArea.height - blockH) / 2.0f);

        DrawGameText(title,
                     (int)(popupArea.x + popupArea.width) - TEXT_INSET_X - MeasureText(title.c_str(), titleFs),
                     titleY,
                     titleFs, FadeColor(darkText, fadeAlpha));

        int fy = titleY + titleFs + titleToDescGap;
        for (int i = 0; i < descLineCount; i++) {
            DrawText(descLines[i].c_str(),
                     (int)(popupArea.x + popupArea.width) - TEXT_INSET_X - MeasureText(descLines[i].c_str(), descFs),
                     fy, descFs, FadeColor(darkText, fadeAlpha));
            fy += lineGap;
        }

        // Click to continue — teks saja, tanpa lingkaran
        if (fadeAlpha >= 0.9f && (int)(GetTime() * 2) % 2 == 0) {
            DrawGameText("[ Klik untuk melanjutkan ]",
                         sw/2 - 120, sh - 80, 18,
                         {180, 140, 80, 255});
        }
    }
};

// =============================================================================
// FILE HANDLING (Academic Req #10) - Save/Load player name and progress
// =============================================================================
namespace FileIO {
    // Exception handling (Academic Req #8)
    void savePlayerName(const std::string& name) {
        try {
            std::ofstream file("save_data.txt");
            if (!file.is_open()) throw std::runtime_error("Cannot open save file");
            file << "player_name=" << name << "\n";
            file << "progress=0\n";
            file.close();
        } catch (const std::exception& e) {
            // Silently handle save failure
            (void)e;
        }
    }

    std::string loadPlayerName() {
        try {
            std::ifstream file("save_data.txt");
            if (!file.is_open()) throw std::runtime_error("No save file");
            std::string line, name = "Detective";
            while (std::getline(file, line)) {
                if (line.find("player_name=") == 0) {
                    name = line.substr(12);
                }
            }
            file.close();
            return name;
        } catch (const std::exception& e) {
            (void)e;
            return "Detective";
        }
    }

    void saveProgress(int chapter, const std::vector<bool>& evidenceFound) {
        try {
            std::ofstream file("progress.dat");
            if (!file.is_open()) throw std::runtime_error("Cannot save progress");
            file << "chapter=" << chapter << "\n";
            file << "evidence=";
            for (bool b : evidenceFound) file << (b ? "1" : "0");
            file << "\n";
            file.close();
        } catch (...) {}
    }
}

// =============================================================================
// MAIN GAME CLASS
// =============================================================================
class Case402Game {
private:
    // Layout constants — sidebar inventory kiri (ala Cube Escape) + offset hotspot map
    static constexpr int SIDEBAR_W = 110;
    // Background room sekarang full 16:9 (sidebar dihapus), jadi hotspot dihitung
    // relatif terhadap x=0, bukan lagi digeser oleh SIDEBAR_W
    static constexpr int ROOM_OX   = 0;

    // Screen dimensions
    int screenW, screenH;

    // Current scene
    GameScene currentScene;
    GameScene previousScene;
    bool paused;

    // Navigation stack (Academic: Stack)
    Case402::GameStateStack navStack;

    // Player info
    std::string playerName;
    char nameInputBuf[32];
    int nameLen;
    bool nameConfirmed;

    // Prolog state
    int prologStep;
    Typewriter typewriter;
    float sceneTimer;
    bool waitingForClick;

    // Chapter 1 state
    RoomType currentRoom;
    Case402::CircularList<std::string> roomList; // Circular for room cycling
    int selectedCloseUp; // which spot is being examined (-1 = none)
    bool inCloseUp;      // true = sedang zoom ke area map (Rusty Lake style)
    int closeUpSubStep; // 0 = close up view, 1 = evidence popup
    int pendingEvidenceIdx; // index evidence aktif dalam combined zoom (laptop+kartu), -1 = none
    bool easterEggActive; // true = sedang tampil teks easter egg di zoom cermin
    EvidencePopup evidencePopup;

    // Evidence system
    Case402::EvidenceLinkedList evidenceList;
    Case402::EvidenceHashTable evidenceHash;
    Case402::AVLTree evidenceTree;
    std::vector<Case402::Evidence*> inventorySlots; // 6 slots
    std::vector<Case402::Evidence*> allEvidence;
    int totalEvidenceFound;

    // Suspects
    std::vector<Case402::Suspect> suspects;
    int currentSuspectIdx;

    // Chapter 2 investigation notes
    // STL vector + iterator (Academic Req #9)
    std::vector<Case402::InvestigationNote> investigationNotes;
    std::vector<std::vector<Case402::InvestigationNote>> suspectNotes;
    bool inChapter2Zoom; // false = lihat meja kerja dulu (map), true = sudah zoom ke board investigasi

    // Chapter 3 - puzzle
    std::vector<int> puzzleOrder;       // current arrangement
    std::vector<int> correctOrder;      // correct arrangement
    std::vector<std::string> photoLabels;
    int selectedPuzzleSlot;
    bool puzzleSolved;
    int chapter3SuspectPick; // 0=Marcus, 1=Olivia, 2=Ethan -> foto mana yang ditampilkan di slot tengah-atas

    // Accusation
    int selectedSuspect; // -1 = none chosen
    int flagCount;

    // Graph for suspect relations
    Case402::SuspectGraph suspectGraph;

    // Fade transition
    float fadeAlpha;
    bool fadingOut;
    bool fadingIn;
    GameScene nextSceneAfterFade;

    // Sound placeholder flags
    bool clickSoundReady;

    // Queue for chapter text
    Case402::TextQueue textQueue;

    // UI buttons
    std::vector<Button> currentButtons;

    // ====== VARIABEL ASSET ======
    // Path dasar folder UI (sesuai struktur folder project: Grapic/UI/...)
    static constexpr const char* UI_PATH = "Graphic/UI/";
    // Path folder asset gameplay Chapter 1 (background zoom, popup, inventory)
    static constexpr const char* ASSET_PATH = "Graphic/Assets/";

    // Tombol Navigasi awal (Main Menu)
    Texture2D btnStartNormal;
    Texture2D btnCreditsNormal;
    Texture2D btnExitNormal;
    Texture2D btnStartHover;
    Texture2D btnCreditsHover;
    Texture2D btnExitHover;

    // Tombol Navigasi Pause
    Texture2D btnPause;
    Texture2D uiPauseBox;     // panel background pause (bukan tombol klik)
    Texture2D btnContinue;
    Texture2D btnMainMenu;

    // Tombol Navigasi Chapter 1
    Texture2D btnCh1Left;
    Texture2D btnCh1Right;
    Texture2D btnCh1Down;

    // Tombol Navigasi Chapter 2
    Texture2D btnCh2Left;
    Texture2D btnCh2LeftHover;
    Texture2D btnCh2Right;
    Texture2D btnCh2RightHover;
    Texture2D btnConfirm;

    // Tombol Navigasi Accusation (Chapter 4 - memilih pelaku)
    Texture2D btnConfirmSuspect;

    // Tombol Back (ending screens)
    Texture2D btnBack;

    // ====== ASSET CHAPTER 2 BOARD (full-screen suspect boards) ======
    Texture2D boardOlivia;
    Texture2D boardMarcus;
    Texture2D boardEthan;
    Texture2D bgChapter2Workspace;   // background meja kerja Chapter 2 (chapter2Workspaces.png)

    // ====== ASSET CHAPTER 3 BOARD (reconstruction puzzle) ======
    Texture2D bgChapter3Puzzle;     // background papan cork (chapter3Board.png)
    Texture2D evCctv;                // Cctv.png
    Texture2D evCctvOlivia;          // CctvOlivia.png
    Texture2D evCctvMarcus;          // CctvMarkus.png
    Texture2D evCctvEthan;           // CctvEthan.png
    // evFingerprint & evAccessLog dihapus -> reuse ivtFingerprint.png & ivtAccessLog.png (Chapter 1) langsung
    Texture2D evLaptop;              // ivtLaptop.png
    Texture2D evSafe;                // Safe.png
    Texture2D evMissingBottle;       // MissingBottle.png
    Texture2D evVictimKilled;        // VictimKilled.png
    // Foto kecil tersangka (tombol pilih di bawah slot tengah-atas)
    Texture2D suspectThumbMarcus;    // marcus.png
    Texture2D suspectThumbOlivia;    // olivia.png
    Texture2D suspectThumbEthan;     // ethan.png

    // ====== ASSET GAMEPLAY CHAPTER 1 (background zoom, popup, inventory) ======
    // Background map ruangan (full layar, belum zoom)
    Texture2D bgRoomLiving;
    Texture2D bgRoomKitchen;

    // Background saat zoom ke masing-masing hotspot (full layar, sidebar inventory tetap ikut di atasnya)
    Texture2D bgZoomMirror;
    Texture2D bgZoomFingerprint;
    Texture2D bgZoomSafe;
    Texture2D bgZoomBottle;
    Texture2D bgZoomPhone;
    Texture2D bgZoomTable;      // untuk combined zoom Laptop + Kartu Akses

    // Gambar item yang muncul di popup "ambil bukti"
    Texture2D popupMirror;
    Texture2D popupFingerprint;
    Texture2D popupSafe;
    Texture2D popupBottle;
    Texture2D popupPhone;
    Texture2D popupLaptop;
    Texture2D popupAccessLog;

    // Ikon kecil untuk slot inventory
    Texture2D ivtMirror;
    Texture2D ivtFingerprint;
    Texture2D ivtSafe;
    Texture2D ivtBottle;
    Texture2D ivtPhone;
    Texture2D ivtLaptop;
    Texture2D ivtAccessLog;

public:
    Case402Game(int w, int h) : screenW(w), screenH(h) {
        currentScene = SCENE_MAIN_MENU;
        previousScene = SCENE_MAIN_MENU;
        paused = false;
        memset(nameInputBuf, 0, sizeof(nameInputBuf));
        nameLen = 0;
        nameConfirmed = false;
        prologStep = 0;
        sceneTimer = 0;
        waitingForClick = false;
        currentRoom = ROOM_LIVING;
        selectedCloseUp = -1;
        inCloseUp = false;
        closeUpSubStep = 0;
        pendingEvidenceIdx = -1;
        easterEggActive = false;
        totalEvidenceFound = 0;
        currentSuspectIdx = 0;
        inChapter2Zoom = false;
        selectedPuzzleSlot = -1;
        puzzleSolved = false;
        chapter3SuspectPick = 0;
        selectedSuspect = -1;
        flagCount = 0;
        fadeAlpha = 1.0f;
        fadingOut = false;
        fadingIn = true;
        nextSceneAfterFade = SCENE_MAIN_MENU;
        clickSoundReady = false;

        initRooms();
        initEvidence();
        initSuspects();
        initChapter3Puzzle();
        loadAssets();
    }

    ~Case402Game() {
        unloadAssets();
    }

    // =====================================================================
    // PROSES LOAD ASSET PNG (TOMBOL NAVIGASI)
    // Semua file diletakkan di folder Grapic/UI/ relatif terhadap .exe
    // =====================================================================
    void loadAssets() {
        // Tombol Main Menu
        btnStartNormal   = LoadTexture((std::string(UI_PATH) + "Start.png").c_str());
        btnCreditsNormal = LoadTexture((std::string(UI_PATH) + "Credits.png").c_str());
        btnExitNormal    = LoadTexture((std::string(UI_PATH) + "Exit.png").c_str());
        btnStartHover    = LoadTexture((std::string(UI_PATH) + "StartHover.png").c_str());
        btnCreditsHover  = LoadTexture((std::string(UI_PATH) + "CreditsHoover.png").c_str());
        btnExitHover     = LoadTexture((std::string(UI_PATH) + "ExitHoover.png").c_str());

        // Tombol & panel Pause
        btnPause    = LoadTexture((std::string(UI_PATH) + "Pause.png").c_str());
        uiPauseBox  = LoadTexture((std::string(UI_PATH) + "PauseBox.png").c_str());
        btnContinue = LoadTexture((std::string(UI_PATH) + "Continue.png").c_str());
        btnMainMenu = LoadTexture((std::string(UI_PATH) + "MainMenu.png").c_str());

        // Tombol navigasi Chapter 1
        btnCh1Left  = LoadTexture((std::string(UI_PATH) + "btnCh1Left.png").c_str()); // kiri
        btnCh1Right = LoadTexture((std::string(UI_PATH) + "btnCh1Right.png").c_str()); // kanan 
        btnCh1Down  = LoadTexture((std::string(UI_PATH) + "btnCh1Down.png").c_str());  // unzoom

        // Tombol navigasi Chapter 2
        btnCh2Left       = LoadTexture((std::string(UI_PATH) + "btnCh2Left.png").c_str());
        btnCh2LeftHover  = LoadTexture((std::string(UI_PATH) + "btnCh2LeftHoover.png").c_str());
        btnCh2Right      = LoadTexture((std::string(UI_PATH) + "btnCh2Right.png").c_str());
        btnCh2RightHover = LoadTexture((std::string(UI_PATH) + "btnCh2RightHoover.png").c_str());
        btnConfirm  = LoadTexture((std::string(UI_PATH) + "Confirm.png").c_str());

        // Tombol confirm suspect (Accusation)
        btnConfirmSuspect = LoadTexture((std::string(UI_PATH) + "confirmSuspect.png").c_str());

        // Tombol Back (ending screens)
        btnBack = LoadTexture((std::string(UI_PATH) + "Back.png").c_str());

        // Chapter 2 suspect boards (full 16:9)
        boardOlivia = LoadTexture((std::string(ASSET_PATH) + "oliviaBoard.png").c_str());
        boardMarcus = LoadTexture((std::string(ASSET_PATH) + "marcusBoard.png").c_str());
        boardEthan  = LoadTexture((std::string(ASSET_PATH) + "ethanBoard.png").c_str());
        bgChapter2Workspace = LoadTexture((std::string(ASSET_PATH) + "chapter2Workspaces.png").c_str());

        // Chapter 3 reconstruction board
        bgChapter3Puzzle = LoadTexture((std::string(ASSET_PATH) + "chapter3Board.png").c_str());
        evCctv           = LoadTexture((std::string(ASSET_PATH) + "Cctv.png").c_str());
        evCctvOlivia     = LoadTexture((std::string(ASSET_PATH) + "CctvOlivia.png").c_str());
        evCctvMarcus     = LoadTexture((std::string(ASSET_PATH) + "CctvMarcus.png").c_str());
        evCctvEthan      = LoadTexture((std::string(ASSET_PATH) + "CctvEthan.png").c_str());
        // Fingerprint & AccessLog reuse texture Chapter 1 (ivtFingerprint / ivtAccessLog) -> sudah di-load di bawah
        evLaptop         = LoadTexture((std::string(ASSET_PATH) + "ivtLaptop.png").c_str());
        evSafe           = LoadTexture((std::string(ASSET_PATH) + "Safe.png").c_str());
        evMissingBottle  = LoadTexture((std::string(ASSET_PATH) + "MissingBottle.png").c_str());
        evVictimKilled   = LoadTexture((std::string(ASSET_PATH) + "VictimKilled.png").c_str());
        suspectThumbMarcus = LoadTexture((std::string(ASSET_PATH) + "marcus.png").c_str());
        suspectThumbOlivia = LoadTexture((std::string(ASSET_PATH) + "olivia.png").c_str());
        suspectThumbEthan  = LoadTexture((std::string(ASSET_PATH) + "ethan.png").c_str());

        // ====== Asset gameplay Chapter 1 ======
        // chapter 1
        // ini tidak harusnya tapi inventorynya ikut ke semua foto jadi full layar
        // map ruangan (belum zoom)
        bgRoomLiving  = LoadTexture((std::string(ASSET_PATH) + "cp1MapLiving.png").c_str());
        bgRoomKitchen = LoadTexture((std::string(ASSET_PATH) + "cp1MapKitchen.png").c_str());

        // zoom layar
        bgZoomMirror      = LoadTexture((std::string(ASSET_PATH) + "zoomMirror.png").c_str());
        bgZoomFingerprint = LoadTexture((std::string(ASSET_PATH) + "zoomFingerprint.png").c_str());
        bgZoomSafe        = LoadTexture((std::string(ASSET_PATH) + "zoomSafe.png").c_str());
        bgZoomBottle      = LoadTexture((std::string(ASSET_PATH) + "zoomBottle.png").c_str());
        bgZoomPhone       = LoadTexture((std::string(ASSET_PATH) + "zoomPhone.png").c_str());
        // untuk dua bukti laptop dan access card
        bgZoomTable       = LoadTexture((std::string(ASSET_PATH) + "zoomtable.png").c_str());

        // pop up layar
        popupMirror      = LoadTexture((std::string(ASSET_PATH) + "popupMirror.png").c_str());
        popupFingerprint = LoadTexture((std::string(ASSET_PATH) + "popupFingerprint.png").c_str());
        popupSafe        = LoadTexture((std::string(ASSET_PATH) + "popupSafe.png").c_str());
        popupBottle      = LoadTexture((std::string(ASSET_PATH) + "popupBottle.png").c_str());
        popupPhone       = LoadTexture((std::string(ASSET_PATH) + "popupPhone.png").c_str());
        // untuk dua bukti laptop dan access card
        popupLaptop      = LoadTexture((std::string(ASSET_PATH) + "popupLaptop.png").c_str());
        popupAccessLog   = LoadTexture((std::string(ASSET_PATH) + "popupAccessLog.png").c_str());

        // Inventory
        ivtMirror      = LoadTexture((std::string(ASSET_PATH) + "ivtMirror.png").c_str());
        ivtFingerprint = LoadTexture((std::string(ASSET_PATH) + "ivtFingerprint.png").c_str());
        ivtSafe        = LoadTexture((std::string(ASSET_PATH) + "ivtSafe.png").c_str());
        ivtBottle      = LoadTexture((std::string(ASSET_PATH) + "ivtBottle.png").c_str());
        ivtPhone       = LoadTexture((std::string(ASSET_PATH) + "ivtPhone.png").c_str());
        // untuk dua bukti laptop dan access card
        ivtLaptop      = LoadTexture((std::string(ASSET_PATH) + "ivtLaptop.png").c_str());
        ivtAccessLog   = LoadTexture((std::string(ASSET_PATH) + "ivtAccessLog.png").c_str());
    }

    void unloadAssets() {
        UnloadTexture(btnStartNormal);
        UnloadTexture(btnCreditsNormal);
        UnloadTexture(btnExitNormal);
        UnloadTexture(btnStartHover);
        UnloadTexture(btnCreditsHover);
        UnloadTexture(btnExitHover);

        UnloadTexture(btnPause);
        UnloadTexture(uiPauseBox);
        UnloadTexture(btnContinue);
        UnloadTexture(btnMainMenu);

        UnloadTexture(btnCh1Left);
        UnloadTexture(btnCh1Right);
        UnloadTexture(btnCh1Down);

        UnloadTexture(btnCh2Left);
        UnloadTexture(btnCh2LeftHover);
        UnloadTexture(btnCh2Right);
        UnloadTexture(btnCh2RightHover);
        UnloadTexture(btnConfirm);

        UnloadTexture(btnConfirmSuspect);
        UnloadTexture(btnBack);

        // Chapter 2 board textures
        UnloadTexture(boardOlivia);
        UnloadTexture(boardMarcus);
        UnloadTexture(boardEthan);
        UnloadTexture(bgChapter2Workspace);

        // Chapter 3 board textures
        UnloadTexture(bgChapter3Puzzle);
        UnloadTexture(evCctv);
        UnloadTexture(evCctvOlivia);
        UnloadTexture(evCctvMarcus);
        UnloadTexture(evCctvEthan);
        // evFingerprint & evAccessLog dihapus -> sudah di-unload via ivtFingerprint/ivtAccessLog
        UnloadTexture(evLaptop);
        UnloadTexture(evSafe);
        UnloadTexture(evMissingBottle);
        UnloadTexture(evVictimKilled);
        UnloadTexture(suspectThumbMarcus);
        UnloadTexture(suspectThumbOlivia);
        UnloadTexture(suspectThumbEthan);

        // Asset gameplay Chapter 1
        UnloadTexture(bgRoomLiving);
        UnloadTexture(bgRoomKitchen);

        UnloadTexture(bgZoomMirror);
        UnloadTexture(bgZoomFingerprint);
        UnloadTexture(bgZoomSafe);
        UnloadTexture(bgZoomBottle);
        UnloadTexture(bgZoomPhone);
        UnloadTexture(bgZoomTable);

        UnloadTexture(popupMirror);
        UnloadTexture(popupFingerprint);
        UnloadTexture(popupSafe);
        UnloadTexture(popupBottle);
        UnloadTexture(popupPhone);
        UnloadTexture(popupLaptop);
        UnloadTexture(popupAccessLog);

        UnloadTexture(ivtMirror);
        UnloadTexture(ivtFingerprint);
        UnloadTexture(ivtSafe);
        UnloadTexture(ivtBottle);
        UnloadTexture(ivtPhone);
        UnloadTexture(ivtLaptop);
        UnloadTexture(ivtAccessLog);
    }

    // ==========================================================================
    // INITIALIZATION
    // ==========================================================================
    void initRooms() {
        roomList.push("Ruang Tamu");
        roomList.push("Dapur");
    }

    void initEvidence() {
        inventorySlots.resize(6, nullptr);

        // Using pointer allocation (Academic Req #3)
        Case402::Evidence* ev1 = new Case402::Evidence(1, "Brankas Terbuka",
            "Brankas tempat penyimpanan obat kanker ini kosong. Penelitian tersebut telah hilang.",
            "safe");
        Case402::Evidence* ev2 = new Case402::Evidence(2, "Gelas Kopi",
            "Terdapat sidik jari yang berbeda dari milik Sheryl. Seseorang lain berada di sini sebelum kematiannya.",
            "cup");
        Case402::Evidence* ev3 = new Case402::Evidence(3, "Ponsel Sheryl",
            "Terdapat pesan terakhir yang diterima beberapa jam sebelum kematian korban.",
            "phone");
        Case402::Evidence* ev4 = new Case402::Evidence(4, "Laptop",
            "Laptop terkunci. Kemungkinan menyimpan data penting mengenai penelitian yang hilang.",
            "laptop");
        Case402::Evidence* ev5 = new Case402::Evidence(5, "Kartu Akses",
            "Catatan akses mungkin dapat menunjukkan siapa yang datang malam itu.",
            "card");
        Case402::Evidence* ev6 = new Case402::Evidence(6, "Pecahan Botol Obat",
            "Apakah ini sampel penelitian? Ditemukan dalam keadaan pecah di lantai.",
            "bottle");

        evidenceList.insert(ev1);
        evidenceList.insert(ev2);
        evidenceList.insert(ev3);
        evidenceList.insert(ev4);
        evidenceList.insert(ev5);
        evidenceList.insert(ev6);

        evidenceHash.insert("safe", ev1);
        evidenceHash.insert("cup", ev2);
        evidenceHash.insert("phone", ev3);
        evidenceHash.insert("laptop", ev4);
        evidenceHash.insert("card", ev5);
        evidenceHash.insert("bottle", ev6);

        evidenceTree.insertEvidence(1, "physical");
        evidenceTree.insertEvidence(3, "digital");
        evidenceTree.insertEvidence(2, "biological");
        evidenceTree.insertEvidence(5, "access");
        evidenceTree.insertEvidence(4, "digital");
        evidenceTree.insertEvidence(6, "chemical");

        // Fill allEvidence vector and sort
        Case402::Evidence* cur = evidenceList.head;
        while (cur) {
            allEvidence.push_back(cur);
            cur = cur->next;
        }
        Case402::sortEvidenceVector(allEvidence);
    }

    // ini mau centangnya aja di kotak
    void initSuspects() {
    Case402::Suspect s1;
    s1.id = 1; s1.name = "Olivia Hartwell"; s1.age = 29; s1.height = 1.65f;
    s1.job = "Asisten Peneliti";
    s1.relation = "Asisten peneliti, sudah bekerja 3 tahun bersama Sheryl.";
    s1.motive = "Kontribusinya tidak pernah dihargai dan merasa dicuri idenya.";
    s1.alibi = "Saya sudah berada di rumah ketika kejadian.";
    s1.cctv_time = "Masuk gedung pada pukul 20:40";
    s1.isGuilty = false;
    s1.checklistItems = { "", "", "", "", "" };
    s1.checked.resize(s1.checklistItems.size(), false);

    Case402::Suspect s2;
    s2.id = 2; s2.name = "Marcus Vale"; s2.age = 51; s2.height = 1.80f;
    s2.job = "Investor";
    s2.relation = "Pendana utama penelitian.";
    s2.motive = "Kecurigaan bahwa obat gagal dan takut kehilangan jutaan dolar.";
    s2.alibi = "Menghadiri acara bisnis.";
    s2.cctv_time = "Masuk gedung pada pukul 21:37";
    s2.isGuilty = false;
    s2.checklistItems = { "", "", "", "", "" };
    s2.checked.resize(s2.checklistItems.size(), false);

    Case402::Suspect s3;
    s3.id = 3; s3.name = "Ethan Cross"; s3.age = 38; s3.height = 1.76f;
    s3.job = "Mantan Rekan Kerja";
    s3.relation = "Mantan rekan kerja, berselisih dengan Sheryl.";
    s3.motive = "Karir hancur jika Sheryl sukses.";
    s3.alibi = "Menghadiri acara bisnis.";
    s3.cctv_time = "Masuk gedung pada pukul 21:50, wajah tidak terlihat jelas namun pakaian cocok";
    s3.isGuilty = true; // THE TRUE CULPRIT
    s3.checklistItems = { "", "", "", "", "" };
    s3.checked.resize(s3.checklistItems.size(), false);

    suspects.push_back(s1);
    suspects.push_back(s2);
    suspects.push_back(s3);

    // Initialize suspect investigation notes
    suspectNotes.clear();
    for (int i = 0; i < 3; i++) {
        std::vector<Case402::InvestigationNote> notes;
        notes.push_back(Case402::InvestigationNote("Deskripsi Saksi", "Laki-laki, tinggi 170-180cm, rambut coklat"));
        notes.push_back(Case402::InvestigationNote("Observasi CCTV", suspects[i].cctv_time));
        notes.push_back(Case402::InvestigationNote("Analisis Sidik Jari", "Sidik jari ditemukan di gelas kopi"));
        notes.push_back(Case402::InvestigationNote("Catatan Alibi", suspects[i].alibi));
        notes.push_back(Case402::InvestigationNote("Hubungan dgn Korban", suspects[i].relation));
        suspectNotes.push_back(notes);
    }

    // Build suspect relationship graph (Academic Req #5)
    suspectGraph.addNode(0, "Sheryl Iris");
    suspectGraph.addNode(1, "Olivia Hartwell");
    suspectGraph.addNode(2, "Marcus Vale");
    suspectGraph.addNode(3, "Ethan Cross");
    suspectGraph.addEdge(0, 1);
    suspectGraph.addEdge(0, 2);
    suspectGraph.addEdge(0, 3);
    suspectGraph.addEdge(1, 2); 
    
    auto bfsResult = suspectGraph.bfs(0);
    auto dfsResult = suspectGraph.dfs(0);
    (void)bfsResult; (void)dfsResult;
}

    // ini aku pake foto sendiri jadinya
    void initChapter3Puzzle() {
        // 7 evidence sesuai aset PNG yang dipakai (lihat getEvidenceTexture()):
        // 0=ivtAccessLog, 1=MissingBottle, 2=ivtFingerprint, 3=VictimKilled, 4=ivtLaptop(Laptop), 5=Safe, 6=Cctv
        photoLabels = {
            "Access Card Log",  // index 0 -> ivtAccessLog.png
            "Missing Bottle",   // index 1 -> MissingBottle.png
            "Fingerprint",      // index 2 -> ivtFingerprint.png
            "Victim Photo",     // index 3 -> VictimKilled.png
            "Laptop",           // index 4 -> ivtLaptop.png
            "Empty Safe",       // index 5 -> Safe.png
            "CCTV Footage"      // index 6 -> Cctv.png
        };
        // Urutan kejadian sesuai referensi: CCTV -> Access Log -> Missing Bottle ->
        // Fingerprint -> Laptop -> Safe -> Victim (obat hilang & pembunuhan)
        correctOrder = {6, 0, 1, 2, 3, 5, 4};
        // Shuffle awal untuk puzzle
        puzzleOrder = {3, 6, 1, 4, 0, 2, 5};
    }

    // Texture evidence sesuai index puzzleOrder (lihat initChapter3Puzzle untuk mapping index ke photoLabels)
    Texture2D getEvidenceTexture(int idx) {
        switch (idx) {
            case 0: return ivtAccessLog;
            case 1: return evMissingBottle;
            case 2: return ivtFingerprint;
            case 3: return evVictimKilled;
            case 4: return ivtLaptop;
            case 5: return evSafe;
            case 6: return evCctv;
            default: return evCctv;
        }
    }

    // Texture tersangka kecil sesuai pilihan chapter3SuspectPick (0=Marcus,1=Olivia,2=Ethan)
    Texture2D getSuspectThumbTexture(int idx) {
        switch (idx) {
            case 0: return suspectThumbMarcus;
            case 1: return suspectThumbOlivia;
            default: return suspectThumbEthan;
        }
    }

    // Texture untuk kartu accusation sesuai urutan suspects[] (0=Olivia,1=Marcus,2=Ethan)
    Texture2D getSuspectAccusationTexture(int suspectIdx) {
        switch (suspectIdx) {
            case 0: return suspectThumbOlivia;
            case 1: return suspectThumbMarcus;
            default: return suspectThumbEthan;
        }
    }

    // 8 slot foto sesuai posisi kotak placeholder di chapter3Board.png (1280x720).
    // Index 0..6 = slot evidence (mengikuti puzzleOrder), index 7 = slot foto tersangka (statis, bukan bagian puzzle).
    Rectangle getPuzzleSlotRect(int i) {
        static const Rectangle slots[8] = {
            {590, 143, 82, 94},   // 0: tengah-atas -> SLOT FOTO TERSANGKA (statis)
            {987, 116, 99, 78},   // 1: kanan-atas
            {164, 170, 79, 62},   // 2: kiri-tengah-atas
            {181, 347, 83, 65},   // 3: kiri-tengah-bawah
            {984, 301, 101, 79},  // 4: kanan-tengah
            {341, 537, 90, 70},   // 5: kiri-bawah
            {575, 536, 88, 71},   // 6: tengah-bawah
            {826, 489, 91, 72},   // 7: kanan-bawah
        };
        return slots[i];
    }

    // Slot index (dari getPuzzleSlotRect) yang dipakai untuk foto tersangka, sisanya untuk 7 evidence.
    static constexpr int SUSPECT_SLOT_INDEX = 0;

    // Mapping slot layout (0..7, termasuk slot tersangka) -> index puzzleOrder (0..6, evidence saja).
    // evidenceSlotOrder[k] = posisi layout (selain SUSPECT_SLOT_INDEX) untuk evidence puzzleOrder[k].
    std::vector<int> evidenceSlotPositions() {
        std::vector<int> positions;
        for (int i = 0; i < 8; i++) if (i != SUSPECT_SLOT_INDEX) positions.push_back(i);
        return positions; // {1,2,3,4,5,6,7}
    }

    // ==========================================================================
    // SCENE TRANSITION
    // ==========================================================================
    void startFadeToScene(GameScene next) {
        fadingOut = true;
        fadingIn = false;
        nextSceneAfterFade = next;
    }

    void updateFade(float dt) {
        if (fadingOut) {
            fadeAlpha = std::min(1.0f, fadeAlpha + dt * 2.5f);
            if (fadeAlpha >= 1.0f) {
                fadingOut = false;
                fadingIn = true;
                navStack.push((int)currentScene);
                currentScene = nextSceneAfterFade;
                onSceneEnter(currentScene);
                fadeAlpha = 1.0f;
            }
        } else if (fadingIn) {
            fadeAlpha = std::max(0.0f, fadeAlpha - dt * 2.0f);
            if (fadeAlpha <= 0.0f) {
                fadingIn = false;
                fadeAlpha = 0.0f;
            }
        }
    }

    void drawFadeOverlay() {
        if (fadeAlpha > 0.01f) {
            DrawRectangle(0, 0, screenW, screenH, {0, 0, 0, (unsigned char)(fadeAlpha * 255)});
        }
    }

    void onSceneEnter(GameScene scene) {
        sceneTimer = 0;
        waitingForClick = false;

        if (scene == SCENE_PROLOG) {
            prologStep = 0;
            typewriter.setText("20 April 2024");
            sceneTimer = 0;
        } else if (scene == SCENE_CHAPTER2_INTRO) {
            prologStep = -1; // step -1 = judul chapter fade-in dulu (seperti Ch1 & Ch3)
            typewriter.setText("");
        } else if (scene == SCENE_CHAPTER1_INTRO) {
            prologStep = 0; // step 0 = typewriter cerita dulu, step 1 = judul chapter
            typewriter.setText(""); // reset agar cerita pembuka chapter 1 pasti ter-trigger
        } else if (scene == SCENE_CHAPTER1_GAMEPLAY) {
            inCloseUp = false;
            selectedCloseUp = -1;
            pendingEvidenceIdx = -1;
            easterEggActive = false;
        } else if (scene == SCENE_CHAPTER2_BOARD) {
            currentSuspectIdx = 0;
            inChapter2Zoom = false;
        } else if (scene == SCENE_CHAPTER3_INTRO) {
            typewriter.setText(""); // reset agar teks prolog chapter 3 pasti ter-trigger
        } else if (scene == SCENE_CHAPTER3_PUZZLE) {
            puzzleSolved = false;
            selectedPuzzleSlot = -1;
            chapter3SuspectPick = 0;
        }
    }

    // ==========================================================================
    // UPDATE
    // ==========================================================================
    void update(float dt) {
        updateFade(dt);
        if (fadingOut || fadingIn) return;

        if (paused) {
            updatePause();
            return;
        }

        // Global ESC for pause (except main menu)
        if (IsKeyPressed(KEY_ESCAPE) && currentScene != SCENE_MAIN_MENU &&
            currentScene != SCENE_ENDING_TRUE && currentScene != SCENE_ENDING_BAD) {
            paused = true;
            return;
        }

        switch (currentScene) {
            case SCENE_MAIN_MENU:        updateMainMenu(); break;
            case SCENE_CREDITS:          updateCredits(); break;
            case SCENE_PROLOG:           updateProlog(dt); break;
            case SCENE_NAME_INPUT:       updateNameInput(); break;
            case SCENE_CHAPTER1_INTRO:   updateChapter1Intro(dt); break;
            case SCENE_CHAPTER1_GAMEPLAY:updateChapter1(dt); break;
            case SCENE_CHAPTER2_INTRO:   updateChapter2Intro(dt); break;
            case SCENE_CHAPTER2_BOARD:   updateChapter2Board(dt); break;
            case SCENE_CHAPTER3_INTRO:   updateChapter3Intro(dt); break;
            case SCENE_CHAPTER3_PUZZLE:  updateChapter3Puzzle(); break;
            case SCENE_ENDING_TRUE:
            case SCENE_ENDING_BAD:       updateEnding(); break;
            default: break;
        }
    }

    // ==========================================================================
    // MAIN MENU
    // ==========================================================================
    void updateMainMenu() {
        if (!IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) return;
        Vector2 mp = GetMousePosition();
        // Koordinat HARUS identik dengan drawMainMenu()
        const int btn1Y = screenH * 58 / 100;
        const int btn2Y = btn1Y + 90;
        const int btn3Y = btn2Y + 90;
        const float targetW = 220.0f, targetH = 65.0f;
        const float btnX = (float)screenW / 2.0f - targetW / 2.0f;
        Rectangle startR  = { btnX, (float)btn1Y, targetW, targetH };
        Rectangle creditR = { btnX, (float)btn2Y, targetW, targetH };
        Rectangle exitR   = { btnX, (float)btn3Y, targetW, targetH };
        if (CheckCollisionPointRec(mp, startR))  startFadeToScene(SCENE_PROLOG);
        if (CheckCollisionPointRec(mp, creditR)) startFadeToScene(SCENE_CREDITS);
        if (CheckCollisionPointRec(mp, exitR))   CloseWindow();
    }

    // ==========================================================================
    // CREDITS
    // ==========================================================================
    void updateCredits() {}

    // ==========================================================================
    // PROLOG
    // ==========================================================================
    void updateProlog(float dt) {
        typewriter.update(dt);
        sceneTimer += dt;

        bool clicked = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        if (typewriter.done && clicked) {
            typewriter.waitingClick = false;
            advancePrologStep();
        } else if (clicked && !typewriter.done) {
            typewriter.skip();
        }
    }

    void advancePrologStep() {
        prologStep++;
        sceneTimer = 0;
        switch (prologStep) {
            // Screen 0 sudah berisi "20 April 2024" (di-set oleh onSceneEnter)
            case 1:
                typewriter.setText(
                    "Malam itu, Dr. Sheryl Iris seharusnya menghadiri konferensi pers untuk "
                    "mengumumkan hasil penelitiannya. Namun, waktu terus berjalan dan ia tidak "
                    "pernah muncul. Karena merasa ada sesuatu yang tidak beres, asistennya pergi "
                    "menuju apartemen nomor 402.");
                break;
            case 2:
                typewriter.setText(
                    "Pintu apartemen tidak terkunci. Tidak ada jawaban dari dalam ucap sang asisten. "
                    "Di dalam ruangan, ia menemukan Dr. Sheryl Iris tergeletak dan tidak bergerak. "
                    "Polisi segera dipanggil dan lokasi diamankan.");
                break;
            case 3:  // Setelah screen 3 ke name input
                startFadeToScene(SCENE_NAME_INPUT);
                break;
        }
    }

    // ==========================================================================
    // NAME INPUT
    // ==========================================================================
    void updateNameInput() {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 125 && nameLen < 20) {
                nameInputBuf[nameLen++] = (char)key;
                nameInputBuf[nameLen] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && nameLen > 0) {
            nameInputBuf[--nameLen] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER) && nameLen > 0) {
            playerName = std::string(nameInputBuf);
            FileIO::savePlayerName(playerName);
            startFadeToScene(SCENE_CHAPTER1_INTRO);
        }
    }

    // ==========================================================================
    // CHAPTER 1 INTRO
    // ==========================================================================
    void updateChapter1Intro(float dt) {
        sceneTimer += dt;
        typewriter.update(dt);

        // Step 0: typewriter cerita pembuka dulu, layar polos di tengah (belum ada judul chapter)
        if (prologStep == 0) {
            if (sceneTimer > 0.5f && typewriter.fullText.empty()) {
                typewriter.setText(TextFormat(
                    "Detektif %s, anda ditugaskan menangani kasus kematian Dr. Iris. "
                    "Temukan siapa pelakunya dan ungkap apa yang sebenarnya terjadi di apartemen 402",
                    playerName.c_str()));
            }

            if (typewriter.done && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                prologStep = 1;
                sceneTimer = 0;
                typewriter.setText("");
            }
            return;
        }

        // Step 1: baru tampilkan judul CHAPTER 1, lalu lanjut ke gameplay
        if (sceneTimer > 2.0f) {
            startFadeToScene(SCENE_CHAPTER1_GAMEPLAY);
        }
    }

    // ==========================================================================
    // CHAPTER 1 GAMEPLAY
    // ==========================================================================
    void updateChapter1(float dt) {
        typewriter.update(dt);

        // ===== MODE 1: MAP VIEW (belum zoom) =====
        if (!inCloseUp) {
            // Navigasi ruangan melingkar — circular linked list jadi satu-satunya
            // sumber kebenaran posisi ruangan (Ruang Tamu -> Dapur -> Kamar Tidur -> kembali)
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                roomList.next();
                currentRoom = (RoomType)((currentRoom + 1) % ROOM_COUNT);
            }
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                roomList.prev();
                currentRoom = (RoomType)((currentRoom - 1 + ROOM_COUNT) % ROOM_COUNT);
            }

            // On-screen arrow buttons (mouse click) — koordinat HARUS sama persis
            // dengan posisi gambar tombol di drawChapter1(), yaitu digeser oleh sidebar kiri
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                Vector2 mp = GetMousePosition();

                float btnScaleW2 = (float)btnCh1Left.width  / 6.0f;
                float btnScaleH2 = (float)btnCh1Left.height / 6.0f;
                float arrowY = screenH / 2.0f - btnScaleH2 / 2.0f;
                Rectangle leftArrowRect  = {131.0f, arrowY, btnScaleW2, btnScaleH2};
                Rectangle rightArrowRect = {(float)screenW - btnScaleW2 - 20.0f, arrowY, btnScaleW2, btnScaleH2};

                if (CheckCollisionPointRec(mp, rightArrowRect)) {
                    roomList.next();
                    currentRoom = (RoomType)((currentRoom + 1) % ROOM_COUNT);
                } else if (CheckCollisionPointRec(mp, leftArrowRect)) {
                    roomList.prev();
                    currentRoom = (RoomType)((currentRoom - 1 + ROOM_COUNT) % ROOM_COUNT);
                } else {
                    checkHotspotClick(mp);
                }
            }
        }
        // ===== MODE 2: ZOOM VIEW (sudah masuk close-up area, ala Rusty Lake) =====
        else {
            // Popup evidence sedang aktif → tangani klik untuk mengambil bukti
            if (evidencePopup.active) {
                evidencePopup.update(dt);
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                    // Untuk combined zoom (-3) gunakan pendingEvidenceIdx,
                    // untuk single zoom gunakan selectedCloseUp seperti biasa
                    int evIdx = (selectedCloseUp == -3) ? pendingEvidenceIdx : selectedCloseUp;
                    if (evIdx >= 0 && evIdx < (int)allEvidence.size()) {
                        Case402::Evidence* ev = allEvidence[evIdx];
                        if (!ev->found) {
                            ev->found = true;
                            for (int i = 0; i < 6; i++) {
                                if (!inventorySlots[i]) {
                                    inventorySlots[i] = ev;
                                    break;
                                }
                            }
                            totalEvidenceFound++;
                            std::vector<bool> foundFlags;
                            for (auto* e : allEvidence) foundFlags.push_back(e->found);
                            FileIO::saveProgress(1, foundFlags);
                        }
                    }
                    evidencePopup.active = false;
                    pendingEvidenceIdx = -1;

                    if (selectedCloseUp == -3) {
                        // Combined zoom: tetap di zoom sampai KEDUA bukti (laptop+kartu) diambil
                        if (allEvidence[3]->found && allEvidence[4]->found) {
                            inCloseUp = false;
                            selectedCloseUp = -1;
                            closeUpSubStep = 0;
                        }
                        // Jika baru satu yang diambil, tetap di combined zoom
                    } else {
                        // Single-item zoom: kembali ke map setelah popup
                        inCloseUp = false;
                        selectedCloseUp = -1;
                        closeUpSubStep = 0;
                    }

                    if (totalEvidenceFound >= 6) {
                        typewriter.setText("Bukti telah cukup ditemukan");
                        waitingForClick = true;
                    }
                }
                return;
            }

            // Belum ada popup: kita berada di scene ZOOM, tunggu klik pada:
            // (a) tombol panah-bawah → kembali ke map
            // (b) area barang yang sudah di-zoom → munculkan popup evidence
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                Vector2 mp = GetMousePosition();

                // Koordinat HARUS sama persis dengan drawCloseUp():
                // - zoomCenterX untuk area klik item (background sekarang full 16:9)
                // - oldMapCenterX untuk panah bawah (posisinya TIDAK ikut berubah)
                float zoomCenterX = screenW / 2.0f;
                float oldMapX = SIDEBAR_W + 20.0f;
                float oldMapW = screenW - SIDEBAR_W - 40.0f;
                float oldMapCenterX = oldMapX + oldMapW / 2.0f;

                // Tombol panah bawah (back to map) — ukuran baru (1/6) sesuai drawCloseUp
                float downScaleW2 = (float)btnCh1Down.width  / 6.0f;
                float downScaleH2 = (float)btnCh1Down.height / 6.0f;
                Rectangle backDownRect = {
                    (screenW / 2.0f - downScaleW2 / 2.0f) + 50.0f,
                    (float)screenH - downScaleH2 - 25.0f,
                    downScaleW2, downScaleH2
                };
                if (CheckCollisionPointRec(mp, backDownRect)) {
                    // Easter egg cermin: hilangkan teks setelah keluar dari zoom
                    if (selectedCloseUp == -2) {
                        easterEggActive = false;
                    }
                    inCloseUp = false;
                    selectedCloseUp = -1;
                    closeUpSubStep = 0;
                    return;
                }

                // Tombol/area barang di dalam zoom (sesuai selectedCloseUp)
                if (selectedCloseUp == -2) {
                    // Cermin: klik dimana saja di area cermin untuk munculkan teks
                    Rectangle mirrorObjRect = {zoomCenterX - 110, 140, 180, 260};
                    if (CheckCollisionPointRec(mp, mirrorObjRect)) {
                        easterEggActive = true;
                        typewriter.setText("i think therefore i am");
                    }
                } else if (selectedCloseUp >= 0 && selectedCloseUp < (int)allEvidence.size()) {
                    // Evidence: klik tombol "ambil barang" di tengah area zoom
                    // Per-evidence click rect (posisi disesuaikan dengan posisi objek di bg zoom)
                    Rectangle itemBtnRect;
                    if      (selectedCloseUp == 0) itemBtnRect = {zoomCenterX - 120, 220, 180, 140}; // Safe: lebih kiri
                    else if (selectedCloseUp == 1) itemBtnRect = {zoomCenterX -  95, 200, 180, 140}; // Fingerprint: atas, kiri dikit
                    else if (selectedCloseUp == 5) itemBtnRect = {zoomCenterX - 110, 230, 180, 140}; // Bottle: kiri+bawah dikit
                    else                           itemBtnRect = {zoomCenterX -  90, 220, 180, 140}; // default
                    if (CheckCollisionPointRec(mp, itemBtnRect)) {
                        Case402::Evidence* ev = allEvidence[selectedCloseUp];
                        evidencePopup.show(ev->name, ev->description, getPopupTexture(selectedCloseUp));
                    }
                } else if (selectedCloseUp == -3) {
                    // Combined Laptop + Kartu Akses: dua tombol bukti berdampingan
                    Rectangle laptopRect = {zoomCenterX - 220.0f, 180.0f, 160.0f, 160.0f};
                    Rectangle cardRect   = {zoomCenterX +  50.0f, 210.0f, 160.0f, 160.0f};
                    if (!allEvidence[3]->found && CheckCollisionPointRec(mp, laptopRect)) {
                        pendingEvidenceIdx = 3;
                        evidencePopup.show(allEvidence[3]->name, allEvidence[3]->description, getPopupTexture(3));
                    } else if (!allEvidence[4]->found && CheckCollisionPointRec(mp, cardRect)) {
                        pendingEvidenceIdx = 4;
                        evidencePopup.show(allEvidence[4]->name, allEvidence[4]->description, getPopupTexture(4));
                    }
                }
            }
        }

        if (totalEvidenceFound >= 6 && waitingForClick &&
            typewriter.done && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            waitingForClick = false;
            startFadeToScene(SCENE_CHAPTER2_INTRO);
        }
    }

    void checkHotspotClick(Vector2& mp) {
        struct HotspotData {
            Rectangle rect;
            int evidenceIdx; // -2 = easter egg cermin, -1 = unused, >=0 evidence index
            const char* label;
        };

        std::vector<HotspotData> hotspots;

        if (currentRoom == ROOM_LIVING) {
            // Koordinat HARUS sama persis dengan drawRoomScene()
            hotspots = {
                {{(float)(ROOM_OX + 420), 310, 110, 115}, 0, "Brankas"},
                {{(float)(ROOM_OX + 710), 432, 115, 100}, 1, "Meja Kopi"},
                {{(float)(ROOM_OX + 895), 600, 110, 100}, 2, "Lantai/HP"},
                {{(float)(ROOM_OX + 715), 192.0f, 135.0f, 140.0f}, -2, "Cermin"}
            };
        } else { // ROOM_KITCHEN
            hotspots = {
                // Pecahan Botol — meja dapur kiri (lingkaran kiri pada gambar)
                {{(float)(ROOM_OX + 395), 383, 105, 105}, 5, "Pecahan Botol"},
                // Laptop & Kartu Akses — satu hotspot gabungan, counter kanan (lingkaran kanan)
                {{(float)(ROOM_OX + 1005), 360, 150, 130}, -3, "Laptop+Kartu"},
            };
        }

        for (auto& hs : hotspots) {
            if (CheckCollisionPointRec(mp, hs.rect)) {
                if (hs.evidenceIdx == -3) {
                    // Combined Laptop+Kartu: masuk zoom hanya jika minimal satu belum diambil
                    if (!allEvidence[3]->found || !allEvidence[4]->found) {
                        selectedCloseUp = -3;
                        inCloseUp = true;
                        closeUpSubStep = 0;
                    }
                    break;
                }
                // Jangan zoom ulang ke evidence yang sudah ditemukan
                if (hs.evidenceIdx >= 0 && allEvidence[hs.evidenceIdx]->found) break;

                // Klik hotspot → ZOOM dulu ke area (belum munculkan popup)
                selectedCloseUp = hs.evidenceIdx;
                inCloseUp = true;
                closeUpSubStep = 0;
                break;
            }
        }
    }

    // ==========================================================================
    // CHAPTER 2 INTRO
    // ==========================================================================
    void updateChapter2Intro(float dt) {
        sceneTimer += dt;
        typewriter.update(dt);

        if (prologStep == -1) {
            if (sceneTimer > 2.0f && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                prologStep = 0;
                sceneTimer = 0;
            }
            return;
        }

        // Step 0: foto Cctv.png + typewriter teks di bawah
        if (prologStep == 0 && sceneTimer > 1.0f && typewriter.fullText.empty()) {
            typewriter.setText("Rekaman CCTV ditemukan di lokasi kejadian.");
        }

        if (typewriter.done && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (prologStep < 3) {
                prologStep++;
                sceneTimer = 0;
                if (prologStep == 1)
                    typewriter.setText("Rekaman menunjukan tiga orang mengunjungi apartemen pada malam hari kejadian.");
                else if (prologStep == 2)
                    typewriter.setText("Pengunjung teridentifikasi.");
                else if (prologStep == 3)
                    typewriter.setText("Pergi ke meja anda. Periksa data satu-persatu pengunjung yang teridentifikasi");
            } else {
                startFadeToScene(SCENE_CHAPTER2_BOARD);
            }
        }
    }

// ==========================================================================
    // CHAPTER 2 BOARD
    // ==========================================================================
    void updateChapter2Board(float dt) {
        typewriter.update(dt);

        // ===== MODE 1: MAP VIEW (meja kerja, belum zoom) =====
        if (!inChapter2Zoom) {
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                Vector2 mp = GetMousePosition();
                Rectangle deskZoomRect = {(float)screenW/2 - 150, (float)screenH/2 - 100, 300, 200};
                if (CheckCollisionPointRec(mp, deskZoomRect)) {
                    inChapter2Zoom = true;
                }
            }
            return;
        }

        // ===== MODE 2: ZOOM VIEW (board investigasi, mirip zoom ruangan Chapter 1) =====
        auto& notes = suspectNotes[currentSuspectIdx];

        // Koordinat HARUS identik dengan drawChapter2Board agar klik akurat
        // Koordinat checklist overlay (skalakan seperti di draw) — dikalibrasi dari pixel screenshot.
        // Jarak antar kotak TIDAK uniform (lihat penjelasan di drawChapter2Board), jadi pakai
        // array offset Y eksplisit yang sama persis.
        float scaleX = (float)screenW / 1456.0f;
        float scaleY = (float)screenH / 816.0f;
        static const float cbTopY[5] = {177.0f, 207.0f, 238.0f, 270.0f, 303.0f};
        float cbAreaX = 574.0f * scaleX;
        float cbW     = 28.0f * scaleX;
        float cbItemH = 30.0f * scaleY;

        // Koordinat HARUS sama persis dengan drawChapter2Board (margin 20px dari tepi)
        const int   BTN_MARGIN2  = 20;
        // Ukuran patokan = btnCh1Left / 6, sama persis dengan tombol Chapter 1
        const float CH2_BTN_W    = (float)btnCh1Left.width  / 6.0f;
        const float CH2_BTN_H    = (float)btnCh1Left.height / 6.0f;
        const float BTN_BOTTOM2  = (float)(screenH - BTN_MARGIN2);
        Rectangle backBtn    = {(float)BTN_MARGIN2, BTN_BOTTOM2 - CH2_BTN_H, CH2_BTN_W, CH2_BTN_H};
        Rectangle nextBtn    = {(float)(screenW - BTN_MARGIN2) - CH2_BTN_W, BTN_BOTTOM2 - CH2_BTN_H, CH2_BTN_W, CH2_BTN_H};
        // Confirm punya ukuran sendiri (160x52) — harus identik dengan draw, posisi center-bottom
        const float CONF_W = 160.0f, CONF_H = 52.0f;
        Rectangle confirmBtn = {(float)screenW/2 - CONF_W/2, BTN_BOTTOM2 - CONF_H, CONF_W, CONF_H};

        if (waitingForClick) {
            // Saat notifikasi "Semua tersangka sudah dikonfirmasi" tampil, klik mana saja lanjut
            if (typewriter.done && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                waitingForClick = false;
                startFadeToScene(SCENE_CHAPTER3_INTRO);
            }
            return;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();

            // 1. Tombol navigasi diperiksa lebih dulu
            if (currentSuspectIdx < 2 && CheckCollisionPointRec(mp, nextBtn)) {
                currentSuspectIdx++;
            } else if (currentSuspectIdx == 2 && CheckCollisionPointRec(mp, confirmBtn)) {
                typewriter.setText("Semua tersangka sudah dikonfirmasi");
                waitingForClick = true;
            } else if (CheckCollisionPointRec(mp, backBtn)) {
                if (currentSuspectIdx > 0) currentSuspectIdx--;
            } else {
                // 2. Investigation notes checkbox — koordinat sama persis dgn draw overlay
                for (int i = 0; i < (int)notes.size(); i++) {
                    float ny = (i < 5 ? cbTopY[i] : cbTopY[4] + (i - 4) * 30.0f) * scaleY;
                    Rectangle clickArea = {cbAreaX - 5, ny - 5, cbW + 250.0f * scaleX, cbItemH};
                    if (CheckCollisionPointRec(mp, clickArea)) {
                        notes[i].isChecked = !notes[i].isChecked;
                        break; // hanya proses satu, hindari klik ganda
                    }
                }
            }
        }
    }
    // ==========================================================================
    // CHAPTER 3 INTRO
    // ==========================================================================
    void updateChapter3Intro(float dt) {
        sceneTimer += dt;
        if (sceneTimer > 1.0f && typewriter.fullText.empty()) {
            typewriter.setText("Semua data telah dianalisis, pahami apa yang terjadi malam itu");
        }
        typewriter.update(dt);

        if (typewriter.done && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            startFadeToScene(SCENE_CHAPTER3_PUZZLE);
        }
    }
    // ==========================================================================
    // CHAPTER 3 PUZZLE
    // ==========================================================================
    // Tombol Confirm di Chapter 3 — pakai Confirm.png, setengah ukuran tombol Start (110x32)
    Rectangle getChapter3ConfirmRect() {
        // Tombol Start di main menu: 220x65 → setengahnya: 110x32
        // Dinaikkan ke ~tengah layar - 35px (dari bawah menjadi screenH/2 + 35)
        const float cW = 110.0f, cH = 32.0f;
        return {(float)screenW/2 - cW/2, (float)screenH/2 + 35.0f, cW, cH};
    }

    void updateChapter3Puzzle() {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();

            // Tombol Confirm pilih tersangka
            Rectangle confirmR = getChapter3ConfirmRect();
            if (CheckCollisionPointRec(mp, confirmR)) {
                // 0=Marcus → bad, 1=Olivia → bad, 2=Ethan → true
                if (chapter3SuspectPick == 2) {
                    startFadeToScene(SCENE_ENDING_TRUE);
                } else {
                    startFadeToScene(SCENE_ENDING_BAD);
                }
                return;
            }

            // Tombol kecil pilih tersangka (Marcus/Olivia/Ethan) tepat di bawah slot foto tengah-atas
            for (int t = 0; t < 3; t++) {
                Rectangle thumbRect = getSuspectThumbRect(t);
                if (CheckCollisionPointRec(mp, thumbRect)) {
                    chapter3SuspectPick = t;
                    return;
                }
            }

            // Slot evidence (7 slot layout, tidak termasuk slot tersangka)
            std::vector<int> positions = evidenceSlotPositions();
            for (int k = 0; k < (int)positions.size(); k++) {
                int layoutIdx = positions[k];
                Rectangle slot = getPuzzleSlotRect(layoutIdx);
                if (CheckCollisionPointRec(mp, slot)) {
                    if (selectedPuzzleSlot == -1) {
                        selectedPuzzleSlot = k;
                    } else {
                        // Swap antar index puzzleOrder (0..6) — bebas, tidak ada kondisi menang
                        std::swap(puzzleOrder[selectedPuzzleSlot], puzzleOrder[k]);
                        selectedPuzzleSlot = -1;
                    }
                    break;
                }
            }
        }
    }

    // Posisi 3 tombol kecil foto tersangka, di bawah slot foto tengah-atas (SUSPECT_SLOT_INDEX)
    Rectangle getSuspectThumbRect(int t) {
        Rectangle suspectSlot = getPuzzleSlotRect(SUSPECT_SLOT_INDEX);
        const float thumbW = 30, thumbH = 30, gap = 6;
        float totalW = thumbW * 3 + gap * 2;
        float startX = suspectSlot.x + suspectSlot.width/2.0f - totalW/2.0f;
        float ty = suspectSlot.y + suspectSlot.height + 55; // digeser lebih bawah, beri jarak dari foto
        float tx = startX + t * (thumbW + gap);
        return {tx, ty, thumbW, thumbH};
    }

    // ==========================================================================
    // ACCUSATION
    // ==========================================================================
    void updateAccusation() {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();

            // Suspect cards — koordinat HARUS identik dengan drawAccusation()
            for (int i = 0; i < 3; i++) {
                float cardX = screenW/2 - 330.0f + i * 230.0f;
                Rectangle card = {cardX, 120, 200, 300};
                if (CheckCollisionPointRec(mp, card)) {
                    selectedSuspect = i;
                }
            }

            // Confirm button
            if (selectedSuspect >= 0) {
                Rectangle confirmBtn = {(float)screenW/2 - btnConfirmSuspect.width/2.0f, (float)screenH - 90, (float)btnConfirmSuspect.width, (float)btnConfirmSuspect.height};
                if (CheckCollisionPointRec(mp, confirmBtn)) {
                    if (suspects[selectedSuspect].isGuilty) {
                        startFadeToScene(SCENE_ENDING_TRUE);
                    } else {
                        startFadeToScene(SCENE_ENDING_BAD);
                    }
                }
            }
        }
    }

    // ==========================================================================
    // RESET GAME STATE — dipanggil setiap kembali ke Main Menu agar main ulang
    // selalu mulai bersih, apapun chapter terakhir yang sedang dimainkan.
    // ==========================================================================
    void resetGameState() {
        // Player & prolog
        playerName = "Detective";
        memset(nameInputBuf, 0, sizeof(nameInputBuf));
        nameLen = 0;
        nameConfirmed = false;
        prologStep = 0;
        sceneTimer = 0;
        waitingForClick = false;
        typewriter.setText("");

        // Chapter 1
        currentRoom = ROOM_LIVING;
        selectedCloseUp = -1;
        inCloseUp = false;
        closeUpSubStep = 0;
        easterEggActive = false;
        evidencePopup.active = false;
        totalEvidenceFound = 0;
        for (auto* ev : allEvidence) ev->found = false;
        for (auto& slot : inventorySlots) slot = nullptr;

        // Chapter 2
        currentSuspectIdx = 0;
        inChapter2Zoom = false;
        for (auto& noteList : suspectNotes)
            for (auto& note : noteList)
                note.isChecked = false;

        // Chapter 3
        selectedPuzzleSlot = -1;
        puzzleSolved = false;
        chapter3SuspectPick = 0;
        puzzleOrder = {3, 6, 1, 4, 0, 2, 5}; // sama persis dengan initChapter3Puzzle()

        // Accusation
        selectedSuspect = -1;
        flagCount = 0;

        // Navigation history
        while (!navStack.empty()) navStack.pop();
    }

    // ==========================================================================
    // ENDING
    // ==========================================================================
    void updateEnding() {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            // Harus identik dengan Back.png rect di drawEndingTrue/Bad
            const float bW = 220.0f, bH = 65.0f;
            Rectangle btnRect = {(float)screenW/2 - bW/2, (float)screenH - bH - 20.0f, bW, bH};
            if (CheckCollisionPointRec(GetMousePosition(), btnRect)) {
                resetGameState();
                startFadeToScene(SCENE_MAIN_MENU);
            }
        }
    }

    // ==========================================================================
    // PAUSE
    // ==========================================================================
    void updatePause() {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();
            // Koordinat HARUS identik dengan drawPause() — pakai ukuran texture
            int boxY  = screenH / 2 - uiPauseBox.height / 2;
            int btn1Y = boxY + 100;
            int btn2Y = btn1Y + 65;
            Rectangle contBtn = {
                (float)(screenW/2 - btnContinue.width/2), (float)btn1Y,
                (float)btnContinue.width, (float)btnContinue.height
            };
            Rectangle menuBtn = {
                (float)(screenW/2 - btnMainMenu.width/2), (float)btn2Y,
                (float)btnMainMenu.width, (float)btnMainMenu.height
            };
            if (CheckCollisionPointRec(mp, contBtn)) paused = false;
            if (CheckCollisionPointRec(mp, menuBtn)) {
                paused = false;
                resetGameState();
                startFadeToScene(SCENE_MAIN_MENU);
            }
        }
    }

    // ==========================================================================
    // DRAW
    // ==========================================================================
    void draw() {
        BeginDrawing();
        ClearBackground({248, 247, 245, 255});

        switch (currentScene) {
            case SCENE_MAIN_MENU:        { drawMainMenu(); } break;
            case SCENE_CREDITS:          { drawCredits(); } break;
            case SCENE_PROLOG:           { drawProlog(); } break;
            case SCENE_NAME_INPUT:       { drawNameInput(); } break;
            case SCENE_CHAPTER1_INTRO:   { drawChapter1Intro(); } break;
            case SCENE_CHAPTER1_GAMEPLAY:{ drawChapter1(); } break;
            case SCENE_CHAPTER2_INTRO:   { drawChapter2Intro(); } break;
            case SCENE_CHAPTER2_BOARD:   { drawChapter2Board(); } break;
            case SCENE_CHAPTER3_INTRO:   { drawChapter3Intro(); } break;
            case SCENE_CHAPTER3_PUZZLE:  { drawChapter3Puzzle(); } break;
            case SCENE_ENDING_TRUE:      { drawEndingTrue(); } break;
            case SCENE_ENDING_BAD:       { drawEndingBad(); } break;
            default: break;
        }

        if (paused) drawPause();
        if (!paused && currentScene != SCENE_MAIN_MENU &&
            currentScene != SCENE_CREDITS &&
            currentScene != SCENE_ENDING_TRUE &&
            currentScene != SCENE_ENDING_BAD) {
            drawPauseButton();
        }

        drawFadeOverlay();
        EndDrawing();
    }

    void drawBackground() {
        // Light gray gradient: putih di atas, abu-abu sedang di bawah
        for (int y = 0; y < screenH; y++) {
            float t = (float)y / screenH;
            unsigned char v = (unsigned char)(255 - t * 45); // 255 → 210
            DrawLine(0, y, screenW, y, {v, v, v, 255});
        }
    }

    void drawDecorativeBorder() {
        DrawRectangleLinesEx({10, 10, (float)screenW-20, (float)screenH-20}, 2, {180, 140, 80, 120});
        DrawRectangleLinesEx({15, 15, (float)screenW-30, (float)screenH-30}, 1, {160, 120, 60, 80});
    }

    // ==========================================================================
    // DRAW MAIN MENU
    // ==========================================================================
    void drawMainMenu() {
        drawBackground();
        drawDecorativeBorder();

        // Posisi relatif terhadap screenH — proporsional di 1280×720
        const int titleY    = screenH * 38 / 100;
        const int subtitleY = titleY + 78;
        const int btn1Y     = screenH * 58 / 100;
        const int btn2Y     = btn1Y + 90;
        const int btn3Y     = btn2Y + 90;

        // Judul — diperbesar selebar mungkin, tetap tidak menyentuh tombol Start
        int titleFs = 64;
        DrawText("CASE 402",
                 screenW/2 - MeasureText("CASE 402", titleFs)/2, titleY, titleFs, {180, 140, 60, 255});
        DrawText("A Murder Mystery",
                 screenW/2 - MeasureText("A Murder Mystery", 20)/2, subtitleY, 20, {120, 90, 50, 255});

        // Posisi tombol
        Vector2 mp       = GetMousePosition();
        float targetW    = 220.0f;
        float targetH    = 65.0f;
        float btnX       = (float)screenW / 2.0f - targetW / 2.0f;
        Vector2 originZero = { 0.0f, 0.0f };

        Rectangle startR  = { btnX, (float)btn1Y, targetW, targetH };
        Rectangle creditR = { btnX, (float)btn2Y, targetW, targetH };
        Rectangle exitR   = { btnX, (float)btn3Y, targetW, targetH };

        bool hovStart  = CheckCollisionPointRec(mp, startR);
        bool hovCredit = CheckCollisionPointRec(mp, creditR);
        bool hovExit   = CheckCollisionPointRec(mp, exitR);

        // --- 1. TOMBOL START ---
        Texture2D currentStartTex = hovStart ? btnStartHover : btnStartNormal;
        Rectangle srcStart = { 0.0f, 0.0f, (float)currentStartTex.width, (float)currentStartTex.height };
        DrawTexturePro(currentStartTex, srcStart, startR, originZero, 0.0f, WHITE);

        // --- 2. TOMBOL CREDITS ---
        Texture2D currentCreditTex = hovCredit ? btnCreditsHover : btnCreditsNormal;
        Rectangle srcCredit = { 0.0f, 0.0f, (float)currentCreditTex.width, (float)currentCreditTex.height };
        DrawTexturePro(currentCreditTex, srcCredit, creditR, originZero, 0.0f, WHITE);

        // --- 3. TOMBOL EXIT ---
        Texture2D currentExitTex = hovExit ? btnExitHover : btnExitNormal;
        Rectangle srcExit = { 0.0f, 0.0f, (float)currentExitTex.width, (float)currentExitTex.height };
        DrawTexturePro(currentExitTex, srcExit, exitR, originZero, 0.0f, WHITE);
    }

    // ==========================================================================
    // DRAW CREDITS
    // ==========================================================================
    void drawCredits() {
        drawBackground();
        drawDecorativeBorder();

        DrawText("CREDITS", screenW/2 - MeasureText("CREDITS", 48)/2, 60, 48, {180, 140, 60, 255});
        DrawLine(100, 120, screenW - 100, 120, {180, 140, 80, 180});

        const char* lines[] = {
            "CASE 402 - A Murder Mystery",
            "",
            "Developed by:",
            "  Programming  :  Intan Yusriyaturizki",
            "  Art & Design :  Adinda Nurul A.",
            "  Story        :  Adinda Nurul Atthiyah & Intan Yusriyaturizki",
            "  Pseudocode   :  Farika Aulya Putri",
            "  Flowchart    :  Nur Izzati Zafira",
            "  Implementasi :  Byan Azriel"
            "",
            "Built with raylib (www.raylib.com)",
            "",
            "Final Project exam - 2026",
        };
        int lineCount = (int)(sizeof(lines) / sizeof(lines[0]));
        int startY = 150;
        for (int i = 0; i < lineCount; i++) {
            int fs     = (i == 0) ? 22 : 18;
            Color col  = (i == 0) ? Color{180,140,60,255} : Color{80,75,90,255};
            DrawText(lines[i], screenW/2 - MeasureText(lines[i], fs)/2,
                     startY + i * 38, fs, col);
        }

        Rectangle backBtn = {(float)screenW/2 - 110, (float)screenH - 80, 220, 50};
        bool hov = CheckCollisionPointRec(GetMousePosition(), backBtn);
        DrawRectangleRec(backBtn, hov ? Color{200,192,215,255} : Color{225,220,235,255});
        DrawRectangleLinesEx(backBtn, 2, {180, 140, 80, 255});
        DrawText("BACK TO MENU",
                 screenW/2 - MeasureText("BACK TO MENU", 20)/2,
                 screenH - 67, 20, {51, 51, 51, 255});
        if (hov && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            startFadeToScene(SCENE_MAIN_MENU);
    }

    // ==========================================================================
    // DRAW PROLOG
    // ==========================================================================
    void drawProlog() {
        // All-white background
        ClearBackground({255, 255, 255, 255});

        const Color textColor = {51, 51, 51, 255}; // #333333

        if (prologStep == 0) {
            // Screen 1: "20 April 2024" — anchor ke tengah berdasarkan lebar teks PENUH
            // agar posisi X tidak bergeser saat typewriter menambah huruf
            int fs = 36;
            const char* dateStr = "20 April 2024";
            std::string shown = typewriter.displayed;
            int twFull = MeasureText(dateStr, fs);     // lebar teks penuh sebagai anchor
            int tx = screenW/2 - twFull/2;             // posisi X tetap, dari tengah
            int ty = screenH/2 - fs/2;
            DrawText(shown.c_str(), tx, ty, fs, textColor);
        } else {
            // Screen 2 & 3: paragraf narasi, typewriter centered, text wrap di tengah
            int fs = 22;
            int textAreaW = 900;
            int startX = screenW/2 - textAreaW/2;
            typewriter.draw(startX, screenH/2 - 60, fs, textColor);
        }

        // Klik hint di paling bawah — selalu tampil saat typewriter selesai
        if (typewriter.done) {
            if ((int)(GetTime() * 2) % 2 == 0) {
                const char* hint = "[ Klik untuk melanjutkan ]";
                DrawText(hint,
                         screenW/2 - MeasureText(hint, 16)/2,
                         screenH - 50, 16, textColor);
            }
        }
    }

    // ==========================================================================
    // DRAW NAME INPUT
    // ==========================================================================
    void drawNameInput() {
        drawBackground();
        DrawText("Masukan nama Anda:", screenW/2 - MeasureText("Masukan nama Anda:", 28)/2,
                 screenH/2 - 120, 28, {180, 140, 80, 255});

        Rectangle inputBox = {(float)screenW/2 - 200, (float)screenH/2 - 60, 400, 50};
        DrawRectangleRec(inputBox, {235, 232, 240, 255});
        DrawRectangleLinesEx(inputBox, 2, {180, 140, 80, 255});

        std::string displayName = std::string(nameInputBuf);
        if ((int)(GetTime() * 2) % 2 == 0) displayName += "|";
        DrawText(displayName.c_str(),
                 (int)(inputBox.x + inputBox.width/2 - MeasureText(displayName.c_str(), 22)/2),
                 (int)(inputBox.y + 15), 22, {51, 51, 51, 255});

        DrawText("Tekan ENTER untuk konfirmasi",
                 screenW/2 - MeasureText("Tekan ENTER untuk konfirmasi", 18)/2,
                 screenH/2 + 20, 18, {120, 100, 70, 255});

        drawDecorativeBorder();
    }

    // ==========================================================================
    // DRAW CHAPTER 1 INTRO
    // ==========================================================================
    void drawChapter1Intro() {
        ClearBackground({255, 255, 255, 255});
        const Color textColor = {51, 51, 51, 255};

        if (prologStep == 0) {
            typewriter.draw(screenW/2 - 450, screenH/2 - 20, 22, textColor);
            if (typewriter.done && (int)(GetTime()*2)%2==0) {
                const char* hint = "[ Klik untuk melanjutkan ]";
                DrawText(hint, screenW/2 - MeasureText(hint, 16)/2, screenH - 50, 16, textColor);
            }
            return;
        }

        // Step 1: judul chapter
        float t = std::min(1.0f, sceneTimer / 2.0f);
        Color titleColor = {(unsigned char)(51*t), (unsigned char)(51*t), (unsigned char)(51*t), 255};
        DrawText("CHAPTER 1", screenW/2 - MeasureText("CHAPTER 1", 48)/2, screenH/2 - 50, 48, titleColor);
        DrawText("Find The Evidence", screenW/2 - MeasureText("Find The Evidence", 28)/2, screenH/2 + 20, 28, titleColor);
    }

    // ==========================================================================
    // DRAW CHAPTER 1 GAMEPLAY
    // ==========================================================================
    void drawChapter1() {
        drawBackground();

        // Room scene
        if (!inCloseUp) {
            drawRoomScene();
        } else {
            drawCloseUp();
        }

        // Ikon bukti yang sudah ditemukan — mengambang transparan di pojok kiri,
        // digambar di atas background foto supaya tidak ikut tertutup
        drawInventorySidebar();

        // Evidence popup
        if (evidencePopup.active) {
            evidencePopup.draw(screenW, screenH);
        }

        // Navigation arrows - HANYA muncul di MAP VIEW. Sekarang ruangan melingkar
        // (Ruang Tamu -> Dapur -> Kamar Tidur -> kembali), jadi kedua tombol SELALU aktif.
        if (!inCloseUp) {
            Vector2 mp2 = GetMousePosition();
            // Tombol kiri/kanan: diperkecil 6x, dijauhkan ke pinggir layar
            float btnScaleW = (float)btnCh1Left.width  / 6.0f;
            float btnScaleH = (float)btnCh1Left.height / 6.0f;
            float arrowY    = screenH / 2.0f - btnScaleH / 2.0f;
            // Kiri: di pinggir kiri layar
            Rectangle leftArrowRect2  = {131.0f, arrowY, btnScaleW, btnScaleH};
            // Kanan: 10px ke dalam dari pinggir kanan layar
            Rectangle rightArrowRect2 = {(float)screenW - btnScaleW - 20.0f, arrowY, btnScaleW, btnScaleH};
            bool leftHov  = CheckCollisionPointRec(mp2, leftArrowRect2);
            bool rightHov = CheckCollisionPointRec(mp2, rightArrowRect2);

            // Draw kiri (scaled)
            DrawTexturePro(btnCh1Left,
                {0, 0, (float)btnCh1Left.width, (float)btnCh1Left.height},
                leftArrowRect2, {0, 0}, 0,
                leftHov ? Color{255,255,255,220} : WHITE);
            // Draw kanan (scaled)
            DrawTexturePro(btnCh1Right,
                {0, 0, (float)btnCh1Right.width, (float)btnCh1Right.height},
                rightArrowRect2, {0, 0}, 0,
                rightHov ? Color{255,255,255,220} : WHITE);

            DrawText("[ A/D atau klik panah — ruangan berputar melingkar ]",
                     (int)(screenW/2 - MeasureText("[ A/D atau klik panah — ruangan berputar melingkar ]",13)/2),
                     screenH - 26, 13, {130,110,70,200});
        }

        // Typewriter if needed — diposisikan di tengah area map (kanan sidebar), bukan rata kiri
        if (waitingForClick) {
            DrawRectangle(SIDEBAR_W, screenH - 100, screenW - SIDEBAR_W, 100, {240,238,245,225});
            float mapCenterX = SIDEBAR_W + (screenW - SIDEBAR_W) / 2.0f;
            typewriter.draw((int)(mapCenterX - 450), screenH - 90, 22, {51, 51, 51, 255});
        }

        // Easter egg typewriter - hanya muncul saat sedang aktif (di scene zoom cermin)
        if (easterEggActive && inCloseUp && selectedCloseUp == -2) {
            DrawRectangle(SIDEBAR_W, screenH - 60, screenW - SIDEBAR_W, 60, {240,238,245,215});
            DrawText("\"i think therefore i am\"",
                     SIDEBAR_W + (screenW - SIDEBAR_W)/2 - MeasureText("\"i think therefore i am\"", 22)/2,
                     screenH - 45, 22, {100, 80, 140, 255});
        }

        drawDecorativeBorder();
    }

    void drawRoomScene() {
        // Background ruangan full 16:9 (sidebar inventory sudah dihapus)
        Rectangle roomBg = {0, 0, (float)screenW, (float)screenH};

        if (currentRoom == ROOM_LIVING) {
            DrawTexturePro(bgRoomLiving,
                {0, 0, (float)bgRoomLiving.width, (float)bgRoomLiving.height},
                roomBg, {0, 0}, 0, WHITE);

            // Brankas — tengah rak bookcase kiri
            drawHotspot(ROOM_OX + 420, 310, 110, 115, !allEvidence[0]->found);
            // Gelas Kopi / Sidik Jari — di atas meja bundar kecil
            drawHotspot(ROOM_OX + 710, 432, 115, 100, !allEvidence[1]->found);
            // Ponsel — area lantai pojok kanan bawah
            drawHotspot(ROOM_OX + 895, 600, 110, 100, !allEvidence[2]->found);
            // Cermin bulat — dinding kanan atas
            drawHotspot(ROOM_OX + 715, 192, 135, 140, true);
        } else { // ROOM_KITCHEN
            DrawTexturePro(bgRoomKitchen,
                {0, 0, (float)bgRoomKitchen.width, (float)bgRoomKitchen.height},
                roomBg, {0, 0}, 0, WHITE);

            // Pecahan Botol — meja dapur kiri
            drawHotspot(ROOM_OX + 395, 383, 105, 105, !allEvidence[5]->found);
            // Laptop + Kartu Akses — satu hotspot gabungan, counter kanan
            bool showLaptopCard = !allEvidence[3]->found || !allEvidence[4]->found;
            drawHotspot(ROOM_OX + 1005, 360, 150, 130, showLaptopCard);
        }
    }

    // Hotspot transparan: area klik TIDAK digambar kotak/border sama sekali, gambar
    // objeknya sendiri sudah ada di dalam bgRoomLiving/bgRoomKitchen. Hover hanya
    // memberi outline tipis berkedip (glow), tidak pernah menghilangkan apapun.
    void drawHotspot(int x, int y, int w, int h, bool hasEvidence) {
        if (!hasEvidence) return; // bukti sudah diambil → tidak ada indikator lagi

        Vector2 mp = GetMousePosition();
        Rectangle rect = {(float)x, (float)y, (float)w, (float)h};
        bool hov = CheckCollisionPointRec(mp, rect);

        // Outline tipis HANYA muncul saat hover — tidak ada kotak solid sama sekali
        if (hov) {
            DrawRectangleLinesEx(rect, 2, Color{51, 51, 51, 200});
        }

        // Indicator dot dihapus — hover outline sudah cukup sebagai penanda
    }

    void drawCloseUp() {
        // ZOOM scene background - sekarang full 16:9 (full layar), sidebar sudah dihapus
        Rectangle zoomArea = {0, 0, (float)screenW, (float)screenH};
        float zoomCenterX = screenW / 2.0f;

        // Posisi UI lama (panah bawah, teks hint) TETAP tidak berubah — dihitung
        // dari SIDEBAR_W seperti sebelumnya, supaya layout teks/tombol tidak geser
        float oldMapX = SIDEBAR_W + 20.0f;
        float oldMapW = screenW - SIDEBAR_W - 40.0f;
        float oldMapCenterX = oldMapX + oldMapW / 2.0f;

        Vector2 mp = GetMousePosition();

        if (selectedCloseUp == -2) {
            // ===== ZOOM CERMIN =====
            drawZoomBackground(bgZoomMirror, zoomArea);

            Rectangle mirrorObjRect = {zoomCenterX - 110, 140, 180, 260};
            drawClickGlow(mirrorObjRect, mp);

            if (!easterEggActive) {
                DrawText("[ Klik cermin untuk melihat ]",
                         (int)(oldMapCenterX - MeasureText("[ Klik cermin untuk melihat ]", 16)/2),
                         screenH - 200, 16, {100, 80, 50, 200});
            }
        } else if (selectedCloseUp >= 0 && selectedCloseUp < (int)allEvidence.size()) {
            // ===== ZOOM EVIDENCE =====
            drawZoomBackground(getBgZoomTexture(selectedCloseUp), zoomArea);

            // Per-evidence click rect (posisi disesuaikan dengan posisi objek di bg zoom)
            Rectangle itemBtnRect;
            if      (selectedCloseUp == 0) itemBtnRect = {zoomCenterX - 120, 220, 180, 140}; // Safe
            else if (selectedCloseUp == 1) itemBtnRect = {zoomCenterX -  95, 200, 180, 140}; // Fingerprint
            else if (selectedCloseUp == 5) itemBtnRect = {zoomCenterX - 110, 230, 180, 140}; // Bottle
            else                           itemBtnRect = {zoomCenterX -  90, 220, 180, 140};
            drawClickGlow(itemBtnRect, mp);

            DrawText("[ Klik barang untuk mengambilnya ]",
                     (int)(oldMapCenterX - MeasureText("[ Klik barang untuk mengambilnya ]", 16)/2),
                     screenH - 200, 16, {100, 80, 50, 200});
        } else if (selectedCloseUp == -3) {
            // ===== COMBINED ZOOM: Laptop + Kartu Akses =====
            drawZoomBackground(bgZoomTable, zoomArea);

            bool laptopFound = allEvidence[3]->found;
            bool cardFound   = allEvidence[4]->found;

            // --- Laptop (kiri) --- area klik transparan, hilang begitu sudah diambil
            Rectangle laptopRect = {zoomCenterX - 220.0f, 180.0f, 160.0f, 160.0f};
            if (!laptopFound) drawClickGlow(laptopRect, mp);

            // --- Kartu Akses (kanan) ---
            Rectangle cardRect = {zoomCenterX + 50.0f, 210.0f, 160.0f, 160.0f};
            if (!cardFound) drawClickGlow(cardRect, mp);

            // Hint teks
            if (!laptopFound || !cardFound) {
                DrawText("[ Klik barang untuk mengambilnya ]",
                         (int)(oldMapCenterX - MeasureText("[ Klik barang untuk mengambilnya ]", 16)/2),
                         screenH - 200, 16, {100, 80, 50, 200});
            }
        }

        // Tombol panah BAWAH — diperkecil 6x, dinaikkan agar terlihat di layar
        float downScaleW = (float)btnCh1Down.width  / 6.0f;
        float downScaleH = (float)btnCh1Down.height / 6.0f;
        Rectangle backDownRect = {
            (screenW / 2.0f - downScaleW / 2.0f) + 50.0f,
            (float)screenH - downScaleH - 25.0f,   // 20px dari bawah layar
            downScaleW, downScaleH
        };
        bool backHov = CheckCollisionPointRec(mp, backDownRect);
        DrawTexturePro(btnCh1Down,
            {0, 0, (float)btnCh1Down.width, (float)btnCh1Down.height},
            backDownRect, {0, 0}, 0,
            backHov ? Color{255,255,255,220} : WHITE);
    }

    // Menggambar background zoom full-area dari texture asset sendiri
    void drawZoomBackground(Texture2D tex, Rectangle area) {
        DrawTexturePro(tex,
            {0, 0, (float)tex.width, (float)tex.height},
            area, {0, 0}, 0, WHITE);
    }

    // Area klik TRANSPARAN — tidak digambar kotak/border sama sekali kecuali saat
    // di-hover, dan hover-nya berupa outline tipis yang muncul (bukan blok solid
    // yang menutupi gambar), supaya gambar item tidak pernah "hilang" saat dihover.
    void drawClickGlow(Rectangle rect, Vector2 mp) {
        bool hov = CheckCollisionPointRec(mp, rect);
        if (hov) {
            DrawRectangleLinesEx(rect, 3, Color{51, 51, 51, 220});
        }
    }

    // Mapping evidence index -> texture background zoom yang sesuai
    Texture2D getBgZoomTexture(int evidenceIdx) {
        switch (evidenceIdx) {
            case 0: return bgZoomSafe;        // Brankas
            case 1: return bgZoomFingerprint; // Meja Kopi / Gelas Kopi
            case 2: return bgZoomPhone;       // Lantai/HP
            case 5: return bgZoomBottle;      // Wastafel / Pecahan Botol
            default: return bgZoomSafe;
        }
    }

    // Mapping evidence index -> texture gambar item untuk popup "ambil bukti"
    Texture2D getPopupTexture(int evidenceIdx) {
        switch (evidenceIdx) {
            case 0: return popupSafe;
            case 1: return popupFingerprint;
            case 2: return popupPhone;
            case 3: return popupLaptop;
            case 4: return popupAccessLog;
            case 5: return popupBottle;
            default: return popupSafe;
        }
    }

    // Mapping evidence index -> ikon kecil slot inventory
    Texture2D getIvtTexture(int evidenceIdx) {
        switch (evidenceIdx) {
            case 0: return ivtSafe;
            case 1: return ivtFingerprint;
            case 2: return ivtPhone;
            case 3: return ivtLaptop;
            case 4: return ivtAccessLog;
            case 5: return ivtBottle;
            default: return ivtSafe;
        }
    }

    // Cari texture inventory untuk pointer Evidence* tertentu (dipakai sidebar,
    // karena urutan inventorySlots adalah urutan ditemukan, bukan urutan index)
    Texture2D getIvtTextureForEvidence(Case402::Evidence* ev) {
        for (size_t i = 0; i < allEvidence.size(); i++) {
            if (allEvidence[i] == ev) return getIvtTexture((int)i);
        }
        return ivtSafe;
    }

    // Daftar bukti yang sudah ditemukan — TRANSPARAN, tanpa panel/kotak slot
    // sama sekali. Hanya ikon item yang mengambang di pojok kiri layar.
    void drawInventorySidebar() {
        int iconSize = 100;
        int padding  = 11;
        int startY   = 31;
        int startX   = 27;

        int shown = 0;
        for (int i = 0; i < 6; i++) {
            if (!inventorySlots[i]) continue;

            int sy = startY + shown * (iconSize + padding);
            Texture2D icon = getIvtTextureForEvidence(inventorySlots[i]);

            float scale = std::min((float)iconSize / icon.width, (float)iconSize / icon.height);
            float drawW = icon.width * scale;
            float drawH = icon.height * scale;
            Rectangle dest = {(float)startX + (iconSize - drawW) / 2.0f,
                               (float)sy + (iconSize - drawH) / 2.0f,
                               drawW, drawH};
            DrawTexturePro(icon, {0, 0, (float)icon.width, (float)icon.height}, dest, {0, 0}, 0, WHITE);

            shown++;
        }
    }

    // ==========================================================================
    // DRAW CHAPTER 2 INTRO
    // ==========================================================================
    void drawChapter2Intro() {
        ClearBackground({255, 255, 255, 255});
        const Color textColor = {51, 51, 51, 255};
        const char* clickHint = "[ Klik untuk melanjutkan ]";

        // Step -1: judul chapter fade-in
        if (prologStep == -1) {
            float t = std::min(1.0f, sceneTimer / 2.0f);
            Color titleColor = {(unsigned char)(51*t), (unsigned char)(51*t), (unsigned char)(51*t), 255};
            DrawText("CHAPTER 2", screenW/2 - MeasureText("CHAPTER 2", 48)/2, screenH/2 - 50, 48, titleColor);
            DrawText("The Suspect", screenW/2 - MeasureText("The Suspect", 28)/2, screenH/2 + 20, 28, titleColor);
            if (sceneTimer > 2.0f && (int)(GetTime()*2)%2==0)
                DrawText(clickHint, screenW/2 - MeasureText(clickHint, 16)/2, screenH - 50, 16, textColor);
            return;
        }

        if (prologStep == 0) {
            // Screen 1: foto Cctv.png + teks — keduanya diposisikan agar pas di tengah layar
            // Alokasikan: foto 50% tinggi layar, teks ~1 baris (22px), sisanya padding atas/bawah
            const int fs = 22;
            const float textH  = (float)fs + 10.0f;   // tinggi area teks
            const float gap    = 18.0f;                // jarak foto ke teks
            const float totalH = (float)screenH * 0.50f + gap + textH;
            const float startY = (screenH - totalH) / 2.0f;  // padding atas agar blok di tengah

            float imgAreaH = (float)screenH * 0.50f;
            Rectangle imgDest = {(float)screenW * 0.15f, startY, (float)screenW * 0.70f, imgAreaH};
            if (evCctv.id > 0) {
                drawTextureFit(evCctv, imgDest);
            }
            // Typewriter teks tepat di bawah foto
            if (sceneTimer > 1.0f) {
                int startX = screenW/2 - 450;
                int textY  = (int)(startY + imgAreaH + gap);
                typewriter.draw(startX, textY, fs, textColor);
                if (typewriter.done && (int)(GetTime()*2)%2==0)
                    DrawText(clickHint, screenW/2 - MeasureText(clickHint, 16)/2, screenH - 40, 16, textColor);
            }

        } else if (prologStep == 1) {
            // Screen 2: typewriter teks di tengah layar
            int fs = 22;
            int textAreaW = 900;
            typewriter.draw(screenW/2 - textAreaW/2, screenH/2 - 30, fs, textColor);
            if (typewriter.done && (int)(GetTime()*2)%2==0)
                DrawText(clickHint, screenW/2 - MeasureText(clickHint, 16)/2, screenH - 40, 16, textColor);

        } else if (prologStep == 2) {
            // Screen 3: 3 foto CCTV berdampingan + teks — blok keseluruhan di tengah layar
            const float photoW = 260.0f, photoH = 320.0f;
            const float hGap   = 40.0f;   // jarak antar foto
            const float vGap   = 24.0f;   // jarak foto ke teks
            const int   fs     = 22;
            const float textH  = (float)fs;
            const float totalBlockH = photoH + vGap + textH;
            const float photoY = (screenH - totalBlockH) / 2.0f;  // sehingga blok pas di tengah

            const float totalW = photoW * 3 + hGap * 2;
            const float startX = screenW / 2.0f - totalW / 2.0f;

            Texture2D cctvTex[3] = { evCctvOlivia, evCctvMarcus, evCctvEthan };
            for (int i = 0; i < 3; i++) {
                float sx = startX + i * (photoW + hGap);
                Rectangle dest = {sx, photoY, photoW, photoH};
                if (cctvTex[i].id > 0) {
                    drawTextureFit(cctvTex[i], dest);
                } else {
                    DrawRectangleRec(dest, {210, 205, 220, 255});
                }
            }
            // Teks pas di bawah foto, rata tengah
            float textY = photoY + photoH + vGap;
            typewriter.draw(screenW/2 - 450, (int)textY, fs, textColor);
            if (typewriter.done && (int)(GetTime()*2)%2==0)
                DrawText(clickHint, screenW/2 - MeasureText(clickHint, 16)/2, screenH - 40, 16, textColor);

        } else {
            // prologStep == 3: instruksi, typewriter di tengah
            typewriter.draw(screenW/2 - 450, screenH/2 - 20, 22, textColor);
            if (typewriter.done && (int)(GetTime()*2)%2==0)
                DrawText(clickHint, screenW/2 - MeasureText(clickHint, 16)/2, screenH - 40, 16, textColor);
        }
    }

    // ==========================================================================
    // DRAW CHAPTER 2 BOARD
    // ==========================================================================
    void drawChapter2Board() {
        drawBackground();

        // ===== MODE 1: MAP VIEW (meja kerja, belum zoom) =====
        if (!inChapter2Zoom) {
            drawChapter2Desk();
            drawDecorativeBorder();
            return;
        }

        // ===== MODE 2: ZOOM VIEW — full-screen suspect board image =====
        // Pilih texture board sesuai suspect yang sedang dilihat
        Texture2D* boardTex = nullptr;
        if (currentSuspectIdx == 0)      boardTex = &boardOlivia;
        else if (currentSuspectIdx == 1) boardTex = &boardMarcus;
        else                             boardTex = &boardEthan;

        // Gambar board full 16:9 (full layar)
        Rectangle boardDest = {0, 0, (float)screenW, (float)screenH};
        if (boardTex && boardTex->id > 0) {
            DrawTexturePro(*boardTex,
                {0, 0, (float)boardTex->width, (float)boardTex->height},
                boardDest, {0, 0}, 0, WHITE);
        }

        // ---- OVERLAY: Hanya checklist Investigation Notes di atas gambar ----
        // Posisi checklist disesuaikan di atas area "INVESTIGATION NOTES" gambar
        // (kira-kira tengah-kanan layar, persis di atas kotak checklist di gambar board)
        auto& notes = suspectNotes[currentSuspectIdx];

        // Koordinat checklist overlay — di atas area kotak di gambar (sesuai ethanBoard.png)
        // Dikalibrasi ulang berdasarkan pengukuran pixel langsung dari screenshot render (1280x720).
        // PENTING: jarak antar kotak TIDAK uniform di asset PNG — kotak ke-5 lebih rapat ke
        // kotak ke-4 (karena label "Detective Note:" / "Relation with victim" wrap 2 baris),
        // jadi posisi tiap kotak dipakai sebagai offset Y eksplisit, bukan dikali rata.
        float scaleX = (float)screenW / 1456.0f;
        float scaleY = (float)screenH / 816.0f;

        // Top-Y tiap kotak (koordinat referensi 1456x816), hasil ukur pixel langsung dari board:
        static const float cbTopY[5] = {177.0f, 207.0f, 238.0f, 270.0f, 303.0f};
        float cbAreaX = 574.0f * scaleX;
        float cbW     = 28.0f * scaleX;
        float cbH     = 21.0f * scaleY;
        // cbItemH dipakai hanya untuk tinggi area klik antar baris (boleh berbeda dari gap kotak)
        float cbItemH = 30.0f * scaleY;

        for (int i = 0; i < (int)notes.size(); i++) {
            float ny = (i < 5 ? cbTopY[i] : cbTopY[4] + (i - 4) * 30.0f) * scaleY;

            // Kotak checkbox — transparan, hanya border + X bila checked.
            // cb merepresentasikan posisi & ukuran kotak PNG yang sebenarnya di board.
            Rectangle cb = {cbAreaX, ny, cbW, cbH};

            // Hover highlight tipis
            Vector2 mp = GetMousePosition();
            // Area klik lebih lebar dari kotak agar mudah diklik
            Rectangle clickArea = {cbAreaX - 5, ny - 5, cbW + 250.0f * scaleX, cbItemH};
            bool hov = CheckCollisionPointRec(mp, clickArea);
            if (hov) DrawRectangleLinesEx(clickArea, 1, {255, 230, 100, 80});

            // X mark bila sudah dicentang — digambar tepat menumpuk di tengah kotak PNG
            if (notes[i].isChecked) {
                float xHalfW = cb.width  * 0.55f;
                float xHalfH = cb.height * 0.55f;
                float xCX    = cb.x + cb.width  / 2.0f; // tengah horizontal kotak
                float xCY    = cb.y + cb.height / 2.0f; // tengah vertikal kotak
                float x1 = xCX - xHalfW/2, y1 = xCY - xHalfH/2;
                float x2 = xCX + xHalfW/2, y2 = xCY + xHalfH/2;
                DrawLineEx({x1, y1}, {x2, y2}, 2.0f, {220, 50, 50, 230});
                DrawLineEx({x2, y1}, {x1, y2}, 2.0f, {220, 50, 50, 230});
            }
        }

        // Navigation buttons — kiri bawah & kanan bawah, ukuran patokan btnCh1 / 6, margin 20px
        const int BTN_MARGIN = 20;
        const float CH2_W = (float)btnCh1Left.width  / 6.0f;
        const float CH2_H = (float)btnCh1Left.height / 6.0f;
        const float CH2_BY = (float)(screenH - BTN_MARGIN) - CH2_H;
        Vector2 mpNav = GetMousePosition();

        // Tombol KIRI (back) — pojok kiri bawah
        if (currentSuspectIdx > 0) {
            Rectangle r = {(float)BTN_MARGIN, CH2_BY, CH2_W, CH2_H};
            bool hov = CheckCollisionPointRec(mpNav, r);
            Texture2D tex = hov ? btnCh2LeftHover : btnCh2Left;
            DrawTexturePro(tex, {0,0,(float)tex.width,(float)tex.height}, r, {0,0}, 0, WHITE);
        }

        // Tombol KANAN (next) atau CONFIRM — pojok kanan bawah
        if (currentSuspectIdx < 2) {
            Rectangle r = {(float)(screenW - BTN_MARGIN) - CH2_W, CH2_BY, CH2_W, CH2_H};
            bool hov = CheckCollisionPointRec(mpNav, r);
            Texture2D tex = hov ? btnCh2RightHover : btnCh2Right;
            DrawTexturePro(tex, {0,0,(float)tex.width,(float)tex.height}, r, {0,0}, 0, WHITE);
        } else {
            // Suspect terakhir: Confirm di tengah bawah — lebih kecil dari Start (220x65), pakai 160x52
            const float CONF_W = 160.0f, CONF_H = 52.0f;
            Rectangle r = {(float)screenW/2 - CONF_W/2,
                            (float)(screenH - BTN_MARGIN) - CONF_H,
                            CONF_W, CONF_H};
            bool hov = CheckCollisionPointRec(mpNav, r);
            DrawTexturePro(btnConfirm, {0,0,(float)btnConfirm.width,(float)btnConfirm.height}, r, {0,0}, 0, hov ? Color{255,255,255,220} : WHITE);
        }

        if (waitingForClick) {
            DrawRectangle(0, screenH - 100, screenW, 50, {0, 0, 0, 180});
            typewriter.draw(screenW/2 - 450, screenH - 90, 20, WHITE);
        }
    }

    // Tampilan "meja kerja" sebelum zoom ke board investigasi (mirip pola Chapter 1)
    void drawChapter2Desk() {
        // Background full-screen meja kerja (chapter2Workspaces.png)
        if (bgChapter2Workspace.id > 0) {
            Rectangle bgDest = {0, 0, (float)screenW, (float)screenH};
            DrawTexturePro(bgChapter2Workspace,
                {0, 0, (float)bgChapter2Workspace.width, (float)bgChapter2Workspace.height},
                bgDest, {0, 0}, 0, WHITE);
        }

        DrawText("MEJA KERJA DETEKTIF", screenW/2 - MeasureText("MEJA KERJA DETEKTIF", 28)/2,
                 60, 28, {51, 51, 51, 255});

        Rectangle deskZoomRect = {(float)screenW/2 - 150, (float)screenH/2 - 100, 300, 200};
        bool hov = CheckCollisionPointRec(GetMousePosition(), deskZoomRect);
        // Box dan teks transparan — hanya pulse indicator yang tetap terlihat
        DrawRectangle(deskZoomRect.x, deskZoomRect.y, deskZoomRect.width, deskZoomRect.height,
                      {0, 0, 0, 0});
        DrawRectangleLinesEx(deskZoomRect, 3, {0, 0, 0, 0});
        DrawText("[ Berkas Data Pengunjung ]",
                 screenW/2 - MeasureText("[ Berkas Data Pengunjung ]", 16)/2,
                 (int)(deskZoomRect.y + deskZoomRect.height/2 - 8), 16, {0, 0, 0, 0});

        // Indicator circle dihapus

        DrawText("[ Klik untuk memeriksa data pengunjung ]",
                 screenW/2 - MeasureText("[ Klik untuk memeriksa data pengunjung ]", 16)/2,
                 screenH - 80, 16, {100, 80, 50, 200});
    }

    void drawWordWrapped(const std::string& text, float x, float y, float maxW, int fontSize, Color c) {
        std::string line, word;
        float ly = y;
        for (size_t i = 0; i <= text.size(); i++) {
            if (i == text.size() || text[i] == ' ' || text[i] == '\n') {
                std::string test = line.empty() ? word : line + " " + word;
                if (MeasureText(test.c_str(), fontSize) > maxW && !line.empty()) {
                    DrawText(line.c_str(), (int)x, (int)ly, fontSize, c);
                    ly += fontSize + 3;
                    line = word;
                } else {
                    line = test;
                }
                word = "";
                if (i < text.size() && text[i] == '\n') {
                    DrawText(line.c_str(), (int)x, (int)ly, fontSize, c);
                    ly += fontSize + 3;
                    line = "";
                }
            } else {
                word += text[i];
            }
        }
        if (!line.empty()) DrawText(line.c_str(), (int)x, (int)ly, fontSize, c);
    }

    // ==========================================================================
    // DRAW CHAPTER 3 INTRO
    // ==========================================================================
    void drawChapter3Intro() {
        ClearBackground({255, 255, 255, 255});
        const Color textColor = {51, 51, 51, 255};

        float t = std::min(1.0f, sceneTimer / 2.0f);
        Color tc = {(unsigned char)(51*t), (unsigned char)(51*t), (unsigned char)(51*t), 255};
        DrawText("CHAPTER 3", screenW/2 - MeasureText("CHAPTER 3", 48)/2, screenH/2 - 80, 48, tc);
        DrawText("Reconstruction", screenW/2 - MeasureText("Reconstruction", 28)/2, screenH/2 - 20, 28, tc);

        // Sub-teks setelah judul muncul
        if (sceneTimer > 1.5f) {
            float subT = std::min(1.0f, (sceneTimer - 1.5f) / 1.0f);
            Color subC = {51, 51, 51, (unsigned char)(255 * subT)};
            const char* sub = "Review kejadian dan ungkap siapa pelaku";
            DrawText(sub, screenW/2 - MeasureText(sub, 20)/2, screenH/2 + 30, 20, subC);
        }

        if (sceneTimer > 2.0f) {
            typewriter.draw((screenW - 900) / 2, screenH - 80, 20, textColor);
            if (typewriter.done && (int)(GetTime()*2)%2==0) {
                const char* hint = "[ Klik untuk melanjutkan ]";
                DrawText(hint, screenW/2 - MeasureText(hint, 16)/2, screenH - 40, 16, textColor);
            }
        }
    }

    // ==========================================================================
    // DRAW CHAPTER 3 PUZZLE
    // ==========================================================================
    // Gambar texture agar pas di dalam rect tujuan tanpa distorsi (aspect-fit, center)
    void drawTextureFit(Texture2D tex, Rectangle dest, Color tint = WHITE) {
        if (tex.id <= 0 || tex.width <= 0 || tex.height <= 0) return;
        float srcAspect = (float)tex.width / (float)tex.height;
        float dstAspect = dest.width / dest.height;
        float w, h;
        if (srcAspect > dstAspect) {
            w = dest.width;
            h = w / srcAspect;
        } else {
            h = dest.height;
            w = h * srcAspect;
        }
        float dx = dest.x + (dest.width - w) / 2.0f;
        float dy = dest.y + (dest.height - h) / 2.0f;
        DrawTexturePro(tex,
            {0, 0, (float)tex.width, (float)tex.height},
            {dx, dy, w, h}, {0, 0}, 0, tint);
    }

    // Perbesar rect dari titik tengahnya dengan faktor skala (dipakai untuk gambar foto evidence
    // 2x lebih besar dari slotnya, sementara rect ASLI tetap dipakai untuk hit-test klik/swap).
    Rectangle scaleRectFromCenter(Rectangle r, float scale) {
        float cx = r.x + r.width / 2.0f;
        float cy = r.y + r.height / 2.0f;
        float w = r.width * scale;
        float h = r.height * scale;
        return {cx - w / 2.0f, cy - h / 2.0f, w, h};
    }

    void drawChapter3Puzzle() {
        ClearBackground({20, 18, 16, 255});

        // Background papan cork asli (full-screen, aset sendiri)
        if (bgChapter3Puzzle.id > 0) {
            DrawTexturePro(bgChapter3Puzzle,
                {0, 0, (float)bgChapter3Puzzle.width, (float)bgChapter3Puzzle.height},
                {0, 0, (float)screenW, (float)screenH}, {0, 0}, 0, WHITE);
        } else {
            DrawRectangle(0, 0, screenW, screenH, {200, 165, 110, 255});
        }

        DrawText("RECONSTRUCTION BOARD", screenW/2 - MeasureText("RECONSTRUCTION BOARD", 22)/2,
                 12, 22, {40, 30, 20, 255});

        std::vector<int> positions = evidenceSlotPositions(); // layout slot index utk 7 evidence
        const float PHOTO_SCALE = 2.0f; // foto evidence digambar 2x lebih besar dari kotak slotnya

        // ---- Slot evidence (7 buah) ----
        for (int k = 0; k < (int)positions.size(); k++) {
            int layoutIdx = positions[k];
            Rectangle slotRect = getPuzzleSlotRect(layoutIdx);

            bool isSelected = (selectedPuzzleSlot == k);
            // Border & highlight dibuat invisible (alpha 0) agar tampilan board rapi —
            // area klik/swap tetap berfungsi seperti biasa, hanya visualnya yang disembunyikan.
            Color border = isSelected ? Color{220, 180, 60, 0} :
                           (puzzleSolved ? Color{80, 200, 80, 0} : Color{40, 35, 30, 0});

            // Foto evidence asli, 2x lebih besar dari slot, tetap center di posisi slot
            int evIdx = puzzleOrder[k];
            drawTextureFit(getEvidenceTexture(evIdx), scaleRectFromCenter(slotRect, PHOTO_SCALE));

            DrawRectangleLinesEx(slotRect, isSelected ? 3 : 2, border);

            // Hover effect — alpha 0 (invisible)
            if (CheckCollisionPointRec(GetMousePosition(), slotRect) && !puzzleSolved) {
                DrawRectangleLinesEx({slotRect.x-2, slotRect.y-2, slotRect.width+4, slotRect.height+4}, 2, {255, 220, 120, 0});
            }
        }

        // ---- Slot foto tersangka (statis, di tengah-atas) + 3 tombol pilih di bawahnya ----
        Rectangle suspectSlot = getPuzzleSlotRect(SUSPECT_SLOT_INDEX);
        drawTextureFit(getSuspectThumbTexture(chapter3SuspectPick), scaleRectFromCenter(suspectSlot, PHOTO_SCALE));
        DrawRectangleLinesEx(suspectSlot, 2, {40, 35, 30, 0});

        for (int t = 0; t < 3; t++) {
            Rectangle thumbRect = getSuspectThumbRect(t);
            bool isPicked = (chapter3SuspectPick == t);
            drawTextureFit(getSuspectThumbTexture(t), thumbRect);
            // Border highlight untuk yang dipilih (tetap visible supaya player tahu mana yang aktif)
            if (isPicked) {
                DrawRectangleLinesEx({thumbRect.x-2, thumbRect.y-2, thumbRect.width+4, thumbRect.height+4}, 3, {220, 180, 60, 220});
            }
            if (CheckCollisionPointRec(GetMousePosition(), thumbRect) && !isPicked) {
                DrawRectangleLinesEx({thumbRect.x-1, thumbRect.y-1, thumbRect.width+2, thumbRect.height+2}, 2, {255, 220, 120, 160});
            }
        }

        // ---- Flags di bawah ketiga tombol pilih suspect ----
        {
            // Hitung flags dari suspectNotes untuk suspect yang sedang dipilih
            int pickedFlags = 0;
            if (chapter3SuspectPick < (int)suspectNotes.size()) {
                for (auto& note : suspectNotes[chapter3SuspectPick]) {
                    if (note.isChecked) pickedFlags++;
                }
            }
            // Posisi: ambil thumbRect paling kanan sebagai referensi bawah
            Rectangle lastThumb = getSuspectThumbRect(2);
            float flagY = lastThumb.y + lastThumb.height + 12;
            const char* flagTxt = TextFormat("Flags: %d/5", pickedFlags);
            const Color flagDark = {51, 51, 51, 255}; // #333333
            int flagFs = 12;
            int flagTxtW = MeasureText(flagTxt, flagFs);
            // Tengahkan di bawah area 3 tombol
            Rectangle firstThumb = getSuspectThumbRect(0);
            float flagAreaCenterX = firstThumb.x + (lastThumb.x + lastThumb.width - firstThumb.x) / 2.0f;
            int flagTxtX = (int)(flagAreaCenterX - flagTxtW/2.0f);

            // Kotak putih di belakang teks (ukuran pas mengikuti teks, sedikit padding) supaya teks
            // tidak tenggelam di atas background board
            const float padX = 6.0f, padY = 4.0f;
            Rectangle flagBg = {(float)flagTxtX - padX, flagY - padY,
                                 (float)flagTxtW + padX * 2.0f, (float)flagFs + padY * 2.0f};
            DrawRectangleRec(flagBg, WHITE);
            DrawRectangleLinesEx(flagBg, 1.5f, flagDark);

            DrawText(flagTxt, flagTxtX, (int)flagY, flagFs, flagDark);
        }

        // ---- Tombol Confirm (Confirm.png, setengah ukuran tombol Start) ----
        {
            Rectangle confirmR = getChapter3ConfirmRect();
            bool hov = CheckCollisionPointRec(GetMousePosition(), confirmR);
            DrawTexturePro(btnConfirm,
                {0, 0, (float)btnConfirm.width, (float)btnConfirm.height},
                confirmR, {0, 0}, 0,
                hov ? Color{255, 255, 255, 200} : WHITE);
        }
    }

    // ==========================================================================
    // DRAW ACCUSATION
    // ==========================================================================
    void drawAccusation() {
        drawBackground();

        DrawText("CHOOSE FILE", screenW/2 - MeasureText("CHOOSE FILE", 36)/2, 30, 36, {110, 85, 40, 255});
        DrawText("Select who you believe committed the murder of Dr. Sheryl Iris",
                 screenW/2 - MeasureText("Select who you believe committed the murder of Dr. Sheryl Iris", 16)/2,
                 80, 16, {51, 51, 51, 255});

        for (int i = 0; i < 3; i++) {
            float cardX = screenW/2 - 330.0f + i * 230.0f;
            float cardY = 120;
            float cardW = 200, cardH = 300;

            bool isSelected = (selectedSuspect == i);
            Color border = isSelected ? Color{180, 140, 60, 255} : Color{180, 172, 192, 255};
            Color bg = isSelected ? Color{238, 230, 205, 255} : Color{228, 224, 238, 255};

            DrawRectangle(cardX, cardY, cardW, cardH, bg);
            DrawRectangleLinesEx({cardX, cardY, cardW, cardH}, isSelected ? 4 : 2, border);

            if (isSelected) {
                // Glow effect
                DrawRectangleLinesEx({cardX-3, cardY-3, cardW+6, cardH+6}, 2, {180, 140, 60, 90});
            }

            // Photo — digeser +30px ke bawah, pakai foto asli
            Rectangle photoRect = {cardX + 15, cardY + 45, cardW - 30, 155};
            DrawRectangleRec(photoRect, {210, 206, 222, 255});
            drawTextureFit(getSuspectAccusationTexture(i), photoRect);
            DrawRectangleLinesEx(photoRect, 1, border);

            // Name — digeser +30px ke bawah
            DrawText(suspects[i].name.c_str(),
                     cardX + (cardW - MeasureText(suspects[i].name.c_str(), 14))/2,
                     cardY + 210, 14, {51, 51, 51, 255});

            DrawText(suspects[i].job.c_str(),
                     cardX + (cardW - MeasureText(suspects[i].job.c_str(), 12))/2,
                     cardY + 230, 12, {95, 85, 75, 255});

            // Flags from investigation notes
            int flags = 0;
            for (auto& note : suspectNotes[i]) if (note.isChecked) flags++;
            DrawText(TextFormat("Flags: %d/5", flags),
                     cardX + (cardW - MeasureText(TextFormat("Flags: %d/5", flags), 13))/2,
                     cardY + 255, 13, flags >= 3 ? Color{200, 70, 55, 255} : Color{80, 145, 85, 255});

            // Hover indicator
            if (CheckCollisionPointRec(GetMousePosition(), {cardX, cardY, cardW, cardH}) &&
                !isSelected) {
                DrawRectangle(cardX, cardY, cardW, cardH, {0, 0, 0, 12});
            }
        }

        // Confirm button
        if (selectedSuspect >= 0) {
            Rectangle confirmBtn = {(float)screenW/2 - btnConfirmSuspect.width/2.0f, (float)screenH - 90, (float)btnConfirmSuspect.width, (float)btnConfirmSuspect.height};
            bool hov = CheckCollisionPointRec(GetMousePosition(), confirmBtn);
            DrawTexture(btnConfirmSuspect, (int)confirmBtn.x, (int)confirmBtn.y, hov ? Color{255,255,255,220} : WHITE);
        } else {
            DrawText("Pilih kartu tersangka terlebih dahulu",
                     screenW/2 - MeasureText("Pilih kartu tersangka terlebih dahulu", 16)/2,
                     screenH - 70, 16, {105, 88, 60, 255});
        }

        drawDecorativeBorder();
    }

    // ==========================================================================
    // DRAW ENDINGS
    // ==========================================================================
    void drawEndingTrue() {
        ClearBackground({255, 255, 255, 255});
        const Color darkText = {51, 51, 51, 255};

        // Label "Ending 1/2" — di atas, tepat tengah horizontal
        const char* label = "Ending 1/2";
        const int labelFs = 24;
        DrawText(label, screenW/2 - MeasureText(label, labelFs)/2, screenH/4, labelFs, darkText);

        // Pesan utama — bukan typewriter, langsung draw, word-wrap, tiap baris dicenter
        std::string msg = std::string("Selamat detektif ") + playerName +
                          " kamu telah berhasil menemukan pelaku pembunuhan dan keadilan dapat ditegakan";
        const int fs = 28;
        const int maxW = 760;
        {
            std::vector<std::string> lines;
            std::string line, word;
            for (size_t i = 0; i <= msg.size(); i++) {
                if (i == msg.size() || msg[i] == ' ') {
                    std::string test = line.empty() ? word : line + " " + word;
                    if (MeasureText(test.c_str(), fs) > maxW && !line.empty()) {
                        lines.push_back(line);
                        line = word;
                    } else { line = test; }
                    word = "";
                } else { word += msg[i]; }
            }
            if (!line.empty()) lines.push_back(line);
            int totalH = (int)lines.size() * (fs + 10);
            int ly = screenH/2 - totalH/2;
            for (auto& l : lines) {
                DrawText(l.c_str(), screenW/2 - MeasureText(l.c_str(), fs)/2, ly, fs, darkText);
                ly += fs + 10;
            }
        }

        // Back button — tengah bawah, seukuran Start (220x65)
        {
            const float bW = 220.0f, bH = 65.0f;
            Rectangle btn = {(float)screenW/2 - bW/2, (float)screenH - bH - 20.0f, bW, bH};
            bool hov = CheckCollisionPointRec(GetMousePosition(), btn);
            DrawTexturePro(btnBack,
                {0,0,(float)btnBack.width,(float)btnBack.height},
                btn, {0,0}, 0,
                hov ? Color{255,255,255,220} : WHITE);
        }

        drawDecorativeBorder();
    }

    void drawEndingBad() {
        ClearBackground({255, 255, 255, 255});
        const Color darkText = {51, 51, 51, 255};

        // Label "Ending 2/2" — di atas, tepat tengah horizontal
        const char* label = "Ending 2/2";
        const int labelFs = 24;
        DrawText(label, screenW/2 - MeasureText(label, labelFs)/2, screenH/4, labelFs, darkText);

        // Pesan utama — bukan typewriter, langsung draw, word-wrap, tiap baris dicenter
        std::string msg = "Case Failed, kamu telah gagal menemukan pelaku serta memasukkan orang tak bersalah ke penjara";
        const int fs = 28;
        const int maxW = 760;
        {
            std::vector<std::string> lines;
            std::string line, word;
            for (size_t i = 0; i <= msg.size(); i++) {
                if (i == msg.size() || msg[i] == ' ') {
                    std::string test = line.empty() ? word : line + " " + word;
                    if (MeasureText(test.c_str(), fs) > maxW && !line.empty()) {
                        lines.push_back(line);
                        line = word;
                    } else { line = test; }
                    word = "";
                } else { word += msg[i]; }
            }
            if (!line.empty()) lines.push_back(line);
            int totalH = (int)lines.size() * (fs + 10);
            int ly = screenH/2 - totalH/2;
            for (auto& l : lines) {
                DrawText(l.c_str(), screenW/2 - MeasureText(l.c_str(), fs)/2, ly, fs, darkText);
                ly += fs + 10;
            }
        }

        // Back button — tengah bawah, seukuran Start (220x65)
        {
            const float bW = 220.0f, bH = 65.0f;
            Rectangle btn = {(float)screenW/2 - bW/2, (float)screenH - bH - 20.0f, bW, bH};
            bool hov = CheckCollisionPointRec(GetMousePosition(), btn);
            DrawTexturePro(btnBack,
                {0,0,(float)btnBack.width,(float)btnBack.height},
                btn, {0,0}, 0,
                hov ? Color{255,255,255,220} : WHITE);
        }

        drawDecorativeBorder();
    }

    // ==========================================================================
    // DRAW PAUSE MENU
    // ==========================================================================
    void drawPause() {
        // Tombol seukuran Start: 220x65, jarak antar tombol 20px, padding box 30px sisi + 30px atas + 20px bawah
        const float btnW    = 220.0f;
        const float btnH    = 65.0f;
        const float btnGap  = 20.0f;
        const float padX    = 40.0f;
        const float padTop  = 70.0f;  // ruang untuk judul PAUSED
        const float padBot  = 24.0f;

        const float boxW    = btnW + padX * 2;
        const float boxH    = padTop + btnH + btnGap + btnH + padBot;
        const float boxX    = screenW / 2.0f - boxW / 2.0f;
        const float boxY    = screenH / 2.0f - boxH / 2.0f;

        const float btn1Y   = boxY + padTop;
        const float btn2Y   = btn1Y + btnH + btnGap;
        const float btnX    = screenW / 2.0f - btnW / 2.0f;

        DrawRectangle(0, 0, screenW, screenH, {200, 200, 210, 160});

        // Box background — gambar PauseBox.png fit ke box, atau fallback rect
        if (uiPauseBox.id > 0) {
            DrawTexturePro(uiPauseBox,
                {0, 0, (float)uiPauseBox.width, (float)uiPauseBox.height},
                {boxX, boxY, boxW, boxH}, {0, 0}, 0, WHITE);
        } else {
            DrawRectangle((int)boxX, (int)boxY, (int)boxW, (int)boxH, {235, 230, 245, 255});
            DrawRectangleLinesEx({boxX, boxY, boxW, boxH}, 2, {180, 140, 80, 255});
        }

        DrawText("PAUSED", screenW/2 - MeasureText("PAUSED", 30)/2,
                 (int)(boxY + 22), 30, {140, 100, 40, 255});

        auto drawBtnScaled = [&](Texture2D tex, float y) -> bool {
            Rectangle dest = {btnX, y, btnW, btnH};
            bool hov = CheckCollisionPointRec(GetMousePosition(), dest);
            DrawTexturePro(tex,
                {0, 0, (float)tex.width, (float)tex.height},
                dest, {0, 0}, 0,
                hov ? Color{255,255,255,220} : WHITE);
            return hov && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
        };

        if (drawBtnScaled(btnContinue, btn1Y)) paused = false;
        if (drawBtnScaled(btnMainMenu, btn2Y)) {
            paused = false;
            startFadeToScene(SCENE_MAIN_MENU);
        }
    }

    void drawPauseButton() {
        float pw = btnPause.width  / 5.0f;
        float ph = btnPause.height / 5.0f;
        float x  = screenW - pw - 10;
        float y  = 10;
        Rectangle pauseBtn = {x, y, pw, ph};
        bool hov = CheckCollisionPointRec(GetMousePosition(), pauseBtn);
        DrawTexturePro(btnPause,
            {0, 0, (float)btnPause.width, (float)btnPause.height},
            pauseBtn, {0, 0}, 0,
            hov ? Color{255,255,255,220} : WHITE);
        if (hov && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) paused = true;
    }

    void run() {
        while (!WindowShouldClose()) {
            float dt = GetFrameTime();
            update(dt);
            draw();
        }
    }
};

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================
int main() {
    const int screenW = 1280;
    const int screenH = 720;

    InitWindow(screenW, screenH, "CASE 402 - A Murder Mystery");
    SetTargetFPS(60);
    SetExitKey(0); // Disable default ESC exit

    // Exception handling wrapping main (Academic Req #8)
    try {
        Case402Game game(screenW, screenH);
        game.run();
    } catch (const std::exception& e) {
        // Log error to file (File Handling - Academic Req #10)
        std::ofstream errLog("error.log");
        errLog << "Fatal error: " << e.what() << "\n";
        errLog.close();
    }

    CloseWindow();
    return 0;
}