
#include <graphics.h>
#include <conio.h>
#include <dos.h>

int main()
{
    initwindow(800, 600, "Bouncing Ball");

    int x = 400;
    int y = 100;
    int dx = 5;
    int dy = 5;
    int radius = 30;

    while (!kbhit())
    {
        cleardevice();

        // Background
        setbkcolor(BLUE);

        // Ball
        setcolor(RED);
        setfillstyle(SOLID_FILL, YELLOW);
        fillellipse(x, y, radius, radius);

        // Move ball
        x += dx;
        y += dy;

        // Bounce from left and right walls
        if (x + radius >= 800 || x - radius <= 0)
            dx = -dx;

        // Bounce from top and bottom walls
        if (y + radius >= 600 || y - radius <= 0)
            dy = -dy;

        delay(20);
    }

    getch();
    closegraph();
    return 0;
}
