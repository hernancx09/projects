
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#define Wlength 15
#define BRDSZ 15
#define MXWRD 20


typedef struct {    
    char wordz[Wlength];
    int x;  
    int y;
    bool ONBOARD;    
    bool HORIZONTAL;
} Word;


void STRTboard(char board[][BRDSZ+2]);
int Getinput(Word words[]);
int Fileinput(char filename[], Word words[]);
void InnerUpper(char wordz[]);
void OuterUpper(Word words[], int numWords);
void sort(Word words[], int numWords);

void Put1(char board[][BRDSZ+2], Word words[], int x, int y, bool HORIZONTAL, int curr);
bool Put2(char board[][BRDSZ+2], Word words[], int curr);
bool Whatplace(char board[][BRDSZ+2], Word word, int x, int y, bool HORIZONTAL, int intersect);
void placerest(char board[][BRDSZ+2], Word words[], int start, int numWords);
void printBoard(FILE *ofp, char board[][BRDSZ+2]);
void Hints(FILE *ofp, Word words[], int numWords);
bool Error(char board[][BRDSZ+2], Word words[], int);

int main(int argc, char *argv[])
{
    Word words[MXWRD];   
    int numWords = 0; 
    char board[BRDSZ+2][BRDSZ+2];   
    STRTboard(board);   
    if(argc == 1){
        numWords = Getinput(words); 
    } else if(argc > 1){
        numWords = Fileinput(argv[1], words);  
    } else if(argc > 3){
        puts("Too many arguments");
        return 1;
    }
    if(numWords == -1){
        puts("Input file not found"); 
        return 1;
    }
    OuterUpper(words, numWords);  
    sort(words, numWords);  
    
    Put1(board, words, (BRDSZ-strlen(words[0].wordz))/2, BRDSZ/2, true, 0);
    bool ONBOARD2nd = false; 
    int i;
    for(i = 1; i < numWords; i++)   
                                    
    {
        if(Put2(board, words, i))   
            break;
    }
    i++;
    placerest(board, words, i, numWords);   

    
    
    
    while(Error(board, words, numWords)) {}

    if(argc < 3)    
    {
        
        printBoard(stdout, board);
        Hints(stdout, words, numWords);
    }
    else if(argc == 3)  
    {
        FILE *ofp = fopen(argv[2], "w");    
        
        printBoard(ofp, board);
        Hints(ofp, words, numWords);
    }
    return 0;
}

void STRTboard(char board[][BRDSZ+2])
{
    int i, j;
    for(i = 0; i < BRDSZ+2; i++)    
    {
        for(j = 0; j < BRDSZ+2; j++)
        {
            board[i][j] = '\0'; 
        }
    }
}

bool isalphastr(char wordz[], int len)
{
    int i;
    bool alphabetic = true;
    for(i = 0; i < len; i++)    
    {
        if(!isalpha(wordz[i]))    
            alphabetic = false; 
    }
    return alphabetic;
}

int Getinput(Word words[])
{
    
    int numWords = 0;
    char input[Wlength];
    printf("Please enter a word or enter a period to end: \n");
    while(1)
    {
        PROMPT:   
        fgets(input, Wlength, stdin);    
        char *pch = strchr(input, '\n');    
        if(pch != NULL) 
            *pch = '\0';
        else    
        {
            while(fgetc(stdin) != '\n') {}  
            puts("Entered word is too long, please try again.");    
            goto PROMPT;    
        }
        if(strcmp(input, ".") == 0) 
            break;
        if(!isalphastr(input, strlen(input)))   
        {
            puts("Entered word contains non-alphabetic characters, please try again."); 
            goto PROMPT;    
        }
        if(strlen(input) < 2)   
        {
            puts("Entered word is too short, please try again.");   
            goto PROMPT;    
        }
        
        strcpy(words[numWords].wordz, input);
        words[numWords].ONBOARD = false;
        words[numWords].x = -1;
        words[numWords].y = -1;
        words[numWords].HORIZONTAL = true;
        numWords++; 
        if(numWords > MXWRD - 1) 
        {
            puts("Max number of words entered.");   
            break;
        }
    }
    return numWords;
}

int Fileinput(char filename[], Word words[])
{
    
    FILE *fp = fopen(filename, "r");    
    if(!fp) 
    {
        return -1;  
    }

    int numWords = 0;
    char input[Wlength];
    while(1)
    {
        fgets(input, Wlength, fp);   
        char *pch = strchr(input, '\n');    
        if(pch != NULL)
            *pch = '\0'; 
        if((strcmp(input, ".") == 0) || (feof(fp))) 
            break;
        
        strcpy(words[numWords].wordz, input);
        words[numWords].ONBOARD = false;
        words[numWords].x = -1;
        words[numWords].y = -1;
        words[numWords].HORIZONTAL = true;
        numWords++; 
    }
    return numWords;
}

void strtoupper(char wordz[])
{
    
    int i;
    for(i = 0; i < strlen(wordz); i++)
    {
        wordz[i] = toupper(wordz[i]);
    }
}

