
N=16384;
nFrameFile=256;
nData=N/nFrameFile;

nFrame=4;
first=16;

x=randn(N,1);

% -- check saveadc/loadadc
saveadc("test.dat","adc0",x,nData);
y=loadadc("test.dat","adc0",nFrame,first);
idx=(first*nData)+(1:(nFrame*nData));
c=max(abs(x(idx)-y));
if (c>0),
  error('save/loadadc: saved and loaded signals are NOT identical');
else
  printf('save/loadadc: test completed: OK\n');
end;

% -- check loadframe
y=loadframe("test.dat","adc0",nFrame,first);
c=max(abs(x(idx)-y));
if (c>0),
  error('loadframe: saved and loaded signals are NOT identical');
else
  printf('loadframe: test completed: OK\n');
end;

% -- check saveproc/loadproc
saveproc("test.dat","proc0",x,nData);
y=loadproc("test.dat","proc0",nFrame,first);
c=max(abs(x(idx)-y));
if (c>0),
 error('save/loadproc: saved and loaded signals are NOT identical');
else
  printf('saveloedproc: test completed: OK\n');
end;
