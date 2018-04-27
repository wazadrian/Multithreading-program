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
const int PC = 1;
const int R = 4;
const int RPCP = 8;
const int BPCP = 8;
const int RRP = 4;
const int BRP = 4;
const int PLIMIT = 10;
//
class Packet;
void Send(Packet &);
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

vector<string> console;
vector<Packet> packetList;
queue<Packet *> buffer[R];
mutex bufferMutex[R];

pthread_cond_t routerCond[R];
pthread_mutex_t routerMutex[R];
//pthread_cond_t routerCond = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t routerMutex = PTHREAD_MUTEX_INITIALIZER;

enum pos2
{
      pc,
      r
};
enum pos1
{
      red,
      blue
};
enum status
{
      created,
      inPipe,
      inRouter,
};

class Packet
{
    public:
      int _id;
      int _source;
      int _destination;
      int _position[3];
      int _status;
      string _message;
      Packet(int source, int destination)
      {
            _id = packetId;
            packetId++;
            _source = source;
            _position[2] = pc;     // 0 - PC, 1 - R
            _position[1] = blue;    // 0 - red, 1 - blue
            _position[0] = source; // line number
            _destination = destination;
            _status = created;
      }
      void PacketRun(string message)
      {
            // while (true)
            {
            _id = 654;
                  _message = message;
                  PrintToConsole("Packet created, id: " +  to_string(_id) + " source: " + to_string(_source) + " destination: " + to_string(_destination));
                  if (_status == created)
                  {

                        //mvprintw(2, 1, "this: %p", this);
                        //Packet * self = this;
                        //      Send(*self);
                        _position[2] = 5;
                        mvprintw(3, 1, "send: %p", this);
                        if (_position[2] == pc)
                        {
                              if (_position[1] == red)
                              {
                                    redPcPipeLineMutex[_position[0]].lock();
                                    int initPos = _position[0];
                                    redPcPipeLine[initPos] = _message;
                                    _status = inPipe;
                                    this_thread::sleep_for(chrono::milliseconds(3000));
                                    bufferMutex[initPos / 2].lock();
                                    buffer[initPos / 2].push(this);
                                    _status = inRouter;
                                    bufferMutex[initPos / 2].unlock();
                                    pthread_mutex_lock(&routerMutex[initPos / 2]);
                                    pthread_cond_signal(&routerCond[initPos / 2]);
                                    pthread_mutex_unlock(&routerMutex[initPos / 2]);

                                    redPcPipeLine[initPos] = "____________________";
                                    redPcPipeLineMutex[initPos].unlock();
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
                        mvwprintw(stats, 5 + i, 0, "Packet %d, pos: %d:%d:%d", packetList[i]._id, packetList[i]._position[2], packetList[i]._position[1], packetList[i]._position[0]);
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
                        mvwprintw(R_ReceivePipeSector[i], 0, 0, redRPipeLine[i].c_str());
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

void Send(Packet &packet)
{
      /*    if (packet._status = -1)
      {
            redPcPipeLineMutex[packet._position].lock();
            redPcPipeLine[packet._position] = packet._message;
            packet._position = 
            redPcPipeLineMutex[packet._position].unlock();
      }
      */
      // packet._message = "saf";
      packet._position[2] = 5;
      mvprintw(3, 1, "send: %p", &packet);
      if (packet._position[2] == pc)
      {
            if (packet._position[1] == red)
            {
                  redPcPipeLineMutex[packet._position[0]].lock();
                  int initPos = packet._position[0];
                  redPcPipeLine[initPos] = packet._message;
                  packet._status = inPipe;
                  this_thread::sleep_for(chrono::milliseconds(3000));
                  bufferMutex[initPos / 2].lock();
                  buffer[initPos / 2].push(&packet);
                  packet._status = inRouter;
                  bufferMutex[initPos / 2].unlock();
                  pthread_mutex_lock(&routerMutex[initPos / 2]);
                  pthread_cond_signal(&routerCond[initPos / 2]);
                  pthread_mutex_unlock(&routerMutex[initPos / 2]);

                  redPcPipeLine[initPos] = "____________________";
                  redPcPipeLineMutex[initPos].unlock();
            }
      }
}

void GenerateNewPacket(int id)
{
      packetGen.lock();
      packetList.push_back(*(new Packet(id, 1)));
      packetThreads.push_back(thread(&Packet::PacketRun, packetList.back(), "abc " + to_string(id)));
      packetGen.unlock();
}

void Pc(int id)
{
      PrintToConsole("PC " + to_string(id) + " initialized");

      while (!endFlag)
      {
            this_thread::sleep_for(chrono::milliseconds(10000));
            GenerateNewPacket(id);
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
                  pthread_mutex_lock(&routerMutex[_id]);
                  PrintToConsole("Router " + to_string(_id) + " is sleeping");
                  pthread_cond_wait(&routerCond[_id], &routerMutex[_id]);
                  PrintToConsole("Router " + to_string(_id) + " awake");
                  while (buffer[_id].size() > 0)
                  {

                        bufferMutex[_id].lock();
                        Packet *packet = buffer[_id].front();
                        buffer[_id].pop();

                        bufferMutex[_id].unlock();
                        if ((*packet)._position[2] == pc &&
                            (*packet)._position[1] == red &&
                            ((*packet)._position[1] == _host[0] ||
                             (*packet)._position[1] == _host[1]))
                        {
                              if ((*packet)._destination == _host[0])
                              {
                                    // wyślij na host 0

                                    (*packet)._position[2] = r;
                                    (*packet)._position[1] = blue;
                                    (*packet)._position[0] = _host[0];
                              }
                              else if ((*packet)._destination == _host[1])
                              {
                                    // wyślij na host 1

                                    (*packet)._position[2] = r;
                                    (*packet)._position[1] = blue;
                                    (*packet)._position[0] = _host[1];
                                    //mvprintw(2, 1, "test: %p", *packet);
                              }
                              else
                              {
                                    //wyślij w prawo
                                    // usun
                                    //packetGen.lock();
                                    //auto it = find(packetThreads.begin(), packetThreads.end(), (*packet));
                                    // if (it != packetThreads.end())
                                    {
                                          // packetThreads[distance(packetThreads.begin(),it)].join();
                                          //     packetThreads.erase(it);
                                    }
                                    //packetGen.unlock();
                              }
                              //Send(*packet);
                        }
                  }
                  pthread_mutex_unlock(&routerMutex[_id]);
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
          Router(0, 0, 1),
          Router(1, 2, 3),
          Router(2, 4, 5),
          Router(3, 6, 7),
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