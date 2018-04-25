#include <thread>
#include <stdio.h>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <atomic>
#include <ncurses.h>
using namespace std;

atomic<bool> endFlag(false);
atomic<int> suspend(0);
bool nCursesEndFlag = false;
mutex chopstick[5];
int time_control = 3000;
int N = 5;
mutex ncursesInput;
int eating_progress[5] = {0, 0, 0, 0, 0};
int thinking_progress[5] = {0, 0, 0, 0, 0};
int waiting[5] = {0, 0, 0, 0, 0};
int chopstick_status[5] = {-1, -1, -1, -1, -1};
int appTime = 0;

void cursesRefresh()
{
      //ncursesInput.lock();
      for (int i = 0; i < 5; i++)
      {
            //mvprintw(i+1,8,to_string(eating_progress[i]).c_str());
            mvprintw(i + 1, 8, "%2d%%", eating_progress[i]);
            mvprintw(i + 1, 18, "%2d%%", thinking_progress[i]);
            if (eating_progress[i] == 0 && thinking_progress[i] == 0 && waiting[i] != -1)
                  mvprintw(i + 1, 24, "%8d ms", appTime - waiting[i]);
            else
                  mvprintw(i + 1, 24, "%8d ms", 0);
            if (chopstick_status[i] != -1)
                  mvprintw(1, 42 + i * 4, "_%d", chopstick_status[i]);
            else
                  mvprintw(1, 42 + i * 4, "__");
      }
      int a = suspend;
      mvprintw(6, 0, "Waiter holding: P%d", a);

      mvprintw(8, 0, "Application time: %d ms", appTime);
      if (endFlag)
            mvprintw(9, 40, "End flag triggered");
      refresh();
      //ncursesInput.unlock();
}

void think(int id)
{
      for (int i = 0; i < 100; i++)
      {
            this_thread::sleep_for(chrono::milliseconds(60));
            thinking_progress[id]++;
      }
      thinking_progress[id] = 0;
}

void pickUpChopsticks(int id)
{
      //chopstick[min(id,(id+1) % 5)].lock();
      //chopstick_status[min(id,(id+1) % 5)] = id;
      chopstick[id].lock();
      chopstick_status[id] = id;
      //cursesPickUpChopstick(id,min(id,(id+1) % 5));
      chopstick[(id + 1) % 5].lock();
      chopstick_status[(id + 1) % 5] = id;
      //cursesPickUpChopstick(id,max(id,(id+1) % 5));
}

void putDownChopsticks(int id)
{
      //cursesPutDownChopstick(id,min(id,(id+1) % 5));
      //chopstick_status[min(id,(id+1) % 5)] = -1;
      //chopstick[min(id,(id+1) % 5)].unlock();
      chopstick_status[id] = -1;
      chopstick[id].unlock();
      //cursesPutDownChopstick(id,max(id,(id+1) % 5));
      //chopstick_status[max(id,(id+1) % 5)] = -1;
      //chopstick[max(id,(id+1) % 5)].unlock();
      chopstick_status[(id + 1) % 5] = -1;
      chopstick[(id + 1) % 5].unlock();
}

void eat(int id)
{
      //cursesEating(id);
      //this_thread::sleep_for(chrono::milliseconds(rand() % 200 + 1301 + time_control));
      for (int i = 0; i < 100; i++)
      {
            this_thread::sleep_for(chrono::milliseconds(50));
            eating_progress[id]++;
      }
      eating_progress[id] = 0;
      //cursesExisting(id);
}

void waiter(int id)
{
      while (suspend == id)
      {
            this_thread::sleep_for(chrono::milliseconds(50));
      }

      //suspend = id;
}

void Philosopher(int id)
{
      //cursesExisting(id);
      //this_thread::sleep_for(chrono::milliseconds(500));
      while (!endFlag)
      {
            think(id);
            waiting[id] = appTime;
            waiter(id);
            pickUpChopsticks(id);
            waiting[id] = -1;
            eat(id);
            suspend = id;
            putDownChopsticks(id);
      }
}

