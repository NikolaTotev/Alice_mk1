#include <SPI.h>
//#include "AS5048.h"
#include <esp_now.h>
#include <thread>
#include <WiFi.h>

typedef struct message {
  int joint_id;
  float angle;
} message;

typedef struct joint_state{
  unsigned long last_update;

  bool joint_data_received[3] = {false,false,false};
  bool joint_data_updated[3] = {false,false,false};
  bool send_state[3] = {false,false,false};
  bool delivery_state[3] = {false,false,false};

  float joint_angles[3] = {0,0,0};
  int current_joint = 0;
} joint_state;

// ===============================================================

esp_now_peer_info_t  peerInfo;
bool send_loop_active = false;
float incoming_angle = 0;
message readings; 
message  incoming_reading;
joint_state state;
int joint_count = 2;
uint8_t joints[][6] = {
  //{0x64, 0xe8, 0x33, 0x8b, 0xec, 0xc8},
  {0xb0, 0x81, 0x84, 0x04, 0x44, 0x3c},
  {0x64, 0xe8, 0x33, 0x88, 0xe2, 0xa8}  
};


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println("Sending");
  Serial.println(status);
  state.delivery_state[state.current_joint] = (status==0);
}

void OnDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incoming_data, int len) {
  memcpy(&incoming_reading, incoming_data, sizeof(incoming_reading));

  int sender = incoming_reading.joint_id;
  incoming_angle = incoming_reading.angle;
  Serial.println(incoming_angle);
  state.joint_angles[sender-1] = incoming_angle;
  state.joint_data_received[sender-1] = true;
  state.joint_data_updated[sender-1] = true;
}

void read_joint(int joint) {
  esp_err_t result = esp_now_send(joints[joint], (uint8_t*)&readings, sizeof(readings));
  Serial.println("Requesting angle:");
  Serial.println(result);
  state.send_state[joint-1] = (result==ESP_OK);
}


void send_init_message() {
  esp_err_t result = esp_now_send(joints[0], (uint8_t *)&readings, sizeof(readings));

  if(result==ESP_OK){

  }
  else{

  }

  update_indicators();
}



bool is_ready_to_send() {
  return state.joint_data_updated[0] 
  && state.joint_data_updated[1];
  // && state.joint_data_updated[2];
}

void reset_state_flags(){
  for(int i = 0; i < joint_count; i++){
    state.joint_data_updated[i] = false;
    state.send_state[i] = false;
    state.delivery_state[i] = false;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if(esp_now_init()!=ESP_OK){
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  for(int i =0; i < joint_count; i++) {
    memcpy(peerInfo.peer_addr, joints[i], 6);
    peerInfo.channel=0;
    peerInfo.encrypt=false;

    if(esp_now_add_peer(&peerInfo)!=ESP_OK){
      return;
    }
  }
   esp_now_register_recv_cb(OnDataRecv);
   readings.angle=0;
   send_loop_active =true;
   send_init_message();
}



void loop() {
      read_joint(state.current_joint);

  if(state.joint_data_received[state.current_joint]){
    state.joint_data_received[state.current_joint] = false;
    state.joint_data_updated[state.current_joint] = true;


    if(state.current_joint<joint_count-1) {
      state.current_joint +=1;
    }
    else{
      state.current_joint =0;
    }

    read_joint(state.current_joint);
  }
  else{
    Serial.println("Waiting for joints");
  }
  
  if(is_ready_to_send()) {
    //send_spi_data();
    send_serial_data();
    reset_state_flags();
  }
  delay(500);
}


void update_indicators(){
  
}



void send_spi_data() {

}

void send_serial_data() {
  unsigned long timestamp = millis();
  state.last_update = timestamp;
  String message = "";

  message.concat(timestamp);
  message.concat("_");

  message.concat(state.joint_angles[0]);
  message.concat("_");

  message.concat(state.joint_angles[1]);
  message.concat("_");

  // message.concat(state.joint_angles[2]);
  // message.concat("_");

  message.concat("\n");

  Serial.print(message);
}











