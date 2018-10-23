
#define BALL_UP 0
#define BALL_RIGHT 1
#define BALL_LEFT 2
#define BALL_DOWN 3

struct Ball {
  int x;
  int y;
  int direction;
};

struct Ball ball;

void drawBallUpdates(){
  
  int pixelIndex = getPixelIndex(ball.x, ball.y);
  pixels.setPixelColor(pixelIndex, pixels.Color(0, 0, 0));  
  Serial.print("x:");
  Serial.print(ball.x);
  Serial.print(" y:");
  Serial.print(ball.y);
  Serial.print(" direction:");
  Serial.println(ball.direction);
  switch (ball.direction){
    case BALL_UP: ball.y++;break;
    case BALL_RIGHT: ball.x++;break;
    case BALL_LEFT: ball.x--;break;
    case BALL_DOWN: ball.y--;break;
  }
  detectBorderAndSwitchDirections();
  pixelIndex = getPixelIndex(ball.x, ball.y);
  pixels.setPixelColor(pixelIndex, pixels.Color(0, 0, 64));  
  pixels.show();
  delay(200);
}

void detectBorderAndSwitchDirections(){
  switch(ball.x) {
    case -1: 
      ball.x++;
      ball.direction = newRandomDirection(ball.direction); break;
    case ROWS: 
      ball.x--;
      ball.direction = newRandomDirection(ball.direction); break;
  }
  switch(ball.y) {
    case -1:
      ball.y++;
      ball.direction = newRandomDirection(ball.direction); break;
    case ROWS: 
      ball.y--;
      ball.direction = newRandomDirection(ball.direction); break;
  }
}

int newRandomDirection(int oldDirection){
  int newDirection = random(0, BALL_DOWN);
  if(newDirection != oldDirection){
    return newDirection;
  } else {
    return newRandomDirection(oldDirection);
  }
}
