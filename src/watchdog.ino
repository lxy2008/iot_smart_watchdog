///////////////////////////////////////////////////////////////

#include <SoftwareSerial.h>

#define RBUF_SIZE 64
#define LONG_DELAY 1000
#define SHORT_DELAY 3

const char* cmd_resolve_ifttt = "AT+CIPDOMAIN=\"maker.ifttt.com\"";
const char* cmd_connect_ifttt = "AT+CIPSTART=\"TCP\",\"%s\",80";
const char* cmd_send_req = "AT+CIPSEND=193";
const char* cmd_send_payload =  "GET /trigger/watchdog/with/key/c6Z9DGeyFPKUaTutX_xoSx HTTP/1.1\r\nUser-Agent: Wget/1.20.1 (darwin18.2.0)\r\nAccept: */*\r\nAccept-Encoding: identity\r\nHost: maker.ifttt.com\r\nConnection: Keep-Alive\r\n\r\n";

SoftwareSerial esp_wifi(8, 9); 

int es_cmd(SoftwareSerial* es,const char* cmd,char* parsebuf){
  int nret = -1;
  int rd = 0;
  char rbuf[RBUF_SIZE] = {0x0};

  if(cmd && strlen(cmd) > 0){
    Serial.print(">>>>>>>>>>>>>>>>>>>>>>");
    Serial.println();
    Serial.print(cmd);
    Serial.println();
    Serial.print("<<<<<<<<<<<<<<<<<<<<<<");
    Serial.println();
    es->print(cmd);
    es->println();
  }
  //delay(10);
  
  int long_delay = 0,short_delay = 0;
  do{
    if(es->available()>0){
      rbuf[rd] = es->read();
      rd ++;
    }else{
      if(strstr(rbuf,"OK") != NULL || 
          strstr(rbuf,"ERROR") != NULL){
           // short exit
           short_delay ++;
       }
      long_delay ++;
      delay(2);
    }
  }while(short_delay < SHORT_DELAY && \
          long_delay < LONG_DELAY && \
          rd < RBUF_SIZE);
  
  Serial.print("--------------------------------- long_delay@");
  Serial.print(long_delay);
  Serial.print(" short_delay@");
  Serial.print(short_delay);
  Serial.print(" rd@");
  Serial.print(rd);
  Serial.println();
  
  if(rd > 0){
    Serial.print(rbuf);
    Serial.println();
    
    if(strstr(rbuf,"OK") != NULL){
      nret = 0;
    }
    if(parsebuf != NULL){
      memcpy(parsebuf,rbuf,RBUF_SIZE);
    }
  }
  return nret;
}
const char* parse_ip(char* resp){
  char* pst = strstr(resp,":");
  if(pst != NULL){
      pst++; // move next
      char* ret = pst;
      while(pst && (isDigit(*pst) || *pst == '.')){
        pst ++;
      }
      if(pst){
        *pst = 0x0;
        Serial.print("--------------------------ip resolved:");
        Serial.print(ret);
        Serial.println();
        return ret;
      }
  }
  return NULL;
}
int ifttt_http_evt(SoftwareSerial* es){
  int nret = 0;
  char cmdbuf[128] = {0x0};
  char rbuf[RBUF_SIZE] = {0x0};
  do{
    // resolving domain "maker.ifttt.com"
    nret = es_cmd(es,cmd_resolve_ifttt,rbuf);
    if(nret != 0){
        break;
     }
     // parse ip
     const char* ip = parse_ip(rbuf);
     if(ip == NULL){
        break;
     }
     // tcp connect to ifttt
     sprintf(cmdbuf,cmd_connect_ifttt,ip);
     nret = es_cmd(es,cmdbuf,NULL);
     if(nret != 0){
        break;
     }
     // tcp send req
     nret = es_cmd(es,cmd_send_req,NULL);
     if(nret != 0){
        break;
     }
     // tcp send payload
     nret = es_cmd(es,cmd_send_payload,NULL);
     if(nret != 0){
        break;
     }
     // clear recvbuf
     int at_num = 10;
     while(at_num > 0){
      es_cmd(es,"AT",NULL); 
      at_num --;
     }
  }while(0);

ERR_RET:
  return nret;
  
}


///////////////////////////////////////////////////////////////

#define MAX_BAD 3
const int trigpin = 7;
const int echopin = 6;
const int train_cnt = 10;
int train_pos = 0;
double train_cms[train_cnt] = {0.0};
double avg_cm = 0.0;
double stdev_cm = 0.0;
int bad_cnt = 0;

double _mean(double a[], int n, double *ps)
{
  double sum = 0.0, aver = 0.0, e = 0.0;
  
  for (int i = 0; i < n; i++){
    sum += a[i];
  }
  aver = sum / n;
  
  if (ps != 0){
    for(int i=0;i<n;i++){
      e+=(a[i]-aver)*(a[i]-aver);
    }
    e/=n-1;
    *ps=sqrt(e);
  }
  
  return aver;
}


int watch_dog(){
  digitalWrite(trigpin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin,LOW);

  double cur_cm = pulseIn(echopin,HIGH)/58.0;
  
  if( train_pos < train_cnt){   // train...
    train_cms[train_pos] = cur_cm;
    Serial.print("train...");
    Serial.print(train_pos);
    Serial.print("/");
    Serial.print(train_cnt);
    Serial.println();
  }else if(train_pos == train_cnt){                      // train end
    int train_idx = 0;
    for(train_idx = 0;train_idx < train_cnt;train_idx ++){
      Serial.print("idx:");
      Serial.print(train_idx);
      Serial.print(" cm:");
      Serial.print(train_cms[train_idx]);
      Serial.println();
    }
    avg_cm = _mean(train_cms,train_cnt,&stdev_cm);
    Serial.print("avg:");
    Serial.print(avg_cm);
    Serial.print("stdev:");
    Serial.print(stdev_cm);
    Serial.println();
   } else{                  // predict
     if(avg_cm - stdev_cm <= cur_cm && cur_cm <= avg_cm + stdev_cm){
        Serial.print("good:");
     }else{
      Serial.print("bad:");
      bad_cnt ++;
      if(bad_cnt < MAX_BAD){
        // trigger ifttt event
        ifttt_http_evt(&esp_wifi);
      }
     }
      Serial.print(cur_cm);
        Serial.println();
   }
    train_pos ++;
}

///////////////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigpin,OUTPUT);
  pinMode(echopin,INPUT);
  
  esp_wifi.begin(9600);
  Serial.print("ready!");
  Serial.println();
}

void loop() {
   //////////////////////////////////////////////////////////////////
   int wr = 0,rd = 0;
   char sbuf[64] = {0x0};
  
  while(Serial.available() > 0){
    sbuf[wr] = Serial.read();
    wr++;
    delay(2);
  }
  
  if(wr > 0){
    int nret = -1;
    if(strcmp(sbuf,"IFTTT") == 0){      
      nret = ifttt_http_evt(&esp_wifi);
    }else if(strncmp(sbuf,"AT",2) == 0){
      nret = es_cmd(&esp_wifi,sbuf,NULL);
    }
    Serial.print("--------------------------------------> ");
    Serial.print(nret);
    Serial.println();
   }
   
   watch_dog();
   delay(1000);
}
