///////////////////////////
// maze generator. i'll put it on my oled to test it
// bgilder
// 16 july 2022
// picked back up 9 sept 2022 lol   
///////////////////////////

////~inspired by~ a mix of onelonecoder and coding train implementations 
////eventually this could output a formatted structure for briefcase maze modules but that's a problem for later brian

#include <gamebox.h>
#include <vector>
#include <stack>

#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait


typedef struct  
{
  int x, y;
  unsigned
    north:1,
    east:1,
    south:1,
    west:1,
    visited:1;
} positionboy;

std::vector<positionboy> blockList; 
std::stack<int> mazeStack;

const int blockSize = 8;   //how many pixels square each block will be
const int rows = (SCREEN_HEIGHT) / blockSize;
const int cols = (SCREEN_WIDTH) / blockSize;
const int drawDelay = 25;
int totalVisits = 0;
int currentPos = 0;  //index in the blocklist


void setup()   
{
  oled_setup(SCREEN_ROTATION);    //gamebox

  display.print("recursive backtracking maze generator\n\n   bgilder 16july2022, 9sept2022\n\n randomly generates fully connected maze\n\n\n press button for new maze");
  display.display();

  while(digitalRead(buttPin))  
  {  delay(1);   //take it easy, bud
  }
  display.clearDisplay();
  display.display();
  randomSeed(millis());
  delay(500);
  populate_vector();
}


void loop()
{
  blockList[currentPos].visited = 1;
  while(totalVisits < cols*rows)
  {
    int uno = blockList[currentPos].x;
    int dos = blockList[currentPos].y;
    neighbors(uno,dos);
    draw_position(uno,dos);
  }
  display.display();
  delay(1000);
  draw_position(blockList[currentPos].x,blockList[currentPos].y);
  display.display();  //hide final position square
  while(digitalRead(buttPin))
  {  delay(1);   //no need to burn yourself out while waiting
  }
  display.clearDisplay();
  display.display();
  delay(500);
  populate_vector();
}


void populate_vector()
{
  while(!blockList.empty())
  {  blockList.pop_back();     //this is hacky but shut up. easy way to let me reuse it
  }
  while(!mazeStack.empty())
  {  mazeStack.pop();          //avoid those memory leaks boo
  }
  totalVisits = 1;
  //currentPos = 0;   //removing this reset starts the new maze where the old one left off. how fun
  for(int j = 0; j < rows; j++)
  {
    for(int i = 0; i < cols; i++)
    {
      positionboy newBoy;
      newBoy.x = i;
      newBoy.y = j;
      newBoy.north = 1;
      newBoy.east = 1;
      newBoy.south = 1;
      newBoy.west = 1;
      newBoy.visited = 0;
      blockList.push_back(newBoy);
      draw_position(i,j);
    }
  }
  display.display();
}


void neighbors(int ex, int wy)
{
  std::vector<int> neighborChoices;
    
  int upindex = ex + (wy-1)*cols;
  int rightindex = (ex+1) + wy*cols;  //or, currentPos+1
  int downindex = ex + (wy+1)*cols;
  int leftindex = (ex-1) + wy*cols;   //or, currentPos-1

  if(wy > 0  &&  !blockList[upindex].visited)  //we're not on the top row & can move up
  { neighborChoices.push_back(0);
  }
  if(ex < cols-1  &&  !blockList[rightindex].visited) //we're not on the right side & can move right
  { neighborChoices.push_back(1);
  }
  if(wy < rows-1  &&  !blockList[downindex].visited) //we're not on the bottom row & can move down
  {  neighborChoices.push_back(2);
  }
  if(ex > 0  &&  !blockList[leftindex].visited)  //we're not on the left side & can move left
  { neighborChoices.push_back(3);
  }

  if(!neighborChoices.empty())
  {
    switch(neighborChoices[random(neighborChoices.size())])   //choose among the viable neighbor movement options
    {
      case 0:   //up
        blockList[currentPos].north = 0;  //if there is an available path to the north, kill the north wall
        blockList[upindex].south = 0;     //and also mark the cell above as not having a south wall anymore
        currentPos = upindex;             //then push to the top of the stack the northern neighbor's coordinates
        break;
      case 1:   //right
        blockList[currentPos].east = 0;   //if there is an available path to the east, kill the east wall
        blockList[rightindex].west = 0;   //and also mark the cell to the right as not having a west wall anymore
        currentPos = rightindex;          //then push to the top of the stack the eastern neighbor's coordinates
        break;
      case 2:   //down
        blockList[currentPos].south = 0;  //if there is an available path to the south, kill the south wall
        blockList[downindex].north = 0;   //and also mark the cell below as not having a north wall anymore
        currentPos = downindex;           //then push to the top of the stack the southern neighbor's coordinates
        break;  
      case 3:   //left 
        blockList[currentPos].west = 0;   //if there is an available path to the west, kill the west wall
        blockList[leftindex].east = 0;    //and also mark the cell to the left as not having an east wall anymore            
        currentPos = leftindex;           //then push to the top of the stack the western neighbor's coordinates
        break;  
    }
    totalVisits++;
    mazeStack.push(currentPos);
    blockList[currentPos].visited = 1;
  }
  else  
  {     //pop off the stack to backtrack
    currentPos = mazeStack.top();
    mazeStack.pop();
  }
  display.fillRect(blockList[currentPos].x*blockSize+1,blockList[currentPos].y*blockSize+1,blockSize-1,blockSize-1,GRAY_5);   
  display.display();    //show where the current position is, as a treat
  delay(drawDelay);
}


void draw_position(int _x, int _y)
{
      int index = _x + _y*cols;
      int xApparent = _x*blockSize;
      int yApparent = _y*blockSize;
      display.drawRect(xApparent,yApparent,blockSize+1,blockSize+1,GRAY_1);   //remove old drawn walls
      if(blockList[index].north)
      { display.drawLine(xApparent,yApparent,xApparent+blockSize,yApparent,GRAY_4);                     //never
      }
      if(blockList[index].east)
      { display.drawLine(xApparent+blockSize,yApparent,xApparent+blockSize,yApparent+blockSize,GRAY_3); //eat
      }
      if(blockList[index].south)
      { display.drawLine(xApparent+blockSize,yApparent+blockSize,xApparent,yApparent+blockSize,GRAY_4); //shredded
      }
      if(blockList[index].west)
      { display.drawLine(xApparent,yApparent+blockSize,xApparent,yApparent,GRAY_3);                     //wheat
      }
      if(blockList[index].visited)
      { display.fillRect(xApparent+1,yApparent+1,blockSize-1,blockSize-1,GRAY_1); //after the walls are drawn, show where we've been
      }
}
