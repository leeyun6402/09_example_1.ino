#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

#define SND_VEL 346.0     
#define INTERVAL 25      
#define PULSE_DURATION 10 
#define _DIST_MIN 100     
#define _DIST_MAX 300     

#define TIMEOUT ((INTERVAL / 2) * 1000.0) 
#define SCALE (0.001 * 0.5 * SND_VEL)     

#define _EMA_ALPHA 0.5    
#define N 10                          

unsigned long last_sampling_time; 
float dist_prev = _DIST_MAX;        
float dist_ema;                     
float dist_median;
float median_samples[N];
int median_sample_index = 0;

float calculateMedian(float arr[], int size) {
  float tempArr[size];
  for (int i = 0; i < size; i++) {
    tempArr[i] = arr[i];
  }

  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (tempArr[j] > tempArr[j+1]) {
        float temp = tempArr[j];
        tempArr[j] = tempArr[j+1];
        tempArr[j+1] = temp;
      }
    }
  }

  if (size % 2 != 0) {
    return tempArr[size/2];
  } else {
    int index1 = size/2 -1;
    int index2 = size/2;
    return (tempArr[index1] + tempArr[index2])/2.0;
  }
}

void medianFilter(float newSample) {
  median_samples[median_sample_index] = newSample;
  median_sample_index = (median_sample_index + 1) % N;
  dist_median = calculateMedian(median_samples, N);
}

void setup() {
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_TRIG,OUTPUT);
  pinMode(PIN_ECHO,INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.begin(57600);

  for (int i = 0; i < N; i++) {
    median_samples[i] = _DIST_MAX;
  }
  dist_ema = _DIST_MAX;
}

void loop() {
  float dist_raw, dist_filtered;
  
  if (millis() < last_sampling_time + INTERVAL)
    return;

  dist_raw = USS_measure(PIN_TRIG,PIN_ECHO);

  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX)) {
      dist_filtered = _DIST_MAX;
  } else if (dist_raw < _DIST_MIN) {
      dist_filtered = _DIST_MIN;
  } else {   
      dist_filtered = dist_raw;
  }

  medianFilter(dist_filtered);
  
  dist_ema = _EMA_ALPHA * dist_filtered + (1-_EMA_ALPHA)*dist_ema;

  Serial.print("Min:");   Serial.print(_DIST_MIN);
  Serial.print(",raw:"); Serial.print(dist_raw);
  Serial.print(",ema:");  Serial.print(dist_ema);
  Serial.print(",median:");  Serial.print(dist_median);
  Serial.print(",Max:");  Serial.print(_DIST_MAX);
  Serial.println("");

  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
    digitalWrite(PIN_LED, 1);       
  else
    digitalWrite(PIN_LED, 0);       

  last_sampling_time += INTERVAL;
}

float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
