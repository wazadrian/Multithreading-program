#include <thread>
#include <stdio.h>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <atomic>
#include <ncurses.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <sstream>
using namespace std;

//config
const int PC = 8;
const int R = 4;
const int RPCP = 8;
const int BPCP = 8;
const int RRP = 4;
const int BRP = 4;
const int PLIMIT = 10;
const int DELAY = 3;
//
class Packet;
int Send(int, string);
void PrintToConsole(string);

atomic<bool> endFlag(false);
atomic<int> packetId(0);

bool nCursesEndFlag = false;
int appTime = 0;

string redPcPipeLine[RPCP];
mutex redPcPipeLineMutex[RPCP];
string bluePcPipeLine[BPCP];
mutex bluePcPipeLineMutex[BPCP];
string redRPipeLine[RRP];
mutex redRPipeLineMutex[RRP];
string blueRPipeLine[BRP];
mutex blueRPipeLineMutex[BRP];

vector<thread> packetThreads;
mutex consoleInput;
mutex packetGen;
mutex uniqueIdGen;

vector<string> console;
vector<Packet> packetList;
queue<int> buffer[R];
bool routerActive[4];
mutex bufferMutex[R];

pthread_cond_t routerCond[R];
pthread_mutex_t routerMutex[R];

int packetPosition[300][3];
int routerPacketId[4];
int routerProgress[4];
string routerPacketMsg[4];

enum pos2
{
      pc,
      r,
      lost
};
enum pos1
{
      red,
      blue
};

class Packet
{
    public:
      int _id;
      int _source;
      int _destination;
      string _message;
      Packet(int source, int destination, string message)
      {
            uniqueIdGen.lock();
            _id = packetId;
            packetId++;
            uniqueIdGen.unlock();
            _source = source;
            packetPosition[_id][2] = pc;     // 0 - PC, 1 - R
            packetPosition[_id][1] = red;    // 0 - red, 1 - blue
            packetPosition[_id][0] = source; // line number
            _destination = destination;
            _message = message;
      }
      void PacketRun()
      {
            {
                  PrintToConsole("Packet created, id: " + to_string(_id) + " source: " + to_string(_source) + " destination: " + to_string(_destination));
                  if (packetPosition[_id][2] == pc)
                  {
                        if (packetPosition[_id][1] == red)
                        {
                              if (Send(_id, (to_string(_id) + " " +_message)) == -1)
                              {
                                    PrintToConsole("Packet sent but router failed.");
                              }
                        }
                  }
            }
      }
};

