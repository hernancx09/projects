
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>

struct options {
	int overall,alien,shots,bombs,bombchance;
};

struct play {
	int x,y;
	char disp;
};

struct aliens {
	int x,y;
	int px,py;
	int alive;
	char direction;
	char disp;
};

struct bomb {
	int x,y;
	int active;
	int loop;
	char disp;
};

struct shoot {
	int x,y;
	int active;
	char disp;
};

#define MAX_BOMBS 1000
void menu(struct options *settings);
void playgame(int, int);
void gameover(int win);

int main() {
   int choice = 10, difficulty, ashot;
   while(1){
   printf("Choose difficulty 1-5\n 1: easy\n 2: regular\n 3: hard\n 4: veteran\n 5: impossible\n");
   scanf("%d", &choice);
   if (choice >= 1 && choice <= 5) printf("Invalid input, try again");
   else{
      printf("\n INVALID CHOICE, TRY AGAIN\n\n");
      usleep(990000);
   }
   } 
   switch(choice)
   {
   case 1:
      difficulty = 15000;
      ashot = 30;
      break;
   case 2:
      difficulty = 12000;
      ashot = 8;
      break;
   case 3:
      difficulty = 10000;
      ashot = 5;
      break;
   case 4:
      difficulty = 7000;
      ashot = 3;
      break;
   case 5:
      difficulty = 1500;
      ashot = 1;
      break;
   default:
      difficulty = 15000;
      break;
   }
   playgame(difficulty, ashot);
   return 0;
}

void playgame(int difficulty, int ashot){
   struct play tank;
   struct aliens aliens[30];
   struct shoot shot[3];
   struct bomb bomb[MAX_BOMBS];
   struct options settings;
   unsigned int input, loops=0, i=0, j=0, shotcount=0, bombcount=0, aliencount=30;
   int random=0, score=0, win=-1;
   char tellscore[30];
   
   initscr();
   clear();
   noecho();
   cbreak();
   nodelay(stdscr,1);
   keypad(stdscr,1);
   srand(time(NULL)); 

  
   settings.overall = difficulty;
   settings.alien = 12;
   settings.shots = ashot;
   settings.bombs = 10;
   settings.bombchance = 5;

  
   tank.x = LINES - 1;
   tank.y = COLS / 2;
   tank.disp = 'A';

  
   for (i=0; i<10; ++i) {
      aliens[i].x = 1;
      aliens[i].y = i*3;
      aliens[i].px = 0;
      aliens[i].py = 0;
      aliens[i].disp = 'W';
      aliens[i].alive = 1;
      aliens[i].direction = 'r';
   }
   for (i=10; i<20; ++i) {
      aliens[i].x = 2;
      aliens[i].y = (i-10)*3;
      aliens[i].px = 0;
      aliens[i].py = 0;
      aliens[i].disp = 'W';
      aliens[i].alive = 1;
      aliens[i].direction = 'r';
   }
   for (i=20; i<30; ++i) {
      aliens[i].x = 3;
      aliens[i].y = (i-20)*3;
      aliens[i].px = 0;
      aliens[i].py = 0;
      aliens[i].disp = 'W';
      aliens[i].alive = 1;
      aliens[i].direction = 'r';
   }
   
  
   for (i=0; i<3; ++i) {
      shot[i].active = 0;
      shot[i].disp = '|';
   }
   
  
   for (i=0; i<MAX_BOMBS; ++i) {
      bomb[i].active = 0;
      bomb[i].disp = 'U';
      bomb[i].loop = 0;
   }
   
  
   move(0,(COLS/2)-9);
   addstr("--SPACE INVADERS--");
   move (0,1);
   addstr("SCORE: ");
   move(0,COLS-19);
   addstr("m = menu  q = quit");
   usleep(900000);
   
   while(1) {
     
      sprintf(tellscore, "%d", score);
      move(0,8);
      addstr(tellscore);
      
     
      move(tank.x,tank.y);
      addch(tank.disp);
      
     
      if (loops % settings.bombs == 0)
      for (i=0; i<MAX_BOMBS; ++i) {
         if (bomb[i].active == 1) {
            if (bomb[i].x < LINES) {
               if (bomb[i].loop != 0) {
                  move(bomb[i].x-1,bomb[i].y);
                  addch(' ');
               }
               else
                  ++bomb[i].loop;
               
               move(bomb[i].x,bomb[i].y);
               addch(bomb[i].disp);
               
               ++bomb[i].x;
            }
            else {
               bomb[i].active = 0;
               bomb[i].loop = 0;
               --bombcount;
               move(bomb[i].x - 1,bomb[i].y);
               addch(' ');
            }
         }
      }
      
     
      if (loops % settings.shots == 0)
      for (i=0; i<3; ++i) {
         if (shot[i].active == 1) {
            if (shot[i].x > 0) {
               if (shot[i].x < LINES - 2) {
                  move(shot[i].x + 1,shot[i].y);
                  addch(' ');
               }
               
               for (j=0; j<30; ++j) {
                  if (aliens[j].alive == 1 && aliens[j].x == shot[i].x && aliens[j].py == shot[i].y) {
                     score += 20;
                     aliens[j].alive = 0;
                     shot[i].active = 0;
                     --shotcount;
                     --aliencount;
                     move(aliens[j].px,aliens[j].py);
                     addch(' ');
                     break;
                  }
               }
               
               if (shot[i].active == 1) {
                  move(shot[i].x,shot[i].y);
                  addch(shot[i].disp);
                  
                  --shot[i].x;
               }
            }
            else {
               shot[i].active = 0;
               --shotcount;
               move(shot[i].x + 1,shot[i].y);
               addch(' ');
            }
         }
      }     
      
     
      if (loops % settings.alien == 0)
      for (i=0; i<30; ++i) {
         if (aliens[i].alive == 1) {
            move(aliens[i].px,aliens[i].py);
            addch(' ');
            
            move(aliens[i].x,aliens[i].y);
            addch(aliens[i].disp);
            
            aliens[i].px = aliens[i].x;
            aliens[i].py = aliens[i].y;
            
           
            random = 1+(rand()%100);
            if ((settings.bombchance - random) >= 0 && bombcount < MAX_BOMBS) {
               for (j=0; j<MAX_BOMBS; ++j) {
                  if (bomb[j].active == 0) {
                     bomb[j].active = 1;
                     ++bombcount;
                     bomb[j].x = aliens[i].x + 1;
                     bomb[j].y = aliens[i].y;
                     break;
                  }
               }
            }
            
           
            if (aliens[i].direction == 'l')
               --aliens[i].y;
            else if (aliens[i].direction == 'r')
               ++aliens[i].y;
               
           
            if (aliens[i].y == COLS - 2) {
               ++aliens[i].x;
               aliens[i].direction = 'l';
            }
            else if (aliens[i].y == 0) {
               ++aliens[i].x;
               aliens[i].direction = 'r';
            }
         }
      }
      
      
     
      if (aliencount == 0) {
         win = 1;
         break;
      }
      for (i=0; i<30; ++i) {
         if (aliens[i].x == LINES-1) {
            win = 0;
            break;
         }
      }
      for (i=0; i<MAX_BOMBS; ++i) {
         if (bomb[i].x == tank.x && bomb[i].y == tank.y) {
            win = 0;
            break;
         }
      }    
      
      move(0,COLS-1);
      refresh();
      usleep(settings.overall);
      ++loops;
      
      input = getch();
      move(tank.x,tank.y);
      addch(' ');
      
     
      if (input == 'q')
         win = 2;
      else if (input == KEY_LEFT)
         --tank.y;
     else if (input == KEY_RIGHT)
         ++tank.y;
      else if (input == ' ' && shotcount < 3) {
         for (i=0; i<3; ++i) {
            if (shot[i].active == 0) {
               shot[i].active = 1;
               ++shotcount;
               --score;
               shot[i].x = LINES - 2;
               shot[i].y = tank.y;
               break;
            }
         }
      }
      else if (input == 'm')
         menu(&settings); 
      
      if (win != -1)
         break;
      
     
      if (tank.y > COLS-2)
         tank.y = COLS - 2;
      else if (tank.y < 0)
         tank.y = 0;     
   }
   
   gameover(win);
   endwin();
}


