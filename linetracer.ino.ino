#define TRIG 11 // 센서의 TRIG는 D11에 연결
#define ECHO 10 // 센서의 ECHO는 D10에 연결
#include<Servo.h>

#include <Wire.h>
#include <Adafruit_MotorShield.h>

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *myMotor_L = AFMS.getMotor(1);
Adafruit_DCMotor *myMotor_R = AFMS.getMotor(3);

Servo servo;

int m=105; //서보모터의 최소각 정의
int M=160; // 서보모터의 최대각 정의
int pos=M; // 서보모터 각 POS는 최대각으로 초기화한다
int a=1; // 밑에서 서보모터의 각을 서서히 줄이거나 더할 때 쓰일 INT
int obstacle_flag = 0; // 장애물 없으면 0, 발견하면 1
int turn_flag = 0; // 물체 회피 전 0, 회피 후 1
int turning_time = 1000;  // 원래 값은 900이었음
int backward_time = 1100;
unsigned long obstacle_distance = 100;

int FAST = 50;
int SLOW = 40;

int a0,b0,c0,a1,b1,c1,a2,b2,c2,a3,b3,c3;
int c0_min, c0_max, c1_min, c1_max, c2_min, c2_max, c3_min, c3_max; // 프로그램에서 알아서 최소값, 최대값 찾아서 세팅하도록 해주기 위함
int test_once = 0; // min과 max 초기화를 위해서 맨 처음 딱 한번만 실행되게끔 하기 위해
float c0_lower, c0_upper, c1_lower, c1_upper, c2_lower, c2_upper, c3_lower, c3_upper;
int road_state = 0;
int mspeed = 0; 
int num_stop = 0;

void setup(){
    Serial.begin(9600);
    AFMS.begin();  
    pinMode(TRIG,OUTPUT); // TRIG는 OUTPUT(신호를 내보냄)
    pinMode(ECHO,INPUT); // ECHO는 INPUT ( 신호를 받음 )
    servo.attach(9); // 모터쉴드V2의 servo 2번을 씀
    pinMode(6,OUTPUT);
    pinMode(7,OUTPUT);
}

void loop(){
    uint8_t i;
    
    digitalWrite(6,HIGH);
    digitalWrite(7,HIGH);
    delayMicroseconds(300);
    a0 = analogRead(A0);
    a1 = analogRead(A1);
    a2 = analogRead(A2);
    a3 = analogRead(A3);
    
    digitalWrite(6,LOW); // 송신부 동작 멈춤
    digitalWrite(7,LOW);
    delayMicroseconds(300);
    b0 = analogRead(A0); // 송신부가 동작하지 않을 때에 주변으로부터 들어오는 신호의 세기 측정
    b1 = analogRead(A1);
    b2 = analogRead(A2);
    b3 = analogRead(A3);
    
    c0 = a0-b0;
    c1 = a1-b1; // 노이즈 제거
    c2 = a2-b2;
    c3 = a3-b3;

    if(test_once==0){
      c0_min = c0;
      c0_max = c0;
      c1_min = c1;
      c1_max = c1;
      c2_min = c2;
      c2_max = c2;
      c3_min = c3;
      c3_max = c3;
      test_once = 1; // 이 행이 실행됨으로써 이 if문이 더이상 실행될 일은 x
    }


// 센서 입력 최대값과 최소값 계산
    if(c0<c0_min){
      c0_min = c0;
    }
    else if(c0>=c0_max){
      c0_max = c0;
    }
    
    if(c1<c1_min){
      c1_min = c1;
    }
    else if(c1>=c1_max){
      c1_max = c1;      
    }

    if(c2<c2_min){
      c2_min = c2;
    }
    else if(c2>=c2_max){
      c2_max = c2;
    }
    
    if(c3<c3_min){
      c3_min = c3;
    }
    else if(c3>=c3_max){
      c3_max = c3;      
    }



// 최대값과 최소값으로부터 경계값 계산
    c0_lower = c0_min + float((c0_max - c0_min)/2.1);
    c0_upper = c0_max - float((c0_max - c0_min)/2.1);
    c1_lower = c1_min + float((c1_max - c1_min)/2.1);
    c1_upper = c1_max - float((c1_max - c1_min)/2.1);
    c2_lower = c2_min + float((c2_max - c2_min)/2.1);
    c2_upper = c2_max - float((c2_max - c2_min)/2.1);
    c3_lower = c3_min + float((c3_max - c3_min)/2.1);
    c3_upper = c3_max - float((c3_max - c3_min)/2.1);



    // 1차로인 경우
    if((c0>=c0_upper || c1>=c1_upper) && (c2>=c2_upper || c3>=c3_upper)){
      road_state = 0;
      mspeed = FAST;
     // Serial.println("GoFast");
      drive(road_state, c0, c0_lower, c0_upper, c1, c1_lower, c1_upper, mspeed);
    }

    // 2차로에서 오른쪽 주행하는 경우
    if((c0>=c0_upper || c1>=c1_upper) && (c2<=c2_lower && c3<=c3_lower)){
      road_state = 1;
      mspeed = SLOW;
      //Serial.println("GoRightLane");
      drive(road_state, c0, c0_lower, c0_upper, c1, c1_lower, c1_upper, mspeed);
    }
  
    // 2차로에서 왼쪽 주행하는 경우
    else if((c0<=c0_lower && c1<=c1_lower) && (c2>=c2_upper || c3>=c3_upper)){
      road_state = 2;
      mspeed = SLOW;
      drive(road_state, c2, c2_lower, c2_upper, c3, c3_lower, c3_upper, mspeed);
    }

    // 양쪽 모두 흰색인 경우
    else if(c0<=c0_lower && c1<=c1_lower && c2<=c2_lower && c3<=c3_lower){
      road_state = 3;
      mspeed = 0;
      drive(road_state, c0, c0_lower, c0_upper, c1, c1_lower, c1_upper, mspeed);
    }

    
//서보모터 제어
  servo.write(pos); // 서보모터에게 각 출력
  pos=pos-5*a; // 각 변화

  if(pos<=m||pos>=M){ // 정한 서보모터의 각 범위를 벗어 난다면
  a*=-1; // 시계방향 또는 반시계방향으로 서보모터의 각 변화방향 바꾸기
  }

  delay(25);

  obstacle_distance = distancee();

  if(obstacle_distance <= 30){ 
    while(road_state == 0)
    {
      myMotor_L->run(RELEASE);
      myMotor_R->run(RELEASE);      
    }
  }
  
  if(obstacle_distance <= 12){
    obstacle_flag = 1;
  }


}