void runWindow()
{

      int parent_x, parent_y, new_x, new_y;

      getmaxyx(stdscr, parent_y, parent_x);
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
      init_pair(6, COLOR_WHITE, COLOR_BLUE);

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
            /*
            R_ReceivePipeSector[0] = newwin(1, transferPipeSizeY, 3 + 1 * pcSizeX + 1 * transferPipeSizeX, 1 + 2 * pcSizeY + 1 * transferPipeSizeY);
            R_ReceivePipeSector[1] = newwin(transferPipeSizeX, 1, 1 + 2 * pcSizeX + 1 * transferPipeSizeX, 5 + 2 * pcSizeY + 2 * transferPipeSizeY);
            R_ReceivePipeSector[2] = newwin(1, transferPipeSizeY, 3 + 2 * pcSizeX + 2 * transferPipeSizeX, 1 + 2 * pcSizeY + 1 * transferPipeSizeY);
            R_ReceivePipeSector[3] = newwin(transferPipeSizeX, 1, 1 + 2 * pcSizeX + 1 * transferPipeSizeX, 5 + 1 * pcSizeY + 1 * transferPipeSizeY);
            */
            for (int i = 0; i < pcSectorNumber; i++)
            {
                  wbkgd(PC_sector[i], COLOR_PAIR(2));
                  string pcname = "PC ";
                  pcname.append(to_string(i));
                  mvwprintw(PC_sector[i], 1, 1, pcname.c_str());
            }

            for (int i = 0; i < routerSectorNumber; i++)
            {
                  routerActive[i] = false;
                  wbkgd(R_sector[i], COLOR_PAIR(3));
                  string rname = "R";
                  rname.append(to_string(i));
                  mvwprintw(R_sector[i], 0, 4, "0%%");
                  mvwprintw(R_sector[i], 0, 0, rname.c_str());
            }
            for (int i = 0; i < PC_SendPipeSectorNumber; i++)
            {
                  wbkgd(PC_SendPipeSector[i], COLOR_PAIR(4));
            }
            for (int i = 0; i < PC_ReceivePipeSectorNumber; i++)
            {
                  wbkgd(PC_ReceivePipeSector[i], COLOR_PAIR(5));
            }
            for (int i = 0; i < R_SendPipeSectorNumber; i++)
            {
                  wbkgd(R_SendPipeSector[i], COLOR_PAIR(4));
            }
            for (int i = 0; i < R_ReceivePipeSectorNumber; i++)
            {
                  wbkgd(R_ReceivePipeSector[i], COLOR_PAIR(5));
            }
      }
      WINDOW *consoleLog = newwin(50, 50, 1, 70);
      WINDOW *stats = newwin(50, 50, 1, 120);
      WINDOW *progress_bar = newwin(15, 1, 5, 0);

      {
            while (!nCursesEndFlag)
            {

                  refresh();
                  this_thread::sleep_for(chrono::milliseconds(100));

                  mvwprintw(stats, 0, 0, "Packet amount: %d", packetThreads.size());
                  for (int i = 0; i < 4; i++)
                  {
                        mvwprintw(stats, 1 + i, 0, "Router %d, buffer elements: %d", i, buffer[i].size());
                  }

                  for (int i = 0; i < packetList.size(); i++)
                  {
                        //mvwprintw(stats, 5 + i, 0, "Packet %d, pos: %d:%d:%d", packetList[i]._id, packetPosition[i][2], packetPosition[i][1], packetPosition[i][0]);
                        string status = "Unknown";  
                        if(packetPosition[i][2]==lost)
                        {
                              status = "packet has been lost";
                        }                       
                        if(packetPosition[i][2]==pc && packetPosition[i][1]==blue)
                        {
                              status = "is near finish";
                        }
                        else if(packetPosition[i][2]==r)
                        {
                              status = "just left Rout";
                        }
                        else if(packetPosition[i][2]==pc)
                        {
                              status = "just left PC";
                        }
                        else if(packetPosition[i][2] == 9)
                        {
                              status = "Delivered to " + to_string(packetPosition[i][0]);
                        }

                        string msg = "Packet id:" + to_string(packetList[i]._id) + ", status: " + status;
                        mvwprintw(stats, 5 + i, 0, msg.c_str());
                  }

                  wresize(progress_bar, _progress, 1);
                  appTime += 1000;

                  mvprintw(1, 1, "Time: %d", appTime);
                  _progress += 1;
                  wbkgd(progress_bar, COLOR_PAIR(2));
                  wrefresh(progress_bar);
                  std::ostringstream consoleString;
                  copy(console.begin(), console.end(), ostream_iterator<string>(consoleString, "\n"));
                  mvwprintw(consoleLog, 0, 0, consoleString.str().c_str());

                  wrefresh(consoleLog);
                  wrefresh(stats);
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
                        if (routerActive[i])
                        {
                              wbkgd(R_sector[i], COLOR_PAIR(6));
                        }
                        else
                        {
                              wbkgd(R_sector[i], COLOR_PAIR(3));
                        }
                        mvwprintw(R_sector[i], 0, 3, to_string(routerProgress[i]).c_str());
                        mvwprintw(R_sector[i], 1, 0, "  ");
                        mvwprintw(R_sector[i], 1, 0, to_string(routerPacketId[i]).c_str());
                        mvwprintw(R_sector[i], 2, 0, routerPacketMsg[i].c_str());
                        wrefresh(R_sector[i]);
                  }
                  for (int i = 0; i < PC_SendPipeSectorNumber; i++)
                  {

                        mvwprintw(PC_SendPipeSector[i], 0, 0, redPcPipeLine[i].c_str());
                        wrefresh(PC_SendPipeSector[i]);
                  }
                  for (int i = 0; i < PC_ReceivePipeSectorNumber; i++)
                  {
                        mvwprintw(PC_ReceivePipeSector[i], 0, 0, bluePcPipeLine[i].c_str());
                        wrefresh(PC_ReceivePipeSector[i]);
                  }
                  for (int i = 0; i < R_SendPipeSectorNumber; i++)
                  {
                        mvwprintw(R_SendPipeSector[i], 0, 0, redRPipeLine[i].c_str());
                        wrefresh(R_SendPipeSector[i]);
                  }
                  for (int i = 0; i < R_ReceivePipeSectorNumber; i++)
                  {
                        mvwprintw(R_ReceivePipeSector[i], 0, 0, blueRPipeLine[i].c_str());
                        wrefresh(R_ReceivePipeSector[i]);
                  }
            }

            delwin(progress_bar);
            delwin(consoleLog);
            delwin(stats);

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
      consoleInput.lock();
      if (console.size() > 30)
            console.erase(console.begin());
      console.push_back(text);
      consoleInput.unlock();
}

