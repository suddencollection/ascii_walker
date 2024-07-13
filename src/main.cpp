#include <chrono>
#include <cmath>
#include <iostream>
#include <ncurses.h>
//#include <locale.h>
#include <ncurses.h>
#include <numbers>
#include <numeric>
#include <string>
#include <thread>

// constexpr int screenWidth{120};
// constexpr int nScreenHeight{40};
// int screenWidth{96};
// int nScreenHeight{32};

std::wstring map{
  L"################"
  L"#              #"
  L"#   #######    #"
  L"#   #          #"
  L"#   #  ####    #"
  L"#      #       #"
  L"#      #       #"
  L"#      #       #"
  L"########       #"
  L"#              #"
  L"#              #"
  L"#              #"
  L"#  ##          #"
  L"#  ##          #"
  L"#              #"
  L"################"};

WINDOW* bufferWin{};
WINDOW* mapWin{};
int screenWidth{};
int screenHeight{};

float playerX{12.f};
float playerY{8.f};
float playerAngle{5};
float playerFov{std::numbers::pi / 2};
float depth{16};
float turningSpeed{1.f};

constexpr int mapHeight{16};
constexpr int mapWidth{16};

void initColors()
{
  // sky
  init_pair(101, COLOR_CYAN, COLOR_CYAN);

  // floor
  init_color(COLOR_GREEN, 0, 300, 0);
  init_pair(102, COLOR_WHITE, COLOR_GREEN);

  // walls
  for(int i{8}; i > 0; --i)
  {
     int shade{75 * i + 400};
     init_color(200 + i, shade, shade, shade);
     init_pair(200 + i, 200 + i, -1);
   }

  for(int i{0}; i < 16; ++i)
  {
    int shade{1000 - (50 * i) - 50};
    init_color(200 + i, shade, shade, shade);
    init_pair(200 + i, 200 + i, -1);
  }
}

void init()
{
  setlocale(LC_ALL, "");

  initscr();
  noecho();
  nodelay(stdscr, true);
  cbreak();
  start_color();
  use_default_colors();
  getmaxyx(stdscr, screenHeight, screenWidth);

  bufferWin = subwin(stdscr, screenHeight, screenWidth, 0, 0);
  mapWin = newwin(mapHeight, mapWidth, 2, 2);

  initColors();
}

void drawMap()
{
  for(int x{}; x < screenWidth; ++x)
  {
    float rayAngle{(playerAngle - playerFov / 2.f) + (static_cast<float>(x) / screenWidth * playerFov)};
    float wallDistance{0.f};

    // float eyeX{ sinf(rayAngle) };
    // float eyeY{ cosf(rayAngle) };
    float eyeX{sinf(rayAngle)};
    float eyeY{cosf(rayAngle)};

    bool wallHited{};
    while(!wallHited && wallDistance < depth)
    {
      wallDistance += 0.1;

      int testX = playerX + eyeX * wallDistance;
      int testY = playerY + eyeY * wallDistance;

      if(testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight)
      {
        wallHited = true;
        wallDistance = depth;
      }
      else
      {
        if(map[testY * mapWidth + testX] == '#')
        {
          wallHited = true;
        }
      }
    }

    int ceilling = (screenHeight / 2.f) - screenHeight / (wallDistance);
    int floor = screenHeight - ceilling;

    int shade = 200 + std::round(wallDistance / depth * 8);
    // init_color(128, shade, shade, shade);

    chtype ceillingCh{COLOR_PAIR(101) | ' '};
    chtype floorCh{COLOR_PAIR(102) | ' '};
    chtype wallCh{ACS_CKBOARD};

    wallCh |= COLOR_PAIR(shade);

    for(int y{}; y < screenHeight; ++y)
    {
      if(y <= ceilling)
      {
        mvwaddch(bufferWin, y, x, ceillingCh);
      }
      else if(y > ceilling && y < floor)
      {
        // wattron(bufferWin, COLOR_PAIR(1));
        mvwaddch(bufferWin, y, x, wallCh);
        // wattroff(bufferWin, COLOR_PAIR(1));
      }
      else
      {
        mvwaddch(bufferWin, y, x, floorCh);
      }
    }
  }
}

void logic(int& input, float timeStep)
{
  switch(input)
  {
    case 'w':
      playerX += std::sin(playerAngle) * 3.f * timeStep;
      playerY += std::cos(playerAngle) * 3.f * timeStep;
      break;
    case 'a':
      playerAngle -= (turningSpeed * timeStep);
      break;
    case 's':
      playerX -= std::sin(playerAngle) * 3.f * timeStep;
      playerY -= std::cos(playerAngle) * 3.f * timeStep;
      break;
    case 'd':
      playerAngle += (turningSpeed * timeStep);
      break;
    default:
      break;
  }

  //  mvwprintw(bufferWin, 0, 0, "ts[%f]", timeStep);
  //  wrefresh(bufferWin);
}

void render()
{
  drawMap();
  //  box(bufferWin, 0, 0);

  mvprintw(1, 0, "angle[%f]", playerAngle);
  mvprintw(2, 0, "playerXY[%f, %f]", playerX, playerY);

  // for(int y{}; y < mapHeight; ++y)
  //   for(int x{}; x < mapWidth; ++x)
  //   {
  //     move(y, x);
  //     waddch(mapWin, map[(y * mapWidth) + x]);
  //   }

  wrefresh(bufferWin);
  //  wrefresh(mapWin);
}

int main()
{
  init();

  if(!can_change_color())
  {
    endwin();
    return -1;
  }

  using TimePoint = std::chrono::steady_clock::time_point;
  using Duration = std::chrono::duration<float>;
  auto& now{std::chrono::steady_clock::now};

  TimePoint start{now()};
  TimePoint end{now()};
  Duration elapsedTime{};

  int input{};
  while(input != 'q')
  {
    end = now();
    elapsedTime = end - start;
    start = end;

    logic(input, elapsedTime.count());
    render();
    input = getch();

    std::this_thread::sleep_for(std::chrono::milliseconds{16});
  }

  endwin();
}
