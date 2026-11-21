#include <graphics.h>
#include <conio.h>
#include <dos.h>

int main()
{
    initwindow(800, 600, "Moving Car");

    int x = -150;

    while (!kbhit())
    {
        cleardevice();

        // Sky
        setbkcolor(LIGHTCYAN);

        // Road
        setcolor(DARKGRAY);
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(0, 400, 800, 550);

        // Road lines
        setcolor(WHITE);
        for (int i = 0; i < 800; i += 120)
            line(i, 475, i + 60, 475);

        // Car body
        setcolor(RED);
        setfillstyle(SOLID_FILL, RED);
        bar(x, 330, x + 150, 400);

        // Car roof
        setcolor(RED);
        setfillstyle(SOLID_FILL, RED);
        line(x + 30, 330, x + 60, 280);
        line(x + 60, 280, x + 115, 280);
        line(x + 115, 280, x + 140, 330);
        line(x + 30, 330, x + 140, 330);

        // Windows
        setcolor(WHITE);
        setfillstyle(SOLID_FILL, WHITE);
        bar(x + 65, 290, x + 105, 325);

        // Wheels
        setcolor(BLACK);
        setfillstyle(SOLID_FILL, BLACK);
        fillellipse(x + 35, 400, 22, 22);
        fillellipse(x + 120, 400, 22, 22);

        // Wheel centers
        setcolor(LIGHTGRAY);
        setfillstyle(SOLID_FILL, LIGHTGRAY);
        fillellipse(x + 35, 400, 10, 10);
        fillellipse(x + 120, 400, 10, 10);

        x += 5;

        if (x > 800)
            x = -150;

        delay(40);
    }

    getch();
    closegraph();
    return 0;
}
