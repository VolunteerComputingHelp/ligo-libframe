//--------  This macro plots 3 channels in one window
{

 FrFile *file = FrFileINew("../data/test.gwf");

 FrVect *vect1 = FrFileIGetV(file, "fastAdc1", 0. ,3.);
 FrVect *vect2 = FrFileIGetV(file, "Adc2", 0. ,3.);
 FrVect *vect3 = FrFileIGetV(file, "Adc3", 0. ,3.);
 
 FrHisto1 = FrVP(vect1, vect2, .02, vect3, .003);
}
