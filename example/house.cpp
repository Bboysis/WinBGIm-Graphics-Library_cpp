
#include <graphics.h>
#include <conio.h>

int main()
{
    initwindow(800, 600, "House Drawing");

    // Sky
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, 800, 600);

    // Grass
    setfillstyle(SOLID_FILL, GREEN);
    bar(0, 400, 800, 600);

    // House body
    setcolor(WHITE);
    setfillstyle(SOLID_FILL, LIGHTGRAY);
    rectangle(250, 250, 550, 470);
    floodfill(300, 300, WHITE);

    // Roof
    setcolor(RED);
    setfillstyle(SOLID_FILL, RED);

    line(220, 250, 400, 100);
    line(400, 100, 580, 250);
    line(220, 250, 580, 250);

    floodfill(400, 200, RED);

    // Door
    setcolor(BROWN);
    setfillstyle(SOLID_FILL, BROWN);
    rectangle(370, 350, 440, 470);
    floodfill(400, 400, BROWN);

    // Windows
    setcolor(BLUE);
    setfillstyle(SOLID_FILL, WHITE);

    rectangle(280, 290, 340, 350);
    floodfill(300, 300, BLUE);

    rectangle(460, 290, 520, 350);
    floodfill(480, 300, BLUE);

    // Sun
    setcolor(YELLOW);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(680, 100, 45, 45);

    // Title
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(300, 30, "MY DREAM HOUSE");

    getch();
    closegraph();

    return 0;
}
