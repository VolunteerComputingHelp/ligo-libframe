function [data,varargout] = frextract(fname,cname,varargin)
% FREXTRACT - Read time-series data from a frame file
%
% [data,tsamp,fsamp,gps0,strStart,unitX,unitY,frInfo] = ...
%    frextract(fname,cname,frameStart,nFrames,dbgLvl)
%
% The input arguments are:
%   1) file name
%   2) ADC or PROC name
%   3) (optional) first frame (NOT GPS) (default=0 first frame in the file)
%   4) (optional) number of frames to read (default = 1 Frame)
%   5) (optional) debug level (default: 0 for no output)
%
% Returned matlab data:                                               
%   1) ADC or PROC data (time series)                                  
%   2) (optional) x axis values relative to the first data point.     
%            This is usual time but it could be frequency in the case 
%            of a frequency series (double)                            
%   3) (optional) frequency values in the case of time series (double)
%   4) (optional) GPS starting time (in second.nanosec)               
%   5) (optional) starting time as a string                           
%   6) (optional) ADC comment as a string                             
%   7) (optional) ADC unit as a string                                
%   8) (optional) additional info: it is a 9 words vector which       
%       content:crate #, channel#, nBits#, biais, slope,              
%               sampleRate, timeOffset(S.N), fShift, overRange        
%      All this values are stored as double                           
%
%  Error types:
%   frextract:nameLength       -- file or channel name too long

%#mex
%#external
