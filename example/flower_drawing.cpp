
#include <graphics.h>
#include <conio.h>

int main()
{
    initwindow(800, 600, "Flower Drawing");
    setbkcolor(LIGHTCYAN);
    cleardevice();

    // Flower petals
    setcolor(MAGENTA);
    setfillstyle(SOLID_FILL, MAGENTA);

    fillellipse(400, 180, 45, 60);
    fillellipse(400, 300, 45, 60);
    fillellipse(340, 240, 60, 45);
    fillellipse(460, 240, 60, 45);
    fillellipse(355, 195, 45, 50);
    fillellipse(445, 195, 45, 50);
    fillellipse(355, 285, 45, 50);
    fillellipse(445, 285, 45, 50);

    // Flower center
    setcolor(YELLOW);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(400, 240, 45, 45);

    // Stem
    setcolor(GREEN);
    setlinestyle(SOLID_LINE, 0, 5);
    line(400, 285, 400, 500);

    // Left leaf
    setcolor(GREEN);
    setfillstyle(SOLID_FILL, GREEN);
    fillellipse(350, 400, 55, 25);

    // Right leaf
    fillellipse(450, 450, 55, 25);

    // Grass
    setfillstyle(SOLID_FILL, GREEN);
    bar(0, 520, 800, 600);

    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(300, 30, "BEAUTIFUL FLOWER");

    getch();
    closegraph();
    return 0;
}
