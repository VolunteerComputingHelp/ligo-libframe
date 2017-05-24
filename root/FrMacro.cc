TH1F* FrHisto1=NULL;
TH1F* FrHisto2=NULL;
TH1F* FrHisto3=NULL;
TH1F* FrHisto4=NULL;
TLegend* FrLegm = NULL;
int FrrScale = 0;
double FrrMax = 0;
double FrrMin = 0;
//-------------------------------------------------------
//                 FrMacro.cc  by B. Mours Aug 12, 2002
//-------------------------- Create a histogram for vector
 TH1F* FrVP(FrVect *vect, 
             char *draw = NULL, 
             int color = 1,
             double xStart = 0.,
             double xEnd = 0.,
             double scale = 1.)
//---------------------------- if draw = 0; do not draw the vector 
{double min, newmin, max, x, value;
 unsigned int i;
 char title[256];

 if(vect == NULL) return(NULL);

 if(vect->GTime == 0)
    {sprintf(title,"%s",vect->name);}
 else
    {sprintf(title,"Time origin: GPS=%.6f UTC=%s  ",
		vect->GTime, FrStrUTC(vect->GTime, 0));}

 if(xEnd > xStart) 
   {int iStart = (xStart-vect->startX[0])/vect->dx[0];
    int nBin   = (xEnd - xStart)/vect->dx[0] + 1;
    xStart = iStart * vect->dx[0]+vect->startX[0];}
 else
   {int iStart = 0;
    int nBin = vect->nData;
    xStart = vect->startX[0];}

 FrHisto1 = new TH1F(vect->name, title,nBin, xStart, xStart+nBin*vect->dx[0]);

 if(color < 0) 
    {FrHisto1->SetLineWidth(3);
     color = - color;}

 FrHisto1->SetStats(0);
 FrHisto1->SetLineColor(color);

 min =  1.e+100;
 max = -1.e+100;

 if(vect->type == FR_VECT_2S)
       {for(i=iStart; i<iStart+nBin; i++)  
          {value = scale*vect->dataS[i];
           if(value > max) max = value;
           if(value < min) min = value;
           x = (i+.5)*vect->dx[0]+vect->startX[0];
          FrHisto1->Fill(x,value);}}

 else if(vect->type == FR_VECT_4S)
       {for(i=iStart; i<iStart+nBin; i++)  
          {value = scale*vect->dataI[i];
           if(value > max) max = value;
           if(value < min) min = value;
           x = (i+.5)*vect->dx[0]+vect->startX[0];
          FrHisto1->Fill(x,value);}}

 else if(vect->type == FR_VECT_8R)
       {for(i=iStart; i<iStart+nBin; i++)  
          {value = scale*vect->dataD[i];
           if(value > max) max = value;
           if(value < min) min = value;
           x = (i+.5)*vect->dx[0]+vect->startX[0];
          FrHisto1->Fill(x,value);}}

 else if(vect->type == FR_VECT_4R)
       {for(i=iStart; i<iStart+nBin; i++)  
          {value = scale*vect->dataF[i];
           if(value > max) max = value;
           if(value < min) min = value;
           x = (i+.5)*vect->dx[0]+vect->startX[0];
          FrHisto1->Fill(x,value);}}

 else if(vect->type == FR_VECT_2U)
       {for(i=iStart; i<iStart+nBin; i++)  
          {value = scale*vect->dataUS[i];
           if(value > max) max = value;
           if(value < min) min = value;
           x = (i+.5)*vect->dx[0]+vect->startX[0];
          FrHisto1->Fill(x,value);}}

 else{printf(" FrVP: type not supported (%d)%s\n",vect->type,vect->name);}

 if(FrrScale != 0)
   {min = FrrMin;
    max = FrrMax;}

 newmin = min-.05*(max-min);
 if(min > 0. && newmin < 0.) newmin = min;
 FrHisto1->SetMinimum(newmin);
 FrHisto1->SetMaximum(max+.05*(max-min));

 FrHisto1->SetXTitle(vect->unitX[0]);
 FrHisto1->SetYTitle(vect->unitY);

 //if(xEnd > xStart) FrHisto1->SetAxisRange(xStart, xEnd, "X");

 if(draw == 0) return(FrHisto1);

 if(draw != NULL)
   {if(strcmp(draw,"DRAW") == 0) 
        {FrHisto1->Draw();
	 FrLegm = new TLegend(.7,.92,.98,.98);
	 FrLegm->AddEntry(FrHisto1,vect->name,"L");
         FrLegm->Draw();}
    else{FrHisto1->Draw(draw);
         if(scale == 1) 
              sprintf(title,"%s",vect->name);
         else sprintf(title,"%s x %.3e",vect->name,scale);
         FrLegm->AddEntry(FrHisto1,title,"L");
         FrLegm->SetY1(FrLegm->GetY1()-.04);}}

 return(FrHisto1);}
