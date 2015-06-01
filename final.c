//Loop_intr.c Loop program using interrupt. Output = delayed input
// Testado na placa n. 2 - 05/10-A809

#include "dsk6713_aic23.h"		//codec-DSK support file
#include "C6713dskinit.h"

Uint32 fs=DSK6713_AIC23_FREQ_16KHZ;	//set sampling rate

short rampa=0;
short cont=1;
short count=1;
short delta;
int index=0;
short seno=0;
short coseno=0;
short x[32];
short y1=0;
short y2=0;
short y3=0;
short deltaX;
short ampl;
int aux;
short inbuf=0;
short bn= 1;
short cn= 0;
short dn = -32767;
short en=0;
short aux1=0;
short aux2=0;
short aux3=0;
short aux4=0;
short mod = 0;
short demod = 0;
short erro = 0;
short s = 0;
short s1_0 = 0;
short s1_1 = 0;
short s2_0 = 0;
short s2_1 = 0;
short sn = 0;
short y4=0;
short cont2= 0;
short port[4]={0,32767,0, -32767};
unsigned char scr=0;

char	intflag = FALSE;
union {Uint32 samples; short channel[2];} AIC_buffer;

interrupt void c_int11()         	//interrupt service routine
{
   output_sample(AIC_buffer.samples);   	//output data
   AIC_buffer.samples= input_sample(); 	    //input data
   intflag = TRUE;
   return;
}

void main()
{
	//short inbuf=0;
	  short LUT[32]={0,3212,6393, 9512, 12540, 15447, 18205, 20788,
       23170, 25330, 27246, 28899, 30274, 31357, 32138, 32610,
       32767, 32610, 32138, 31357, 30274, 28899, 27246, 25330,
       23170, 20788, 18205, 15447, 12540, 9512, 6393, 3212};
	  comm_intr();                   	//init DSK, codec, McBSP

	  ampl=32766;		//0.5*32767
	  delta=16384;

	  while(1){                	   	//infinite loop
	    if(intflag != FALSE){
	      intflag = FALSE;

    //bpsk modem com scrambler
	      if(cont==16){
          //scrambler
	         en=((scr & 1)^((scr & 2)>>1))^bn;
	         scr=(scr>>1)|(en<<7);

	         cn=en^cn; //codificador
	         bn=bn^1;  //novo bit
	      	 cont=0;
	         dn=32767*((cn<<1)-1); //mapeador
	      }
//------------------------------------------------------
//transient
	      if(count==4000){
	    	  erro=32767;
	    	  count=1;
	      }
	      count=count+1;
//------------------------------------------------------
	      cont=cont+1;
//modulacao do sinal
	      mod = port[cont2];
	      cont2= cont2+1;
	      sn=((dn*mod)<<1)>>16;    //Q15 virgula fixa
	      cont2=cont2&3;

        //deltamin=8192:
        //deltamax=24576;
	      delta=16384+(erro>>2);  //controlo de frequencia
	      //delta=8192;
	      rampa=rampa+delta;
//indexacao da LUT
		  index=rampa>>10;
		  index=31 & index;
//seno
		  aux=ampl*LUT[index];
		  aux=aux<<1;
	      if(rampa<0){
	    	  seno=-aux>>16;
	      }else{
	    	  seno=aux>>16;
	      }
//coseno
	      if(rampa<0){
	    	  //zona 1
	    	  if(index<=15){
	    		  aux=ampl*(-LUT[(index+16) & 31]);
	    	  	aux=aux<<1;
	    	  }else{
	    	  //zona 2
	    		  aux=ampl*(LUT[(index-16) & 31]);
	    	  	aux=(aux<<1);
	    	  }
	    	  coseno=aux>>16;
	      }else{
	    	  if(index<=15){
	    		  //zona 3
	    		  aux=ampl*LUT[(index+16) & 31];aux=aux<<1;
	    	  }else{
	    		  //zona 4
	    		  aux=-ampl*LUT[(index-16) & 31];aux=aux<<1;
	    	  }
	    	  coseno=aux>>16;
	      }

//demodulating loop
	     	//s1=((sn*coseno)<<1)>>16;  //sem zero, beta = 0
        //s2=((sn*seno)<<1)>>16;

       //beta=-1:
 	  	  	s1_1=s1_0;
	      	s1_0=((sn*coseno)<<1)>>16;

	      	s2_1=s2_0;
	      	s2_0=((sn*seno)<<1)>>16;

//-----------------------------------------------------------------
//filtros
 //data filters
  //data filter 1 (com coseno)
   y1= (((23528*y1)<<1)>>16)+(((4620*s1_0)<<1)>>16)+(((4620*s1_1)<<1)>>16);
  //data filter 2 (com seno)
   y2=(((23528*y2)<<1)>>16)+(((4620*s2_0)<<1)>>16)+(((4620*s2_1)<<1)>>16);

  s=((y1*y2)<<1)>>16; //entrada do loop filter

	//upper arm
	 //s=y1;

	//loop filter
	  erro = (((32639*erro)<<1)>>16)+(((128*s)<<1)>>16); //10Hz
	  // erro= (((31529*erro)<<1)>>16)+(((1238*s)<<1)>>16);//100Hz

	demod=y1;

//-----------------------------------------------------------------
// Processamento especifico
//-----------------------------------------------------------------
  inbuf = AIC_buffer.channel[LEFT];
	AIC_buffer.channel[LEFT] =erro;
	//inbuf = AIC_buffer.channel[RIGHT];
	AIC_buffer.channel[RIGHT] = demod;
	    }
	  }
	}
