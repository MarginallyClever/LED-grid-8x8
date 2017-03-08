//---------------------------------------------------
// one 8x8 LED grid with two shift registers on Arduino
// dan@marginallyclever.com 2017-02-28
//---------------------------------------------------


//---------------------------------------------------
// CONSTANTS
//---------------------------------------------------

/* schematic map - led grid - shift register pin
u1 qa -r1 - 0
u1 qb -c4 - 1
u1 qc -c6 - 2
u1 qd -r4 - 3
u1 qe -c1 - 4
u1 qf -r2 - 5
u1 qg -c7 - 6
u1 qh -c8 - 7

u2 qa -r5 - 8
u2 qb -r7 - 9
u2 qc -c2 -10
u2 qd -c3 -11
u2 qe -r8 -12
u2 qf -c5 -13
u2 qg -r6 -14
u2 qh -r3 -15

now sort by row and column for fast retreival
u1 qa -r1 - 0
u1 qf -r2 - 5
u2 qh -r3 -15
u1 qd -r4 - 3
u2 qa -r5 - 8
u2 qg -r6 -14
u2 qb -r7 - 9
u2 qe -r8 -12

u1 qe -c1 - 4
u2 qc -c2 -10
u2 qd -c3 -11
u1 qb -c4 - 1
u2 qf -c5 -13
u1 qc -c6 - 2
u1 qg -c7 - 6
u1 qh -c8 - 7
*/
const int GRID_COLUMNS[] = {0,5,15,3,8,14,9,12};
const int GRID_ROWS   [] = {4,10,11,1,13,2,6,7};

// There are two common models of LED grid.
// one has columns cathodes. The other has row cathodes.
// comment out the next line if columns are anodes.
//#define COLUMNS_ARE_CATHODES


#ifdef COLUMNS_ARE_CATHODES
#define ROW_ON HIGH
#define ROW_OFF LOW
#define COLUMN_ON LOW
#define COLUMN_OFF HIGH
#else
#define ROW_ON LOW
#define ROW_OFF HIGH
#define COLUMN_ON HIGH
#define COLUMN_OFF LOW
#endif


#define SR_SER 2  // data
#define SR_LAT 3  // latch
#define SR_CLK 4  // clock


#define GRID_WIDTH      8
#define GRID_HEIGHT     8
#define TOTAL_GRID_AREA (GRID_HEIGHT*GRID_WIDTH)


#define PINS_PER_REGISTER   8
#define NUM_REGISTERS       2
#define TOTAL_REGISTER_PINS (PINS_PER_REGISTER*NUM_REGISTERS)


#define BAUD 57600  // Serial speed


//---------------------------------------------------
// GLOBALS
//---------------------------------------------------

// This is a map of which lights are on and off
// We can't ask the grid what it's doing, so we have to remember
// what we told it to do.  
char doubleBuffer[TOTAL_GRID_AREA];

// This is a map of register pin high or low state(s).
char registers[TOTAL_REGISTER_PINS];


//---------------------------------------------------
// METHODS
//---------------------------------------------------


void setup() {
  Serial.begin(BAUD);
  pinMode(SR_SER,OUTPUT);
  pinMode(SR_CLK,OUTPUT);
  pinMode(SR_LAT,OUTPUT);
  clearBuffer();
  Serial.println("\n\nREADY");
}


void loop() {
  blinkAll();
  lightOneAtATime();
  fillZigZag();
  spirals();
  drawSmiley();
}


//---------------------------------------------------


void blinkAll() {
  Serial.println("blinkAll");
  const int WAIT = 100;
  
  for(int i=0;i<5;++i) {
    clearBuffer();
    displayBufferForMs(WAIT);

    // light all
    for(int i=0;i<TOTAL_GRID_AREA;++i) {
      doubleBuffer[i]=1;
    }
    displayBufferForMs(WAIT);
  }
}


void lightOneAtATime() {
  Serial.println("lightOneAtATime");
  int x,y;
  const int WAIT=50;

  clearBuffer();
  
  for(y=0;y<GRID_HEIGHT;++y) {
    for(x=0;x<GRID_WIDTH;++x) {
      setBuffer(x,y,1);
      displayBufferForMs(WAIT);
      setBuffer(x,y,0);
    }
  }
}


void fillZigZag() {
  Serial.println("fillZigZag");
  int x=0,y=0;
  int heading=0;
  const int WAIT=50;

  clearBuffer();
  
  for(y=0;y<GRID_HEIGHT;++y) {
    for(x=0;x<GRID_WIDTH;++x) {
      setBuffer(y,x,1);
      displayBufferForMs(WAIT);
    }
    ++y;
    for(x=GRID_WIDTH-1;x>=0;--x) {
      setBuffer(y,x,1);
      displayBufferForMs(WAIT);
    }
  }  
}


