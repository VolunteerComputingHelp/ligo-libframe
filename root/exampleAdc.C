//--------  This macro plots one adc channel on two windows
{

FrameH *frame;
FrFile *ifile;
FrAdcData *adc1;
FrVect *vect;
TH1F *histo;
 
can = new TCanvas("can","RUN",700,600);
can.Divide(1,2);

//-------open the file

file=FrFileINew("../data/test.gwf");

//------ extract a 5 seconds length vector starting 2s after the first frame

double tstart = 2.+ FrFileITStart(file);;
double len    = 5.; 

vect = FrFileIGetV(file,  "fastAdc1", tstart, len);
FrVectDump(vect, stdout, 2);

//------ plot this vector

can.cd(1);
histo = FrVP(vect,"DRAW");   

//------ close/reopen the file

FrFileIEnd(file);
file=FrFileINew("../data/test.gwf");

//------- read all frames and plot the one second frame

histo = NULL;
for(;(frame= FrameRead(file)) != NULL;frame= FrameFree(frame))
  {
   adc1 = FrAdcDataFind(frame, "fastAdc1");
   FrAdcDataDump(adc1,stdout,2);
   if(adc1 == NULL) break;
   can.cd(2);
   if(histo != NULL) delete histo;
   histo = FrAP(adc1,"DRAW");
   printf(" double return\m");
   can->Modified();
   can->Update();
}
}
