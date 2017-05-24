//--------  This macro plots one adc channel on two windows
{

FrameH *frame;
FrFile *ifile;
FrAdcData *adc1;
FrVect *vect;
TH1F *histo;
 
file=FrFileINew("/space/data/e2/dss/H-657998000.DSS");

//------- read all frames and plot on adc

histo = NULL;
for(;(frame= FrameRead(file)) != NULL;frame= FrameFree(frame))
  {
   adc1 = FrAdcDataFind(frame, "H2:LSC-AS_Q");
   if(adc1 == NULL) break;
   if(histo != NULL) delete histo;
   histo = FrAP(adc1,"DRAW");
   
   gPad->Modified();
   gPad->Update();
}
}
