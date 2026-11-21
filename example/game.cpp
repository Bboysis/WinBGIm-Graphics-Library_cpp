 /* =====================================================================
   CUBE TRUCK 3D  v2  -  blocky 3D monster-truck racing game
   Dev-C++ + WinBGIm (graphics.h).  One file, no other libraries.

   graphics.h only draws 2D shapes, so the 3D is made with our own maths:
     1. perspective projection  : 3D point -> 2D screen point   (proj)
     2. back-face culling       : skip faces that look away     (drawPoly)
     3. painter's algorithm     : draw far things first         (render)
     4. lighting + distance fog : shade each face               (drawPoly)
     5. 3D rotation (matrix)    : the truck tilts on ramps      (applyXf)
     6. double buffering        : draw on hidden page, then show (main)

   CONTROLS
     UP / W = gas      DOWN / S = brake     LEFT, RIGHT / A, D = steer
     SPACE = jump      P = pause            M = sound on/off
     ENTER = start / continue / next level  ESC = quit

   GAME
     Collect coins, hit the blue BOOST pads, jump the gaps, use the ramps,
     avoid the red crates.  3 lives, checkpoint flags, 5 different worlds.
   ===================================================================== */
#include <graphics.h>
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

/* ------------------------------ constants ------------------------------ */
const int    SW = 900, SH = 600;                       // window size
const double PI = 3.14159265358979;
const double SEG_LEN = 4.0;                            // length of one road piece
const double LANE_W = 2.4, ROAD_HALF = 3.6;            // 3 lanes
const double RAMP_H = 2.0, GRAV = 24.0, NOGROUND = -1000.0;