int Send(int packetId, string message)
{
      if (packetPosition[packetId][2] == pc)
      {
            if (packetPosition[packetId][1] == red)
            {
                  if (redPcPipeLineMutex[packetPosition[packetId][0]].try_lock())
                  {
                        int initPos = packetPosition[packetId][0];
                        redPcPipeLine[initPos] = message;
                        this_thread::sleep_for(chrono::seconds(DELAY));
                        if (bufferMutex[(initPos % 7 + 1) / 2].try_lock())
                        {
                              buffer[(initPos % 7 + 1) / 2].push(packetId);
                              bufferMutex[(initPos % 7 + 1) / 2].unlock();
                        }
                        else
                        {
                              PrintToConsole("Red Pc pipeline " + to_string(packetId) + " couldn't access buffer");
                              packetPosition[packetId][2] = lost;
                        }
                        redPcPipeLine[initPos] = "            ";
                        redPcPipeLineMutex[initPos].unlock();
                  }
                  else
                  {
                        PrintToConsole("Packet " + to_string(packetId) + " lost");
                        packetPosition[packetId][2] = lost;
                  }
            }
            else if (packetPosition[packetId][1] == blue)
            {
                  if (bluePcPipeLineMutex[packetPosition[packetId][0]].try_lock())
                  {
                        int initPos = packetPosition[packetId][0];
                        bluePcPipeLine[initPos] = message;
                        this_thread::sleep_for(chrono::seconds(DELAY));
                        bluePcPipeLine[initPos] = "            ";
                        bluePcPipeLineMutex[initPos].unlock();
                        PrintToConsole("PC " +
                                       to_string(packetPosition[packetId][0]) +
                                       " received packet id:" +
                                       message);
                        packetPosition[packetId][2] = 9;
                        packetPosition[packetId][1] = 9;
                  }
                  else
                  {
                        PrintToConsole("Packet " + to_string(packetId) + " lost");
                        packetPosition[packetId][2] = lost;
                  }
            }
      }
      else if (packetPosition[packetId][2] == r)
      {
            if (packetPosition[packetId][1] == red)
            {
                  if (redRPipeLineMutex[packetPosition[packetId][0]].try_lock())
                  {
                        int initPos = packetPosition[packetId][0];
                        redRPipeLine[initPos] = message;
                        this_thread::sleep_for(chrono::seconds(DELAY));
                        if (bufferMutex[(initPos + 1) % 4].try_lock())
                        {
                              buffer[(initPos + 1) % 4].push(packetId);
                              bufferMutex[(initPos + 1) % 4].unlock();
                        }
                        else
                        {
                              PrintToConsole("Pipeline " + to_string(initPos) + " couldn't get access to buffer");
                              packetPosition[packetId][2] = lost;
                        }
                        redRPipeLine[initPos] = "            ";
                        redRPipeLineMutex[initPos].unlock();
                  }
                  else
                  {
                        PrintToConsole("Packet " + to_string(packetId) + " lost");
                        packetPosition[packetId][2] = lost;
                  }
            }
      }
      return 0;
}

