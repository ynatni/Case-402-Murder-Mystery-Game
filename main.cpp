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

        // Satu gambar popup besar, melebar dari area kiri sampai area kanan
        // (gabungan leftPanel + rightPanel jadi satu), gambar ini sudah termasuk
        // "kotak tulisan" sendiri dari asetmu — tidak ada DrawRectangle apapun lagi.
        Rectangle leftPanel = {50, 100, (float)sw/2 - 70, (float)sh - 200};
        Rectangle rightPanel = {(float)sw/2, 100, (float)sw/2 - 50, (float)sh - 200};
        Rectangle popupArea = {leftPanel.x, leftPanel.y,
                                (rightPanel.x + rightPanel.width) - leftPanel.x,
                                leftPanel.height};

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

        // Deskripsi — ditumpuk di atas gambar, posisi tetap sama seperti sebelumnya
        DrawGameText(title,
                     (int)(rightPanel.x + 20),
                     (int)(rightPanel.y + 20),
                     22, FadeColor({220, 180, 100, 255}, fadeAlpha));

        // Word-wrap description
        int fy = (int)(rightPanel.y + 60);
        std::string word, desc = description;
        std::string line;
        for (size_t i = 0; i <= desc.size(); i++) {
            if (i == desc.size() || desc[i] == ' ') {
                std::string testLine = line.empty() ? word : line + " " + word;
                if (MeasureText(testLine.c_str(), 18) > rightPanel.width - 40) {
                    DrawText(line.c_str(), (int)(rightPanel.x + 20), fy, 18,
                             FadeColor(WHITE, fadeAlpha));
                    fy += 25;
                    line = word;
                } else {
                    line = testLine;
                }
                word = "";
            } else {
                word += desc[i];
            }
        }
        if (!line.empty()) DrawText(line.c_str(), (int)(rightPanel.x + 20), fy, 18,
                                     FadeColor(WHITE, fadeAlpha));

        // Click to continue
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

    // ====== ASSET CHAPTER 2 BOARD (full-screen suspect boards) ======
    Texture2D boardOlivia;
    Texture2D boardMarcus;
    Texture2D boardEthan;

    // ====== ASSET CHAPTER 3 BOARD (reconstruction puzzle) ======
    Texture2D bgChapter3Puzzle;     // background papan cork (chapter3Board.png)
    Texture2D evCctv;                // Cctv.png
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
        btnStartHover    = LoadTexture((std::string(UI_PATH) + "StartHoover.png").c_str());
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

        // Chapter 2 suspect boards (full 16:9)
        boardOlivia = LoadTexture((std::string(ASSET_PATH) + "oliviaBoard.png").c_str());
        boardMarcus = LoadTexture((std::string(ASSET_PATH) + "marcusBoard.png").c_str());
        boardEthan  = LoadTexture((std::string(ASSET_PATH) + "ethanBoard.png").c_str());

        // Chapter 3 reconstruction board
        bgChapter3Puzzle = LoadTexture((std::string(ASSET_PATH) + "chapter3Board.png").c_str());
        evCctv           = LoadTexture((std::string(ASSET_PATH) + "Cctv.png").c_str());
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

        // Chapter 2 board textures
        UnloadTexture(boardOlivia);
        UnloadTexture(boardMarcus);
        UnloadTexture(boardEthan);

        // Chapter 3 board textures
        UnloadTexture(bgChapter3Puzzle);
        UnloadTexture(evCctv);
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
        correctOrder = {6, 0, 1, 2, 4, 5, 3};
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
            case 4: return evLaptop;
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
        } else if (scene == SCENE_CHAPTER3_PUZZLE) {
            puzzleSolved = false;
            selectedPuzzleSlot = -1;
            chapter3SuspectPick = 0;
        } else if (scene == SCENE_CHAPTER3_ACCUSATION) {
            selectedSuspect = -1;
            // Count flags from ALL suspect notes (1/6 per checklist item)
            flagCount = 0;
            for (int si = 0; si < (int)suspectNotes.size(); si++) {
                for (auto& note : suspectNotes[si]) {
                    if (note.isChecked) flagCount++;
                }
            }
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
            case SCENE_CHAPTER3_ACCUSATION: updateAccusation(); break;
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

        // Auto-advance step 3 (shock image, 3 second)
        if (prologStep == 3 && sceneTimer >= 3.0f) {
            prologStep = 4;
            sceneTimer = 0;
            typewriter.setText("TIDAK MUNGKIN");
        }

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
            case 1:  typewriter.setText("Pesan tidak dibalas"); break;
            case 2:  typewriter.setText("Panggilan tidak terjawab\n\nAku sebaiknya mendatangi kamar Mrs. Iris"); break;
            case 3:  typewriter.setText("Pintu tidak terkunci"); break;
            // Step 3->4 is auto
            case 4:  typewriter.setText("TIDAK MUNGKIN"); break;
            case 5:  typewriter.setText("Mrs. Iris... apa yang terjadi"); break;
            case 6:  typewriter.setText("...bersimbah darah. Apakah mungkin..."); break;
            case 7:  typewriter.setText(""); break; // brankas scene
            case 8:  // Newspaper
                typewriter.setText("Kasus pembunuhan dilaporkan, polisi segera mengamankan lokasi kejadian.\n\nPolisi masih menyelidiki keterlibatan pihak lain dalam kasus kematian Dr. Sheryl Iris.\n\nPenelitian obat kanker yang akan diumumkan pada hari yang sama juga dilaporkan hilang.");
                break;
            case 9:  // After newspaper - go to name input
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

        // Step -1: hanya judul chapter fade-in, klik untuk lanjut ke cerita+CCTV
        if (prologStep == -1) {
            if (sceneTimer > 2.0f && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                prologStep = 0;
                sceneTimer = 0;
                typewriter.setText("Rekaman CCTV ditemukan di lokasi kejadian.");
            }
            return;
        }

        // Step 0: mulai typewriter setelah 1.5 detik (fade-in sudah selesai)
        if (prologStep == 0 && sceneTimer > 1.5f && typewriter.fullText.empty()) {
            typewriter.setText("Rekaman CCTV ditemukan di lokasi kejadian.");
        }

        if (typewriter.done && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (prologStep < 3) {
                prologStep++;
                sceneTimer = 0;
                if (prologStep == 1)
                    typewriter.setText("Rekaman menunjukkan tiga orang yang mengunjungi Apartemen 402 pada malam pembunuhan.");
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
        // Koordinat checklist overlay (skalakan seperti di draw)
        float scaleX = (float)screenW / 1456.0f;
        float scaleY = (float)screenH / 816.0f;
        float cbAreaX = 480.0f * scaleX;
        float cbAreaY = 175.0f * scaleY;
        float cbItemH = 65.0f * scaleY;
        float cbW     = 18.0f * scaleX;

        // Koordinat HARUS sama persis dengan drawChapter2Board (margin 20px dari tepi)
        const int BTN_MARGIN2 = 20;
        const int BTN_BOTTOM2 = screenH - BTN_MARGIN2;
        Rectangle backBtn    = {(float)BTN_MARGIN2, (float)(BTN_BOTTOM2 - btnCh2Left.height), (float)btnCh2Left.width, (float)btnCh2Left.height};
        Rectangle nextBtn    = {(float)(screenW - BTN_MARGIN2 - btnCh2Right.width), (float)(BTN_BOTTOM2 - btnCh2Right.height), (float)btnCh2Right.width, (float)btnCh2Right.height};
        Rectangle confirmBtn = {(float)(screenW - BTN_MARGIN2 - btnConfirm.width), (float)(BTN_BOTTOM2 - btnConfirm.height), (float)btnConfirm.width, (float)btnConfirm.height};

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
                    float ny = cbAreaY + i * cbItemH;
                    Rectangle clickArea = {cbAreaX - 5, ny, cbW + 250.0f * scaleX, cbItemH};
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
    void updateChapter3Puzzle() {
        if (puzzleSolved) {
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                startFadeToScene(SCENE_CHAPTER3_ACCUSATION);
            }
            return;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();

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
                        // Swap antar index puzzleOrder (0..6)
                        std::swap(puzzleOrder[selectedPuzzleSlot], puzzleOrder[k]);
                        selectedPuzzleSlot = -1;
                        // Check win
                        if (puzzleOrder == correctOrder) {
                            puzzleSolved = true;
                            typewriter.setText("Case reconstruction complete");
                        }
                    }
                    break;
                }
            }
        }
    }

    // Posisi 3 tombol kecil foto tersangka, persis di bawah slot foto tengah-atas (SUSPECT_SLOT_INDEX)
    Rectangle getSuspectThumbRect(int t) {
        Rectangle suspectSlot = getPuzzleSlotRect(SUSPECT_SLOT_INDEX);
        const float thumbW = 30, thumbH = 30, gap = 6;
        float totalW = thumbW * 3 + gap * 2;
        float startX = suspectSlot.x + suspectSlot.width/2.0f - totalW/2.0f;
        float ty = suspectSlot.y + suspectSlot.height + 8;
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
            // Y = screenH-80, identik dengan drawEndingTrue() & drawEndingBad()
            Rectangle btnRect = {(float)screenW/2 - 120, (float)screenH - 80, 240, 50};
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
            case SCENE_CHAPTER3_ACCUSATION: { drawAccusation(); } break;
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
        const int badgeY    = screenH / 9;
        const int titleY    = screenH * 38 / 100;
        const int subtitleY = titleY + 62;
        const int btn1Y     = screenH * 58 / 100;
        const int btn2Y     = btn1Y + 90;
        const int btn3Y     = btn2Y + 90;

        // Badge "CASE 402"
        DrawRectangle(screenW/2 - 70, badgeY, 140, 140, {230, 225, 240, 255});
        DrawRectangleLinesEx({(float)screenW/2 - 70, (float)badgeY, 140, 140}, 3, {180, 140, 80, 255});
        DrawText("CASE", screenW/2 - 35, badgeY + 35, 28, {180, 140, 80, 255});
        DrawText("402",  screenW/2 - 28, badgeY + 70, 36, {51, 51, 51, 255});

        // Judul
        DrawText("CASE 402",
                 screenW/2 - MeasureText("CASE 402", 52)/2, titleY, 52, {180, 140, 60, 255});
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
            "  Programming  :  [Nama Programmer]",
            "  Art & Design :  [Nama Artist / DKV]",
            "",
            "Built with raylib (www.raylib.com)",
            "",
            "Academic Project - 2024",
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
        drawBackground();

        // Scene-specific image placeholders
        struct SceneInfo { const char* label; Color bgColor; };
        static SceneInfo infos[] = {
            {"", {10,10,20,255}},                          // Step 0: date
            {"[Asisten dengan ponsel - Chat Sheryl]", {20,15,30,255}},  // 1
            {"[Tangan membuka pintu]", {25,20,30,255}},    // 2
            {"[Asisten terkejut]", {40,15,15,255}},        // 3 - tense
            {"[Sheryl terbaring di lantai]", {35,10,10,255}}, // 4
            {"[Brankas close-up - kosong]", {20,15,35,255}}, // 5-6
            {"[Brankas close-up - kosong]", {20,15,35,255}}, // 6
            {"", {5,5,10,255}},                            // 7 - black
            {"[Halaman Koran]", {220,200,150,255}},        // 8 - newspaper
        };

        int idx = std::min(prologStep, 8);
        ClearBackground(infos[idx].bgColor);

        if (prologStep == 8) {
            // Newspaper style
            DrawRectangle(50, 50, screenW - 100, screenH - 100, {220, 200, 150, 255});
            DrawRectangleLinesEx({50,50,(float)screenW-100,(float)screenH-100}, 4, {60,40,20,255});
            DrawText("JAKARTA POST", 80, 70, 32, {40,20,10,255});
            DrawLine(80, 110, screenW - 80, 110, {60,40,20,255});
            DrawText("KASUS PEMBUNUHAN GUNCANG APARTEMEN MEWAH", 80, 125, 14, {40,20,10,255});
            DrawLine(80, 145, screenW - 80, 145, {100,70,30,255});
        } else if (prologStep >= 1 && prologStep <= 7) {
            // Draw scene placeholder box
            DrawRectangle(100, 50, screenW - 200, screenH/2, {20,20,35,255});
            DrawRectangleLinesEx({100,50,(float)screenW-200,(float)screenH/2}, 2, {80,60,100,255});
            if (prologStep < 8) {
                DrawText(infos[idx].label,
                         screenW/2 - MeasureText(infos[idx].label,18)/2,
                         50 + screenH/4 - 10, 18, {120,100,150,255});
            }
        }

        // Typewriter text overlay
        if (prologStep != 7) {
            int ty = (prologStep >= 1 && prologStep <= 7) ? screenH/2 + 40 : screenH/2 - 60;
            typewriter.draw(100, ty, 22, prologStep == 8 ? Color{40,20,10,255} : Color{51,51,51,255});
        }

        // Click hint
        if (typewriter.done && prologStep != 7) {
            if ((int)(GetTime() * 2) % 2 == 0) {
                DrawText("[ Klik untuk melanjutkan ]",
                         screenW/2 - 115, screenH - 60, 18, {120, 90, 40, 255});
            }
        }

        drawDecorativeBorder();
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
        ClearBackground({248, 247, 245, 255});

        if (prologStep == 0) {
            // Step 0: hanya typewriter cerita pembuka, persis di tengah layar
            typewriter.draw(screenW/2 - 450, screenH/2 - 20, 22, {51, 51, 51, 255});

            if (typewriter.done && (int)(GetTime()*2)%2==0) {
                DrawText("[ Klik untuk melanjutkan ]",
                         screenW/2 - MeasureText("[ Klik untuk melanjutkan ]", 16)/2,
                         screenH - 60, 16, {110, 85, 45, 255});
            }
            return;
        }

        // Step 1: judul chapter, persis di tengah layar
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

        // Pulsing indicator dot tetap ada sebagai penanda "ada bukti di sini",
        // sedikit lebih terang saat hover supaya kelihatan responsif
        float pulse = (sinf(GetTime() * 3) + 1) * 0.5f;
        unsigned char alpha = hov ? 255 : (unsigned char)(180 + 60 * pulse);
        DrawCircle(x + w - 12, y + 12, 6, {51, 51, 51, alpha});
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
        // Background terang sama seperti Chapter 1 & 3
        ClearBackground({248, 247, 245, 255});

        // Step -1: HANYA judul chapter fade-in, persis seperti Chapter 1 & 3 (tanpa CCTV)
        if (prologStep == -1) {
            float t = std::min(1.0f, sceneTimer / 2.0f);
            Color titleColor = {(unsigned char)(51*t), (unsigned char)(51*t), (unsigned char)(51*t), 255};

            DrawText("CHAPTER 2", screenW/2 - MeasureText("CHAPTER 2", 48)/2, screenH/2 - 50, 48, titleColor);
            DrawText("The Suspect", screenW/2 - MeasureText("The Suspect", 28)/2, screenH/2 + 20, 28, titleColor);

            if (sceneTimer > 2.0f && (int)(GetTime()*2)%2==0) {
                DrawText("[ Klik untuk melanjutkan ]",
                         screenW/2 - MeasureText("[ Klik untuk melanjutkan ]", 16)/2,
                         screenH - 60, 16, {110, 85, 45, 255});
            }
            return;
        }

        if (prologStep == 0) {
            // Judul sudah ditampilkan di step -1 (opening), di sini langsung konten CCTV
            DrawRectangle(100, 200, screenW-200, screenH-320, {228, 224, 236, 255});
            DrawRectangleLinesEx({100, 200, (float)screenW-200, (float)screenH-320}, 2, {180, 172, 195, 255});
            DrawText("[Rekaman CCTV - Gambar tidak jelas]",
                     screenW/2 - MeasureText("[Rekaman CCTV - Gambar tidak jelas]", 18)/2,
                     screenH/2 - 10, 18, {130, 122, 145, 255});

            if (sceneTimer > 1.5f) {
                typewriter.draw(100, screenH - 100, 20, {51, 51, 51, 255});
                if (typewriter.done && (int)(GetTime()*2)%2==0)
                    DrawText("[ Klik untuk melanjutkan ]",
                             screenW/2 - MeasureText("[ Klik untuk melanjutkan ]", 16)/2,
                             screenH - 40, 16, {110, 85, 45, 255});
            }
        } else if (prologStep == 1) {
            typewriter.draw((screenW - 900) / 2, screenH/2 - 20, 22, {51, 51, 51, 255});
            if (typewriter.done && (int)(GetTime()*2)%2==0)
                DrawText("[ Klik untuk melanjutkan ]",
                         screenW/2 - MeasureText("[ Klik untuk melanjutkan ]", 16)/2,
                         screenH - 40, 16, {110, 85, 45, 255});
        } else if (prologStep == 2) {
            // Show 3 suspects silhouettes — benar-benar center terhadap layar
            DrawText("Pengunjung Teridentifikasi", screenW/2 - MeasureText("Pengunjung Teridentifikasi",28)/2, 100, 28, {51,51,51,255});

            const int cardW = 150, cardH = 250, gap = 50;
            const int totalW = 3 * cardW + 2 * gap;
            const int startX = screenW/2 - totalW/2;

            for (int i = 0; i < 3; i++) {
                int sx = startX + i * (cardW + gap);
                DrawRectangle(sx, 200, cardW, cardH, {218, 214, 228, 255});
                DrawRectangleLinesEx({(float)sx, 200, (float)cardW, (float)cardH}, 2, {180,140,80,255});
                DrawText(suspects[i].name.c_str(),
                         sx + (cardW - MeasureText(suspects[i].name.c_str(),13))/2,
                         458, 13, {51, 51, 51, 255});
            }
            if ((int)(GetTime()*2)%2==0)
                DrawText("[ Klik untuk melanjutkan ]",
                         screenW/2 - MeasureText("[ Klik untuk melanjutkan ]", 16)/2,
                         screenH - 40, 16, {110, 85, 45, 255});
        } else {
            // prologStep == 3: typewriter instruksi, persis di tengah layar
            typewriter.draw(screenW/2 - 450, screenH/2 - 20, 22, {51, 51, 51, 255});
            if (typewriter.done && (int)(GetTime()*2)%2==0)
                DrawText("[ Klik untuk melanjutkan ]",
                         screenW/2 - MeasureText("[ Klik untuk melanjutkan ]", 16)/2,
                         screenH - 40, 16, {110, 85, 45, 255});
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
        // "INVESTIGATION NOTES" di gambar sekitar x=480..780, y=130..520 (dari referensi 1456x816)
        // Kita skalakan ke screenW x screenH
        float scaleX = (float)screenW / 1456.0f;
        float scaleY = (float)screenH / 816.0f;

        // Area kotak checklist di gambar asli (berdasarkan posisi di ethanBoard.png):
        // kotak mulai sekitar x=480, y=175, lebar=280, item-item berjarak ~65px
        float cbAreaX = 480.0f * scaleX;
        float cbAreaY = 175.0f * scaleY;
        float cbItemH = 65.0f * scaleY;
        float cbW     = 18.0f * scaleX;
        float cbH     = 18.0f * scaleY;

        for (int i = 0; i < (int)notes.size(); i++) {
            float ny = cbAreaY + i * cbItemH;

            // Kotak checkbox — transparan, hanya border + X bila checked
            Rectangle cb = {cbAreaX, ny + cbItemH * 0.18f, cbW, cbH};

            // Hover highlight tipis
            Vector2 mp = GetMousePosition();
            // Area klik lebih lebar dari kotak agar mudah diklik
            Rectangle clickArea = {cbAreaX - 5, ny, cbW + 250.0f * scaleX, cbItemH};
            bool hov = CheckCollisionPointRec(mp, clickArea);
            if (hov) DrawRectangleLinesEx(clickArea, 1, {255, 230, 100, 80});

            // X mark bila sudah dicentang
            if (notes[i].isChecked) {
                // Gambar X dengan dua garis diagonal, tebal dan jelas
                int x1 = (int)cb.x, y1 = (int)cb.y;
                int x2 = (int)(cb.x + cb.width), y2 = (int)(cb.y + cb.height);
                DrawLineEx({(float)x1, (float)y1}, {(float)x2, (float)y2}, 3.0f, {220, 50, 50, 230});
                DrawLineEx({(float)x2, (float)y1}, {(float)x1, (float)y2}, 3.0f, {220, 50, 50, 230});
            }
        }

        // Navigation buttons — kiri bawah & kanan bawah, mepet samping dengan margin 20px
        const int BTN_MARGIN = 20;
        const int BTN_BOTTOM = screenH - BTN_MARGIN;
        Vector2 mpNav = GetMousePosition();

        // Tombol KIRI (back) — pojok kiri bawah
        if (currentSuspectIdx > 0) {
            int bx = BTN_MARGIN;
            int by = BTN_BOTTOM - btnCh2Left.height;
            Rectangle r = {(float)bx, (float)by, (float)btnCh2Left.width, (float)btnCh2Left.height};
            bool hov = CheckCollisionPointRec(mpNav, r);
            DrawTexture(hov ? btnCh2LeftHover : btnCh2Left, bx, by, WHITE);
        }

        // Tombol KANAN (next) atau CONFIRM — pojok kanan bawah
        if (currentSuspectIdx < 2) {
            int bx = screenW - BTN_MARGIN - btnCh2Right.width;
            int by = BTN_BOTTOM - btnCh2Right.height;
            Rectangle r = {(float)bx, (float)by, (float)btnCh2Right.width, (float)btnCh2Right.height};
            bool hov = CheckCollisionPointRec(mpNav, r);
            DrawTexture(hov ? btnCh2RightHover : btnCh2Right, bx, by, WHITE);
        } else {
            // Suspect terakhir: Confirm di pojok kanan bawah
            int bx = screenW - BTN_MARGIN - btnConfirm.width;
            int by = BTN_BOTTOM - btnConfirm.height;
            Rectangle r = {(float)bx, (float)by, (float)btnConfirm.width, (float)btnConfirm.height};
            bool hov = CheckCollisionPointRec(mpNav, r);
            DrawTexture(btnConfirm, bx, by, hov ? Color{255,255,255,220} : WHITE);
        }

        if (waitingForClick) {
            DrawRectangle(0, screenH - 100, screenW, 50, {0, 0, 0, 180});
            typewriter.draw(screenW/2 - 450, screenH - 90, 20, WHITE);
        }
    }

    // Tampilan "meja kerja" sebelum zoom ke board investigasi (mirip pola Chapter 1)
    void drawChapter2Desk() {
        DrawText("MEJA KERJA DETEKTIF", screenW/2 - MeasureText("MEJA KERJA DETEKTIF", 28)/2,
                 60, 28, {51, 51, 51, 255});

        Rectangle deskZoomRect = {(float)screenW/2 - 150, (float)screenH/2 - 100, 300, 200};
        bool hov = CheckCollisionPointRec(GetMousePosition(), deskZoomRect);
        DrawRectangle(deskZoomRect.x, deskZoomRect.y, deskZoomRect.width, deskZoomRect.height,
                      hov ? Color{222, 212, 235, 255} : Color{210, 205, 225, 255});
        DrawRectangleLinesEx(deskZoomRect, 3, {180, 140, 80, 255});
        DrawText("[ Berkas Data Pengunjung ]",
                 screenW/2 - MeasureText("[ Berkas Data Pengunjung ]", 16)/2,
                 (int)(deskZoomRect.y + deskZoomRect.height/2 - 8), 16, {90, 65, 30, 255});

        // Pulse indicator biar terlihat interaktif
        float pulse = (sinf(GetTime() * 3) + 1) * 0.5f;
        DrawCircle((int)(deskZoomRect.x + deskZoomRect.width - 16), (int)(deskZoomRect.y + 16), 6,
                   {(unsigned char)(180*pulse), 220, 80, 255});

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
        ClearBackground({248, 247, 245, 255});

        float t = std::min(1.0f, sceneTimer / 2.0f);
        Color tc = {(unsigned char)(51*t), (unsigned char)(51*t), (unsigned char)(51*t), 255};
        DrawText("CHAPTER 3", screenW/2 - MeasureText("CHAPTER 3", 48)/2, screenH/2 - 60, 48, tc);
        DrawText("Reconstruction", screenW/2 - MeasureText("Reconstruction", 28)/2, screenH/2 + 10, 28, tc);

        if (sceneTimer > 2.0f) {
            typewriter.draw((screenW - 900) / 2, screenH - 80, 20, {51, 51, 51, 255});
            if (typewriter.done && (int)(GetTime()*2)%2==0)
                DrawText("[ Klik untuk melanjutkan ]",
                         screenW/2 - MeasureText("[ Klik untuk melanjutkan ]", 16)/2,
                         screenH - 40, 16, {110, 85, 45, 255});
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
            DrawRectangleLinesEx(thumbRect, isPicked ? 3 : 1, isPicked ? Color{220, 180, 60, 0} : Color{40, 35, 30, 0});
            if (CheckCollisionPointRec(GetMousePosition(), thumbRect)) {
                DrawRectangleLinesEx({thumbRect.x-1, thumbRect.y-1, thumbRect.width+2, thumbRect.height+2}, 2, {255, 220, 120, 0});
            }
        }

        if (puzzleSolved) {
            DrawRectangle(0, screenH/2 - 60, screenW, 120, {240, 238, 245, 235});
            DrawText("Case Reconstruction Complete!", screenW/2 - MeasureText("Case Reconstruction Complete!", 32)/2,
                     screenH/2 - 20, 32, {51, 51, 51, 255});
            if ((int)(GetTime()*2)%2==0)
                DrawText("[ Klik untuk melanjutkan ]",
                         screenW/2 - MeasureText("[ Klik untuk melanjutkan ]", 18)/2,
                         screenH/2 + 30, 18, {110, 85, 45, 255});
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

            // Photo
            DrawRectangle(cardX + 20, cardY + 15, cardW - 40, 150, {210, 206, 222, 255});
            DrawRectangleLinesEx({cardX+20, cardY+15, cardW-40, 150}, 1, {178,170,192,255});
            DrawText("[PHOTO]", cardX + 45, cardY + 85, 14, {140, 132, 155, 255});

            // Name
            DrawText(suspects[i].name.c_str(),
                     cardX + (cardW - MeasureText(suspects[i].name.c_str(), 14))/2,
                     cardY + 175, 14, {51, 51, 51, 255});

            DrawText(suspects[i].job.c_str(),
                     cardX + (cardW - MeasureText(suspects[i].job.c_str(), 12))/2,
                     cardY + 195, 12, {95, 85, 75, 255});

            // Flags from investigation notes
            int flags = 0;
            for (auto& note : suspectNotes[i]) if (note.isChecked) flags++;
            DrawText(TextFormat("Flags: %d/5", flags),
                     cardX + (cardW - MeasureText(TextFormat("Flags: %d/5", flags), 13))/2,
                     cardY + 220, 13, flags >= 3 ? Color{200, 70, 55, 255} : Color{80, 145, 85, 255});

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
        ClearBackground({5, 20, 5, 255});
        for (int y = 0; y < screenH; y += 4) DrawLine(0, y, screenW, y, {0,0,0,20});

        DrawText("CASE CLOSED", screenW/2 - MeasureText("CASE CLOSED", 56)/2, 80, 56, {80, 220, 80, 255});
        DrawLine(100, 150, screenW - 100, 150, {80, 180, 80, 255});

        DrawText("Detektif berhasil mengidentifikasi pelaku.",
                 screenW/2 - MeasureText("Detektif berhasil mengidentifikasi pelaku.", 22)/2, 180, 22, WHITE);
        DrawText(TextFormat("Selamat, Detektif %s!", playerName.c_str()),
                 screenW/2 - MeasureText(TextFormat("Selamat, Detektif %s!", playerName.c_str()), 26)/2,
                 220, 26, {180, 220, 180, 255});

        DrawRectangle(screenW/2 - 250, 270, 500, 200, {10, 30, 10, 255});
        DrawRectangleLinesEx({(float)screenW/2 - 250, 270, 500, 200}, 2, {80, 180, 80, 255});
        DrawText("VERDICT", screenW/2 - MeasureText("VERDICT", 24)/2, 285, 24, {80, 220, 80, 255});
        DrawText("Ethan Cross", screenW/2 - MeasureText("Ethan Cross", 32)/2, 320, 32, {220, 80, 80, 255});
        DrawText("BERSALAH ATAS PEMBUNUHAN DR. SHERYL IRIS",
                 screenW/2 - MeasureText("BERSALAH ATAS PEMBUNUHAN DR. SHERYL IRIS", 16)/2,
                 365, 16, WHITE);
        DrawText("DAN PENCURIAN PROTOTYPE OBAT KANKER",
                 screenW/2 - MeasureText("DAN PENCURIAN PROTOTYPE OBAT KANKER", 16)/2,
                 388, 16, WHITE);
        DrawText("Divonis 20 tahun penjara.",
                 screenW/2 - MeasureText("Divonis 20 tahun penjara.", 18)/2, 420, 18, {180, 180, 180, 255});

        // Back button
        Rectangle btn = {(float)screenW/2 - 120, (float)screenH - 80, 240, 50};
        bool hov = CheckCollisionPointRec(GetMousePosition(), btn);
        DrawRectangleRec(btn, hov ? Color{40,80,40,255} : Color{20,40,20,255});
        DrawRectangleLinesEx(btn, 2, {80, 180, 80, 255});
        DrawText("BACK TO MAIN MENU", screenW/2 - MeasureText("BACK TO MAIN MENU", 18)/2, screenH - 65, 18, WHITE);

        drawDecorativeBorder();
    }

    void drawEndingBad() {
        ClearBackground({20, 5, 5, 255});
        for (int y = 0; y < screenH; y += 4) DrawLine(0, y, screenW, y, {0,0,0,20});

        DrawText("CASE FAILED", screenW/2 - MeasureText("CASE FAILED", 56)/2, 80, 56, {220, 60, 60, 255});
        DrawLine(100, 150, screenW - 100, 150, {180, 60, 60, 255});

        DrawText("Detektif gagal mengidentifikasi pelaku.",
                 screenW/2 - MeasureText("Detektif gagal mengidentifikasi pelaku.", 22)/2, 180, 22, WHITE);
        DrawText(TextFormat("Kasus ini tetap terbuka, Detektif %s.", playerName.c_str()),
                 screenW/2 - MeasureText(TextFormat("Kasus ini tetap terbuka, Detektif %s.", playerName.c_str()), 22)/2,
                 215, 22, {220, 160, 160, 255});

        DrawRectangle(screenW/2 - 260, 265, 520, 180, {30, 10, 10, 255});
        DrawRectangleLinesEx({(float)screenW/2 - 260, 265, 520, 180}, 2, {180, 60, 60, 255});
        DrawText("Pembunuh yang sesungguhnya masih bebas berkeliaran.",
                 screenW/2 - MeasureText("Pembunuh yang sesungguhnya masih bebas berkeliaran.", 16)/2,
                 285, 16, WHITE);
        DrawText("Kebenaran terkubur bersama rahasia Apartemen 402.",
                 screenW/2 - MeasureText("Kebenaran terkubur bersama rahasia Apartemen 402.", 16)/2,
                 315, 16, {200, 180, 180, 255});
        DrawText("Apakah keadilan akan pernah ditegakkan?",
                 screenW/2 - MeasureText("Apakah keadilan akan pernah ditegakkan?", 18)/2,
                 360, 18, {180, 140, 140, 255});

        Rectangle btn = {(float)screenW/2 - 120, (float)screenH - 80, 240, 50};
        bool hov = CheckCollisionPointRec(GetMousePosition(), btn);
        DrawRectangleRec(btn, hov ? Color{80,20,20,255} : Color{40,10,10,255});
        DrawRectangleLinesEx(btn, 2, {180, 60, 60, 255});
        DrawText("BACK TO MAIN MENU", screenW/2 - MeasureText("BACK TO MAIN MENU", 18)/2, screenH - 65, 18, WHITE);

        drawDecorativeBorder();
    }

    // ==========================================================================
    // DRAW PAUSE MENU
    // ==========================================================================
    void drawPause() {
        const int boxY   = screenH / 2 - uiPauseBox.height / 2;
        const int boxX   = screenW / 2 - uiPauseBox.width / 2;
        const int titleY = boxY + 20;
        const int btn1Y  = boxY + 100;
        const int btn2Y  = btn1Y + 65;

        DrawRectangle(0, 0, screenW, screenH, {200, 200, 210, 160});
        DrawTexture(uiPauseBox, boxX, boxY, WHITE);
        DrawText("PAUSED", screenW/2 - MeasureText("PAUSED", 36)/2, titleY, 36, {140, 100, 40, 255});

        auto drawBtn = [&](Texture2D tex, int y) -> bool {
            int x = screenW/2 - tex.width/2;
            Rectangle r = {(float)x, (float)y, (float)tex.width, (float)tex.height};
            bool hov = CheckCollisionPointRec(GetMousePosition(), r);
            DrawTexture(tex, x, y, hov ? Color{255,255,255,220} : WHITE);
            return hov && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
        };

        if (drawBtn(btnContinue, btn1Y)) paused = false;
        if (drawBtn(btnMainMenu, btn2Y)) {
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