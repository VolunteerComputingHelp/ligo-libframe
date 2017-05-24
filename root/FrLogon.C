{
gSystem->Load("libFrameROOT.so");
gROOT->LoadMacro("FrMacro.cc");

//----- Print the version used --------

double v;
v = FrLibVersion(stdout);
printf(" This is FrLogon in Fr\n");
}