void GenerateNewPacket(int id)
{
      packetGen.lock();
      if(packetId < 32) 
      {
            int rnd = rand() % 8;
            packetList.push_back(*(new Packet(id, rnd, "To" + to_string(rnd))));
            packetThreads.push_back(thread(&Packet::PacketRun, packetList.back()));
      }
      packetGen.unlock();
}

void Pc(int id)
{
      PrintToConsole("PC " + to_string(id) + " initialized");

      while (!endFlag)
      {
            this_thread::sleep_for(chrono::seconds(DELAY));
            GenerateNewPacket(id);
            this_thread::sleep_for(chrono::seconds(10));
      }
}

class Router
{
    public:
      int _id;
      int _host[2];
      Router(int id, int host0, int host1)
      {
            _id = id;
            _host[0] = host0;
            _host[1] = host1;
      }
      void RouterRun()
      {
            PrintToConsole("Router " + to_string(_id) + " initialized");
            while (!endFlag)
            {
                  routerActive[_id] = false;
                  this_thread::sleep_for(chrono::seconds(1));
                  routerActive[_id] = true;
                  this_thread::sleep_for(chrono::seconds(1));
                  while (buffer[_id].size() > 0)
                  {
                        if (bufferMutex[_id].try_lock())
                        {
                              int packetId = buffer[_id].front();
                              buffer[_id].pop();
                              bufferMutex[_id].unlock();
                              routerPacketId[_id] = packetId;
                              routerPacketMsg[_id] = packetList[packetId]._message;
                              for(int i = 1; i < 10; i++)
                              {
                                    routerProgress[_id] = i;
                                    this_thread::sleep_for(chrono::milliseconds(DELAY*250));
                              }
                              routerProgress[_id] = 0;
                              routerPacketId[_id] = 0;
                              routerPacketMsg[_id] = "";
                              if (packetList[packetId]._destination == _host[0])
                              {
                                    packetPosition[packetId][2] = pc;
                                    packetPosition[packetId][1] = blue;
                                    packetPosition[packetId][0] = _host[0];
                              }
                              else if (packetList[packetId]._destination == _host[1])
                              {

                                    packetPosition[packetId][2] = pc;
                                    packetPosition[packetId][1] = blue;
                                    packetPosition[packetId][0] = _host[1];
                              }
                              else
                              {
                                    packetPosition[packetId][2] = r;
                                    packetPosition[packetId][1] = red;
                                    packetPosition[packetId][0] = _id;
                              }
                              if (Send(packetId, (to_string(packetId) + " " + packetList[packetId]._message)) == -1)
                              {
                                    PrintToConsole("Router " + to_string(_id) + " send failed " + to_string(packetId));
                                    packetPosition[packetId][2] = lost;
                              }
                        }
                        else
                        {
                              PrintToConsole("Router " + to_string(_id) + " couldn't access buffer");
                              packetPosition[packetId][2] = lost;
                        }
                  }
                  routerActive[_id] = false;
            }
      }
};

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
      thread threadRouter[R];
      Router router[4] = {
          Router(0, 7, 0),
          Router(1, 1, 2),
          Router(2, 3, 4),
          Router(3, 5, 6),
      };
      for (int i = 0; i < R; i++)
      {
            threadRouter[i] = thread(&Router::RouterRun, router[i]);
      }
      getchar();

      endFlag = true;

      PrintToConsole("SYSTEM: shutdown PCs...");
      for (int i = 0; i < PC; i++)
      {
            pc[i].join();
      };

      PrintToConsole("SYSTEM: shutdown Packets...");
      for (int i = 0; i < packetThreads.size(); i++)
      {
            packetThreads[i].join();
      };

      PrintToConsole("SYSTEM: shutdown Routers...");
      for (int i = 0; i < R; i++)
      {
            pthread_mutex_lock(&routerMutex[i]);
            pthread_cond_signal(&routerCond[i]);
            pthread_mutex_unlock(&routerMutex[i]);
            threadRouter[i].join();
      };

      nCursesEndFlag = true;
      window.join();
      endwin();
      printf("Program is completed...");
      return 0;
}