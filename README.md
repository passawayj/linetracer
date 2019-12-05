#목적  


####만든 라인트레이서가 1차선과 2차선을 구별할 줄 알며, 2차선에서의 장애물을 초음파로 인식하여 피할 수 있어야한다. 또한, 한바퀴를 다 돌아 출발지로 돌아오면 막힌 벽을 인식하여 알아서 멈추도록 해야함



#제작한 HW의 특징  


####차체가 1차선의 넓이만큼 커다랗고 길을 읽는 적외선센서가 양 옆에 2개씩, 총 4개가 있다. 초음파 센서가 차체의 위에 서보모터와 함께 있으며, 차는 2개의 DC모터를 이용하여 주행한다.



#전반적인 코드 흐름


####1. 적외선 센서의 값을 받아들이되, 노이즈 제거를 거친다. 또한, 검은선을 인식하고 있는 지, 아닌 지의 기준또한 매번 업데이트하는 과정을 거친다  
####2. 항상 적외선 센서로 지금 놓여진 길이 1차선인지, 2차선인지 구별한다.  
####3. 현재 도로의 상태에 따라 주행을 다르게 하는 함수를 사용한다  
####4. 초음파 센서도 항상 감시하고, 물체가 인식될 시에 장애물을 피해 차선을 바꾸는 함수를 사용한다  



#사용할 변수들


```C
int m=105; //서보모터의 최소각 정의
int M=160; // 서보모터의 최대각 정의
int pos=M; // 서보모터 각 POS는 최대각으로 초기화한다
int a=1; // 밑에서 서보모터의 각을 서서히 줄이거나 더할 때 쓰일 INT
int obstacle_flag = 0; // 장애물 없으면 0, 발견하면 1
int turn_flag = 0; // 물체 회피 전 0, 회피 후 1
int turning_time = 900;
int backward_time = 1100;

int FAST = 50;
int SLOW = 40;

int a0,b0,c0,a1,b1,c1,a2,b2,c2,a3,b3,c3;
int c0_min, c0_max, c1_min, c1_max, c2_min, c2_max, c3_min, c3_max; // 프로그램에서 알아서 최소값, 최대값 찾아서 세팅하도록 해주기 위함
int test_once = 0; // min과 max 초기화를 위해서 맨 처음 딱 한번만 실행되게끔 하기 위해
double c0_lower, c0_upper, c1_lower, c1_upper, c2_lower, c2_upper, c3_lower, c3_upper;
int road_state = 0;
int mspeed = 0; 
int num_stop = 0;
```


####적외선센서와 초음파센서 사용을 위한 변수 선언 및 주행에 쓰일 변수를 선언하였다. 

####obstacle_flag는 초음파로 장애물을 인식할 시 1이 되는 flag이며 이것을 가지고 장애물을 피해 다른 차선으로 바꾸는 동작을 수행할 수 있다.  
####turn_flag는 장애물을 피해 다른 차선으로 바꾸는 동작을 할 때에 쓰이는 flag이며 이것이 1일때, 차가 옆 차선으로 바꾸기 위해 꺾으며, 0일 때 다른 동작을 수행할 수 있게된다.  
####turning_time, backward_time은 옆 차선으로 바꾸기위해 꺾는 시간변수, 처음에 장애물을 만나고 어느정도 후진하는 시간변수이다.
####FAST,SLOW는 총 주행속도를 최적화하기 위한 속도이다. 1차선에선 비교적 안정적으로 운전이 가능하기 때문에 50의 속도로 달리며, 2차선일 때는 안정적이지 못하므로 40의 속도로 달리게 한다.  
####road_state의 경우 1차선, 2차선-오른쪽, 2차선-왼쪽, 도로한복판을 구별하여 값을 가지는 변수이다.  
####num_stop은 잘 운전하다 센서가 가끔 도로한복판에 있지 않은데 도로한복판이라고 인식하는 경우를 막기위해 필요한 변수이다. 이 변수가 45 이상이 될 때에 정말 도로 한복판이라는 것을 뜻하기로함.