void runWindow()
{
      int _progress = 1;
      initscr();
      if (has_colors() == FALSE)
      {
            endwin();
            printf("Your terminal does not support color \n");
            exit(1);
      }
      start_color();
      raw();
      curs_set(false);

      init_pair(1, COLOR_BLACK, COLOR_BLACK);
      init_pair(2, COLOR_BLACK, COLOR_GREEN);
      init_pair(3, COLOR_BLACK, COLOR_BLUE);
      init_pair(4, COLOR_WHITE, COLOR_RED);
      init_pair(5, COLOR_BLACK, COLOR_CYAN);

      int pcSectorNumber = 8;
      WINDOW *PC_sector[pcSectorNumber];

      int routerSectorNumber = 4;
      WINDOW *R_sector[routerSectorNumber];

      int PC_SendPipeSectorNumber = 8;
      WINDOW *PC_SendPipeSector[PC_SendPipeSectorNumber];

      int PC_ReceivePipeSectorNumber = 8;
      WINDOW *PC_ReceivePipeSector[PC_ReceivePipeSectorNumber];

      int pcSizeX = 3;
      int pcSizeY = 6;
      int transferPipeSizeX = 6;
      int transferPipeSizeY = 12;

      PC_sector[0] = newwin(pcSizeX, pcSizeY, 1, 1 + 1 * pcSizeY + 1 * transferPipeSizeY);
      PC_sector[1] = newwin(pcSizeX, pcSizeY, 1, 1 + 2 * pcSizeY + 2 * transferPipeSizeY);
      PC_sector[2] = newwin(pcSizeX, pcSizeY, 1 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 3 * pcSizeY + 3 * transferPipeSizeY);
      PC_sector[3] = newwin(pcSizeX, pcSizeY, 1 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 3 * pcSizeY + 3 * transferPipeSizeY);
      PC_sector[4] = newwin(pcSizeX, pcSizeY, 1 + 3 * pcSizeX + 3 * transferPipeSizeX, 1 + 2 * pcSizeY + 2 * transferPipeSizeY);
      PC_sector[5] = newwin(pcSizeX, pcSizeY, 1 + 3 * pcSizeX + 3 * transferPipeSizeX, 1 + 1 * pcSizeY + 1 * transferPipeSizeY);
      PC_sector[6] = newwin(pcSizeX, pcSizeY, 1 + 2 * pcSizeX + 2 * transferPipeSizeX, 1);
      PC_sector[7] = newwin(pcSizeX, pcSizeY, 1 + 1 * pcSizeX + 1 * transferPipeSizeX, 1);

      R_sector[0] = newwin(pcSizeX, pcSizeY, 1 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 1 * pcSizeY + 1 * transferPipeSizeY);
      R_sector[1] = newwin(pcSizeX, pcSizeY, 1 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 2 * pcSizeY + 2 * transferPipeSizeY);
      R_sector[2] = newwin(pcSizeX, pcSizeY, 1 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 2 * pcSizeY + 2 * transferPipeSizeY);
      R_sector[3] = newwin(pcSizeX, pcSizeY, 1 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 1 * pcSizeY + 1 * transferPipeSizeY);

      PC_SendPipeSector[0] = newwin(transferPipeSizeX, 1, 1 + 1 * pcSizeX + 0 * transferPipeSizeX, 2 + 1 * pcSizeY + 1 * transferPipeSizeY);
      PC_SendPipeSector[1] = newwin(transferPipeSizeX, 1, 1 + 1 * pcSizeX + 0 * transferPipeSizeX, 2 + 2 * pcSizeY + 2 * transferPipeSizeY);
      PC_SendPipeSector[2] = newwin(1, transferPipeSizeY, 1 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 3 * pcSizeY + 2 * transferPipeSizeY);
      PC_SendPipeSector[3] = newwin(1, transferPipeSizeY, 1 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 3 * pcSizeY + 2 * transferPipeSizeY);
      PC_SendPipeSector[4] = newwin(transferPipeSizeX, 1, 1 + 3 * pcSizeX + 2 * transferPipeSizeX, 2 + 2 * pcSizeY + 2 * transferPipeSizeY);
      PC_SendPipeSector[5] = newwin(transferPipeSizeX, 1, 1 + 3 * pcSizeX + 2 * transferPipeSizeX, 2 + 1 * pcSizeY + 1 * transferPipeSizeY);
      PC_SendPipeSector[6] = newwin(1, transferPipeSizeY, 1 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 1 * pcSizeY + 0 * transferPipeSizeY);
      PC_SendPipeSector[7] = newwin(1, transferPipeSizeY, 1 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 1 * pcSizeY + 0 * transferPipeSizeY);

      PC_ReceivePipeSector[0] = newwin(transferPipeSizeX, 1, 1 + 1 * pcSizeX + 0 * transferPipeSizeX, 5 + 1 * pcSizeY + 1 * transferPipeSizeY);
      PC_ReceivePipeSector[1] = newwin(transferPipeSizeX, 1, 1 + 1 * pcSizeX + 0 * transferPipeSizeX, 5 + 2 * pcSizeY + 2 * transferPipeSizeY);
      PC_ReceivePipeSector[2] = newwin(1, transferPipeSizeY, 3 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 3 * pcSizeY + 2 * transferPipeSizeY);
      PC_ReceivePipeSector[3] = newwin(1, transferPipeSizeY, 3 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 3 * pcSizeY + 2 * transferPipeSizeY);
      PC_ReceivePipeSector[4] = newwin(transferPipeSizeX, 1, 1 + 3 * pcSizeX + 2 * transferPipeSizeX, 5 + 2 * pcSizeY + 2 * transferPipeSizeY);
      PC_ReceivePipeSector[5] = newwin(transferPipeSizeX, 1, 1 + 3 * pcSizeX + 2 * transferPipeSizeX, 5 + 1 * pcSizeY + 1 * transferPipeSizeY);
      PC_ReceivePipeSector[6] = newwin(1, transferPipeSizeY, 3 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 1 * pcSizeY + 0 * transferPipeSizeY);
      PC_ReceivePipeSector[7] = newwin(1, transferPipeSizeY, 3 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 1 * pcSizeY + 0 * transferPipeSizeY);

      for (int i = 0; i < pcSectorNumber; i++)
      {
            wbkgd(PC_sector[i], COLOR_PAIR(2));
            string pcname = "PC ";
            pcname.append(to_string(i));
            mvwprintw(PC_sector[i], 1, 1, pcname.c_str());
      }

      for (int i = 0; i < routerSectorNumber; i++)
      {
            wbkgd(R_sector[i], COLOR_PAIR(3));
            string rname = "R ";
            rname.append(to_string(i));
            mvwprintw(R_sector[i], 1, 1, rname.c_str());
      }
      for (int i = 0; i < PC_SendPipeSectorNumber; i++)
      {
            wbkgd(PC_SendPipeSector[i], COLOR_PAIR(4));
            wbkgd(PC_ReceivePipeSector[i], COLOR_PAIR(5));

            mvwprintw(PC_SendPipeSector[i], 0, 0, "aaaaaaaa");
      }
      for (int i = 0; i < PC_ReceivePipeSectorNumber; i++)
      {
            wbkgd(PC_ReceivePipeSector[i], COLOR_PAIR(5));
            mvwprintw(PC_ReceivePipeSector[i], 0, 0, "bbbbbbbbb");
      }

      WINDOW *progress_bar = newwin(15, 1, 5, 0);
      wrefresh(progress_bar);

      while (!nCursesEndFlag)
      {
            //cursesRefresh();
            this_thread::sleep_for(chrono::milliseconds(1000));
            wresize(progress_bar, _progress, 1);
            wclear(stdscr);
            wclear(progress_bar);
            //appTime+=50;
            _progress += 1;
            wbkgd(progress_bar, COLOR_PAIR(2));
            wrefresh(progress_bar);
            if (_progress > 15)
            {

                  wbkgd(progress_bar, COLOR_PAIR(1));
                  wrefresh(progress_bar);
                  _progress = 1;
            }
            for (int i = 0; i < pcSectorNumber; i++)
            {
                  wrefresh(PC_sector[i]);
            }
            for (int i = 0; i < routerSectorNumber; i++)
            {
                  wrefresh(R_sector[i]);
            }
            for (int i = 0; i < PC_SendPipeSectorNumber; i++)
            {
                  wrefresh(PC_SendPipeSector[i]);
            }
            for (int i = 0; i < PC_ReceivePipeSectorNumber; i++)
            {
                  wrefresh(PC_ReceivePipeSector[i]);
            }
      }

      delwin(progress_bar);

      for (int i = 0; i < pcSectorNumber; i++)
      {
            delwin(PC_sector[i]);
      }

      for (int i = 0; i < routerSectorNumber; i++)
      {
            delwin(R_sector[i]);
      }
      for (int i = 0; i < PC_SendPipeSectorNumber; i++)
      {
            delwin(PC_SendPipeSector[i]);
      }
      for (int i = 0; i < PC_ReceivePipeSectorNumber; i++)
      {
            delwin(PC_ReceivePipeSector[i]);
      }
}
int main()
{
      thread window;
      window = thread(runWindow);
      //runWindow();
      /*
      srand(time(NULL));
      thread philosopher[N];
      for (int i = 0; i < N; i++)
      {
            philosopher[i] = thread(Philosopher, i);
      }
*/ getchar();
      endFlag = true;
      /*      for (int i = 0; i < N; i++)
      {
            philosopher[i].join();
      };
*/ nCursesEndFlag = true;
      window.join();
      endwin();
      printf("Dinner is completed...\n");
      return 0;
}