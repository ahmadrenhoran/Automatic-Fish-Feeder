#include <virtuabotixRTC.h> // RTC DS1302
#include <IRremote.hpp> // IR Remote
#include <Servo.h> // Servo
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // 1602LCD
#include <math.h>

/*
  Double Linked List
  Untuk mengatur pengolahan data jam
*/
#define null NULL
#define info(P) P->info
#define next(P) P->next
#define prev(P) P->prev
#define first(P) P.first
#define last(P) P.last

typedef struct node *address;

struct node {
    int info = 0;
    address next;
    address prev;
};

struct List {
    address first;
    address last;
};

List L; 



LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Servo
Servo servo3;
int pos = 0;
byte btn = 12;
long btnValue = 0;

// RTC DS1302
virtuabotixRTC myRTC(6,7,8);

// Buzzer
int pinBuzzer= 5;

// Menu
int addData = 0, delData = 0, showData = 0;

address adrAlarm;

address adrShow;
int cursorShow;

// Data Input
int input[4] = {0, 0, 0, 0};
int index = 0;

void setup()
{ 
  Serial.begin(9600);

  IrReceiver.begin(2, ENABLE_LED_FEEDBACK);  // IR remote
  servo3.attach(3); // Servo
  servo3.write(180);
  delay(300);

 // detik, menit, jam, hari dalam seminggu, tanggal, bulan, tahun 
  myRTC.setDS1302Time(00,05,20,4,5,1,2022);
  
 // initialize the LCD
  lcd.begin();
  
 // Buzzer
  pinMode(pinBuzzer, OUTPUT);

// CreateList
  first(L) = null;
  last(L) = null;
}
void loop()
{ 
  menu();
  
}


void menu() {
  if (IrReceiver.decode()) {
      IrReceiver.resume(); // Enable receiving of the next value

    if(IrReceiver.decodedIRData.decodedRawData == 0xEA15FF00) { // tambah data
      addData++;
      addMENU:
        lcd.clear();
        lcd.print("<Add a Timer>");
        for (int i = 0; i < 3; i++) {
              input[i] = 0;
        }
        lcd.setCursor(2, 1);
        lcd.print(":");
        lcd.setCursor(0, 1);
        IrReceiver.resume();
    } else if (IrReceiver.decodedIRData.decodedRawData == 0xF807FF00) { // hapus data
      delData++;
      delMENU:
        lcd.clear();
        lcd.print("<Delete a Timer>");
        for (int i = 0; i < 3; i++) {
              input[i] = 0;
        }
        lcd.setCursor(2, 1);
        lcd.print(":");
        lcd.setCursor(0, 1);
        IrReceiver.resume();
    } else if(IrReceiver.decodedIRData.decodedRawData == 0xF609FF00) { // lihat data
      
      lcd.clear();
      if(first(L) == null) {
        lcd.print("Data is empty");
        delay(5000);
      } else {
        showData++;
        cursorShow = 0;
        adrShow = first(L);
      }
    } else if(IrReceiver.decodedIRData.decodedRawData == 0xBA45FF00) { // keluar
      addData = 0, delData = 0, showData = 0;
      index = 0;
      lcd.clear();
    }
  }

  if((addData % 2) == 1) { // add menu
    if (IrReceiver.decode()) {
      
      int data = checkRemoteCMD();
      if(data != -99) {
        
        input[index] = data;
        lcd.print(input[index]);
        index++;
        if(index == 4) {
           delay(3000);
           index = 0; 
           if((convert(input) > 2359) || ((convert(input)) < 0) || ((convertMINUTE(convert(input))) > 59)) {
              lcd.clear();
              lcd.print("Invalid Input!");
              delay(3000);
              goto addMENU;
           } else {
             lcd.clear();
             lcd.print("Data added");
             lcd.setCursor(0, 1);
             lcd.print("successfully!");
             insertData(L, convert(input)); 
             addData++;
             delay(3000);
             lcd.clear();

             address q = first(L);
             while( next(q) != null) {
              if(info(q) > ((myRTC.hours*100)+(myRTC.minutes))) break;
              q = next(q);
             }
             adrAlarm = q;
           } 
          
        } else if(index == 2) {
          lcd.setCursor(3,1);
        }
        
      }
      
    }

  } else if((delData % 2) == 1) { // del menu
    if (IrReceiver.decode()) {
      int data = checkRemoteCMD();
      if(data != -99) {
        
        input[index] = data;
        lcd.print(input[index]);
        index++;
        if(index == 4) {
          delay(3000);
          index = 0;
           if((convert(input) > 2359) || ((convert(input)) < 0) || ((convertMINUTE(convert(input))) > 59)) {
              lcd.clear();
              lcd.print("Invalid Input!");
              delay(3000);
              goto delMENU;
           } else {
             bool found = deleteData(L, convert(input));
             if(found) {
               lcd.clear();
               lcd.print("Data deleted");
               lcd.setCursor(0, 1);
               lcd.print("successfully!");
               delay(3000); 
               lcd.clear();
               addData++;

             address q = first(L);
             while( next(q) != null) {
              if(info(q) > ((myRTC.hours*100)+(myRTC.minutes))) break;
              q = next(q);
             }
             adrAlarm = q;

             } else {
               lcd.clear();
               lcd.print("Data not found!");
               delay(3000);
               goto delMENU;
             }
           }  
          
        } else if(index == 2) {
          lcd.setCursor(3,1);
        }
        
      }
      
    }

  } else if((showData % 2) == 1) {
   if(adrShow != null)  {
     if(cursorShow == 0) {
       lcd.setCursor(0, 0);
     } else if (cursorShow == 2) {
       lcd.setCursor(0, 1);
     } 
     int hour = convertHOUR(info(adrShow));
     int minute = convertMINUTE(info(adrShow));
     lcd.print("[");
     lcd.print(hour);
     lcd.print(":");
     lcd.print(minute);
     lcd.print("] ");
     adrShow = next(adrShow);
     cursorShow++;
     if(cursorShow == 4) {
       cursorShow = 0;
       delay(5000);
       lcd.clear();
     }
     
   } else {
     delay(5000);
     cursorShow = 0;
     showData++;
   }
  } else {
    if(adrAlarm != null) {
      if((myRTC.hours == convertHOUR(info(adrAlarm)))  && (myRTC.minutes == convertMINUTE(info(adrAlarm))) && (myRTC.seconds == 0) ) {
        soundNotification();
        servoGiveFood();
        
        if(next(adrAlarm) == null) {
          adrAlarm = first(L);
        } else {
          adrAlarm = next(adrAlarm);
        }
        
      }
    }
    showTime();
  }
}


