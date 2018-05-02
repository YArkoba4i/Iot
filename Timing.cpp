#include "Timing.h"


Timing::Timing() : RtcDS1307<TwoWire>(Wire)
{

	Wire.begin(_SDA, _SCL);
	
}


Timing::~Timing()
{
}

void Timing::initTime() {

	this->Begin();
	this->SetIsRunning(true);
	
	for (size_t i = 0; i < 5; i++)
	{

		if (this->GetIsRunning()) {
			if (this->IsDateTimeValid())
				return;
		}
		//Serial.printf("initTime %d", i);
	}

//	this->printTime(this->GetDateTime());
}

void Timing::setDateTime() {
	RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
	printDateTime(compiled);
	Serial.println();

	SetDateTime(compiled);
}

//----------------------------------------------------------------------------------
// getTimeNow(
//----------------------------------------------------------------------------------
time_t Timing::getTimeNow() {
	
	RtcDateTime now = this->GetDateTime();
	return  (time_t)now.Epoch64Time();
	//return  now.Epoch32Time();

}

//----------------------------------------------------------------------------------
// increaseTime(uint min);
//----------------------------------------------------------------------------------
void Timing::increaseTime(uint sec) {

	RtcDateTime now = this->GetDateTime();
	uint32_t next = now.TotalSeconds();

	next = next + sec;

	this->SetDateTime(RtcDateTime(next));

}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
uint8_t Timing::getHH() {


	RtcDateTime now = this->getTimeNow();
	return now.Hour();

}


//----------------------------------------------------------------------------------
//		uint8_t Timing::getMM()
//----------------------------------------------------------------------------------
uint8_t Timing::getMM() {

	RtcDateTime now = this->getTimeNow();
	return now.Minute();
}


//----------------------------------------------------------------------------------
//		uint8_t Timing::getSS()
//----------------------------------------------------------------------------------
uint8_t Timing::getSS() {

	RtcDateTime now = this->getTimeNow();
	return now.Second();
}

//----------------------------------------------------------------------------------
//	uint16_t Timing::getAMwkUPmins()
//----------------------------------------------------------------------------------
uint16_t Timing::getAMwkUPmins() {

	
	if (getHH()  < Wk_UP_Hr) // if night
	{
		//Serial.println("Night");
		return  (60 * (Wk_UP_Hr - (getHH() + 1))) + (60 - getMM());
	}
	else if (getHH() > Wk_UP_Hr)
	{
		return (60 - getMM()) + ((24 - ((getHH() + 1) - Wk_UP_Hr)) * 60);
	}
	else
	{
		return 0;
	}

}


//----------------------------------------------------------------------------------
//	uint16_t Timing::getAMWakeUPSecons()
//----------------------------------------------------------------------------------
uint32_t Timing::getAMWakeUPSecons() {

	time_t next = getTimeNow();
//	time_t next = 0;
	printTime(next);

	//time(&next);
	struct tm beg;
	beg = *localtime(&next);

	//if((beg.tm_hour > 6)  )
	
	beg.tm_mday++;
	beg.tm_hour = Wk_UP_Hr;
	beg.tm_min = 0;
	beg.tm_sec = 0;

	next = mktime(&beg);

	//Serial.println("Wake UP time is ");
	printTime(next);


	uint32_t ss_left = (uint32_t)difftime(next, getTimeNow());

	return ss_left;

}


//----------------------------------------------------------------------------------
//		isFreshStart
//----------------------------------------------------------------------------------
bool Timing::isFreshStart(time_t timeNow, time_t mesure_time) {
	//;
	time_t timeDiff = (timeNow - mesure_time);

	struct tm * timeinfo;
	timeinfo = localtime(&timeDiff);
	//Serial.printf("Time Diff = %d\n", timeinfo->tm_hour);

	if ((uint)timeinfo->tm_hour > this->two_days) {

		return true;
	}
	else
		return false;
}



//----------------------------------------------------------------------------------
//	isDay()
//----------------------------------------------------------------------------------
bool Timing::isDay() {

	//Serial.println("bool Timing::isDay()");

	if ((this->getHH() >= Wk_UP_Hr - 1) && (this->getHH() <= Sleep_Hr))
	{

		Serial.printf("\nIs Day = %d:%d \n", this->getHH(), this->getMM());
		return true;
	}
	else {

//		Serial.printf("\nIs Night = %d:%d \n", this->getHH(), this->getMM());
		return false;
	}
}



//----------------------------------------------------------------------------------
//	isWakeUPHour()
//----------------------------------------------------------------------------------
bool Timing::isWakeUPHour() {


	if (((getHH() == (Wk_UP_Hr - 1)) && (getMM() > 40))  // > 05:40 
		|| ((getHH() == Wk_UP_Hr) && (getMM() < 15))) // 6:15 <

	{
		Serial.printf("\nIs Wake UP Hour = %d:%d \n", this->getHH(), this->getMM());
		return true;
	}
	else
	{
//		Serial.printf("\nIs NOT Wake UP Hour = %d:%d \n", this->getHH(), this->getMM());
		return false;
	}

}