unsigned long distancee(){
  unsigned long distance;
  digitalWrite(TRIG,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG,LOW);
  distance = pulseIn(ECHO, HIGH)/58.2; // ECHO 의 HIGH시간을 통해 거리 계산
  return distance;
}


// 조금 더 미세 조정이 필요하다면 직진 외에는 SLOW 속도로 작동하게끔 코드 수정하기 (함수 입력으로 int SLOW 추가하고 밑에 조건문에서 setSpeed(SLOW))
void drive(int road_state, int sensor1, double sensor1_lower, double sensor1_upper, int sensor2, double sensor2_lower, double sensor2_upper, int mspeed ){
     
// 장애물 회피 위해 전진하던 중 검은선을 만난 경우
      if((obstacle_flag == 1) && (turn_flag == 1)){
        if(road_state != 3){
          obstacle_flag = 0;
          turn_flag = 0;
        }
      }
  
      // 장애물 발견하고 방향전환
      if((obstacle_flag == 1) && (turn_flag == 0)){

        myMotor_L->run(BACKWARD);
        myMotor_R->run(BACKWARD);
        myMotor_L->setSpeed(FAST);
        myMotor_R->setSpeed(FAST);
        delay(backward_time);

        
        if(road_state == 1){
          myMotor_L->run(BACKWARD);
          myMotor_R->run(FORWARD);
          myMotor_L->setSpeed(SLOW);
          myMotor_R->setSpeed(SLOW);
          delay(turning_time); // 적당한 값 실험으로 찾기
          turn_flag = 1;
        }
        else if(road_state == 2){
          myMotor_L->run(FORWARD);
          myMotor_R->run(BACKWARD);
          myMotor_L->setSpeed(SLOW);
          myMotor_R->setSpeed(SLOW);
          delay(turning_time); // 적당한 값 실험으로 찾기
          turn_flag = 1;
        }
        myMotor_L->run(FORWARD);
        myMotor_R->run(FORWARD);
        myMotor_L->setSpeed(SLOW);
        myMotor_R->setSpeed(SLOW);
        delay(200);
               
      }
      // 차로 변경 - 이 과정에서는 stop 조건과 동일하지만 순서상 이 코드가 먼저 실행되므로 상관 x
      // 장애물을 만나 회피를 위해 회전까지는 마친 상태
      if((obstacle_flag == 1) && (turn_flag == 1)){
        myMotor_L->run(FORWARD);
        myMotor_R->run(FORWARD);
        myMotor_L->setSpeed(SLOW);
        myMotor_R->setSpeed(SLOW);
        return;
      }
      // R검 L검
      else if((sensor1 >= sensor1_upper) && (sensor2 >= sensor2_upper)){
        myMotor_L->run(FORWARD);
        myMotor_R->run(FORWARD);
        myMotor_L->setSpeed(mspeed);
        myMotor_R->setSpeed(mspeed);
        num_stop = 0;
      }
      // R검 L흰
      else if((sensor1 >= sensor1_upper) && (sensor2 <= sensor2_lower)){
        myMotor_L->run(FORWARD);
        myMotor_R->run(RELEASE);
        myMotor_L->setSpeed(mspeed);
        num_stop = 0;
      }
      // R흰 L검
      else if((sensor1 <= sensor1_lower) && (sensor2 >= sensor2_upper)){
        myMotor_L->run(RELEASE);
        myMotor_R->run(FORWARD);
        myMotor_R->setSpeed(mspeed);
        num_stop = 0;
      }
      // R흰 L흰
      else if((sensor1 <= sensor1_lower) && (sensor2 <= sensor2_lower)){
        num_stop = num_stop+1;
        if(num_stop>=80){  
          myMotor_L->run(RELEASE);
          myMotor_R->run(RELEASE);
          num_stop = 0;
        }
      }
}


// 이 경우에 장애물을 피하는 과정에서 Stop이 실행되지 않도록 보완이 필요
// 장애물을 만나면 인터럽트 실행 -> 그 안에서 다른 검은선 만나는 과정까지 해결한 다음 원래 흐름으로 복귀하면 Stop 실행 막을 수 있음