void showTime() {
  myRTC.updateTime();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(myRTC.hours);
  lcd.print(":");
  lcd.print(myRTC.minutes);
  lcd.print(":");
  lcd.print(myRTC.seconds);
  lcd.print("  ");


  lcd.setCursor(0, 1);
  lcd.print("Date: ");
  lcd.print(myRTC.dayofmonth);
  lcd.print("/");
  lcd.print(myRTC.month);
  lcd.print("/");
  lcd.print(myRTC.year);
}


 
// Servo movement to give food
void servoGiveFood() {

  servo3.write(180);
  delay(300);
  servo3.write(140);
  delay(300);
  servo3.write(180);
  delay(300);
  
  
}


void soundNotification() {
  digitalWrite(pinBuzzer, HIGH);
  delay(1000);
  digitalWrite(pinBuzzer, LOW);
  delay(1000);
}

// Detecting IR remote
int checkRemoteCMD() {
  if (IrReceiver.decode()) {
    
    int i = -99;
   switch(IrReceiver.decodedIRData.decodedRawData) {
     
    case 0xE916FF00:
        i = 0;
      break;
    case 0xF30CFF00:
        i = 1;
      break;
    case 0xE718FF00:
        i = 2;
      break;
    case 0xA15EFF00:
        i = 3;
      break;
    case 0xF708FF00:
        i = 4;
      break;
    case 0xE31CFF00:
        i = 5;
      break;
    case 0xA55AFF00:
        i = 6;
      break;
    case 0xBD42FF00:
        i = 7;
      break;
    case 0xAD52FF00:
        i = 8;
      break;
    case 0xB54AFF00:
        i = 9;
      break;
    }
    return i;
  }
}






address createElement(int data) {
  address P = new node;
  info(P) = data;
  next(P) = null;
  prev(P) = null;

  return P;
}

void printList(List L) {

    if(first(L) != null) {

        address P = first(L);
        while(P != null) {
            lcd.print(info(P));
            lcd.clear();
            P = next(P);
        }
        

    }
}

void insertData(List &L, int data) { // data ascending dari first sampai last
  if(first(L) == null) {
    insertFirst(L, data);
  } else {
    if(data < info(first(L))) {
        insertFirst(L, data);
    } else if( data > info(last(L))) {
        insertLast(L, data);
    } else {
        address q = first(L);
        while(data > info(next(q))) {
            q = next(q);
        }
        address p = createElement(data);
        next(p) = next(q);
        prev(p) = q;
        prev(next(q)) = p;
        next(q) = p;
    }
  }
}

void insertFirst(List &L, int data) {
    address p = createElement(data);
    if(first(L) == null) {
        first(L) = p;
        last(L) = p;
    } else {
        next(p) = first(L);
        prev(first(L)) = p;
        first(L) = p;
    }
}

void insertLast(List &L, int data) {
  address p = createElement(data);
  if(first(L) == null) {
    insertFirst(L, data);
  } else {
    next(last(L)) = p;
    prev(p) = last(L);
    last(L) = p;
  }
}


boolean deleteData(List &L, int data) { // -> true if the data is exist

  if(first(L) != null) {
    address q = first(L);
      while(info(q) != data & q != null) {
        q = next(q);
      }
      if(info(q) == data) {
        if(q == first(L)) {
          deleteFirst(L);
        } else if (q == last(L)) {
          deleteLast(L);
        } else {
          address p = prev(q);
          next(p) = next(q);
          prev(next(q)) = p;
          next(q) = null;
          prev(q) = null;
        }
        return true;
      } else {
        return false;
      }
  }
}

void deleteFirst(List &L) {
    if (first(L) == last(L)) {
      first(L) = null;
      last(L) = null;
    } else {
      address p = next(first(L));
      prev(p) = null;
      next(first(L)) = null;
      first(L) = p;
    }
}

void deleteLast(List &L) {
  if(first(L) == last(L)) {
    deleteFirst(L);
  } else {
    address p = prev(last(L));
    prev(last(L)) = null;
    next(p) = null;
    last(L) = p;
  }
}

int convert(int input[]) { // convert int[] ke int, e.g [1, 7, 5, 6] -> 1756
  int res =0;
  for(int i = 0; i < 4; i++) {
    res +=  ceil(input[3-i] * pow(10, i)); // pembulatan ke atas
  }
  return res;
}

int convertHOUR(int input) { // convert ke jam, e.g 1756 -> 17

  int digit1 = (input / 1000) * 10; 
  int digit2 = (input - (digit1 * 100)) / 100;

  return digit1 + digit2;


}

int convertMINUTE(int input) { // convert ke menit,  e.g 1756 -> 56
    int digit1 = input - (convertHOUR(input) * 100);
    int digit2 = input - ((convertHOUR(input) * 100) + digit1);

    return digit1 + digit2;
}