void OuterUpper(Word words[], int numWords)
{
    
    int i;
    for(i = 0; i < numWords; i++)
        strtoupper(words[i].wordz);
}

void sort(Word words[], int numWords)
{
    
    bool sorted = true;
    char temp[Wlength];  
    int i;
    while(1)
    {
        sorted = true;
        for(i = 0; i < numWords-1; i++) 
        {
            if(strlen(words[i].wordz) < strlen(words[i+1].wordz))   
            {
                strcpy(temp, words[i+1].wordz);           
                strcpy(words[i+1].wordz, words[i].wordz);
                strcpy(words[i].wordz, temp);
                sorted = false;                         
            }
        }
        if(sorted)  
            break;
    }
}

void Put1(char board[][BRDSZ+2], Word words[], int x, int y, bool HORIZONTAL, int curr)
{
    
    int i;
    if(!HORIZONTAL)  
    {
        for(i = 0; i < strlen(words[curr].wordz); i++)    
        {
            board[i+y+1][x+1] = words[curr].wordz[i]; 
        }
    }
    else    
    {
        for(i = 0; i < strlen(words[curr].wordz); i++)    
        {
            board[y+1][i+x+1] = words[curr].wordz[i]; 
        }
    }
    
    words[curr].ONBOARD = true;
    words[curr].x = x;
    words[curr].y = y;
    words[curr].HORIZONTAL = HORIZONTAL;
}

bool Put2(char board[][BRDSZ+2], Word words[], int curr)
{
    
    int i, j;
    for(i = 0; i < BRDSZ; i++)  
    {      
        if(board[BRDSZ/2+1][i+1] != '\0')   
        {
            for(j = 0; j < strlen(words[curr].wordz); j++)    
            {
                if(board[BRDSZ/2+1][i+1] == words[curr].wordz[j]) 
                {
                    if((j <= BRDSZ/2-1) && (strlen(words[curr].wordz)-(j+1) <= BRDSZ - BRDSZ/2-1))    
                    {
                        Put1(board, words, i, BRDSZ/2-j, false, curr); 
                        return true;    
                    }
                }
            }
        }
    }
    return false;   
}

bool Whatplace(char board[][BRDSZ+2], Word word, int x, int y, bool HORIZONTAL, int intersect)
{
    
    int i;
    bool safe = true;
    if(!HORIZONTAL)  
    {
        for(i = 0; i < strlen(word.wordz); i++)   
        {
            
            
            if(((board[i+y+1][x+1] != '\0') && (board[i+y+1][x+1] != word.wordz[i])) || ((board[i+y+1][x] != '\0') && (i != intersect)) || ((board[i+y+1][x+2] != '\0') && (i != intersect)) || ((i == 0) && (board[i+y][x+1] != '\0')) || ((i == strlen(word.wordz)-1) && (board[i+y+2][x+1])))
                safe = false;
        }
    }
    else    
    {
        for(i = 0; i < strlen(word.wordz); i++)
        {
            if(((board[y+1][i+x+1] != '\0') && (board[y+1][i+x+1] != word.wordz[i])) || ((board[y][i+x+1] != '\0') && (i != intersect)) || ((board[y+2][i+x+1] != '\0') && (i != intersect)) || ((i == 0) && (board[y+1][i+x] != '\0')) || ((i == strlen(word.wordz)-1) && (board[y+1][i+x+2])))
                safe = false;
        }
    }
    return safe;    
}

