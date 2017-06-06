/*
 * Main Arduino Code for ITSP.
 * 
 * Written by Abhisek Sahu

Problem Statement:
recieves data from IP code : casetype , step pin, dir pin and time and type of rotation for each stepper motor.
moves all the steppers at 300 RPM ;adhering to the constraint that the amount of unwinding to unwinding is always greater than a critical ration so that the unwound 
thread never feels any tension, and in the shortest possible time.
 
 */

void setup() {

  Serial.begin(115200);

// necessary for moving the dartboard from lower left corner to the center.
//hard-coded according to board values.
//thise movement is similar to our casetype 1.
if(Serial.available()==0)
 {
  pinMode(8,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(11,OUTPUT);
  
  digitalWrite( 8 , HIGH);
 digitalWrite( 2 , HIGH);
 digitalWrite( 4 , HIGH);
 digitalWrite( 12 , LOW);

 for( int j=0 ; j< 183 ; j++)
 {
  digitalWrite(11,HIGH);
  delayMicroseconds(500);
  digitalWrite(11,LOW);
  delayMicroseconds(500);
 }

 for (int i=0; i< 206 ; i++)
 {
  digitalWrite(9,HIGH);
  if(i >= 158) {digitalWrite(5,HIGH);}
  if(i >= 119) {digitalWrite(3,HIGH);}

  delayMicroseconds(500);

   digitalWrite(9,LOW);
  if(i >= 158) {digitalWrite(5,LOW);}
  if(i >= 119) {digitalWrite(3,LOW);}


  delayMicroseconds(500);
  }
 }
  //this part is for movement from centre to desired position.
   while(Serial.available()==0)
  {
}
  
  int Casetype = Serial.parseInt();
  
  //Serial.println(Casetype);
  // takes value 1 of only 1 winding, or 2 when 2 windings take place;
  
  switch(Casetype) {
    case 1 : 

    {

    while(Serial.available()==0) { //Serial.println("..1");
   };
          int unwinPin1 = Serial.parseInt() ; // stepPin no. of the unwinding pin with the highest no. of steps
          Serial.println(unwinPin1);

    while(Serial.available()==0)   { //Serial.println("..2");
    };             //Serial.println(dunwinPin1);

    int dunwinPin1= Serial.parseInt(); // dirpin no. of the stepper with the highest no. of steps
          
        while(Serial.available()==0) { //Serial.println("..3");
    };
        int tu1=  Serial.parseInt()       ; // no. of steps of the unwinding pin with highest no. of steps
        //Serial.println(tu1);
        
          while(Serial.available()==0) { // Serial.println("..4");
    };
          int unwinPin2=Serial.parseInt()   ; // stepPin no. of the unwinding pin with the 2nd highest no. of steps
          //Serial.println(unwinPin2);
          
          while(Serial.available()==0) { //Serial.println("..5");
    };
          int dunwinPin2= Serial.parseInt() ; // dir Pin no. of the unwinding pin with the 2nd highest no. of steps
          //Serial.println(dunwinPin2);
          
          while(Serial.available()==0) {
            // Serial.println("..6");
    };
          int tu2= Serial.parseInt()   ; //  no. of steps  of the unwinding pin with the 2nd highest no. of steps
          //Serial.println(tu2);
          
          while(Serial.available()==0){
             //Serial.println("..7");
    };
          int unwinPin3= Serial.parseInt()  ;
          //Serial.println(unwinPin3);
          
          while(Serial.available()==0) {
             //Serial.println("..8");
    };
           int dunwinPin3= Serial.parseInt();
           //Serial.println(dunwinPin3);
           
           while(Serial.available()==0) {
             //Serial.println("..9");
    };
          int tu3= Serial.parseInt()        ;
          //Serial.println(tu3);
          
          while(Serial.available()==0) {
             //Serial.println("..10");
   };
          int winPin1= Serial.parseInt()         ;
          //Serial.println(winPin1);
          
          while(Serial.available()==0) {
             //Serial.println("..11");
    };
           int dwinPin1=Serial.parseInt() ;
           //Serial.println(dwinPin1);
           
           while(Serial.available()==0) {
             //Serial.println("..12");
   };
          int tw1=Serial.parseInt() ;
         // Serial.println(tw1);
            
            
            pinMode(unwinPin1,OUTPUT);
            pinMode(unwinPin2 ,OUTPUT);
            pinMode(unwinPin3 ,OUTPUT);
            pinMode(winPin1,OUTPUT);
     
            pinMode(dunwinPin1 ,OUTPUT);
            pinMode(dunwinPin2 ,OUTPUT);
            pinMode(dunwinPin3 ,OUTPUT);
            pinMode(dwinPin1 ,OUTPUT);

            digitalWrite(dunwinPin1 , HIGH);
            digitalWrite(dunwinPin1 , HIGH);
            digitalWrite(dunwinPin1 , HIGH);
            digitalWrite(dwinPin1 , LOW);
            
if(Serial.available()==0)

         { if(tw1 <= tu1)
           {
           
            
            
            for(int i=0 ; i< tu1 ; i++)
             {
               if(i< tu2) {digitalWrite(unwinPin2, HIGH);}
                if(i< tu3) {digitalWrite(unwinPin3, HIGH);} 
                 if( i >= tu1-tw1) {digitalWrite(winPin1, HIGH);}
                  digitalWrite(unwinPin1, HIGH);

                  
                  
               delayMicroseconds(500);
               
               if(i< tu2) {digitalWrite(unwinPin2, LOW);}
                if(i< tu3) {digitalWrite(unwinPin3, LOW);} 
                 if( i >= tu1-tw1) {digitalWrite(winPin1, LOW);}
                  digitalWrite(unwinPin1, LOW);
                  
               delayMicroseconds(500);
                //Serial.println("end of an interation loop 1");
                //Serial.println(i);
            }
           }

          if(tw1 > tu1)
          { 
            for(int i=0 ; i< tw1 ; i++)
            {
            if(i< tu2) {digitalWrite(unwinPin2, HIGH);}
                if(i< tu3) {digitalWrite(unwinPin3, HIGH);} 
                 if( i < tu1) {digitalWrite(unwinPin1, HIGH);}
                  digitalWrite(winPin1, HIGH);
                  
               delayMicroseconds(500);
               
               if(i< tu2) {digitalWrite(unwinPin2, LOW);}
                if(i< tu3) {digitalWrite(unwinPin3, LOW);} 
                 if( i < tu1) {digitalWrite(unwinPin1, LOW);}
                  digitalWrite(winPin1, LOW);
                  
               delayMicroseconds(500);
                //Serial.println("end of an interation loop 2");
                //Serial.println(i);
            }
          }

         } 

    }
      break;
  

     case 2 :
     {
     while(Serial.available()==0){};
           int unwinPin1 =Serial.parseInt()  ;

           while(Serial.available()==0){};
            int dunwinPin1=Serial.parseInt() ;

            while(Serial.available()==0){};
          int tu1= Serial.parseInt()        ;

          while(Serial.available()==0){};
          int unwinPin2=  Serial.parseInt() ;

          while(Serial.available()==0){};
           int dunwinPin2=Serial.parseInt() ;

           while(Serial.available()==0){};
          int tu2= Serial.parseInt()        ;

          while(Serial.available()==0){};
          int winPin1= Serial.parseInt()  ;

          while(Serial.available()==0){};
           int dwinPin1= Serial.parseInt();

           while(Serial.available()==0){};
          int tw1= Serial.parseInt()        ;

          while(Serial.available()==0){};
          int winPin2=Serial.parseInt()          ;

          while(Serial.available()==0){};
           int dwinPin2= Serial.parseInt();

           while(Serial.available()==0){};
          int tw2=Serial.parseInt() ;

           pinMode(unwinPin1,OUTPUT);
            pinMode(unwinPin2 ,OUTPUT);
            pinMode(winPin2 ,OUTPUT);
            pinMode(winPin1,OUTPUT);
     
            pinMode(dunwinPin1 ,OUTPUT);
            pinMode(dunwinPin2 ,OUTPUT);
            pinMode(dwinPin2 ,OUTPUT);
            pinMode(dwinPin1 ,OUTPUT);

             digitalWrite(dunwinPin1 , HIGH);
            digitalWrite(dunwinPin1 , HIGH);
            digitalWrite(dwinPin1 , LOW);
            digitalWrite(dwinPin1 , LOW);

           if( tu1 > tw1)
            {
              for( int j=0 ; j < tu1-tw1 ; j++)
               {
                 if(j < tu2) {digitalWrite(unwinPin2, HIGH);}
                  digitalWrite(unwinPin1, HIGH);
                  
               delayMicroseconds(500);
               
               if(j < tu2) {digitalWrite(unwinPin2, LOW);}
                digitalWrite(unwinPin1, LOW);
                  
               delayMicroseconds(500);
               }

              for( int k=0 ; k < tw1 ; k++)
                {
                  if(k < tu2 - tu1 + tw1) {digitalWrite(unwinPin2, HIGH);}
                    digitalWrite(unwinPin1, HIGH);
                     if( k< tw2) { digitalWrite(winPin2 , HIGH) ;}
                      digitalWrite(winPin1, HIGH);
                      
                  
                  delayMicroseconds(500);
               
               if(k < tu2 - tu1 + tw1) {digitalWrite(unwinPin2, LOW);}
                digitalWrite(unwinPin1, LOW);
                 if( k< tw2) { digitalWrite(winPin2 , LOW) ;}
                      digitalWrite(winPin1, LOW);
                  
                  delayMicroseconds(500);
                }
            }

           if ( tu1 <= tw1) 
            {
              for(int m=0 ; m < tw1 ; m++)
               {
                digitalWrite ( unwinPin1 , HIGH );
                 digitalWrite ( unwinPin2 , HIGH);
                  if( m >= tw1-tw2 ) { digitalWrite (winPin2 , HIGH) ;}
                   digitalWrite ( winPin1 , HIGH );

                  delayMicroseconds(500);

                 digitalWrite ( unwinPin1 , LOW );
                  digitalWrite ( unwinPin2 , LOW);
                   if( m >= tw1-tw2 ) { digitalWrite (winPin2 , LOW) ;}
                     digitalWrite ( winPin1 , LOW );

                   delayMicroseconds(500);

                 
               }
            }

     }

      break;
  
  // put your setup code here, to run once:
}
}

void loop() 
  {
 
    
  // put your main code here, to run repeatedly:

}



