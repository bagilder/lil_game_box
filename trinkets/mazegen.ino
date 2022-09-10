///////////////////////////
// maze generator. i'll put it on my oled to test it
// bgilder
// 16 july 2022
// picked back up 9 sept 2022 lol
///////////////////////////



#include <gamebox.h>
#include <bits/stdc++.h>
  using namespace std;

#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait


typedef struct  
{
//public:
  int x, y;
  unsigned
    north:1,
    east:1,
    south:1,
    west:1,
    visited:1;
} positionboy;

const int blockSize = 8;   //how many pixels square each block will be
const int rows = (SCREEN_HEIGHT) / blockSize;
const int cols = (SCREEN_WIDTH) / blockSize;
long totalVisits = 0;


std::vector<positionboy> blockList;  //will arduino allow me to do vector stack shenanigans? we'll see
std::stack<int> mazeStack;

int currentPos = 0;  //dude you fool just use this to store the index of the blocklist






void setup()   
{
  oled_setup(SCREEN_ROTATION);

  display.print("maze generator algo   bgilder 16jul2022\n\n  randomly generates fully connected maze\n\n press button for new maze");
  display.display();



  while(digitalRead(buttPin))  
  {
      delay(1);
  }
  display.clearDisplay();
  display.display();
  randomSeed(millis()%253);
  delay(500);
  populate_vector();
  blockList[currentPos].visited = 1;
  totalVisits = 1;
  //mazeStack.push(currentPos);
}


void loop()
{
  while(totalVisits< cols*rows)
  {
    int uno = blockList[currentPos].x;
    int dos = blockList[currentPos].y;
    draw_position(uno,dos);
    neighbors(uno,dos);

    display.display();
    delay(10);
  }


  while(digitalRead(buttPin))
  {
    //
  }
  display.clearDisplay();
  display.display();
  delay(500);
  //populate_vector();
}


void populate_vector()
{
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
      draw_position(i,j);  //generates index of current position in vector
    }
  }
display.display();
}

void neighbors(int i, int j)
{
  std::vector<int> neighborChoices;
  int upindex = i + (j-1)*cols;
  int rightindex = (i+1) + j*cols;
  int downindex = i + (j+1)*cols;
  int leftindex = (i-1) + j*cols;
  
  if(j > 0)  //we're not on the top row
  {
    if(!blockList[upindex].visited)
      neighborChoices.push_back(0);
  }
  if(i < cols-1) //we're on the right side
  {
    if(!blockList[rightindex].visited)
      neighborChoices.push_back(1);
  }
  if(j < rows-1)
  {
    if(!blockList[downindex].visited)
      neighborChoices.push_back(2);
  }
  if(i > 0)
  {
    if(!blockList[leftindex].visited);
      neighborChoices.push_back(3);  
  }

  if(!neighborChoices.empty())
  {
    byte neighborChoiceChosen = neighborChoices[random(neighborChoices.size()+1)];
    blockList[currentPos].visited = 1;
    switch(neighborChoiceChosen)
    {
      case 0:   //up
            //if there is an available path to the north, make north the thing we do (meaning kill the wall, i think) (remember to use blocklist instead of up)
        blockList[currentPos].north = 0;
            //and also mark the cell above as having a path to the south (ditto)
        blockList[upindex].south = 0;
            //then push the top of the stack the northern neighbor's coordinates
        currentPos = upindex;
        break;
      case 1:   //right
            //if there is an available path to the east, make east the thing we do
        blockList[currentPos].east = 0;
            //and also mark the cell to the right as having a path to the west
        blockList[rightindex].west = 0;
            //then push the top of the stack the northern neighbor's coordinates
        currentPos = rightindex;
        break;
      case 2:   //down
            //if there is an available path to the south, make south the thing we do
        blockList[currentPos].south = 0;
            //and also mark the cell below as having a path to the north
        blockList[downindex].north = 0;
            //then push the top of the stack the northern neighbor's coordinates
        currentPos = downindex;
        break;  
      case 3:   //left
            //if there is an available path to the west, make west the thing we do
        blockList[currentPos].west = 0;
            //and also mark the cell to the left as having a path to the east
        blockList[leftindex].east = 0;
            //then push the top of the stack the northern neighbor's coordinates
        currentPos = leftindex;
        break;  
    }
    totalVisits++;
    mazeStack.push(currentPos);
  }
  else
  {
        //pop off the stack to backtrack
    mazeStack.pop();
  }

  
}

void draw_position(int _x, int _y)
{
      int index = _x + _y*cols;
      int xApparent = _x*blockSize;
      int yApparent = _y*blockSize;
      display.drawRect(xApparent,yApparent,blockSize+1,blockSize+1,GRAY_1);
      if(blockList[index].north)
      { display.drawLine(xApparent,yApparent,xApparent+blockSize,yApparent,GRAY_4);                     //draw north wall
      }
      if(blockList[index].east)
      { display.drawLine(xApparent+blockSize,yApparent,xApparent+blockSize,yApparent+blockSize,GRAY_3); //draw east wall
      }
      if(blockList[index].south)
      { display.drawLine(xApparent+blockSize,yApparent+blockSize,xApparent,yApparent+blockSize,GRAY_4); //draw south wall
      }
      if(blockList[index].west)
      { display.drawLine(xApparent,yApparent+blockSize,xApparent,yApparent,GRAY_3);                     //draw west wall
      }
      if(blockList[index].visited)
      { display.fillRect(xApparent+1,yApparent+1,blockSize-1,blockSize-1,GRAY_1);
      }
}