#도로판단
```c
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
    //  Serial.println("GoLeftLane");
      drive(road_state, c2, c2_lower, c2_upper, c3, c3_lower, c3_upper, mspeed);
    }

    // 양쪽 모두 흰색인 경우
    else if(c0<=c0_lower && c1<=c1_lower && c2<=c2_lower && c3<=c3_lower){
      road_state = 3;
      mspeed = 0;
  //    Serial.println("Stop");
      drive(road_state, c0, c0_lower, c0_upper, c1, c1_lower, c1_upper, mspeed);
    }
```
    
    
    
####차체의 오른쪽에 달린 센서가 c0,c1이고, 왼쪽에 달린 센서가 c2,c3이다. 차체에 젤 끝에 있는 센서들이 c0, c3이다. 이것으로 if문을 사용하여 도로판단을 할수있고. 주행함수인 drive에 센서값들과 road_state를 입력으로 준다.


#장애물 인식하기


####아래 코드에 주석을 달아 설명진행


```c
 if(distancee() <= 15){
    obstacle_flag = 1;
  }
```


15cm내에 장애물이 있으면 obstacle_flag=1이되어 장애물을 피해야한다는 상황임을 알린다.


#drive 함수

```c
void drive(int road_state, int sensor1, double sensor1_lower, double sensor1_upper, int sensor2, double sensor2_lower, double sensor2_upper, int mspeed ){
 /*   Serial.print("obstacle flag : ");
    Serial.print(obstacle_flag);
    Serial.print(" / turn_flag : ");
    Serial.println(turn_flag);
*/

      if((obstacle_flag == 1) && (distancee()<10)){
        myMotor_L->run(RELEASE);
        myMotor_R->run(RELEASE);
        return;
      }
      
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
          //Serial.println("장애물 회피 위해 좌회전 완료");
          turn_flag = 1;
        }
        else if(road_state == 2){
          myMotor_L->run(FORWARD);
          myMotor_R->run(BACKWARD);
          myMotor_L->setSpeed(SLOW);
          myMotor_R->setSpeed(SLOW);
          delay(turning_time); // 적당한 값 실험으로 찾기
          //Serial.println("장애물 회피 위해 우회전 완료");
          turn_flag = 1;
        }       
      }
      // 차로 변경 - 이 과정에서는 stop 조건과 동일하지만 순서상 이 코드가 먼저 실행되므로 상관 x
      // 장애물을 만나 회피를 위해 회전까지는 마친 상태
      if((obstacle_flag == 1) && (turn_flag == 1)){
        myMotor_L->run(FORWARD);
        myMotor_R->run(FORWARD);
        myMotor_L->setSpeed(SLOW);
        myMotor_R->setSpeed(SLOW);
        //Serial.println("차로 변경중");
        return;
      }
      // R검 L검
      else if((sensor1 >= sensor1_upper) && (sensor2 >= sensor2_upper)){
        myMotor_L->run(FORWARD);
        myMotor_R->run(FORWARD);
        myMotor_L->setSpeed(mspeed);
        myMotor_R->setSpeed(mspeed);
        //Serial.println("직진");
      }
      // R검 L흰
      else if((sensor1 >= sensor1_upper) && (sensor2 <= sensor2_lower)){
        myMotor_L->run(FORWARD);
        //myMotor_R->run(BACKWARD);
        myMotor_R->run(RELEASE);
        myMotor_L->setSpeed(mspeed);
        //myMotor_R->setSpeed(mspeed);
        //Serial.println("우회전");
      }
      // R흰 L검
      else if((sensor1 <= sensor1_lower) && (sensor2 >= sensor2_upper)){
        //myMotor_L->run(BACKWARD);
        myMotor_L->run(RELEASE);
        myMotor_R->run(FORWARD);
        //myMotor_L->setSpeed(mspeed);
        myMotor_R->setSpeed(mspeed);
        //Serial.println("좌회전");
      }
      // R흰 L흰
      else if((sensor1 <= sensor1_lower) && (sensor2 <= sensor2_lower)){
        num_stop = num_stop+1;
        if(num_stop>=45){
          myMotor_L->run(RELEASE);
          myMotor_R->run(RELEASE);
        //  Serial.println("정지");
          num_stop = 0;
        }
      }
      else{
      //  Serial.println("미세조정 과정에서 문제 발생");
      }
}
```



    
    


