#include <thread>
#include <stdio.h>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <atomic>
#include <ncurses.h>
#include <vector>
#include <iterator>
#include <iostream>
#include <sstream>
using namespace std;

atomic<bool> endFlag(false);
bool nCursesEndFlag = false;
int appTime = 0;

//config
const int PC = 8;
const int R = 4;
const int RPCP = 8;
const int BPCP = 8;
const int RRP = 4;
const int BRP = 4;
const int PLIMIT = 10;
//

string redPcPipeLine[RPCP];
string bluePcPipeLine[BPCP];
string redRPipeLine[RRP];
string blueRPipeLine[BRP];

//thread packet[PLIMIT];
vector<thread> packet;
vector<string> console;
mutex consoleInput;
mutex packetGen;


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

      int pcSectorNumber = PC;
      WINDOW *PC_sector[pcSectorNumber];

      int routerSectorNumber = R;
      WINDOW *R_sector[routerSectorNumber];

      int PC_SendPipeSectorNumber = RPCP;
      WINDOW *PC_SendPipeSector[PC_SendPipeSectorNumber];

      int PC_ReceivePipeSectorNumber = BPCP;
      WINDOW *PC_ReceivePipeSector[PC_ReceivePipeSectorNumber];

      int R_SendPipeSectorNumber = RRP;
      WINDOW *R_SendPipeSector[R_SendPipeSectorNumber];

      int R_ReceivePipeSectorNumber = BRP;
      WINDOW *R_ReceivePipeSector[R_ReceivePipeSectorNumber];

      int pcSizeX = 3;
      int pcSizeY = 6;
      int transferPipeSizeX = 6;
      int transferPipeSizeY = 12;
      {
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

            R_SendPipeSector[0] = newwin(1, transferPipeSizeY, 1 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 2 * pcSizeY + 1 * transferPipeSizeY);
            R_SendPipeSector[1] = newwin(transferPipeSizeX, 1, 1 + 2 * pcSizeX + 1 * transferPipeSizeX, 2 + 2 * pcSizeY + 2 * transferPipeSizeY);
            R_SendPipeSector[2] = newwin(1, transferPipeSizeY, 1 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 2 * pcSizeY + 1 * transferPipeSizeY);
            R_SendPipeSector[3] = newwin(transferPipeSizeX, 1, 1 + 2 * pcSizeX + 1 * transferPipeSizeX, 2 + 1 * pcSizeY + 1 * transferPipeSizeY);

            R_ReceivePipeSector[0] = newwin(1, transferPipeSizeY, 3 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 2 * pcSizeY + 1 * transferPipeSizeY);
            R_ReceivePipeSector[1] = newwin(transferPipeSizeX, 1, 1 + 2 * pcSizeX + 1 * transferPipeSizeX, 5 + 2 * pcSizeY + 2 * transferPipeSizeY);
            R_ReceivePipeSector[2] = newwin(1, transferPipeSizeY, 3 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 2 * pcSizeY + 1 * transferPipeSizeY);
            R_ReceivePipeSector[3] = newwin(transferPipeSizeX, 1, 1 + 2 * pcSizeX + 1 * transferPipeSizeX, 5 + 1 * pcSizeY + 1 * transferPipeSizeY);

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
                  //string text = redPcPipeLine[i].;
                  mvwprintw(PC_SendPipeSector[i], 0, 0, redPcPipeLine[i].c_str());
            }
            for (int i = 0; i < PC_ReceivePipeSectorNumber; i++)
            {
                  wbkgd(PC_ReceivePipeSector[i], COLOR_PAIR(5));
                  mvwprintw(PC_ReceivePipeSector[i], 0, 0, bluePcPipeLine[i].c_str());
            }
            for (int i = 0; i < R_SendPipeSectorNumber; i++)
            {
                  wbkgd(R_SendPipeSector[i], COLOR_PAIR(4));
                  mvwprintw(R_SendPipeSector[i], 0, 0, redRPipeLine[i].c_str());
            }
            for (int i = 0; i < R_ReceivePipeSectorNumber; i++)
            {
                  wbkgd(R_ReceivePipeSector[i], COLOR_PAIR(5));
                  mvwprintw(R_ReceivePipeSector[i], 0, 0, redRPipeLine[i].c_str());
            }
      }

      WINDOW *consoleLog = newwin(50, 50, 1, 70);

      std::ostringstream consoleString;
      copy(console.begin(),console.end(), ostream_iterator<string>(consoleString,"\n"));
      mvwprintw(consoleLog, 0, 0, consoleString.str().c_str());

      WINDOW *progress_bar = newwin(15, 1, 5, 0);
      wrefresh(progress_bar);

      {
            while (!nCursesEndFlag)
            {
                  //cursesRefresh();
                  this_thread::sleep_for(chrono::milliseconds(1000));
                  wresize(progress_bar, _progress, 1);
                  wclear(stdscr);
                  wclear(progress_bar);
                  _progress += 1;
                  wbkgd(progress_bar, COLOR_PAIR(2));
                  wrefresh(progress_bar);
                  if (_progress > 15)
                  {

                        wbkgd(progress_bar, COLOR_PAIR(1));
                        wrefresh(progress_bar);
                        _progress = 1;
                  }

                  wrefresh(consoleLog);
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
                  for (int i = 0; i < R_SendPipeSectorNumber; i++)
                  {
                        wrefresh(R_SendPipeSector[i]);
                  }
                  for (int i = 0; i < R_ReceivePipeSectorNumber; i++)
                  {
                        wrefresh(R_ReceivePipeSector[i]);
                  }
            }

            delwin(progress_bar);
            delwin(consoleLog);

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
            for (int i = 0; i < R_SendPipeSectorNumber; i++)
            {
                  delwin(R_SendPipeSector[i]);
            }
            for (int i = 0; i < R_ReceivePipeSectorNumber; i++)
            {
                  delwin(R_ReceivePipeSector[i]);
            }
      }
}

