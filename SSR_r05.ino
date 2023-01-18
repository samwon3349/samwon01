//    2022.12.18
//    50kw 자동/수동모드 유압 작동 프로그램(피치만 우선 자동)
//


int solA1 = 13; //솔레노이드 A1, 실린더 후진, feathering
int solB1 = 12; //솔레노이드 B1, 실린더 전진, operating
int solA2 = 11; //솔레노이드 A2, 요회전
int solB2 = 10; //솔레노이드 B2, 요회전
int MC = 9;     //MC, 유압모터작동
int i;          //
int count = 0;  //

float runtime; // 작동시간 입력값
float loopcount; // 작동시간동안 반복루프 카운트

int mode = 1; // 초기 상태는 시스템을 정지, 수동모드 1~5, 자동모드 1~3
int in_val; // 입력값확인용
int sel_AME = 11; // 자동 A or a = 12, 수동 M or m = 11, 정지 E or e = 13 초기값 수동모드 m = 11
String sel_mode; // 자동, 수동, 비상정지 모드 선택변수
String sys_state = "System OFF,"; //system의 상태확인을 위한 변수, 초기값은 system off
String VB_state = "Valve off"; //유압모터 작동상태 확인은 위한 변수, 초기값은 MC off


void setup() {

  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("50kw System control with SSR");

  pinMode(solA1, OUTPUT);  // 솔레노이드 A1
  pinMode(solB1, OUTPUT);  // 솔레노이드 B1
  pinMode(solA2, OUTPUT);  // 솔레노이드 A2
  pinMode(solB2, OUTPUT);  // 솔레노이드 B2
  pinMode(MC, OUTPUT);   // 유압모터

}


