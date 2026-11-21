 #include <graphics.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// =====================================================
// 3D VOXEL MONSTER TRUCK GAME
// C++98 + WinBGIm
// =====================================================

#define SCREEN_W 1000
#define SCREEN_H 700
#define MAX_FACES 5000
#define PI 3.14159265

// Colors
#define PINK 13
#define SKY LIGHTBLUE

// -----------------------------------------------------
// 3D VECTOR
// -----------------------------------------------------

struct Vec3
{
    float x, y, z;

    Vec3() {}

    Vec3(float a, float b, float c)
    {
        x = a;
        y = b;
        z = c;
    }
};

Vec3 add(Vec3 a, Vec3 b)
{
    return Vec3(a.x+b.x, a.y+b.y, a.z+b.z);
}

Vec3 sub(Vec3 a, Vec3 b)
{
    return Vec3(a.x-b.x, a.y-b.y, a.z-b.z);
}

Vec3 cross(Vec3 a, Vec3 b)
{
    return Vec3(
        a.y*b.z-a.z*b.y,
        a.z*b.x-a.x*b.z,
        a.x*b.y-a.y*b.x
    );
}

float dot(Vec3 a, Vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 normalize(Vec3 a)
{
    float length = sqrt(dot(a,a));

    if(length < 0.0001)
        return Vec3(0,0,1);

    return Vec3(a.x/length, a.y/length, a.z/length);
}

// -----------------------------------------------------
// CAMERA
// -----------------------------------------------------

Vec3 cameraPos;
Vec3 cameraRight;
Vec3 cameraUp;
Vec3 cameraForward;

float focalLength = 550.0;

struct ScreenPoint
{
    int x, y;
};

ScreenPoint projectPoint(Vec3 p)
{
    Vec3 relative = sub(p, cameraPos);

    float depth = dot(relative, cameraForward);

    ScreenPoint result;

    if(depth < 0.5)
        depth = 0.5;

    float horizontal = dot(relative, cameraRight);
    float vertical = dot(relative, cameraUp);

    result.x = (int)(SCREEN_W/2 + focalLength*horizontal/depth);
    result.y = (int)(SCREEN_H/2 - focalLength*vertical/depth);

    return result;
}

void setupCamera(float truckX, float truckY, float distance)
{
    cameraPos = Vec3(
        truckX,
        5.5 + truckY,
        distance - 10
    );

    Vec3 target = Vec3(
        truckX,
        1.0 + truckY,
        distance + 12
    );

    cameraForward = normalize(sub(target,cameraPos));

    Vec3 worldUp = Vec3(0,1,0);

    cameraRight = normalize(cross(cameraForward,worldUp));
    cameraUp = normalize(cross(cameraRight,cameraForward));
}

// -----------------------------------------------------
// FACE STORAGE FOR PAINTER'S ALGORITHM
// -----------------------------------------------------

struct Face
{
    Vec3 points[4];
    int color;
    float depth;
};

Face faces[MAX_FACES];
int faceCount = 0;

void addFace(Vec3 a, Vec3 b, Vec3 c, Vec3 d, int color)
{
    if(faceCount >= MAX_FACES)
        return;

    Vec3 center = Vec3(
        (a.x+b.x+c.x+d.x)/4,
        (a.y+b.y+c.y+d.y)/4,
        (a.z+b.z+c.z+d.z)/4
    );

    // Back-face culling
    Vec3 normal = cross(sub(b,a),sub(c,a));
    Vec3 toCamera = sub(cameraPos,center);

    if(dot(normal,toCamera) <= 0)
        return;

    Vec3 relative = sub(center,cameraPos);

    float depth = dot(relative,cameraForward);

    if(depth <= 0.5)
        return;

    faces[faceCount].points[0] = a;
    faces[faceCount].points[1] = b;
    faces[faceCount].points[2] = c;
    faces[faceCount].points[3] = d;

    faces[faceCount].color = color;
    faces[faceCount].depth = depth;

    faceCount++;
}

// Far faces must be drawn first.
int compareFaces(const void *a, const void *b)
{
    Face *fa = (Face*)a;
    Face *fb = (Face*)b;

    if(fa->depth < fb->depth)
        return 1;

    if(fa->depth > fb->depth)
        return -1;

    return 0;
}

// -----------------------------------------------------
// 3D BOX / VOXEL
// -----------------------------------------------------

void drawBox(
    float x, float y, float z,
    float width, float height, float depth,
    int color
)
{
    float hx = width/2;
    float hy = height/2;
    float hz = depth/2;

    Vec3 v[8];

    v[0] = Vec3(x-hx,y-hy,z-hz);
    v[1] = Vec3(x+hx,y-hy,z-hz);
    v[2] = Vec3(x+hx,y+hy,z-hz);
    v[3] = Vec3(x-hx,y+hy,z-hz);
    v[4] = Vec3(x-hx,y-hy,z+hz);
    v[5] = Vec3(x+hx,y-hy,z+hz);
    v[6] = Vec3(x+hx,y+hy,z+hz);
    v[7] = Vec3(x-hx,y+hy,z+hz);

    // Six faces of the cube
    addFace(v[4],v[5],v[6],v[7],color);
    addFace(v[1],v[0],v[3],v[2],color-1);
    addFace(v[5],v[1],v[2],v[6],color-2);
    addFace(v[0],v[4],v[7],v[3],color-2);
    addFace(v[3],v[7],v[6],v[2],color+1);
    addFace(v[0],v[1],v[5],v[4],color-3);
}

// -----------------------------------------------------
// DRAW ALL STORED FACES
// -----------------------------------------------------

void renderFaces()
{
    qsort(faces,faceCount,sizeof(Face),compareFaces);

    int i;

    for(i=0;i<faceCount;i++)
    {
        int points[10];

        int j;
        int visible = 1;

        for(j=0;j<4;j++)
        {
            Vec3 relative = sub(faces[i].points[j],cameraPos);

            if(dot(relative,cameraForward) < 0.5)
            {
                visible = 0;
                break;
            }

            ScreenPoint p = projectPoint(faces[i].points[j]);

            points[j*2] = p.x;
            points[j*2+1] = p.y;
        }

        if(visible)
        {
            points[8] = points[0];
            points[9] = points[1];

            setfillstyle(SOLID_FILL,faces[i].color);
            setcolor(BLACK);

            fillpoly(4,points);
        }
    }
}

// -----------------------------------------------------
// MONSTER TRUCK
// -----------------------------------------------------

void drawTruck(float x, float jump, float distance)
{
    float z = distance;

    // Yellow main body
    drawBox(x,0.9+jump,z,3.0,1.0,4.0,YELLOW);

    // Grey driver cabin
    drawBox(x,1.9+jump,z-0.3,2.1,1.3,2.0,LIGHTGRAY);

    // Front windshield
    drawBox(x,2.0+jump,z+0.75,1.7,0.7,0.12,CYAN);

    // Four large black box wheels
    drawBox(x-1.65,0.35+jump,z-1.3,0.85,0.9,1.1,BLACK);
    drawBox(x+1.65,0.35+jump,z-1.3,0.85,0.9,1.1,BLACK);

    drawBox(x-1.65,0.35+jump,z+1.3,0.85,0.9,1.1,BLACK);
    drawBox(x+1.65,0.35+jump,z+1.3,0.85,0.9,1.1,BLACK);

    // Wheel centers
    drawBox(x-2.10,0.35+jump,z-1.3,0.08,0.35,0.35,LIGHTGRAY);
    drawBox(x+2.10,0.35+jump,z-1.3,0.08,0.35,0.35,LIGHTGRAY);

    drawBox(x-2.10,0.35+jump,z+1.3,0.08,0.35,0.35,LIGHTGRAY);
    drawBox(x+2.10,0.35+jump,z+1.3,0.08,0.35,0.35,LIGHTGRAY);
}

// -----------------------------------------------------
// SCENE
// -----------------------------------------------------

int levelLength(int level)
{
    return 220 + level*80;
}

void drawScene(
    float truckX,
    float jump,
    float distance,
    int level
)
{
    // Large green ground
    drawBox(0,-1.0,distance+80,100,1,220,LIGHTGREEN);

    // Pink road segments with gaps
    int currentSegment = (int)(distance/8);

    int i;

    for(i=currentSegment-2;i<currentSegment+25;i++)
    {
        if(i < 0)
            continue;

        // Some road sections are missing
        if((i+level)%7 == 5)
            continue;

        float z = i*8+4;

        drawBox(0,-0.25,z,8,0.5,8,PINK);
    }

    // Red cube obstacles
    int obstacleSpacing = 18-level*2;

    if(obstacleSpacing < 10)
        obstacleSpacing = 10;

    int firstObstacle = (int)(distance/obstacleSpacing)-1;

    for(i=firstObstacle;i<firstObstacle+12;i++)
    {
        if(i < 0)
            continue;

        float z = i*obstacleSpacing+12;

        int position = (i*37+level*13)%5;
        float x = (position-2)*1.4;

        drawBox(x,0.9,z,2.0,1.8,2.0,RED);
    }

    // Pink pillars on the sides
    for(i=currentSegment-1;i<currentSegment+14;i++)
    {
        if(i < 0)
            continue;

        float z = i*16+8;

        drawBox(-5.5,2.0,z,1.0,4.0,1.0,PINK);
        drawBox(5.5,2.0,z,1.0,4.0,1.0,PINK);
    }

    // White block clouds
    for(i=0;i<8;i++)
    {
        float z = distance+15+i*24;

        float x = (i%2==0) ? -9 : 9;

        drawBox(x,9,z,4.0,1.5,2.0,WHITE);
        drawBox(x+1,9.7,z,2.0,1.0,2.0,WHITE);
    }

    // Finish line
    float finishZ = levelLength(level);

    if(finishZ-distance < 180)
    {
        int tile;
        for(tile=0;tile<8;tile++)
        {
            int color = (tile%2==0) ? WHITE : BLACK;

            drawBox(
                -3.5+tile,
                0.08,
                finishZ,
                1.0,
                0.15,
                2.0,
                color
            );
        }
    }

    // Truck
    drawTruck(truckX,jump,distance);
}

// -----------------------------------------------------
// HUD
// -----------------------------------------------------

void drawHUD(
    int level,
    float speed,
    float distance,
    int crashed,
    int completed
)
{
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT,HORIZ_DIR,2);

    char title[80];

    sprintf(title,"Level %d: The Cube",level);

    outtextxy(25,20,title);

    char speedText[50];

    sprintf(speedText,"Speed: %d", (int)(speed*100));

    outtextxy(25,55,speedText);

    // Progress bar background
    setfillstyle(SOLID_FILL,DARKGRAY);
    bar(25,100,325,120);

    float progress = distance/levelLength(level);

    if(progress > 1)
        progress = 1;

    setfillstyle(SOLID_FILL,LIGHTGREEN);

    bar(
        25,
        100,
        25+(int)(300*progress),
        120
    );

    setcolor(WHITE);
    rectangle(25,100,325,120);

    outtextxy(25,135,"UP: Gas  DOWN: Brake");
    outtextxy(25,165,"LEFT/RIGHT: Steer");
    outtextxy(25,195,"SPACE: Jump  ESC: Quit");

    if(crashed)
    {
        setcolor(RED);
        settextstyle(DEFAULT_FONT,HORIZ_DIR,4);
        outtextxy(330,280,"TRUCK CRASHED!");

        setcolor(WHITE);
        settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
        outtextxy(335,340,"Press ENTER to retry");
    }

    if(completed)
    {
        setcolor(LIGHTGREEN);
        settextstyle(DEFAULT_FONT,HORIZ_DIR,4);
        outtextxy(300,280,"LEVEL COMPLETE!");

        setcolor(WHITE);
        settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
        outtextxy(335,340,"Press ENTER to continue");
    }
}