//-------------------------- Create a histogram for a set of channel
 TH1F* FrVP(FrVect *vect1,
            FrVect *vect2 = NULL,
            double scale2 = 1.,
            FrVect *vect3 = NULL,
            double scale3 = 1.,
            FrVect *vect4 = NULL,
            double scale4 = 1.)
//---------------------------- if draw = 0; do not draw the vector 
{char title[256];
 double min, max;

//------ get min and max

 FrrScale = 1;

 FrVectMinMax(vect1, &FrrMin, &FrrMax);
 if(vect2 != NULL)
   {FrVectMinMax(vect2, &min, &max);
    min = min*scale2;
    max = max*scale2;
    if(min < FrrMin) FrrMin = min;
    if(max > FrrMax) FrrMax = max;}
 if(vect3 != NULL)
   {FrVectMinMax(vect3, &min, &max);
    min = min*scale3;
    max = max*scale3;
    if(min < FrrMin) FrrMin = min;
    if(max > FrrMax) FrrMax = max;}
 if(vect4 != NULL)
   {FrVectMinMax(vect4, &min, &max);
    min = min*scale4;
    max = max*scale4;
    if(min < FrrMin) FrrMin = min;
    if(max > FrrMax) FrrMax = max;}

 
 FrHisto1 = FrVP(vect1, "DRAW",1,0.,0.,1.); 

 if(vect2 != NULL) FrHisto2 = FrVP(vect2, "SAME",2,0.,0.,scale2); 
 if(vect3 != NULL) FrHisto3 = FrVP(vect3, "SAME",4,0.,0.,scale3); 
 if(vect4 != NULL) FrHisto4 = FrVP(vect4, "SAME",28,0.,0.,scale4); 

 FrLegm->Draw();

 FrrScale = 0;

 return(FrHisto1);}

//-------------------------- Create a histogram for ADC channel
TH1F* FrAP(FrAdcData *myadc, char *draw = NULL) 
//-------------------------------------------------------------
{FrVect *vect;

 printf(" Plot Adc %s\n",myadc->name);

 if(myadc == NULL) return(NULL);
 vect = myadc->data;
 if(vect == NULL) return(NULL);

     // check that the ADC vector has the right name

 if(vect->name == NULL)
    {FrStrCpy(&vect->name, myadc->name);}
 else if(strcmp(vect->name,myadc->name) != 0)
     {free(vect->name);
      FrStrCpy(&vect->name, myadc->name);}

    // here we check the vector units

 if(vect->unitX[0] == NULL) FrStrCpy(&vect->unitX[0],"time [s]");
 if(vect->unitY    == NULL) FrStrCpy(&vect->unitY,"ADC count [s]");

   // create the histo for the vector

 FrVP(vect, draw);

 return(FrHisto1);}

//-------------------------- Create a histogram for a set of channel
 TH1F* FrCP(char *fileName, 
            double tStart = 0.,
            double len = 2.,
            char *channel1,
            char *channel2 = NULL,
            double scale2 = 1.,
            char *channel3 = NULL,
            double scale3 = 1.,
            char *channel4 = NULL,
            double scale4 = 1.)
//-------------------------------------------------------------------- 
{
 FrVect *vect1=NULL, *vect2=NULL, *vect3=NULL, *vect4=NULL;

 FrFile *file = FrFileINew(fileName);

 vect1 = FrFileIGetV(file, channel1, tStart, len);

 if(channel2 != NULL) vect2 = FrFileIGetV(file, channel2, tStart, len);
 if(channel3 != NULL) vect3 = FrFileIGetV(file, channel3, tStart, len);
 if(channel4 != NULL) vect4 = FrFileIGetV(file, channel4, tStart, len);
 
 FrHisto1 = FrVP(vect1, vect2, scale2, vect3, scale3, vect4, scale4);

 FrVectFree(vect1);
 FrVectFree(vect2);
 FrVectFree(vect3);
 FrVectFree(vect4);

 FrFileIClose(file);

 return(FrHisto1);}

//-------------------------- Create a histogram for a set of channels
 TH1F* FrCP(char *fileName, 
            char *channel1,
            char *channel2 = NULL,
            double scale2 = 1.,
            char *channel3 = NULL,
            double scale3 = 1.,
            char *channel4 = NULL,
            double scale4 = 1.)
//---------------------------- if draw = 0; do not draw the vector 
{ 
 double tStart = 0.;
 double len = 2.;
 
 return(FrCP(fileName, tStart, len, channel1, 
                                    channel2, scale2, 
                                    channel3, scale3, 
                                    channel4, scale4));}
