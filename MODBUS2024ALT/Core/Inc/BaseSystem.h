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
int HeartFlag = 0;
int VacuumState;
int GripperState;
float GoalPoint;
float ShelvePosition_1 = 1;
float ShelvePosition_2 = 2;
float ShelvePosition_3 = 3;
float ShelvePosition_4 = 4;
float ShelvePosition_5 = 5;
int PickOder;
int PlaceOder;
int PickShelves;
int PlaceShelves;

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
		registerFrame[0x04].U16 = 0000000000000000;
		registerFrame[0x04].U16 = 0000000000000001;//Gripper Movement Actual Status = 'Lead Switch 2 Status'
		registerFrame[0x10].U16 = 0001; //Z-axis Moving Status = Set Shelve
		registerFrame[0x11].U16 = 190; //Z-axis Actual Position = 19 ค่าจริง*10
		registerFrame[0x12].U16 = 200; //Z-axis Actual Speed = 20
		registerFrame[0x13].U16 = 210; //Z-axis Acceleration = 21
		registerFrame[0x40].U16 = 220; //X-axis Actual Position = 22
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
	if (registerFrame[0x01].U16 == 0b0001){
		registerFrame[0x01].U16 = 0b0000; // z-axis reset BaseSystem status
		registerFrame[0x10].U16 = 0b0001; // z-axis update z-xis moving status to "set shelves"

		//Joggig for set shelve
		registerFrame[0x23].U16 = ShelvePosition_1; // ค่า Shelve ที่ต้องส่งให้ BaseSytem
		registerFrame[0x24].U16 = ShelvePosition_2;
		registerFrame[0x25].U16 = ShelvePosition_3;
		registerFrame[0x26].U16 = ShelvePosition_4;
		registerFrame[0x27].U16 = ShelvePosition_5;

		// reset z-axis moving state after finish Jogging
		//if (Finish Jogging){
		registerFrame[0x10].U16 = 0b0000;
		//}

	}
}
void GetGoalPoint(){
	if(registerFrame[0x01].U16 == 0000000000001000){ // if run point mode
		 GoalPoint = registerFrame[0x30].U16 ; //Get Goal point from BaseSytem(Point Mode) that we pick/write After pressing Run Button
	}
}

void RunPointMode(){
	if(registerFrame[0x01].U16 == 0000000000001000){ //if run point mode
		registerFrame[0x01].U16 = 0b0000;  //Reset BaseSystem Status
		registerFrame[0x10].U16 = 0b00010000; //update Z Status "Go Point"

		//going to point (use Goal point(0x30) for target z-axis position)

		//if (Gripper at GoalPoint){
		registerFrame[0x10].U16 = 0b0000; // reset z-axis moving state after finish jogging
		//}
		}
	}

void SetHome(){
	if(registerFrame[0x01].U16 == 0b0010){ //BaseSystem Status "Home"
		registerFrame[0x01].U16 = 0b0000; //reset Base System Status
		registerFrame[0x10].U16 = 0b0010; // update Z-axis moving status "Home"

		//Homing

		//if (Gripper at HomePoint){
		registerFrame[0x10].U16 = 0b0000; // reset z-axis moving state after finish homing
		//}
		}
}

void GetPick_PlaceOrder(){
	if(registerFrame[0x01].U16 == 0b0100){ // after pressing run button on Jogmode (before running)
		PickOder = registerFrame[0x21].U16 ; // ค่าชั้นที่ต้อง Pick from BaseSystem
		PlaceOder = registerFrame[0x22].U16 ;// ค่าชั้นที่ต้อง Place from BaseSystem
	}
}

void RunJogMode(){
	if(registerFrame[0x01].U16 == 0b0100){ //after pressing run button on Jogmode
		registerFrame[0x01].U16 = 0b0000;

		//Loop{
		//Pick
		registerFrame[0x10].U16 = 0b0100; // go pick state
		PickShelves = registerFrame[0x21].U16; // ค่าที่ได้จาก BaseSystem from Set Pick Shelve

		//Going to Pick from Shelve 5 round(Use PickShelve to do task)
		//When finish Pick from round(i) shelve --> Go Place

		//Place
		registerFrame[0x10].U16 = 0b1000; // go place state
		PlaceShelves = registerFrame[0x22].U16; // ค่าที่ได้จาก BaseSystem from Set Place Shelve

		//Going to Place from Shelve 5 round(Use PlaceShelve to do task)
		//When finish Place from round(i) shelve --> Return Pick
		//When Finish Place round 5 --> Out of Loop
		//}

		//if(All Pick&Place == Finish){
		registerFrame[0x10].U16 = 0b0000; // after finish jogging
		//}
	}
}

#endif /* INC_BASESYSTEM_H_ */