void loop()
{
 // 메인 루프 시작
  Switch(sel_AME) { //자동, 수동, 정지모드에 따른 케이스 시작
    case 11: //  수동모드


// 설정값으로 시스템 구동먼저 시작
    switch(mode) { //수동 세부 mode에 따른 케이스문 시작
    case 1:  // case1 - 모두 차단
      sys_state = "System OFF";  // 시스템 상태 재입력
      digitalWrite(solA1, LOW);
      digitalWrite(solB1, LOW);
      digitalWrite(solA2, LOW);
      digitalWrite(solB2, LOW);
      delay(500);
      digitalWrite(MC, LOW);
    break;  // case 1 종료

    case 2: // case2 - 솔레노이드 밸브 A1 / 피치 제어 폴딩(feathering)
      sys_state = "Sol valve A1 on, folding";  // 시스템 상태 재입력
      Serial.println("input time(sec), min = 0.1sec, max = 30sec"); // 작동시간 입력안내
      delay(1000); // 입력대기 1초
      if(Serial.available()>0)
      {
        runtime = Serial.parseFloat(); // 작동시간 읽기
        if(runtime == 0 || runtime > 30)
        {
          Serial.println("Wrong input, time input again"); //숫자가 아닌 다른 입력이거나 30초이상인 경우
          count = count +1;
          if (count > 4 )
          {
            mode = 1;
            count = 0;
          }
        }
        else
        {
          loopcount = runtime * 1000; // loopcount 설정
          Serial.print("folding "); // 작동시간안내
          Serial.print(runtime);    // 작동시간 안내
          Serial.println(" sec");   // 작동시간 안내
          digitalWrite(MC, HIGH);    // 유압모터 작동
          delay(500);
          digitalWrite(solA1, HIGH);  // 실린더 후진
          for(i = 0; i <= loopcount; i +=100)
          {
             delay(100); // 입력된 작동시간동안 동작
          }
          Serial.println("Stop folding"); // 작동종료 안내
          Serial.println("");
          stops();
          mode = 1; // 작동모드를 정지모드로 변경
        }
      }
      else
      {
        count = count +1;
          if (count > 4 )
          {
            mode = 1;
            count = 0;
          }
      }
    break; // case 2 종료

    case 3: // case 3 - 솔레노이드 밸브 b1/ 피치제어 전진

      sys_state = "Sol valve B1 on, unfolding";  // 시스템 상태 재입력
      Serial.println("input unfolding time(sec), min = 0.1sec, max = 30sec"); // 작동시간 입력안내
      delay(1000); // 입력대기 1초
      if(Serial.available()>0)
      {
        runtime = Serial.parseFloat(); // 작동시간 읽기
        if(runtime == 0 || runtime > 30)
        {
          Serial.println("Wrong input, time input again"); //숫자가 아닌 다른 입력이거나 30초이상인 경우
          count = count +1;
          if (count > 4 )
          {
            mode = 1;
            count = 0;
          }
        }
        else
        {
          loopcount = runtime * 1000; // loopcount 설정
          Serial.print("unfolding "); // 작동시간안내
          Serial.print(runtime);    // 작동시간 안내
          Serial.println(" sec");   // 작동시간 안내
          digitalWrite(MC, HIGH);    // 시스템 작동용 핀 출력 송출
          delay(500);
          digitalWrite(solB1, HIGH);  // 솔레노이드 B 작동
          for(i = 0; i <= loopcount; i +=100)
          {
             delay(100); // 입력된 작동시간동안 동작
          }
          Serial.println("Stop unfolding"); // 작동종료 안내
          Serial.println("");
          stops();
          mode = 1; // 작동모드를 정지모드로 변경

        }
      }
      else
      {
        count = count +1;
          if (count > 4 )
          {
            mode = 1;
            count = 0;
          }
      }
    break; // case 3 종료

    case 4: // 요 회전
    sys_state = "Sol valve A2 on, Rotation";  // 시스템 상태 재입력
      Serial.println("input rotation time(sec), min = 0.1sec, max = 30sec"); // 작동시간 입력안내
      delay(1000); // 입력대기 1초
      if(Serial.available()>0)
      {
        runtime = Serial.parseFloat(); // 작동시간 읽기
        if(runtime == 0 || runtime > 30)
        {
          Serial.println("Wrong input, time input again"); //숫자가 아닌 다른 입력이거나 30초이상인 경우
          count = count +1;
          if (count > 4 )
          {
            mode = 1;
            count = 0;
          }
        }
        else
        {
          loopcount = runtime * 1000; // loopcount 설정
          Serial.print("rotation "); // 작동시간안내
          Serial.print(runtime);    // 작동시간 안내
          Serial.println(" sec");   // 작동시간 안내
          digitalWrite(MC, HIGH);    // 시스템 작동용 핀 출력 송출
          delay(500);
          digitalWrite(solA2, HIGH);  // 솔레노이드 B 작동
          for(i = 0; i <= loopcount; i +=100)
          {
             delay(100); // 입력된 작동시간동안 동작
          }
          Serial.println("Stop rotation"); // 작동종료 안내
          Serial.println("");
          stops();
          mode = 1; // 작동모드를 정지모드로 변경

        }
      }
      else
      {
        count = count +1;
          if (count > 4 )
          {
            mode = 1;
            count = 0;
          }
      }
    break;

    case 5: // 요 회전
     sys_state = "Sol valve A2 on, Rotation";  // 시스템 상태 재입력
      Serial.println("input rotation time(sec), min = 0.1sec, max = 30sec"); // 작동시간 입력안내
      delay(1000); // 입력대기 1초
      if(Serial.available()>0)
      {
        runtime = Serial.parseFloat(); // 작동시간 읽기
        if(runtime == 0 || runtime > 30)
        {
          Serial.println("Wrong input, time input again"); //숫자가 아닌 다른 입력이거나 30초이상인 경우
          count = count +1;
          if (count > 4 )
          {
            mode = 1;
            count = 0;
          }
        }
        else
        {
          loopcount = runtime * 1000; // loopcount 설정
          Serial.print("rotation "); // 작동시간안내
          Serial.print(runtime);    // 작동시간 안내
          Serial.println(" sec");   // 작동시간 안내
          digitalWrite(MC, HIGH);    // 시스템 작동용 핀 출력 송출
          delay(500);
          digitalWrite(solB2, HIGH);  // 솔레노이드 B 작동
          for(i = 0; i <= loopcount; i +=100)
          {
             delay(100); // 입력된 작동시간동안 동작
          }
          Serial.println("Stop rotation"); // 작동종료 안내
          Serial.println("");
          stops();
          mode = 1; // 작동모드를 정지모드로 변경

        }
      }
      else
      {
        count = count +1;
          if (count > 4 )
          {
            mode = 1;
            count = 0;
          }
      }
    break;

    default: // 오류입력 모드선택값으로 1~5 이외의 다른 입력을 했을 경우
//      Serial.println("Your input was wrong. Select Mode again"); //오류입력 확인
      mode = 1;
    break; // 오류입력 종료
    } //mode에 따른 케이스문 종료

    // 모드 선택을 위한 모드 안내문 출력
    if( mode == 1)
    {
    Serial.println("Select mode (1~5)");
    Serial.println("1: System OFF, 2: Folding, 3: Unfolding, 4: Rotation(CW), 5: Rotation(CCW)");
    delay(1000); // 모드 입력 시간을 주기 위해 2초간 대기
    //모드 선택을 위한 안내문 종료
    }

    // 모드선택 시작
    if(Serial.available()) {  // 입력이 있는 경우 모드선택
      in_val = Serial.parseInt(); // 입력값을 입력확인변수에 입력
      switch(in_val) { // 입력값에 따른 모드값 변환 케이스문 시작
        case 1: // case 1 - 입력값이 1, system off 인 경우
          Serial.println("Input value is 1, System OFF");
          mode = in_val; // 입력값에 따라 모드값 변환
        break; // case 1 종료

        case 2: // case 2 - 입력값이 2, Sol A1 실린더 후진
          Serial.println("Input value is 2, Sol A1");
          mode = in_val; // 입력값에 따라 모드값 변환
        break; // case 2 종료

        case 3: // case 3 - 입력값이 3, Sol B1
          Serial.println("Input value is 3, Sol B1 ");
          mode = in_val; // 입력값에 따라 모드값 변환
        break; // case 3 종료

        case 4: // case 4 - 입력값이 4, Sol A2
          Serial.println("Input value is 4, Sol A2 ");
          mode = in_val; // 입력값에 따라 모드값 변환
        break; // case 4 종료

        case 5: // case 5 - 입력값이 5, Sol B2
          Serial.println("Input value is 4, Sol B2 ");
          mode = in_val; // 입력값에 따라 모드값 변환
        break; // case 5 종료

        default: // 오류입력 모드선택값으로 1~5 이외의 다른 입력을 했을 경우
          Serial.print("Your input was ");
          Serial.println(in_val);
          Serial.println("Your input was wrong. Select Mode again"); //오류입력 확인
          mode = 1;
        break; // 오류입력 종료
      }  // 입력값에 따른 모드값 변환 케이스문 종료
    } // 입력이 있는 경우 모드선택 종료, 입력값이 없는 경우는 고려하지 않고 루프 재시작
    Serial.println(""); // 구별을 위해 한줄 띄우기
  break;  // 수동루프 케이스 종료, 선정된 sel_AME = 11 이므로 초기화 생략

  case 12: // 자동루프 시작
  //rpm 체크

  //자동모드 중 세부모드 선정

  case 13; // 비상정지루프 시작
    Serial.println("Energency stop start")  // 비상정지 시작 안내
    digitalWrite(MC, HIGH);    // 유압모터 작동
    delay(500);
    digitalWrite(solA1, HIGH);  // 실린더 후진
    delay(60000);   // 60초동안 실린더 후진동작 작동
    Serial.println("Stop feathering"); // 작동종료 안내
    Serial.println("");
    sel_AME = 11;  // 비상정지 후 모드를 수동모드로 전환
    stops();
  break;  // 비상정지 케이스 종료

  default:  // sel_AME를 11~13이외의 입력오류
    sel_AME = 11;   // 수동모드로 초기화
  break;
}  //자동, 수동, 비상정지 모드 루프끝
  // 자동, 수동모드 선택을 위한 모드 안내문 출력
  if( E == 11)
  {
  Serial.println("The operating mode is manual mode")
  Serial.println("Select Auto/Manual/E-Stop mode");
  Serial.println("Auto Mode: input A or a, Manual Mode: input M or m, E-Stop mode: E or e");
  delay(1000); // 모드 입력 시간을 주기 위해 2초간 대기
  //모드 선택을 위한 안내문 종료
  }
  // 자동, 수동 모드선택 시작
  if(Serial.available()) {  // 입력이 있는 경우 모드선택
    sel_mode = Serial.parseInt(); // 입력값을 입력확인변수에 입력
    if (sel_mode=="m") {  // 선택모드 m
      sel_AME = 11;
      Serial.println("You select Manual mode");
    }
    else if(sel_mode=="M") {
      sel_AME = 11;
      Serial.println("You select Manual mode");
    }
    else if(sel_mode=="a" ) {
      sel_AME = 12;
      Serial.println("You select Auto mode");
    }
    else if(sel_mode=="A") {
      sel_AME = 12;
      Serial.println("You select Auto mode");
    }
    else if(sel_mode=="E") {
      sel_AME = 13;
      Serial.println("You select Emergency Stop mode");
    }
    else if(sel_mode=="e") {
      sel_AME = 13;  //
      Serial.println("You select Emergency Stop mode");
    }
    else {
      Serial.println("Your input was wrong"); //입력값 오류
      sel_AME = 11;  //입력오류로 sel_AME 수동모드로 ''
    }
  }  // 입력값에 따른 자동, 수동 모드값 변환 종료

  } // 입력이 있는 경우 모드선택 종료, 입력값이 없는 경우는 고려하지 않고 루프 재시작
  Serial.println(""); // 구별을 위해 한줄 띄우기
}  // 메인 루프 끝

void stops()
{
  sys_state = "System OFF";  // 시스템 상태 재입력
      digitalWrite(solA1, LOW);
      digitalWrite(solB1, LOW);
      digitalWrite(solA2, LOW);
      digitalWrite(solB2, LOW);
      delay(500);
      digitalWrite(MC, LOW);
}