void menu(struct options *settings) {
   char option, buf[30];
   int new;

   clear();
   echo();
   nocbreak();
   nodelay(stdscr,0);

   move(0,0);
   addstr("1. Change overall game speed");
   move(1,0);
   addstr("2. Change alien motion speed");
   move(2,0);
   addstr("3. Change tank shot speed");
   move(3,0);
   addstr("4. Change alien bomb speed");
   move(4,0);
   addstr("5. Change alien bomb dropping frequency");
   move(5,0);
   addstr("6. Return to the game");
   move(6,0);
   addstr("7. Exit the game");
   move(8,0);
   addstr("Enter your option: ");
   refresh();
   
   while(1) {
      move(8,19);
      option = getch();
      move(9,0);
      deleteln();
      move(10,0);
      deleteln();
      move(11,0);
      deleteln();
      
      if (option == '1') {
         sprintf(buf,"Current value: %d\n", settings->overall);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
        
         if (new < 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->overall = new;
         }  
      }
      else if (option == '2') {
         sprintf(buf,"Current value: %d\n", settings->alien);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
        
         if (new <= 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->alien = new;
         }  
      }
      else if (option == '3') {
         sprintf(buf,"Current value: %d\n", settings->shots);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
        
         if (new <= 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->shots = new;
         }  
      }
      else if (option == '4') {
         sprintf(buf,"Current value: %d\n", settings->bombs);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
        
         if (new <= 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->bombs = new;
         }  
      }
      else if (option == '5') {
         sprintf(buf,"Current value: %d\n", settings->bombchance);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
        
         if (new > 100 || new < 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->bombchance = new;
         }  
      }
      else if (option == '6') {
         break;
      }
      else if (option == '7') {
         endwin();
         exit(0);
      }
      else {
         move(9,0);
         addstr("ERROR: Invalid selection");
         move(8,19);
         addstr(" ");
         refresh();        
      }
   }
   
   clear();
   noecho();
   cbreak();
   nodelay(stdscr,1);
   
   move(0,(COLS/2)-9);
   addstr("--SPACE INVADERS--");
   move (0,1);
   addstr("SCORE: ");
   move(0,COLS-19);
   addstr("m = menu  q = quit");
}


void gameover(int win) {

   nodelay(stdscr, 0);
   
   if (win == 0) {
      clear();
      move((LINES/2)-1,(COLS/2)-5);
      addstr("YOU LOSE!");
      move((LINES/2),(COLS/2)-11);
      addstr("PRESS ANY KEY TO EXIT");
      move(0,COLS-1);
      refresh();
      getch();
   }
   
   else if (win == 1) {
      clear();
      move((LINES/2)-1,(COLS/2)-5);
      addstr("YOU WIN!");
      move((LINES/2),(COLS/2)-11);
      addstr("PRESS ANY KEY TO EXIT");
      move(0,COLS-1);
      refresh();
      getch();
   }
}