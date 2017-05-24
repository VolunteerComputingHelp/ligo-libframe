//---------- This macro dump in ASCII a few adc channels
{

struct FrameH *frame;
struct FrFile *ifile;
struct FrAdcData *adc;
struct FrVect *vect; 
double dt, t;
int i;

ifile=FrFileINew("../data/test.gwf");

for(;(frame= FrameRead(ifile)) != NULL;frame= FrameFree(frame))
  {
    adc  = FrAdcDataFind(frame, "fastAdc1");
    vect = adc->data;
    dt = vect->dx[0];

    for(i=0; i< vect->nData; i++)
      {t = i*dt+frame->GTimeS;
       if(i%512 == 0) printf(" [GPS:%.4f s] %8d\n",t,vect->dataS[i]);}
  }
}
