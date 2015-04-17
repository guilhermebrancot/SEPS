#include "dsk6713_aic23.h"		//codec-DSK support file
#include "C6713dskinit.h"

Uint32 fs=DSK6713_AIC23_FREQ_16KHZ;	//set sampling rate

short cont= 1;
short cont2= 0;
short bn= 1;
short cn= 0;
short dn = -32767;
short sin[4]={0,32767,0, -32767};	//portadora
short mod = 0;
short sn = 0;
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
  comm_intr();                   	//init DSK, codec, McBSP

  while(1){                	   	//infinite loop
    if(intflag != FALSE){
      intflag = FALSE;
      if(cont==16){			//condicao para criacao de novo bit
    	  cn=bn^cn;			//codificacao de bn
    	  bn=bn^1;			//calculo de novo bit
    	  cont=0;			//reset de contador
    	dn=32767*((cn<<1)-1);	//mapeamento do bit codificado
      }
      cont=cont+1;

      mod = sin[cont2];			//sinal da portadora
      cont2= cont2+1;
      sn=((dn*mod)<<1)>>16;     //calculo do sinal modulado
       cont2=cont2&3;

      //LUT[bn]= AIC_buffer.channel[LEFT];	// faz loop do canal esquerdo
      AIC_buffer.channel[LEFT] =dn;
      //inbuf = AIC_buffer.channel[RIGHT];	// faz loop do canal direito
      AIC_buffer.channel[RIGHT] = sn;
    }
  }
}