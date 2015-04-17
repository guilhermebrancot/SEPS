#include "dsk6713_aic23.h"		//codec-DSK support file
#include "C6713dskinit.h"

Uint32 fs=DSK6713_AIC23_FREQ_16KHZ;	//set sampling rate

short rampa=0;
short delta;
int index=0;
short LUTresultados=0;
short x[32];
short LUT[32];
short Y1=0;
short Y2=0;
short Y;
short deltaX;
short ampl;
int aux;
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
  short 	inbuf;
  short LUT[32]={0,3212,6393, 9512, 12540, 15447, 18205, 20788, 
  23170, 25330, 27246, 28899, 30274, 31357, 32138, 32610, 32767, 
  32610, 32138, 31357, 30274, 28899, 27246, 25330, 23170, 20788, 
  18205, 15447, 12540, 9512, 6393, 3212};
  comm_intr();                   	//init DSK, codec, McBSP

  ampl=32766;		//0.5*32767
  delta=16384;
  inbuf=0;

  while(1){                	   	//infinite loop
    if(intflag != FALSE){
      intflag = FALSE;
      //deltamin=8192:
      //deltamax=24576;
      delta=16384-(inbuf>>2);
      rampa=rampa+delta;		//rampa
      deltaX=1023 & rampa;
      deltaX=deltaX<<5;			//isola deltaX
	  index=rampa>>10;
	  index=31 & index;			//isola indice
	  aux=ampl*LUT[index];
	  aux=aux<<1;				//elimina sinal replicado
      if(rampa<0){
    	  LUTresultados=-aux>>16;
      }else{
    	  LUTresultados=aux>>16;
      }
	  aux=ampl*LUT[index+1];	//calculo de Y2
	  aux=aux<<1;
      if(rampa<0){
    	  Y2=-aux>>16;
      }else{
    	  Y2=aux>>16;			// Y2=LUT(i+1) convertido para Q15
      }
      Y1=LUTresultados;			// Y1=LUT(i) em Q15
      aux=(Y2-Y1)*deltaX;		
	  aux=aux<<1;
	  Y=Y1+(aux>>16);		//interpolacao
      //-----------------------------------------------------------------
      inbuf = AIC_buffer.channel[LEFT];		// faz loop do canal esquerdo
      AIC_buffer.channel[LEFT] = -LUTresultados;
      //inbuf = AIC_buffer.channel[RIGHT];	// faz loop do canal direito
      AIC_buffer.channel[RIGHT] = -Y;

    }
  }
}
 