void placerest(char board[][BRDSZ+2], Word words[], int start, int numWords)
{
    
    int w, i, j, c;
    bool HORIZONTAL = false;
    for(w = start; w < numWords; w++)   
    {        
        for(i = 0; i < BRDSZ; i++)  
        {            
            for(j = 0; j < BRDSZ; j++)  
            {                
                if(board[i+1][j+1] != '\0') 
                {
                    if((board[i][j+1] == '\0') && (board[i+2][j+1] == '\0'))    
                    {                                                           
                        HORIZONTAL = false;  
                        for(c = 0; c < strlen(words[w].wordz); c++)   
                        {                            
                            if(board[i+1][j+1] == words[w].wordz[c])  
                            {
                                if((c <= i) && (strlen(words[w].wordz)-(c+1) <= BRDSZ - (i+1)))   
                                {
                                    if(Whatplace(board, words[w], j, i-c, HORIZONTAL, c))   
                                    {
                                        Put1(board, words, j, i-c, HORIZONTAL, w);  
                                        goto NEXTWORD;  
                                    }
                                }
                            }
                        }
                    }
                    if((board[i+1][j] == '\0') && (board[i+1][j+2] == '\0'))    
                    {                                                           
                        HORIZONTAL = true;   
                        for(c = 0; c < strlen(words[w].wordz); c++)   
                        {                            
                            if(board[i+1][j+1] == words[w].wordz[c])
                            {
                                if((c <= j) && (strlen(words[w].wordz)-(c+1) <= BRDSZ - (j+1)))
                                {
                                    if(Whatplace(board, words[w], j-c, i, HORIZONTAL, c))
                                    {
                                        Put1(board, words, j-c, i, HORIZONTAL, w);
                                        goto NEXTWORD;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        NEXTWORD:;
    }
}

bool Error(char board[][BRDSZ+2], Word words[], int numWords)
{
    bool wordsONBOARD = false;
    int w, i, j, c;
    bool HORIZONTAL = false;
    for(w = 0; w < numWords; w++)   
    {
        if(!words[w].ONBOARD)    
        {        
            for(i = 0; i < BRDSZ; i++)
            {            
                for(j = 0; j < BRDSZ; j++)
                {                
                    if(board[i+1][j+1] != '\0')
                    {
                        if((board[i][j+1] == '\0') && (board[i+2][j+1] == '\0'))
                        {
                            HORIZONTAL = false;
                            for(c = 0; c < strlen(words[w].wordz); c++)
                            {                            if(board[i+1][j+1] == words[w].wordz[c])
                                {
                                    if((c <= i) && (strlen(words[w].wordz)-(c+1) <= BRDSZ - (i+1)))
                                    {
                                        if(Whatplace(board, words[w], j, i-c, HORIZONTAL, c))
                                        {
                                            Put1(board, words, j, i-c, HORIZONTAL, w);
                                            wordsONBOARD = true;
                                            goto NEXTWORD2;
                                        }
                                    }
                                }
                            }
                        }
                        if((board[i+1][j] == '\0') && (board[i+1][j+2] == '\0'))
                        {
                            HORIZONTAL = true;
                            for(c = 0; c < strlen(words[w].wordz); c++)
                            {                            if(board[i+1][j+1] == words[w].wordz[c])
                                {
                                    if((c <= j) && (strlen(words[w].wordz)-(c+1) <= BRDSZ - (j+1)))
                                    {
                                        if(Whatplace(board, words[w], j-c, i, HORIZONTAL, c))
                                        {
                                            Put1(board, words, j-c, i, HORIZONTAL, w);
                                            wordsONBOARD = true;
                                            goto NEXTWORD2;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            NEXTWORD2:;
        }
    }
    return wordsONBOARD; 
}

void printBoard(FILE *ofp, char board[][BRDSZ+2])
{
    
    int irow, jcol;
    
    fprintf(ofp, "\nSolution:\n");  
    for(irow = 0; irow < BRDSZ+2; irow++)   
    {
        for(jcol = 0; jcol < BRDSZ+2; jcol++)
        {
            if((irow == 0) ||  (irow == BRDSZ+1))   
                fprintf(ofp, "-"); 
            else if((jcol == 0) || (jcol == BRDSZ+1))   
                fprintf(ofp, "|");  
            else if(board[irow][jcol] == '\0')  
                fprintf(ofp, "."); 
            else    
                fprintf(ofp, "%c", board[irow][jcol]); 
            if(jcol == BRDSZ+1) 
                fprintf(ofp, "\n"); 
        }
    }
    
    
    fprintf(ofp, "\nCrossword Puzzle:\n");
    for(irow = 0; irow < BRDSZ+2; irow++)
    {
        for(jcol = 0; jcol < BRDSZ+2; jcol++)
        {
            if((irow == 0) ||  (irow == BRDSZ+1))
                fprintf(ofp, "-");
            else if((jcol == 0) || (jcol == BRDSZ+1))
                fprintf(ofp, "|");
            else if(board[irow][jcol] == '\0')  
                fprintf(ofp, "#"); 
            else    
                fprintf(ofp, " "); 
            if(jcol == BRDSZ+1)
                fprintf(ofp, "\n");
        }
    }
}

void Hints(FILE *ofp, Word words[], int numWords)
{
    fprintf(ofp, "\nCLUES:\n");
    fprintf(ofp, " Location | Direction | Anagram\n");  
    int i,k,randomized;
    char temp[15],jumble[16], copy[16];
    for(i = 0; i < numWords; i++)   
    {
        if(words[i].ONBOARD) 
        {
            strcpy(temp, words[i].wordz); 
            
            fprintf(ofp, " (%2d, %2d) |", words[i].x, words[i].y); 
            if(words[i].HORIZONTAL)  
                fprintf(ofp, "    Across | ");
            else    
                fprintf(ofp, "      Down | ");
        
        strcpy(jumble, words[i].wordz);
        strcpy(copy, words[i].wordz);
        k = 0;
        while (k < strlen(jumble)) {
            randomized = rand() % strlen(jumble);
            if (copy[randomized] == '.') {
                continue;
            } else {
                jumble[k] = copy[randomized];
                copy[randomized] = '.';
                k++;
            }
        }
            fprintf(ofp, "%s\n", jumble); 
        } else if (!words[i].ONBOARD){
            fprintf(ofp, "\nThe following words could not be placed on the board:\n");
            for(i = 0; i < numWords; i++){
                if(!words[i].ONBOARD)    
                fprintf(ofp, "%s\n", words[i].wordz); 
            }
        }
        
    }
}
 