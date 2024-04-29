/*
 * BaseSystem.h
 *
 *  Created on: Apr 28, 2024
 *      Author: ponpon
 */

#ifndef INC_BASESYSTEM_H_
#define INC_BASESYSTEM_H_

#include <ModBusRTU.h>

ModbusHandleTypedef hmodbus;
u16u8_t registerFrame[200];
uint16_t HeartFlag = 0;
uint16_t VacuumState;
uint16_t GripperState;
uint16_t GoalPoint;
uint16_t ShelvePosition_1 = 1;
uint16_t ShelvePosition_2 = 2;
uint16_t ShelvePosition_3 = 3;
uint16_t ShelvePosition_4 = 4;
uint16_t ShelvePosition_5 = 5;
uint16_t PickOder;
uint16_t PlaceOder;

void Heartbeat(){
	static uint64_t timestamp = 0;
		 	  if(HAL_GetTick() >= timestamp)
		 	  {
		 		  timestamp += 200;
				  //if (registerFrame[0].U16 == 18537) {
		 		 //registerFrame[0].U16 = 0x00;
		 		  registerFrame[0x00].U16 = 22881;
				  //}
		 	  }
}

void Routine(){
	static uint64_t timestamp2 = 0;
		if(HAL_GetTick() >= timestamp2) //ส่งค่าพวกนี้หลังจาก BaseSytem อ่าน Heartbeat ทุกๆ 200 ms
		{
		timestamp2 += 200;
		if (registerFrame[0x00].U16 == 18537){
			HeartFlag = 1 ; // ได้รับค่าจาก Heartbeat Protocal
		//BaseSytem Read
		registerFrame[0x04].U16 = 0b0000;
		registerFrame[0x04].U16 = 0b0001;//Gripper Movement Actual Status = 'Lead Switch 2 Status'
		registerFrame[0x10].U16 = 1; //Z-axis Moving Status = Set Shelve
		registerFrame[0x11].U16 = 19*10; //Z-axis Actual Position = 19 ค่าจริง*10
		registerFrame[0x12].U16 = 20*10; //Z-axis Actual Speed = 20
		registerFrame[0x13].U16 = 21*10; //Z-axis Acceleration = 21
		registerFrame[0x40].U16 = 22*10; //X-axis Actual Position = 22
		}
}
}
void VacuumOn_Off(){ // อ่านค่า Vacuum จากการกดปุ่มที่ BaseSytem
	if(registerFrame[0x02].U16 == 0b0000){
		VacuumState = 0; //Off
	}
	if(registerFrame[0x02].U16 == 0b0001){
		VacuumState = 1; //On
		}
}

void Gripper_Movement(){ // อ่านค่า Gripper จากการกดปุ่มที่ BaseSytem
	if(registerFrame[0x03].U16 == 0b0000){
			GripperState = 0; //Backward
		}
	if(registerFrame[0x03].U16 == 0b0001){
			GripperState = 1; //Forward
			}
}

void Set_Shelves(){ //Setting Shelve Position
	if (registerFrame[0x01].U16 == 1){
		registerFrame[0x01].U16 = 0; // z-axis reset BaseSystem status
		registerFrame[0x10].U16 = 1; // z-axis update z-xis moving status to "set shelves"

		//Joggig for set shelve
		HAL_Delay(1500); // delay 3s for testing

		registerFrame[0x23].U16 = ShelvePosition_1; // ค่า Shelve ที่ต้องส่งให้ BaseSytem
		registerFrame[0x24].U16 = ShelvePosition_2;
		registerFrame[0x25].U16 = ShelvePosition_3;
		registerFrame[0x26].U16 = ShelvePosition_4;
		registerFrame[0x27].U16 = ShelvePosition_5;

		HAL_Delay(1500);//Delay 3s for testing

		// reset z-axis moving state after finish Jogging
		//if (Finish Jogging){
		registerFrame[0x10].U16 = 0;
		//}

	}
}
void GetGoalPoint(){
	if(registerFrame[0x01].U16 == 8){ // if run point mode
		 GoalPoint = (registerFrame[0x30].U16)/10 ; //Get Goal point from BaseSytem(Point Mode) that we pick/write After pressing Run Button
		 //ค่าที่ได้จาก BaseSytem จะได้ค่าที่เรากรอก*100 ดังนั้นต้องการ10 ถึงจะได้ค่าจริงที่เรากรอก
	}
}

void RunPointMode(){
	if(registerFrame[0x01].U16 == 8){ //if run point mode
		registerFrame[0x01].U16 = 0;  //Reset BaseSystem Status
		registerFrame[0x10].U16 = 16; //update Z Status "Go Point"

		//going to point (use Goal point(0x30) for target z-axis position)
		HAL_Delay(3000); //for test
		//if (Gripper at GoalPoint){
		registerFrame[0x10].U16 = 0; // reset z-axis moving state after finish jogging
		//}
		}
	}

void SetHome(){
	if(registerFrame[0x01].U16 == 2){ //BaseSystem Status "Home"
		registerFrame[0x01].U16 = 0; //reset Base System Status
		registerFrame[0x10].U16 = 2; // update Z-axis moving status "Home"

		//Homing
		HAL_Delay(3000); //for test

		//if (Gripper at HomePoint){
		registerFrame[0x10].U16 = 0; // reset z-axis moving state after finish homing
		//}
		}
}

void GetPick_PlaceOrder(){
	if(registerFrame[0x01].U16 == 4){ // after pressing run button on Jogmode (before running)
		PickOder = registerFrame[0x21].U16 ; // ค่าชั้นที่ต้อง Pick from BaseSystem
		PlaceOder = registerFrame[0x22].U16 ;// ค่าชั้นที่ต้อง Place from BaseSystem
		//ค่าที่ได้จะเรียงติดกัน ex.ถ้าเซ็ตค่าในUIชั้นแรกที่ต้อง Pick คือ ชั้น1-5 ตามลำดับ ค่าชั้นที่ต้องPick จะได้ 12345
	}
}

void RunJogMode(){
	if(registerFrame[0x01].U16 == 4){ //after pressing run button on Jogmode
		registerFrame[0x01].U16 = 0;

		//Loop{
		//Pick
		registerFrame[0x10].U16 = 4; // go pick state
		HAL_Delay(1500); //for test

		//Going to Pick from Shelve 5 round(Use PickOder to do task)
		//When finish Pick from round(i) shelve --> Go Place

		//Place
		registerFrame[0x10].U16 = 8; // go place state
		HAL_Delay(1500); //for test

		//Going to Place from Shelve 5 round(Use PlaceOder to do task)
		//When finish Place from round(i) shelve --> Return Pick
		//When Finish Place round 5 --> Out of Loop
		//}

		//if(All Pick&Place == Finish){
		registerFrame[0x10].U16 = 0; // after finish jogging
		//}
	}
}

#endif /* INC_BASESYSTEM_H_ */
