
#include <graphics.h>
#include <conio.h>

int main()
{
    initwindow(800, 600, "Traffic Light");

    // Background
    setbkcolor(LIGHTGRAY);
    cleardevice();

    // Traffic light body
    setcolor(BLACK);
    setfillstyle(SOLID_FILL, BLACK);
    bar(300, 80, 500, 500);

    // Red light
    setcolor(RED);
    setfillstyle(SOLID_FILL, RED);
    fillellipse(400, 160, 65, 65);

    // Yellow light
    setcolor(YELLOW);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(400, 290, 65, 65);

    // Green light
    setcolor(GREEN);
    setfillstyle(SOLID_FILL, GREEN);
    fillellipse(400, 420, 65, 65);

    // Title
    setcolor(BLACK);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(260, 30, "TRAFFIC LIGHT");

    getch();
    closegraph();

    return 0;
}