// -----------------------------------------------------
// MAIN GAME
// -----------------------------------------------------

int main()
{
    initwindow(SCREEN_W,SCREEN_H,"3D Monster Truck - WinBGIm");

    // Double buffering
    setactivepage(0);
    setvisualpage(1);

    int page = 0;

    int level = 1;

    float truckX = 0;
    float speed = 0;
    float distance = 0;

    float jump = 0;
    float jumpVelocity = 0;

    int crashed = 0;
    int completed = 0;

    while(1)
    {
        // Quit
        if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;

        // Retry or next level
        if(GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            if(crashed)
            {
                truckX = 0;
                speed = 0;
                distance = 0;
                jump = 0;
                jumpVelocity = 0;
                crashed = 0;
            }
            else if(completed)
            {
                level++;

                if(level > 5)
                    level = 5;

                truckX = 0;
                speed = 0;
                distance = 0;
                jump = 0;
                jumpVelocity = 0;
                completed = 0;
            }

            delay(250);
        }

        if(!crashed && !completed)
        {
            // Acceleration
            if(GetAsyncKeyState(VK_UP) & 0x8000)
                speed += 0.015;

            // Brake
            if(GetAsyncKeyState(VK_DOWN) & 0x8000)
                speed -= 0.025;

            if(speed > 0.9+level*0.1)
                speed = 0.9+level*0.1;

            if(speed < 0)
                speed = 0;

            // Natural slowdown
            if(!(GetAsyncKeyState(VK_UP) & 0x8000))
                speed *= 0.995;

            // Steering
            if(GetAsyncKeyState(VK_LEFT) & 0x8000)
                truckX -= 0.12;

            if(GetAsyncKeyState(VK_RIGHT) & 0x8000)
                truckX += 0.12;

            if(truckX < -3.0)
                truckX = -3.0;

            if(truckX > 3.0)
                truckX = 3.0;
                // Jump
            if((GetAsyncKeyState(VK_SPACE) & 0x8000) && jump == 0)
                jumpVelocity = 0.48;

            jump += jumpVelocity;
            jumpVelocity -= 0.025;

            if(jump < 0)
            {
                jump = 0;
                jumpVelocity = 0;
            }

            // Move forward
            distance += speed;

            // Check road gaps
            int segment = (int)(distance/8);

            if((segment+level)%7 == 5 && jump < 1.0 && speed > 0.3)
            {
                crashed = 1;
                speed = 0;
            }

            // Check red obstacles
            int spacing = 18-level*2;

            if(spacing < 10)
                spacing = 10;

            int obstacle = (int)((distance-12)/spacing);

            if(obstacle >= 0)
            {
                float obstacleZ = obstacle*spacing+12;
                int position = (obstacle*37+level*13)%5;
                float obstacleX = (position-2)*1.4;

                if(fabs(distance-obstacleZ) < 2.0 &&
                   fabs(truckX-obstacleX) < 1.8 &&
                   jump < 1.2)
                {
                    crashed = 1;
                    speed = 0;
                }
            }

            // Check road edges
            if(fabs(truckX) > 3.4)
            {
                crashed = 1;
                speed = 0;
            }

            // Finish
            if(distance >= levelLength(level))
            {
                completed = 1;
                speed = 0;
            }
        }

        // ---------------------------------------------
        // DRAW FRAME
        // ---------------------------------------------

        setactivepage(page);

        setbkcolor(SKY);
        cleardevice();

        setupCamera(truckX,jump,distance);

        faceCount = 0;

        drawScene(truckX,jump,distance,level);

        renderFaces();

        // HUD is drawn after the 3D scene
        drawHUD(level,speed,distance,crashed,completed);

        setvisualpage(page);

        page = 1-page;

        delay(20);
    }

    closegraph();

    return 0;
}