//----------------------------------------------------------------------------------
//	isDayHours()
//----------------------------------------------------------------------------------
bool Timing::isDayHours() {


	if (((getHH() == Measure_Hr-1) && (getMM() >= 55)) // > 06:55 
		|| ((getHH() >= Measure_Hr) && (getHH() < Sleep_Hr - 1)) // > 7:00 & 21:00 <
		|| ((getHH() == (Sleep_Hr - 1)) && (getMM() <= 50))) // 21:50 <=
	{
		Serial.printf("\nIs Day Hours = %d:%d \n", this->getHH(), this->getMM());
		return true;
	}
	else
	{
//		Serial.printf("\nIs NOT Day Hours = %d:%d \n", this->getHH(), this->getMM());
		return false;
	}

}


//----------------------------------------------------------------------------------
//	isSleepHour()
//----------------------------------------------------------------------------------
bool Timing::isSleepHour() {


	if (((getHH() == (Sleep_Hr - 1)) && (getMM() > 50)) // > 21:50
		|| ((getHH() == Sleep_Hr) && (getMM() < 10))) // < 22:10
	{
		Serial.printf("\nIs Sleep Hours = %d:%d \n", this->getHH(), this->getMM());
		return true;
	}
	else
	{
//		Serial.printf("\nIs NOT Sleep Hours = %d:%d \n", this->getHH(), this->getMM());
		return false;
	}

}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
uint16_t Timing::getNextMeasuringMinLeft() {

	double  intpart;

	modf((double)getMM() / INTERVAL, &intpart);

	uint8_t mm_left = (uint8_t)((intpart + 1) * INTERVAL - getMM());

	if (mm_left < 2)
		mm_left = INTERVAL;

	return mm_left;

}



//----------------------------------------------------------------------------------
//	getNextMeasuringSecLeft()
//----------------------------------------------------------------------------------
uint Timing::getNextMeasuringSecLeft() {

	//Serial.println("getNextMeasuringSecLeft() ...");
	time_t next = getTimeNow();
	//printTime(next);

	double  intpart;

	double fractpart = modf((double)getMM() / INTERVAL, &intpart);

	// getting next minute interval measure
	uint nextMin = (uint)((intpart + ceil(fractpart))*INTERVAL);


	// time structure
	//ctime(&next);
	struct tm beg;
	beg = *localtime(&next);

	// setting next minute interval measure
	if (nextMin == 60) { nextMin = 0; beg.tm_hour++; }
	beg.tm_min = nextMin;
	beg.tm_sec = 0;

	next = mktime(&beg);

	//Serial.println("Next measuring time is ");
	//printTime(next);

	uint ss_left = (uint)fabs(difftime(next, getTimeNow()));
	//Serial.printf("ss_left = %d\n", ss_left);
	if (ss_left < 180) {

		next += INTERVAL * 60;

		//Serial.println("Next measuring time is ");
		//printTime(next);


		uint ss_left = (uint)fabs(difftime(next, getTimeNow()));
		return ss_left;
	}
	else
	{
		return ss_left;
	}
}

//----------------------------------------------------------------------------------
//	getNextMeasuringSecLeft
//----------------------------------------------------------------------------------
uint Timing::getNextMeasuringSecLeft(uint8_t hour, uint8_t minute) {


	time_t next = getTimeNow();
	//	printTime(next);

	//time(&next);
	struct tm beg;
	beg = *localtime(&next);

	double  intpart;
	modf((double)getMM() / INTERVAL, &intpart);
	beg.tm_hour = hour;
	beg.tm_min = minute;
	beg.tm_sec = 0;


	next = mktime(&beg);

//	Serial.println("Next measuring time is ");
	printTime(next);

	uint ss_left = (uint)fabs(difftime(next, getTimeNow()));

	if (ss_left < 240) {
		beg.tm_min = ++intpart * INTERVAL;
		next = mktime(&beg);

		//Serial.println("Next measuring time is ");
		printTime(next);


		uint ss_left = (uint)fabs(difftime(next, getTimeNow()));
		return ss_left;
	}
	else
	{
		return ss_left;
	}
}

//----------------------------------------------------------------------------------
//	printTime(time_t time)
//----------------------------------------------------------------------------------
void Timing::printTime(time_t time) {
	Serial.printf("\n%s\n", asctime(gmtime(&time)));
}

void Timing::printDateTime(const RtcDateTime& dt)
{
	char datestring[20];

	snprintf_P(datestring,
		countof(datestring),
		PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
		dt.Month(),
		dt.Day(),
		dt.Year(),
		dt.Hour(),
		dt.Minute(),
		dt.Second());
	Serial.print(datestring);
}