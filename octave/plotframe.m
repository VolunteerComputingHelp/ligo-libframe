% plotframe.m -- this is a example of how could be used loadframe.cc

% This file may be used under the terms defined by the GNU 
% General Public License [write to the Free Software Foundation, 
% 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA]
%
% WITHOUT ANY WARRANTY; without even the implied warranty 
% of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
%
% Author: Eric Chassande-Mottin [ecm@obs-nice.fr], Sun Oct 14 23:10:52 CEST 2001
%

% -- clean context

clear

% -- define frame file name and channel name

pathtofile=[getenv("FRROOT") '/data/'];
filename = 'test.dat';
file=[pathtofile filename];
channel = 'Adc0';

nframes=6;

% -- load the date

[x,fs,valid,t0,timegps,unit] = loadadc(file,channel,nframes);

N=2^(nextpow2(length(x))-1);
x=x(1:N);
xf = fft(x);

% -- make plot

dt=1;
index=1:dt:N;

t=index/fs;

subplot(2,1,1)
plot(t,x(index),'b');
ylabel([channel ' [' unit ']']);
xlabel('time [s]');
title([timegps ' [' num2str(N/fs) ' sec long]']);
grid

df=2;
index=2:df:N/2;

f=fs*(index-1)/N;

subplot(2,1,2)
loglog(f,abs(xf(index))*sqrt(fs/N),'b');
ylabel(['amplitude [' unit '.Hz^{1/2}]']);
xlabel('frequency [Hz]')
title(['spectrum of ' channel])
grid

