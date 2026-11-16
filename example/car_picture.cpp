
#include <graphics.h>
#include <conio.h>

int main()
{
    initwindow(800, 600, "My Car Picture");

    // Sky
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, 800, 400);

    // Road
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(0, 400, 800, 600);

    // Road lines
    setcolor(WHITE);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(0, 500, 800, 500);

    // Car body
    setcolor(RED);
    setfillstyle(SOLID_FILL, RED);
    rectangle(200, 300, 600, 420);
    floodfill(300, 350, RED);

    // Car roof
    setcolor(RED);
    setfillstyle(SOLID_FILL, RED);
    line(280, 300, 340, 220);
    line(340, 220, 480, 220);
    line(480, 220, 540, 300);
    line(280, 300, 540, 300);
    floodfill(400, 260, RED);

    // Windows
    setcolor(WHITE);
    setfillstyle(SOLID_FILL, WHITE);
    rectangle(350, 235, 405, 290);
    floodfill(370, 250, WHITE);

    rectangle(415, 235, 475, 290);
    floodfill(430, 250, WHITE);

    // Wheels
    setcolor(BLACK);
    setfillstyle(SOLID_FILL, BLACK);
    fillellipse(290, 420, 45, 45);
    fillellipse(520, 420, 45, 45);

    // Wheel centers
    setcolor(WHITE);
    setfillstyle(SOLID_FILL, LIGHTGRAY);
    fillellipse(290, 420, 20, 20);
    fillellipse(520, 420, 20, 20);

    // Headlight
    setcolor(YELLOW);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(590, 340, 12, 15);

    // Title
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(270, 80, "MY FIRST CAR");

    getch();
    closegraph();

    return 0;
}