void spirals() {
  Serial.println("spirals");

  int x=0,y=0;
  int heading=0;
  const int WAIT=50;

  clearBuffer();

  // while we can still move to an unlit spot
  while(getBuffer(x,y)==0) {
    // light the current spot
    setBuffer(x,y,1);
    displayBufferForMs(WAIT);

    // check every direction for an unlit dot
    int tests=0;
    do {
      switch(heading) {
        case 0:
          if(inBuffer(x+1,y) && getBuffer(x+1,y)==0) {
            ++x;  // move right if no light there
            tests=4;
            break;
          } else ++heading;
        case 1:
          if(inBuffer(x,y+1) && getBuffer(x,y+1)==0) {
            ++y;  // move down if no light there
            tests=4;
            break;
          } else ++heading;
        case 2:        
          if(inBuffer(x-1,y) && getBuffer(x-1,y)==0) {
            --x;  // move left if no light there
            tests=4;
            break;
          } else ++heading;
        case 3:
          if(inBuffer(x,y-1) && getBuffer(x,y-1)==0) {
            --y;  // move up if no light there
            tests=4;
            break;
          } else heading=0;
      }
      ++tests;
    } while(tests<4);  // four tests for four directions.
  }
}


void drawSmiley() {
  const int WAIT=500;
  char smiley[] = {
    0,0,0,0,0,0,0,0,
    0,0,1,0,0,1,0,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,1,0,1,1,0,1,0,
    0,1,0,0,0,0,1,0,
    0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,
  };

  for(int i=0;i<TOTAL_GRID_AREA;++i) {
    doubleBuffer[i]=smiley[i];
  }
  displayBufferForMs(WAIT);
}


//---------------------------------------------------


// wipe the memory.  next time displayBuffer() is called
// all the lights should be off.
void clearBuffer() {
  for(int i=0;i<TOTAL_GRID_AREA;++i) {
    doubleBuffer[i]=0;
  }
}


// keep displaying the memory until the wait time elapses.
void displayBufferForMs(long wait) {
  long t = millis()+wait;
  while(millis() < t) {
    displayBuffer();
  }
}


// copy the memory of the buffer to the shift registers
// which will display the memory on the grid
void displayBuffer() {/*
  int x,y;
  for(y=0;y<GRID_HEIGHT;++y) {
    //setOneRowOn(y);
    setRow(y,ROW_ON);
    for(x=0;x<GRID_WIDTH;++x) {
      char state = getBuffer(x,y)==1 ? COLUMN_ON : COLUMN_OFF;
      setColumn( x, state );
    }
    updateRegisters();
    setRow(y,ROW_OFF);
  }*/
  int x,y;
  for(y=0;y<GRID_HEIGHT;++y) {
    for(x=0;x<GRID_WIDTH;++x) {
      char state = getBuffer(x,y)==1 ? COLUMN_ON : COLUMN_OFF;
      char statey = getBuffer(x,y)==1 ? ROW_ON : ROW_OFF;
      setRow(y,statey);
      setColumn( x, state );
      updateRegisters();
      setRow(y,ROW_OFF);
      setColumn( x, COLUMN_OFF );
    }
  }
}


// return 0 if (x,y) is outside the grid and 1 if it is inside.
char inBuffer(int x,int y) {
  return ( x >= 0 && x < GRID_WIDTH && 
           y >= 0 && y < GRID_HEIGHT );
}


// what is the buffer at row y, column x?
char getBuffer(int x,int y) {
  return doubleBuffer[y*GRID_WIDTH+x];
}


// set the buffer at row y, column x to state
void setBuffer(int x,int y,char state) {
  doubleBuffer[y*GRID_WIDTH+x]=state;
}


// state should be ROW_ON or ROW_OFF
void setRow(int row,char state) {
  registers[GRID_ROWS[row]]=state;
}


void setOneRowOn(int row) {
  for(int i=0;i<GRID_HEIGHT;++i) {
    registers[GRID_ROWS[i]]=ROW_OFF;
  }
  registers[GRID_ROWS[row]]=ROW_ON;
}


// state should be COLUMN_ON or COLUMN_OFF
void setColumn(int col,char state) {
  registers[GRID_COLUMNS[col]]=state;
}


// feed the 15th shift register value in first and the 0th last.
void updateRegisters() {
  for(int i=0;i<TOTAL_REGISTER_PINS;++i) {
    // set the data
    digitalWrite(SR_SER,registers[TOTAL_REGISTER_PINS-1-i]);
    // shift everything in one step
    digitalWrite(SR_CLK,HIGH);
    digitalWrite(SR_CLK,LOW);
  }
  // flip the latch - copy the shift registers to the outputs
  digitalWrite(SR_LAT,HIGH);
  digitalWrite(SR_LAT,LOW);
}