/* ------------------------- small maths helpers ------------------------- */
struct V3 { double x, y, z; };
V3 mkv(double x, double y, double z) { V3 v = { x, y, z }; return v; }
V3 sub(const V3 &a, const V3 &b) { return mkv(a.x - b.x, a.y - b.y, a.z - b.z); }
V3 cross(const V3 &a, const V3 &b) { return mkv(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
double dot(const V3 &a, const V3 &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
double clampd(double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }
int    clampi(double v) { return (int)clampd(v, 0, 255); }
double lerp(double a, double b, double t) { return a + (b - a) * t; }
double rnd() { return rand() / (double)RAND_MAX; }
double rndr(double a, double b) { return a + (b - a) * rnd(); }

/* ------------------------------- themes -------------------------------- */
struct Theme {
    const char *name;
    int skyTop[3], skyBot[3], road[3], curbA[3], curbB[3], line[3], pillar[3], crate[3], sun[3];
    bool night;
};
Theme themes[5] = {
 { "The Cube",    {70,160,235}, {175,230,250}, {205,95,125},  {255,255,255}, {225,60,70},  {250,170,185}, {150,60,95},  {200,45,65},  {255,245,190}, false },
 { "Sunset Mesa", {235,130,110},{255,215,170}, {225,130,70},  {255,255,255}, {90,60,60},   {255,205,120}, {170,70,60},  {120,60,160}, {255,240,200}, false },
 { "Emerald Sky", {40,150,170}, {175,240,220}, {70,170,110},  {255,255,255}, {40,120,200}, {190,250,200}, {40,120,120}, {240,170,40}, {255,255,220}, false },
 { "Night Run",   {10,14,50},   {70,55,125},   {95,85,175},   {230,230,255}, {255,90,160}, {175,175,255}, {55,45,115},  {255,120,60}, {235,235,255}, true  },
 { "Ice World",   {110,170,230},{225,245,255}, {150,200,235}, {255,255,255}, {90,140,210}, {235,250,255}, {120,160,215},{230,80,80},  {255,255,255}, false }
};
const Theme *th = &themes[0];
int fogR = 175, fogG = 230, fogB = 250;
int starX[70], starY[70];

/* ------------------------- camera + 3D drawing -------------------------- */
double camX = 0, camY = 6, camZ = -10, focal = 520, shake = 0;
double rcx, rcy, rcz;                        // camera position used this frame
const double CAM_PITCH = 0.50;               // camera looks down (radians)
double cP, sP;

// 3D world point -> 2D screen point (false = behind the camera)
bool proj(const V3 &p, int &sx, int &sy)
{
    double dx = p.x - rcx, dy = p.y - rcy, dz = p.z - rcz;
    double y2 =  dy * cP + dz * sP;                    // rotate around X axis
    double z2 = -dy * sP + dz * cP;
    if (z2 < 0.6) return false;
    sx = SW / 2 + (int)(focal * dx / z2);              // perspective divide
    sy = SH / 2 - (int)(focal * y2 / z2);
    return true;
}

// extra rotation applied to everything drawn while xf.on (used for the truck/coins)
struct Xf { bool on; double px, py, pz, cy, sy, cr, sr, cp, sp; } xf;
void setXf(double x, double y, double z, double yaw, double roll, double pitch)
{
    xf.on = true; xf.px = x; xf.py = y; xf.pz = z;
    xf.cy = cos(yaw);   xf.sy = sin(yaw);
    xf.cr = cos(roll);  xf.sr = sin(roll);
    xf.cp = cos(pitch); xf.sp = sin(pitch);
}
void applyXf(V3 &p)
{
    if (!xf.on) return;
    double x = p.x - xf.px, y = p.y - xf.py, z = p.z - xf.pz;
    double t = x * xf.cy + z * xf.sy;  z = -x * xf.sy + z * xf.cy;  x = t;     // yaw   (turn)
    t = x * xf.cr - y * xf.sr;         y =  x * xf.sr + y * xf.cr;  x = t;     // roll  (lean)
    t = y * xf.cp + z * xf.sp;         z = -y * xf.sp + z * xf.cp;  y = t;     // pitch (nose up)
    p.x = x + xf.px; p.y = y + xf.py; p.z = z + xf.pz;
}

const V3 LIGHT = { -0.40, 0.80, -0.45 };                   // direction of the sun

// Draw one flat convex polygon (n <= 8) with lighting, fog and back-face culling.
// 'inside' is any point inside the solid, it tells us which way the face points.
void drawPoly(int n, const V3 *pts, const V3 &inside, int r, int g, int b)
{
    V3 p[10], in = inside;
    for (int i = 0; i < n; i++) { p[i] = pts[i]; applyXf(p[i]); }
    applyXf(in);
    V3 nrm = cross(sub(p[1], p[0]), sub(p[2], p[0]));
    double len = sqrt(dot(nrm, nrm));
    if (len < 1e-9) return;
    nrm.x /= len; nrm.y /= len; nrm.z /= len;
    V3 c = mkv(0, 0, 0);
    for (int i = 0; i < n; i++) { c.x += p[i].x; c.y += p[i].y; c.z += p[i].z; }
    c.x /= n; c.y /= n; c.z /= n;
    if (dot(nrm, sub(c, in)) < 0) { nrm.x = -nrm.x; nrm.y = -nrm.y; nrm.z = -nrm.z; }
    V3 toCam = sub(mkv(rcx, rcy, rcz), c);
    if (dot(nrm, toCam) <= 0) return;                       // back-face culling
    int poly[20], minx = 100000, maxx = -100000, miny = 100000, maxy = -100000;
    for (int i = 0; i < n; i++) {
        if (!proj(p[i], poly[2*i], poly[2*i+1])) return;
        if (poly[2*i]   < minx) minx = poly[2*i];    if (poly[2*i]   > maxx) maxx = poly[2*i];
        if (poly[2*i+1] < miny) miny = poly[2*i+1];  if (poly[2*i+1] > maxy) maxy = poly[2*i+1];
    }
    if (maxx < 0 || minx > SW || maxy < 0 || miny > SH) return;   // off screen
    double d  = dot(nrm, LIGHT);
    double sh = 0.45 + 0.55 * (d > 0 ? d : 0);                    // lighting
    double fg = clampd((sqrt(dot(toCam, toCam)) - 28) / 90, 0, 0.8);   // distance fog
    int R = clampi(lerp(r * sh, fogR, fg)), G = clampi(lerp(g * sh, fogG, fg)), B = clampi(lerp(b * sh, fogB, fg));
    setfillstyle(SOLID_FILL, COLOR(R, G, B));
    setcolor(COLOR(clampi(R * 0.6), clampi(G * 0.6), clampi(B * 0.6)));
    fillpoly(n, poly);
}

struct Box { double x, y, z, sx, sy, sz; int r, g, b; };
Box mk(double x, double y, double z, double sx, double sy, double sz, int r, int g, int b)
{ Box t = { x, y, z, sx, sy, sz, r, g, b }; return t; }

void drawBox(const Box &b)
{
    double hx = b.sx / 2, hy = b.sy / 2, hz = b.sz / 2;
    V3 c[8];
    for (int i = 0; i < 8; i++)
        c[i] = mkv(b.x + ((i & 1) ? hx : -hx), b.y + ((i & 2) ? hy : -hy), b.z + ((i & 4) ? hz : -hz));
    static const int F[6][4] = { {0,1,3,2}, {4,5,7,6}, {0,1,5,4}, {2,3,7,6}, {0,2,6,4}, {1,3,7,5} };
    V3 in = mkv(b.x, b.y, b.z), q[4];
    for (int f = 0; f < 6; f++) {
        for (int k = 0; k < 4; k++) q[k] = c[F[f][k]];
        drawPoly(4, q, in, b.r, b.g, b.b);
    }
}

// a slab whose top can be sloped (z0 end at height h0, z1 end at height h1)
void drawSlab(double xa, double xb, double za, double zb, double h0, double h1, double t, int r, int g, int b)
{
    V3 A = mkv(xa, h0, za), B = mkv(xb, h0, za), C = mkv(xb, h1, zb), D = mkv(xa, h1, zb);
    V3 A2 = mkv(xa, h0 - t, za), B2 = mkv(xb, h0 - t, za), C2 = mkv(xb, h1 - t, zb), D2 = mkv(xa, h1 - t, zb);
    V3 in = mkv((xa + xb) / 2, (h0 + h1) / 2 - t / 2, (za + zb) / 2), q[4];
    q[0] = A;  q[1] = B;  q[2] = C;  q[3] = D;   drawPoly(4, q, in, r, g, b);     // top
    q[0] = A2; q[1] = B2; q[2] = B;  q[3] = A;   drawPoly(4, q, in, r, g, b);     // near end
    q[0] = A2; q[1] = A;  q[2] = D;  q[3] = D2;  drawPoly(4, q, in, r, g, b);     // left side
    q[0] = B2; q[1] = B;  q[2] = C;  q[3] = C2;  drawPoly(4, q, in, r, g, b);     // right side
    q[0] = D2; q[1] = C2; q[2] = C;  q[3] = D;   drawPoly(4, q, in, r, g, b);     // far end
}

// 8-sided prism (wheel when axis = 0, coin when axis = 2)
V3 ringPt(int axis, double cx, double cy, double cz, double r, double a, double off)
{
    if (axis == 0) return mkv(cx + off, cy + r * sin(a), cz + r * cos(a));
    return mkv(cx + r * cos(a), cy + r * sin(a), cz + off);
}
void drawPrism(int axis, double cx, double cy, double cz, double rad, double thick, double ang,
               const int *s0, const int *s1, const int *cap, const int *hub, double hubScale)
{
    const int N = 8;
    V3 ring[2][N], hr[2][N], q[4], ctr = mkv(cx, cy, cz);
    double h = thick / 2;
    for (int k = 0; k < N; k++) {
        double a = ang + 2 * PI * k / N;
        ring[0][k] = ringPt(axis, cx, cy, cz, rad, a, -h);
        ring[1][k] = ringPt(axis, cx, cy, cz, rad, a,  h);
        hr[0][k]   = ringPt(axis, cx, cy, cz, rad * hubScale, a, -h - 0.015);
        hr[1][k]   = ringPt(axis, cx, cy, cz, rad * hubScale, a,  h + 0.015);
    }
    for (int k = 0; k < N; k++) {                              // the tread (side faces)
        int k2 = (k + 1) % N;
        q[0] = ring[0][k]; q[1] = ring[0][k2]; q[2] = ring[1][k2]; q[3] = ring[1][k];
        const int *c = (k & 1) ? s1 : s0;
        drawPoly(4, q, ctr, c[0], c[1], c[2]);
    }
    for (int e = 0; e < 2; e++) {                              // the two round faces
        drawPoly(N, ring[e], ctr, cap[0], cap[1], cap[2]);
        if (hubScale > 0) drawPoly(N, hr[e], ctr, hub[0], hub[1], hub[2]);
    }
}

/* ----------------------------- world data ------------------------------- */
struct Seg   { bool solid; double h0, h1; };
struct Crate { double x, z; };
struct Coin  { double x, y, z; bool got; };
struct Pad   { double x, z; };
struct Flag  { double z; bool got; };
struct Part  { double x, y, z, vx, vy, vz, life, maxlife, size; int r, g, b; bool grav; };

std::vector<Seg>   segs;
std::vector<Crate> crates;
std::vector<Coin>  coins;
std::vector<Pad>   pads;
std::vector<Flag>  flags;
std::vector<Part>  parts;

enum { ST_TITLE, ST_PLAY, ST_PAUSE, ST_CRASH, ST_OVER, ST_WIN };
int    state = ST_TITLE, level = 1, nSeg = 40, lives = 3, score = 0, coinsGot = 0, stars = 0, frameNo = 0;
double px, py, pz, vy, speed, maxSpeed, boost, boostCool, invuln, levelTime, airTime, coyote;
double lastCp, wheelAng, pitchVis, rollVis, popupT, titleT, steerNow;
bool   onGround, truckVisible, braking;
char   msg[64], popupMsg[48];

/* ------------------------------- sound ---------------------------------- */
volatile int sfxF[16], sfxD[16];
volatile int sfxHead = 0, sfxTail = 0;
bool soundOn = true;
DWORD WINAPI sfxThread(LPVOID)                 // beeps in a helper thread so the game never freezes
{
    for (;;) {
        if (sfxHead != sfxTail) {
            int f = sfxF[sfxTail], d = sfxD[sfxTail];
            sfxTail = (sfxTail + 1) % 16;
            if (soundOn) Beep(f, d);
        } else Sleep(5);
    }
    return 0;
}
void sfx(int f, int d)
{
    int n = (sfxHead + 1) % 16;
    if (n == sfxTail) return;
    sfxF[sfxHead] = f; sfxD[sfxHead] = d; sfxHead = n;
}

/* ------------------------------- keyboard ------------------------------- */
bool kDown[256], kHit[256];
void pollKeys()
{
    static const int KEYS[] = { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_SPACE, VK_RETURN, VK_ESCAPE,
                                'W', 'A', 'S', 'D', 'P', 'M' };
    for (unsigned i = 0; i < sizeof(KEYS) / sizeof(KEYS[0]); i++) {
        int k = KEYS[i];
        bool d = (GetAsyncKeyState(k) & 0x8000) != 0;
        kHit[k] = d && !kDown[k];                      // true only on the frame the key goes down
        kDown[k] = d;
    }
}

/* ------------------------------ level setup ----------------------------- */
double groundAt(double x, double z, double *slope = NULL)
{
    int i = (int)floor(z / SEG_LEN);
    if (slope) *slope = 0;
    if (i < 0 || i >= (int)segs.size() || !segs[i].solid || fabs(x) > ROAD_HALF + 0.2) return NOGROUND;
    if (slope) *slope = (segs[i].h1 - segs[i].h0) / SEG_LEN;
    return segs[i].h0 + (segs[i].h1 - segs[i].h0) * (z - i * SEG_LEN) / SEG_LEN;
}

void addCoin(double x, double y, double z) { Coin c = { x, y, z, false }; coins.push_back(c); }
void addCrate(double x, double z)          { Crate c = { x, z };          crates.push_back(c); }
void addPad(double x, double z)            { Pad p = { x, z };            pads.push_back(p); }

void setTheme(int idx)
{
    th = &themes[idx];
    fogR = th->skyBot[0]; fogG = th->skyBot[1]; fogB = th->skyBot[2];
    for (int i = 0; i < 70; i++) { starX[i] = rand() % SW; starY[i] = rand() % (SH - 120); }
}

void initLevel(int lv)
{
    level = lv;
    setTheme((lv - 1) % 5);
    nSeg = 36 + 8 * lv; if (nSeg > 100) nSeg = 100;
    srand(lv * 131 + 7);                                   // same level = same map
    Seg s0 = { true, 0, 0 };
    segs.assign(nSeg + 8, s0);
    crates.clear(); coins.clear(); pads.clear(); flags.clear(); parts.clear();
    Flag f0 = { 0, true }; flags.push_back(f0);
    int i = 6, lastFlag = 0;
    while (i < nSeg - 4) {
        int r = rand() % 100;
        if (i - lastFlag >= 18 && i < nSeg - 10) {                  // checkpoint flag
            Flag f = { i * SEG_LEN, false }; flags.push_back(f);
            lastFlag = i; i += 2; continue;
        }
        if (r < 14 + lv * 2 && i + 8 < nSeg) {                      // RAMP + big gap
            int gap = (lv <= 2) ? 2 : 2 + rand() % 2, rs = i + 1;
            addPad(0, i * SEG_LEN + SEG_LEN / 2);
            segs[rs].h0 = 0; segs[rs].h1 = RAMP_H;
            for (int k = 0; k < gap; k++) segs[rs + 1 + k].solid = false;
            for (int c = 0; c < 5; c++) {
                double t = (c + 0.5) / 5;
                addCoin(0, RAMP_H * (1 - t) + 0.8 + 1.8 * sin(PI * t), (rs + 1) * SEG_LEN + t * gap * SEG_LEN);
            }
            i = rs + 1 + gap + 3; continue;
        }
        if (r < 26 + lv * 2) {                                      // small gap (jump with SPACE)
            int gap = (lv <= 1) ? 1 : 1 + rand() % 2;
            for (int k = 0; k < gap; k++) segs[i + k].solid = false;
            for (int c = 0; c < 3; c++) {
                double t = (c + 0.5) / 3;
                addCoin(0, 0.8 + 1.5 * sin(PI * t), i * SEG_LEN - 2 + t * (gap * SEG_LEN + 4));
            }
            i += gap + 3; continue;
        }
        if (r < 36 + lv * 2) {                                      // hill
            segs[i].h0 = 0; segs[i].h1 = 1.3; segs[i + 1].h0 = 1.3; segs[i + 1].h1 = 0;
            for (int c = 0; c < 3; c++) addCoin(0, 1.3 + 0.9, (i + 1) * SEG_LEN + (c - 1) * 1.6);
            i += 4; continue;
        }
        int rr = rand() % 100;                                      // normal piece
        double zc = i * SEG_LEN + SEG_LEN / 2;
        if (rr < 34 + lv * 3) {
            int pat = rand() % 4, freeLane = rand() % 3;
            for (int ln = 0; ln < 3; ln++) {
                bool full = (pat == 0 && lv >= 2);                   // full row = must jump
                if (full || (pat < 3 && ln != freeLane && (pat != 1 || ln == (freeLane + 1) % 3)))
                    addCrate((ln - 1) * LANE_W, zc);
            }
        } else if (rr < 72) {
            double lx = (rand() % 3 - 1) * LANE_W;
            for (int c = 0; c < 4; c++) addCoin(lx, 0.8, i * SEG_LEN + 0.5 + c * 1.0);
        } else if (rr < 80) addPad((rand() % 3 - 1) * LANE_W, zc);
        i++;
    }
    px = 0; pz = 3; py = 0; vy = 0; speed = 6; boost = 0; boostCool = 0; invuln = 0;
    maxSpeed = 13 + lv * 1.5; if (maxSpeed > 24) maxSpeed = 24;
    onGround = true; truckVisible = true; coyote = 0; airTime = 0;
    lastCp = 0; wheelAng = 0; pitchVis = 0; rollVis = 0; levelTime = 0; coinsGot = 0; shake = 0;
    camX = 0; camY = 5.8; camZ = pz - 10; focal = 520;
    srand((unsigned)GetTickCount());
    state = ST_PLAY;
    strcpy(popupMsg, "GO!"); popupT = 1.2;
}

void startGame() { score = 0; lives = 3; initLevel(1); }

/* ----------------------------- particles -------------------------------- */
void spawn(double x, double y, double z, double vx, double vy_, double vz, double life, double size,
           int r, int g, int b, bool grav)
{
    if (parts.size() > 400) return;
    Part p = { x, y, z, vx, vy_, vz, life, life, size, r, g, b, grav };
    parts.push_back(p);
}
void burst(double x, double y, double z, int n, int r, int g, int b, double power)
{
    for (int i = 0; i < n; i++)
        spawn(x, y, z, rndr(-1, 1) * power, rndr(0.2, 1.2) * power, rndr(-1, 1) * power,
              rndr(0.5, 1.2), rndr(0.12, 0.3), r, g, b, true);
}
void updateParts(double dt)
{
    for (int i = (int)parts.size() - 1; i >= 0; i--) {
        Part &p = parts[i];
        p.life -= dt;
        if (p.grav) p.vy -= 18 * dt;
        p.x += p.vx * dt; p.y += p.vy * dt; p.z += p.vz * dt;
        if (p.life <= 0) { parts[i] = parts[parts.size() - 1]; parts.pop_back(); }
    }
}

/* ------------------------------ game logic ------------------------------ */
void popup(const char *s) { strncpy(popupMsg, s, 47); popupMsg[47] = 0; popupT = 1.4; }

void loseLife(const char *why, bool explode)
{
    lives--; strcpy(msg, why); speed = 0; boost = 0; shake = 1.0;
    sfx(170, 280);
    if (explode) {
        truckVisible = false;
        for (int i = 0; i < 36; i++) {
            int t = rand() % 3;
            spawn(px, py + 1, pz, rndr(-4, 4), rndr(2, 9), rndr(-4, 4), rndr(0.8, 1.8), rndr(0.2, 0.5),
                  t == 0 ? 245 : (t == 1 ? 240 : 40), t == 0 ? 190 : (t == 1 ? 110 : 40), t == 0 ? 40 : (t == 1 ? 30 : 45), true);
        }
    }
    state = (lives > 0) ? ST_CRASH : ST_OVER;
}

void respawn()
{
    px = 0; pz = lastCp + 2; py = groundAt(0, pz); if (py < NOGROUND / 2) py = 0;
    vy = 0; speed = 7; boost = 0; onGround = true; truckVisible = true;
    invuln = 2.0; pitchVis = 0; rollVis = 0; parts.clear();
    camX = 0; camZ = pz - 10; camY = 5.8;
    state = ST_PLAY; popup("GO!");
}

void updatePlay(double dt)
{
    levelTime += dt;
    bool gas   = kDown[VK_UP]   || kDown['W'];
    bool brk   = kDown[VK_DOWN] || kDown['S'];
    bool left  = kDown[VK_LEFT] || kDown['A'];
    bool right = kDown[VK_RIGHT]|| kDown['D'];
    braking = brk;

    // ---- speed ----
    if (gas) speed += 14 * dt; else speed -= 3.5 * dt;
    if (brk) speed -= 28 * dt;
    if (boost > 0) { speed += 22 * dt; boost -= dt; }
    double top = maxSpeed * (boost > 0 ? 1.4 : 1.0);
    if (speed > top) speed -= 8 * dt;
    if (speed > top + 2) speed = top + 2;
    if (speed < 0) speed = 0;
    if (boostCool > 0) boostCool -= dt;
    if (invuln > 0) invuln -= dt;

    // ---- steering (3D lean) ----
    steerNow = (right ? 1 : 0) - (left ? 1 : 0);
    px += steerNow * 6.2 * dt * (0.45 + 0.55 * speed / maxSpeed);
    px = clampd(px, -ROAD_HALF + 1.25, ROAD_HALF - 1.25);
    rollVis += (-steerNow * 0.14 - rollVis) * clampd(8 * dt, 0, 1);

    // ---- jump (with a little 'coyote time' so late presses still work) ----
    coyote = onGround ? 0.12 : coyote - dt;
    if (kDown[VK_SPACE] && coyote > 0) { vy = 9.5; onGround = false; coyote = 0; sfx(520, 40); }

    pz += speed * dt;
    wheelAng += speed * dt / 0.55;

    // ---- gravity + ground (ramps, hills, gaps) ----
    double sl, g = groundAt(px, pz, &sl);
    if (onGround) {
        if (g > NOGROUND / 2 && fabs(py - g) < 0.4) {
            double vg = sl * speed;
            if (vg < vy - 4.0 && speed > 8) onGround = false;       // drove over a crest -> take off
            else { py = g; vy = vg; }
        } else onGround = false;                                    // road ended -> fly / fall
    }
    if (!onGround) {
        airTime += dt;
        vy -= GRAV * dt; py += vy * dt;
        g = groundAt(px, pz, &sl);
        if (g > NOGROUND / 2 && py <= g && vy <= 0 && py > g - 1.0) {    // landing
            if (vy < -7) { shake = 0.5; burst(px, g + 0.2, pz, 10, 235, 225, 225, 2.5); sfx(220, 60); }
            if (airTime > 0.7) { score += 5; popup("NICE JUMP!  +5"); }
            py = g; vy = sl * speed; onGround = true; airTime = 0;
        }
    }
    double target = onGround ? atan(sl) : atan2(vy, speed + 2) * 0.8;
    pitchVis += (clampd(target, -0.6, 0.6) - pitchVis) * clampd(6 * dt, 0, 1);

    // ---- dust behind the wheels, flames when boosting ----
    if (onGround && speed > 3 && (frameNo & 1) == 0)
        for (int s = -1; s <= 1; s += 2)
            spawn(px + s * 0.95, py + 0.2, pz - 1.0, rndr(-0.6, 0.6), rndr(0.3, 1.3), rndr(-1.5, 0),
                  0.55, rndr(0.18, 0.34), 236, 226, 226, false);
    if (boost > 0)
        for (int s = -1; s <= 1; s += 2)
            spawn(px + s * 0.5, py + 1.7, pz - 1.2, rndr(-0.4, 0.4), rndr(0.5, 2), -speed * 0.3 - rndr(0, 3),
                  0.35, rndr(0.15, 0.3), 255, rand() % 2 ? 190 : 110, 30, false);

    // ---- falling off the world ----
    if (py < -9) { loseLife("You fell off the road!", false); return; }

    // ---- crates ----
    if (invuln <= 0)
        for (size_t j = 0; j < crates.size(); j++)
            if (fabs(px - crates[j].x) < 1.0 + 0.6 && fabs(pz - crates[j].z) < 1.25 + 0.6 && py < 0.95) {
                loseLife("You hit a crate!", true); return;
            }
    // ---- coins ----
    for (size_t j = 0; j < coins.size(); j++)
        if (!coins[j].got && fabs(px - coins[j].x) < 1.3 && fabs(pz - coins[j].z) < 1.3 && fabs(py + 0.9 - coins[j].y) < 1.4) {
            coins[j].got = true; coinsGot++; score += 10; sfx(1250, 35);
            burst(coins[j].x, coins[j].y, coins[j].z, 7, 255, 215, 60, 2.2);
        }
    // ---- boost pads ----
    for (size_t j = 0; j < pads.size(); j++)
        if (boostCool <= 0 && fabs(px - pads[j].x) < 1.2 && fabs(pz - pads[j].z) < 1.3 && onGround) {
            boost = 1.6; boostCool = 2.0; popup("BOOST!"); sfx(900, 90);
        }
    // ---- checkpoints ----
    for (size_t j = 0; j < flags.size(); j++)
        if (!flags[j].got && pz >= flags[j].z) {
            flags[j].got = true; lastCp = flags[j].z; popup("CHECKPOINT!"); sfx(800, 70);
        }
    // ---- finish ----
    if (pz >= nSeg * SEG_LEN + 2) {
        state = ST_WIN; speed = 0;
        double ratio = coins.empty() ? 1.0 : (double)coinsGot / coins.size();
        stars = (ratio >= 0.85) ? 3 : (ratio >= 0.5 ? 2 : 1);
        score += 100 + 25 * lives;
        sfx(1000, 150); sfx(1300, 250);
    }
}

void updateCamera(double dt)
{
    camX += (px * 0.55 - camX) * clampd(6 * dt, 0, 1);
    camZ  = pz - 10 - speed * 0.06;
    double ty = 5.8 + (py > 0 ? py : 0) * 0.8;
    camY += (ty - camY) * clampd(5 * dt, 0, 1);
    double ft = 520 - speed * 4 - (boost > 0 ? 40 : 0);
    focal += (ft - focal) * clampd(4 * dt, 0, 1);
}

void update(double dt)
{
    if (kHit['M']) { soundOn = !soundOn; popup(soundOn ? "SOUND ON" : "SOUND OFF"); }
    frameNo++;
    if (popupT > 0) popupT -= dt;
    if (shake > 0) shake = clampd(shake - 1.6 * dt, 0, 5);
    switch (state) {
    case ST_TITLE:
        titleT += dt;
        if (kHit[VK_RETURN]) startGame();
        break;
    case ST_PLAY:
        if (kHit['P']) { state = ST_PAUSE; break; }
        updatePlay(dt); updateCamera(dt); updateParts(dt);
        break;
    case ST_PAUSE:
        if (kHit['P'] || kHit[VK_RETURN]) state = ST_PLAY;
        break;
    case ST_CRASH: case ST_OVER:
        if (truckVisible && py > -40) { vy -= GRAV * dt; py += vy * dt; pitchVis += dt * 2.5; }
        updateParts(dt);
        if (kHit[VK_RETURN]) { if (state == ST_CRASH) respawn(); else { int lv = level; lives = 3; initLevel(lv); } }
        break;
    case ST_WIN:
        updateParts(dt);
        if (kHit[VK_RETURN]) { int lv = level + 1; initLevel(lv); }
        break;
    }
}

/* ------------------------------- drawing -------------------------------- */
void drawSky()
{
    for (int y = 0; y < SH; y += 10) {
        double t = (double)y / SH;
        setfillstyle(SOLID_FILL, COLOR(clampi(lerp(th->skyTop[0], th->skyBot[0], t)),
                                       clampi(lerp(th->skyTop[1], th->skyBot[1], t)),
                                       clampi(lerp(th->skyTop[2], th->skyBot[2], t))));
        bar(0, y, SW, y + 10);
    }
    if (th->night)
        for (int i = 0; i < 70; i++)
            if (((frameNo / 8) + i) % 9 != 0) {
                setfillstyle(SOLID_FILL, COLOR(255, 255, 230));
                bar(starX[i], starY[i], starX[i] + 2, starY[i] + 2);
            }
    setfillstyle(SOLID_FILL, COLOR(clampi(lerp(th->skyTop[0], th->sun[0], 0.35)),
                                   clampi(lerp(th->skyTop[1], th->sun[1], 0.35)),
                                   clampi(lerp(th->skyTop[2], th->sun[2], 0.35))));
    fillellipse(SW - 150, 95, 64, 64);                                 // glow
    setfillstyle(SOLID_FILL, COLOR(th->sun[0], th->sun[1], th->sun[2]));
    fillellipse(SW - 150, 95, 40, 40);                                 // sun / moon
}

void drawCloud(double x, double y, double z)
{
    drawBox(mk(x, y, z, 5.0, 1.2, 3.0, 255, 255, 255));
    drawBox(mk(x + 1, y + 0.9, z, 3.0, 1.0, 2.4, 255, 255, 255));
    drawBox(mk(x - 1.4, y + 0.7, z + 0.3, 2.4, 0.8, 2.0, 250, 250, 255));
}

void drawDecor()
{
    int k0 = (int)floor(camZ / 30.0);
    for (int k = k0 + 7; k >= k0 - 1; k--) {
        unsigned int h = (unsigned int)(k + 1000) * 2654435761u; h >>= 20;
        int a = h & 7, b = (h >> 3) & 7;
        double z = k * 30.0;
        drawCloud(-12 - a * 2, -9 - b * 0.4, z + 7);
        drawCloud( 14 + b * 2, -12, z + 21);
        drawBox(mk(-17 - a * 1.5, -11, z, 3.2, 16, 3.2, th->pillar[0], th->pillar[1], th->pillar[2]));
        drawBox(mk(-17 - a * 1.5, -2.95, z, 1.6, 0.1, 1.6, th->pillar[0] + 40, th->pillar[1] + 40, th->pillar[2] + 40));
        drawBox(mk( 18 + b * 1.5, -11, z + 14, 3.2, 14 + b, 3.2, th->pillar[0] + 12, th->pillar[1] + 8, th->pillar[2] + 8));
    }
}

void drawPad(double x, double z)
{
    double g0 = groundAt(x, z - 1.2), g1 = groundAt(x, z + 1.2);
    if (g0 < NOGROUND / 2) g0 = 0;
    if (g1 < NOGROUND / 2) g1 = 0;
    drawSlab(x - 1.0, x + 1.0, z - 1.2, z + 1.2, g0 + 0.04, g1 + 0.04, 0.03, 40, 200, 255);
    for (int c = 0; c < 2; c++) {                                     // two yellow arrows
        double zb = z - 0.9 + c * 0.9;
        double y = (g0 + g1) / 2 + 0.07;
        V3 t[3] = { mkv(x - 0.7, y, zb), mkv(x + 0.7, y, zb), mkv(x, y, zb + 0.75) };
        drawPoly(3, t, mkv(x, y - 1, zb), 255, 230, 60);
    }
}

void drawFlag(const Flag &f)
{
    int r = f.got ? 60 : 225, g = f.got ? 210 : 70, b = f.got ? 90 : 60;
    for (int s = -1; s <= 1; s += 2) {
        drawBox(mk(s * (ROAD_HALF + 0.9), 1.6, f.z + 0.5, 0.2, 3.2, 0.2, 240, 240, 245));
        drawBox(mk(s * (ROAD_HALF + 0.9) - s * 0.5, 2.8, f.z + 0.5, 0.9, 0.55, 0.06, r, g, b));
    }
    drawBox(mk(0, 3.3, f.z + 0.5, 2 * (ROAD_HALF + 0.9), 0.35, 0.25, r, g, b));
}

void drawGate()
{
    double z = nSeg * SEG_LEN + 1.0;
    for (int s = -1; s <= 1; s += 2) drawBox(mk(s * (ROAD_HALF + 0.5), 2.2, z, 0.5, 4.4, 0.5, 245, 245, 245));
    for (int c = 0; c < 10; c++) {
        int w = (c & 1) ? 25 : 245;
        drawBox(mk(-ROAD_HALF - 0.5 + c * (2 * ROAD_HALF + 1) / 10 + 0.36, 4.6, z, 0.72, 0.55, 0.5, w, w, w));
    }
}

void drawCrate(const Crate &c)
{
    drawBox(mk(c.x, 0.55, c.z, 1.2, 1.1, 1.2, th->crate[0], th->crate[1], th->crate[2]));
    drawBox(mk(c.x, 0.55, c.z - 0.61, 0.85, 0.85, 0.03, clampi(th->crate[0] + 45), clampi(th->crate[1] + 45), clampi(th->crate[2] + 45)));
    drawBox(mk(c.x, 1.12, c.z, 0.8, 0.05, 0.8, clampi(th->crate[0] + 70), clampi(th->crate[1] + 70), clampi(th->crate[2] + 70)));
}

void drawCoin(const Coin &c)
{
    static const int side0[3] = { 215, 150, 20 }, side1[3] = { 240, 180, 30 };
    static const int cap[3] = { 255, 205, 50 }, hub[3] = { 255, 240, 140 };
    setXf(c.x, c.y, c.z, frameNo * 0.12, 0, 0);
    drawPrism(2, c.x, c.y, c.z, 0.45, 0.14, PI / 8, side0, side1, cap, hub, 0.6);
    xf.on = false;
}

void drawSegment(int i)
{
    if (i < 0 || i >= (int)segs.size() || !segs[i].solid) return;
    const Seg &s = segs[i];
    double z0 = i * SEG_LEN, z1 = z0 + SEG_LEN;
    bool fin = (i >= nSeg);
    int rr = fin ? 70 : th->road[0], gg = fin ? 175 : th->road[1], bb = fin ? 85 : th->road[2];
    drawSlab(-ROAD_HALF, ROAD_HALF, z0, z1, s.h0, s.h1, 0.7, rr, gg, bb);
    for (int ln = 0; ln < 3; ln++)                                      // checkerboard lanes
        if ((i + ln) & 1)
            drawSlab(-ROAD_HALF + ln * LANE_W, -ROAD_HALF + (ln + 1) * LANE_W, z0, z1,
                     s.h0 + 0.015, s.h1 + 0.015, 0.02, rr - 18, gg - 14, bb - 14);
    const int *ca = (i & 1) ? th->curbA : th->curbB;                    // red/white curbs
    for (int sd = -1; sd <= 1; sd += 2) {
        double xa = (sd < 0) ? -ROAD_HALF - 0.4 : ROAD_HALF, xb = xa + 0.4;
        drawSlab(xa, xb, z0, z1, s.h0 + 0.3, s.h1 + 0.3, 0.3, ca[0], ca[1], ca[2]);
    }
    if (!fin)
        for (int sd = -1; sd <= 1; sd += 2) {                           // dashed lane lines
            double xa = sd * LANE_W / 2 - 0.07;
            double za = z0 + 1.0, zb = z0 + 3.0;
            drawSlab(xa, xa + 0.14, za, zb, s.h0 + (s.h1 - s.h0) * 0.25 + 0.03, s.h0 + (s.h1 - s.h0) * 0.75 + 0.03,
                     0.02, th->line[0], th->line[1], th->line[2]);
        }
    if (i == nSeg)                                                      // checkered finish line
        for (int row = 1; row >= 0; row--)
            for (int c = 0; c < 6; c++) {
                int w = ((c + row) & 1) ? 25 : 245;
                drawSlab(-ROAD_HALF + c * LANE_W / 2.0 * 1.0, -ROAD_HALF + (c + 1) * LANE_W / 2.0 * 1.0,
                         z0 + 0.4 + row * 0.9, z0 + 1.3 + row * 0.9, 0.06, 0.06, 0.03, w, w, w);
            }
}

void drawSegObjects(int i)
{
    double z0 = i * SEG_LEN, z1 = z0 + SEG_LEN;
    for (size_t j = 0; j < pads.size(); j++)  if (pads[j].z >= z0 && pads[j].z < z1) drawPad(pads[j].x, pads[j].z);
    for (size_t j = 0; j < flags.size(); j++) if (flags[j].z >= z0 && flags[j].z < z1 && j > 0) drawFlag(flags[j]);
    if (i == nSeg) drawGate();
    for (size_t j = 0; j < crates.size(); j++) if (crates[j].z >= z0 && crates[j].z < z1) drawCrate(crates[j]);
    for (size_t j = 0; j < coins.size(); j++)  if (!coins[j].got && coins[j].z >= z0 && coins[j].z < z1) drawCoin(coins[j]);
}

double dist2(double x, double y, double z) { return (x-rcx)*(x-rcx) + (y-rcy)*(y-rcy) + (z-rcz)*(z-rcz); }

void tb(double x, double y, double z, double dx, double dy, double dz, double sx, double sy, double sz, int r, int g, int b)
{ drawBox(mk(x + dx, y + dy, z + dz, sx, sy, sz, r, g, b)); }

void drawTruck(double x, double y, double z, double pitch, double roll, double yaw, bool brake)
{
    static const double WX[4] = { -0.95, 0.95, -0.95, 0.95 };
    static const double WZ[4] = {  0.95, 0.95, -0.95, -0.95 };
    static const int tread0[3] = { 26, 26, 32 }, tread1[3] = { 52, 52, 60 };
    static const int cap[3] = { 36, 36, 42 }, hub[3] = { 165, 165, 175 };
    setXf(x, y + 0.7, z, yaw, roll, pitch);
    double d[5]; int ord[5], i, j;
    double cy = cos(yaw), sy = sin(yaw);
    for (i = 0; i < 4; i++) {
        double wx = WX[i] * cy + WZ[i] * sy, wz = -WX[i] * sy + WZ[i] * cy;
        d[i] = dist2(x + wx, y + 0.5, z + wz);
    }
    d[4] = dist2(x, y + 1.0, z);
    for (i = 0; i < 5; i++) ord[i] = i;
    for (i = 1; i < 5; i++) {                                    // far -> near
        int t = ord[i]; j = i - 1;
        while (j >= 0 && d[ord[j]] < d[t]) { ord[j + 1] = ord[j]; j--; }
        ord[j + 1] = t;
    }
    for (int n = 0; n < 5; n++) {
        int id = ord[n];
        if (id < 4) {
            drawPrism(0, x + WX[id], y + 0.55, z + WZ[id], 0.55, 0.5, wheelAng + PI / 8, tread0, tread1, cap, hub, 0.5);
        } else {
            tb(x, y, z,  0, 0.75,  0.00, 1.20, 0.25, 2.50,  60,  60,  72);     // chassis
            tb(x, y, z,  0, 1.10,  0.00, 1.50, 0.50, 2.40, 245, 190,  40);     // yellow body
            tb(x, y, z,  0, 1.45,  0.75, 1.40, 0.20, 0.90, 250, 205,  70);     // hood
            tb(x, y, z,  0, 1.65, -0.30, 1.20, 0.55, 1.10, 178, 184, 198);     // grey cab
            tb(x, y, z,  0, 1.68, -0.86, 0.95, 0.30, 0.03,  35,  40,  58);     // rear window
            tb(x, y, z, -0.61, 1.68, -0.30, 0.03, 0.30, 0.80, 35, 40, 58);      // side windows
            tb(x, y, z,  0.61, 1.68, -0.30, 0.03, 0.30, 0.80, 35, 40, 58);
            tb(x, y, z, -0.40, 2.00, -0.30, 0.30, 0.10, 0.12, 255, 250, 200);   // roof lights
            tb(x, y, z,  0.40, 2.00, -0.30, 0.30, 0.10, 0.12, 255, 250, 200);
            tb(x, y, z, -0.77, 1.12,  0.15, 0.03, 0.16, 1.40, 240, 110, 30);    // orange stripes
            tb(x, y, z,  0.77, 1.12,  0.15, 0.03, 0.16, 1.40, 240, 110, 30);
            tb(x, y, z,  0.00, 0.90, -1.28, 1.40, 0.22, 0.10,  50,  50,  58);   // rear bumper
            tb(x, y, z, -0.52, 1.70, -1.00, 0.12, 0.70, 0.12, 170, 170, 182);   // exhaust pipes
            tb(x, y, z,  0.52, 1.70, -1.00, 0.12, 0.70, 0.12, 170, 170, 182);
            int tl = brake ? 255 : 170;
            tb(x, y, z, -0.55, 1.12, -1.23, 0.30, 0.16, 0.05, tl, brake ? 70 : 25, brake ? 70 : 25);   // tail lights
            tb(x, y, z,  0.55, 1.12, -1.23, 0.30, 0.16, 0.05, tl, brake ? 70 : 25, brake ? 70 : 25);
        }
    }
    xf.on = false;
}

void drawShadow()
{
    double g = groundAt(px, pz);
    if (g < NOGROUND / 2) return;
    double k = 1.0 - clampd((py - g) / 6.0, 0, 0.7);
    double g0 = groundAt(px, pz - 1.3 * k), g1 = groundAt(px, pz + 1.3 * k);
    if (g0 < NOGROUND / 2) g0 = g;
    if (g1 < NOGROUND / 2) g1 = g;
    drawSlab(px - 1.15 * k, px + 1.15 * k, pz - 1.3 * k, pz + 1.3 * k, g0 + 0.05, g1 + 0.05, 0.02,
             th->road[0] / 3, th->road[1] / 3, th->road[2] / 3);
}

void drawParts()
{
    for (size_t i = 0; i < parts.size(); i++) {
        const Part &p = parts[i];
        double s = p.size * (0.3 + 0.7 * p.life / p.maxlife);
        drawBox(mk(p.x, p.y, p.z, s, s, s, p.r, p.g, p.b));
    }
}

/* ------------------------------- HUD / UI ------------------------------- */
const int PR = 28, PG = 36, PB = 58;                     // panel colour
void panel(int x1, int y1, int x2, int y2)
{
    setfillstyle(SOLID_FILL, COLOR(PR, PG, PB)); bar(x1, y1, x2, y2);
    setcolor(COLOR(235, 240, 255)); rectangle(x1, y1, x2, y2);
}
void label(int x, int y, const char *s, int size, int r, int g, int b)
{
    char buf[128]; strncpy(buf, s, 127); buf[127] = 0;
    settextstyle(DEFAULT_FONT, HORIZ_DIR, size);
    setbkcolor(COLOR(PR, PG, PB)); setcolor(COLOR(r, g, b));
    outtextxy(x, y, buf);
}
void labelC(int cx, int y, const char *s, int size, int r, int g, int b)
{
    char buf[128]; strncpy(buf, s, 127); buf[127] = 0;
    settextstyle(DEFAULT_FONT, HORIZ_DIR, size);
    label(cx - textwidth(buf) / 2, y, s, size, r, g, b);
}
void drawHeart(int x, int y, bool on)
{
    int r = on ? 235 : 70, g = on ? 50 : 70, b = on ? 70 : 85;
    setfillstyle(SOLID_FILL, COLOR(r, g, b)); setcolor(COLOR(r, g, b));
    fillellipse(x - 5, y, 5, 5); fillellipse(x + 5, y, 5, 5);
    int tri[6] = { x - 10, y + 2, x + 10, y + 2, x, y + 14 };
    fillpoly(3, tri);
}
void drawStar(int cx, int cy, int r, bool on)
{
    int pt[20];
    for (int i = 0; i < 10; i++) {
        double a = -PI / 2 + i * PI / 5, rr = (i & 1) ? r * 0.45 : r;
        pt[2*i] = cx + (int)(rr * cos(a)); pt[2*i+1] = cy + (int)(rr * sin(a));
    }
    setfillstyle(SOLID_FILL, on ? COLOR(255, 205, 50) : COLOR(80, 85, 105));
    setcolor(on ? COLOR(255, 240, 150) : COLOR(120, 125, 145));
    fillpoly(10, pt);
}

void drawHUD()
{
    char buf[96];
    sprintf(buf, "Level %d: %s", level, th->name);
    panel(SW / 2 - 175, 8, SW / 2 + 175, 42);  labelC(SW / 2, 18, buf, 2, 255, 255, 255);

    panel(10, 8, 215, 74);                                           // coins + lives
    setfillstyle(SOLID_FILL, COLOR(255, 205, 50)); setcolor(COLOR(255, 240, 150)); fillellipse(30, 26, 10, 10);
    sprintf(buf, "x %d / %d", coinsGot, (int)coins.size()); label(48, 19, buf, 2, 255, 225, 90);
    for (int i = 0; i < 3; i++) drawHeart(32 + i * 28, 55, i < lives);

    int secs = (int)levelTime;                                       // time + score
    panel(SW - 215, 8, SW - 10, 74);
    sprintf(buf, "TIME %02d:%02d", secs / 60, secs % 60); label(SW - 205, 17, buf, 2, 255, 255, 255);
    sprintf(buf, "SCORE %d", score);                      label(SW - 205, 44, buf, 2, 120, 230, 140);

    panel(SW - 175, SH - 112, SW - 10, SH - 44);                     // speedometer
    sprintf(buf, "%d", (int)(speed * 8));  labelC(SW - 118, SH - 104, buf, 4, 255, 255, 255);
    label(SW - 70, SH - 90, "km/h", 1, 190, 200, 230);
    double sp = clampd(speed / (maxSpeed * 1.4), 0, 1);
    setfillstyle(SOLID_FILL, COLOR(70, 80, 110)); bar(SW - 165, SH - 62, SW - 20, SH - 52);
    setfillstyle(SOLID_FILL, COLOR(clampi(80 + 175 * sp), clampi(230 - 150 * sp), 80));
    bar(SW - 165, SH - 62, SW - 165 + (int)(145 * sp), SH - 52);
    if (boost > 0) { label(SW - 165, SH - 42 - 0, "", 1, 0, 0, 0); }

    double prog = clampd(pz / (nSeg * SEG_LEN), 0, 1);               // progress bar
    panel(30, SH - 36, SW - 195, SH - 14);
    setfillstyle(SOLID_FILL, COLOR(110, 220, 130)); bar(34, SH - 32, 34 + (int)((SW - 195 - 68) * prog), SH - 18);
    for (size_t j = 1; j < flags.size(); j++) {
        int fx = 34 + (int)((SW - 195 - 68) * flags[j].z / (nSeg * SEG_LEN));
        setfillstyle(SOLID_FILL, flags[j].got ? COLOR(255, 255, 255) : COLOR(225, 70, 60)); bar(fx, SH - 34, fx + 3, SH - 16);
    }
    int mx = 34 + (int)((SW - 195 - 68) * prog);
    setfillstyle(SOLID_FILL, COLOR(255, 205, 40)); bar(mx - 5, SH - 38, mx + 5, SH - 12);

    if (popupT > 0 && state == ST_PLAY) {                            // floating message
        panel(SW / 2 - 150, 70, SW / 2 + 150, 106);  labelC(SW / 2, 80, popupMsg, 2, 255, 235, 120);
    }
    if (boost > 0)                                                    // speed lines
        for (int i = 0; i < 14; i++) {
            int side = (i & 1) ? 1 : -1, y = 130 + (i * 53 + frameNo * 17) % 380;
            int x1 = SW / 2 + side * (170 + (i * 37) % 150), len = 40 + (i * 13) % 50;
            setcolor(COLOR(255, 255, 255)); line(x1, y, x1 + side * len, y + len / 3);
        }
}

void drawOverlay()
{
    char buf[96];
    if (state == ST_CRASH || state == ST_OVER) {
        panel(SW / 2 - 250, 190, SW / 2 + 250, 350);
        labelC(SW / 2, 208, state == ST_OVER ? "GAME OVER" : "OUCH!", 4, 255, 120, 110);
        labelC(SW / 2, 258, msg, 2, 255, 255, 255);
        if (state == ST_CRASH) { sprintf(buf, "Lives left: %d     ENTER = continue", lives); }
        else strcpy(buf, "ENTER = try this level again");
        labelC(SW / 2, 300, buf, 2, 255, 225, 120);
    } else if (state == ST_WIN) {
        panel(SW / 2 - 250, 150, SW / 2 + 250, 380);
        labelC(SW / 2, 168, "LEVEL COMPLETE!", 4, 120, 235, 140);
        for (int i = 0; i < 3; i++) drawStar(SW / 2 - 70 + i * 70, 250, 28, i < stars);
        sprintf(buf, "Coins %d / %d    Score %d", coinsGot, (int)coins.size(), score);
        labelC(SW / 2, 298, buf, 2, 255, 255, 255);
        labelC(SW / 2, 336, "ENTER = next level", 2, 255, 225, 120);
    } else if (state == ST_PAUSE) {
        panel(SW / 2 - 170, 240, SW / 2 + 170, 330);
        labelC(SW / 2, 258, "PAUSED", 4, 255, 255, 255);
        labelC(SW / 2, 298, "P = continue", 2, 255, 225, 120);
    }
}

void drawTitleUI()
{
    panel(SW / 2 - 330, 40, SW / 2 + 330, 150);
    labelC(SW / 2, 58, "CUBE TRUCK 3D", 6, 255, 215, 70);
    labelC(SW / 2, 118, "a blocky monster-truck adventure", 2, 190, 215, 255);
    panel(SW / 2 - 330, 400, SW / 2 + 330, 560);
    labelC(SW / 2, 414, "UP/DOWN = gas/brake    LEFT/RIGHT = steer    SPACE = jump", 1, 255, 255, 255);
    labelC(SW / 2, 436, "Collect coins   Hit BOOST pads   Jump gaps   Avoid crates", 1, 190, 215, 255);
    labelC(SW / 2, 458, "P = pause     M = sound     ESC = quit", 1, 190, 215, 255);
    if ((int)(titleT * 2) % 2 == 0) labelC(SW / 2, 500, "PRESS ENTER TO START", 3, 120, 235, 140);
    else labelC(SW / 2, 500, "                    ", 3, 120, 235, 140);
}

void render()
{
    double sx = (shake > 0) ? rndr(-1, 1) * shake * 0.25 : 0, sy = (shake > 0) ? rndr(-1, 1) * shake * 0.25 : 0;
    rcx = camX + sx; rcy = camY + sy; rcz = camZ;
    cP = cos(CAM_PITCH); sP = sin(CAM_PITCH);
    drawSky();
    drawDecor();
    int tseg = (int)floor(pz / SEG_LEN);
    int i1 = (int)floor(camZ / SEG_LEN) + 24, i0 = (int)floor((camZ - 2) / SEG_LEN);
    if (i0 < 0) i0 = 0;
    if (i1 > nSeg + 7) i1 = nSeg + 7;
    bool truckDone = false;
    for (int i = i1; i >= i0; i--) {                          // far -> near (painter's algorithm)
        drawSegment(i);
        drawSegObjects(i);
        if (!truckDone && (i == tseg - 1 || i == i0)) {      // truck goes in right after the road under it
            if (truckVisible && state != ST_TITLE) {
                if (invuln <= 0 || ((int)(invuln * 10) & 1)) {
                    drawShadow();
                    drawTruck(px, py, pz, pitchVis, rollVis, 0, braking);
                }
            }
            truckDone = true;
        }
    }
    if (state == ST_TITLE) drawTruck(px, py, pz, 0.0, 0.0, sin(titleT * 0.8) * 0.9, false);
    drawParts();
    if (state == ST_TITLE) drawTitleUI(); else { drawHUD(); drawOverlay(); }
}

void initTitle()
{
    startGame();
    state = ST_TITLE; titleT = 0;
    pz = 10; px = 0; py = 0; camX = 0; camY = 3.6; camZ = pz - 6.5; focal = 560;
    wheelAng = 0; popupT = 0;
}

/* -------------------------------- main loop ----------------------------- */
int main()
{
    initwindow(SW, SH, "Cube Truck 3D");
    CreateThread(NULL, 0, sfxThread, NULL, 0, NULL);
    initTitle();
    int page = 0;
    DWORD last = GetTickCount();
    for (;;) {
        pollKeys();
        if (kDown[VK_ESCAPE]) break;
        DWORD now = GetTickCount();
        double dt = (now - last) / 1000.0;
        last = now;
        if (dt > 0.05) dt = 0.05;
        update(dt);
        setactivepage(page);            // draw on the hidden page ...
        render();
        setvisualpage(page);            // ... then show it  (double buffering)
        page = 1 - page;
        delay(8);
    }
    closegraph();
    return 0;
}

