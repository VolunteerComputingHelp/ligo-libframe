% test of saveproc.oct
% to run the test: octave saveproc_test.m
clear
x=randn(2048,1);
saveproc('test.dat','signal',x,1024);
y=loadproc('test.dat','signal',2);
m=max(abs(x-y));
if (m>0), 
 printf("error in saveadc: data have not been written/read properly\n");
else
 printf("saveadc: write/read test OK\n");
end;
