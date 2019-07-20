
const int trigpin = 9;
const int echopin = 8;
const int train_cnt = 10;
int train_pos = 0;
double train_cms[train_cnt] = {0.0};
double avg_cm = 0.0;
double stdev_cm = 0.0;
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigpin,OUTPUT);
  pinMode(echopin,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
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
     }
      Serial.print(cur_cm);
        Serial.println();
   }
    train_pos ++;
   
   delay(1000);
  
  
}
