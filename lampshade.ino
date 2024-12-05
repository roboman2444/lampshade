//int onoffpin	= 18;
//int dimpin	= 19;

//i can't pwm on 19
int dimpin = 11;
int onoffpin = 12;



int button1 = 19;	//analog, 3 buttons
int button2 = 18;	//analog, 2 buttons
int button3 = 17;	//digital, one button

int ledpin2 = 16;
int ledpin1 = 15;

void panelinit(){
	digitalWrite(onoffpin, LOW); // initial state
	digitalWrite(dimpin, LOW); // initial state

	//this works


	digitalWrite(dimpin, HIGH); //go up
	delay(100);
//	delay(50);
	digitalWrite(dimpin, LOW); // 500pass


	//now we are at the thing
	delay(2);


	int i;
	for(i = 0; i < 20; i++){
		digitalWrite(dimpin, HIGH);
		delayMicroseconds(50);
		digitalWrite(dimpin, LOW);
		delayMicroseconds(3950);
	}

	//finished burst
//	digitalWrite(dimpin, HIGH);
	delay(100);
	digitalWrite(onoffpin, HIGH);
}

void paneloff(){
	digitalWrite(onoffpin, LOW); // initial state
	digitalWrite(dimpin, LOW); // initial state
}
void setup() {

	Serial.begin(115200);

	pinMode(onoffpin, OUTPUT);
	pinMode(dimpin, OUTPUT);

	pinMode(button3, INPUT_PULLUP);

	pinMode(ledpin1, OUTPUT);
	pinMode(ledpin2, OUTPUT);

	digitalWrite(ledpin1, LOW); // initial state
	digitalWrite(ledpin2, LOW); // initial state

	paneloff();
//	panelinit();
}


#define PINSCHANGEMAX 255
int pinschange[6] = {0};
char pinstate = 0;

void readbuttons(){

	//populate internal pinstate
	char res = 0;

	//digital, internally pulled up. Goes low when pressed
	res |= !(digitalRead(button3));

	//analog. Externally pulled up. Goes kinda-low when pressed
	int a2 = analogRead(button2);
	int a1 = analogRead(button1);

/*
	Serial.print("a1: ");
	Serial.print(a1);
	Serial.print(" a2: ");
	Serial.print(a2);
	Serial.print("\n");
*/
	//parsing

	//a little gross.... maybe clean later
	if(a1 < 1000){
	// 768, 512, 922
	// 437, 484, 708
	// 418
		if(a1 < (768+20) && a1 > (768-20))	res |= 2;
		else if(a1 < (512+20) && a1 > (512-20))	res |= 4;
		else if(a1 < (922+20) && a1 > (922-20))	res |= 8;

		else if(a1 < (437+20) && a1 > (437-9))	res |= (2 | 4);
		else if(a1 < (484+20) && a1 > (484-20))	res |= (4 | 8);
		else if(a1 < (708+20) && a1 > (708-20))	res |= (2 | 8);

		else if(a1 < (418+9) && a1 > (418-20))	res |= (2 | 4 | 8);
	}
	if(a2 < 1000){
	//	512 768
	//	438
		if(a2 < (512+20) && a2 > (512-20))	res |= 16;
		else if(a2 < (768+20) && a2 > (768-20))	res |= 32;
		else if(a2 < (438+20) && a2 > (438-20))	res |= (32 | 16);
	}

//	Serial.println((int)res);


	//compare to previous state, increment counter if same
	//im sure i could fancy this with some bitshit, but this is very readable
	int i;
	char pinsame = ~(pinstate ^ res);
	for(i = 0; i < 6; i++){
		if(pinsame & (1<<i)){
			if(pinschange[i] < PINSCHANGEMAX) pinschange[i]++;
		}
		else pinschange[i] = 0;
	}

	//finally set previous state to current
	pinstate = res;
}


char powerstate = 0;
char powerstateupdate = 0;

int bright = 0;

void buttonlogic() {
	//only trigger ONCE per press, at the 5 cycle point
	//power
	if(pinstate & 0x1 && pinschange[0] == 5){
		//power button pressed
		powerstate = !powerstate;
		powerstateupdate = 1;
	}

	//maxbright
	if(pinstate & 0x32 && pinschange[5] == 5){
		bright = 255;
	}

	//upbright
	if(pinstate & 0x16 && pinschange[4] == 5){
		bright += 16;
		if(bright > 255) bright = 255;
	}

	//midbright
	if(pinstate & 0x8 && pinschange[3] == 5){
		bright = 128;
	}

	//downbright
	if(pinstate & 0x4 && pinschange[2] == 5){
		bright -= 16;
		if(bright < 0) bright = 0;
	}


	//minbright
	if(pinstate & 0x2 && pinschange[1] == 5){
		bright = 0;
	}


}

void ledlogic() {
	digitalWrite(ledpin1, !!pinstate);
	digitalWrite(ledpin2, powerstate);
}


#define INTERVAL 10
unsigned long prevmillis = 0;
void loop() {
	readbuttons();
	buttonlogic();
	ledlogic();

	if(powerstate){
		if(powerstateupdate){
			panelinit();
		}

		analogWrite(dimpin, bright);
	} else {
		if(powerstateupdate){
			paneloff();
		}
	}
	powerstateupdate = 0;


	//somewhat self-correcting timer
	//delay if i got through everything in time
	//and then, only delay what's left
	unsigned long curmillis = millis();
	if(curmillis - prevmillis < INTERVAL)
		delay((prevmillis + INTERVAL) - curmillis);
//		Serial.println((prevmillis + INTERVAL) - curmillis);
	prevmillis = curmillis;
/*
	delay(5000);
	digitalWrite(dimpin, LOW);
	delay(5000);
	digitalWrite(dimpin, HIGH);


	//i could do this...
	//or i could pwm it?

	//PWM uses different pins.... i don't feel like switching them up

	delay(5000);


	digitalWrite(dimpin, LOW);
	analogWrite(dimpin, 127);
	delay(1000);
	digitalWrite(dimpin, LOW);
*/
/*
	for(int i = 0; i < 1000; i++){
		digitalWrite(dimpin, LOW);
		delay(1);
		digitalWrite(dimpin, HIGH);
		delay(1);
	}
*/



}
