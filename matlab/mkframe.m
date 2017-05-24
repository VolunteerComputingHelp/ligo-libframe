function mkframe(file, data, mode, varargin)
% MKFRAME - write frame files (according with LIGO standards)
%   mkframe(file, data, mode[,frameLength,gpsStart,parameters,comment])
%
% file = frame file name string
% data = array of structures of data to be written to frame file
% mode   = 'n' to write new frame file
%          'a' to add to existing frame file
% frameLength = [optional] length of frame in seconds (default = 16 sec)
% gpsStart = [optional] GPS time of frame (default = current time)
% parameters = [optional] a structure of parameters and desired channel name
%					(default = NULL)
% comment = [optional] Comment to be added to the frame (default = NULL)
%
% Each element of the data array must be structures with these fields
%
% data.channel = channel name string
% data.data    = time-series data
% data.type    = type of data ('s','us','i','ui','l','ul','d','f', or 'dc')
% data.mode    = mode( 'a' for ADC, 's' for static )
%
% The parameters input must be a structure with the fields
%
% parameters.name = parameters channel name string
% parameters.parameters = 2-D array of parameter names and values
%
%  Error message IDs
%   mkframe:badInput - error in input parameters
%   mkframe:badField - error in data array
%   mkframe:frameFail - error in reading/writing frame file
%