void PrintToConsole(string text)
{
      // console = text + "\n" + console;
      consoleInput.lock();
      //console.
      if(console.size() > 30) console.erase(console.begin()); 
      console.push_back(text);
      consoleInput.unlock();
}

void Packet(int PCid)
{
      PrintToConsole("Packet from " + to_string(PCid) + " is active");
}

void GenerateNewPacket(int id)
{
      packetGen.lock();
      packet.push_back(thread(Packet, id));
      packetGen.unlock();
      PrintToConsole("Packet generated by PC " + to_string(id));
}

void Pc(int id)
{
      PrintToConsole("PC " + to_string(id) + " is running");


      while(!endFlag)
      {

      PrintToConsole("test");
      GenerateNewPacket(id);
      this_thread::sleep_for(chrono::milliseconds(1000));
      }
}

void Router(int id)
{
      PrintToConsole("Router " + to_string(id) + " is running");
}

int main()
{
      thread window;
      window = thread(runWindow);
      //runWindow();

      srand(time(NULL));

      PrintToConsole("SYSTEM: Starting PCs...");

      thread pc[PC];
      for (int i = 0; i < PC; i++)
      {
            pc[i] = thread(Pc, i);
      }
      PrintToConsole("SYSTEM: Starting Routers...");
      thread router[R];
      for (int i = 0; i < R; i++)
      {
            router[i] = thread(Router, i);
      }
      getchar();

      endFlag = true;


      PrintToConsole("SYSTEM: shutdown PCs...");
      for (int i = 0; i < PC; i++)
      {
            pc[i].join();
      };

      PrintToConsole("SYSTEM: shutdown Routers...");
      for (int i = 0; i < R; i++)
      {
            router[i].join();
      };

            PrintToConsole("SYSTEM: shutdown Packets...");
      for (int i = 0; i < packet.size(); i++)
      {
            packet[i].join();
      };
      nCursesEndFlag = true;
      window.join();
      endwin();
      printf("Program is completed...");
      return 0;
}