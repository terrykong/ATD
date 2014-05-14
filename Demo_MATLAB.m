function Final_Demo(varargin)

clear all; clc; close all; format compact;

trials = 10;
SNRTest = 20;%12;%-10.37;
PfaTest = 3e-2;
nCellsTest = 40;

success = 0;
thistrial = 1;
trialindex = 602;
indexdelta = 583;

%-----------------------------------------------------------
%%%%%%%%%%%
%Constants%
%%%%%%%%%%%
nm = 1852; % 1 nautical mile = 1852 m
c = 299279458; % speed of light
%-----------------------------------------------------------
%%%%%%%%%%%%
%Parameters%
%%%%%%%%%%%%

% Simulation Parameters
tStep = 0.0001; % in seconds

% ATD Parameters
ADres = 8; % Resolution of A/D converter
nCells = nCellsTest;%40; % Number of windows
cut = floor(nCells/2); % Index of the cut cell
gaurd = 1; % Number of gaurd cells on one side (symmetric)
Pfa = PfaTest;%1e-2; % Probability of false alarm
    ADmask = ADres*2-1;
    percision = sprintf('uint%d',ADres);

% Presentation Parameters
windowFactor = 5; % Real-Time plot displays nCells*windowFactor points

% Real-Time Input Parameters
mu = 0; % Gaussian Noise Parameters N(mu,sig^2)
sig = 1; % ~
SNR = SNRTest;%-10.37; % in db
rangeMax = 50*nm; %50 nautical miles 
nRangeCells = 600;
T = 0.0006; % Interpulse period
    dR = rangeMax/nRangeCells; % range resolution
    PW = dR*2/(c); % pulse width

%-----------------------------------------------------------

if exist('sPort','var')
    delete(sPort);
end
if ~isempty(instrfind)
    delete(instrfind);
end

ports = '/dev/cu.usbmodem1411';
sPort = serial(ports, 'baudrate', 115200, 'terminator', [],'Timeout',5);
% Make buffers sufficiently large
set(sPort,'OutputBufferSize',620*4)
set(sPort,'InputBufferSize',620*16) 
%pause(2);
fopen(sPort);

% Create the GUI
S.fh = figure('units','pix',...
              'pos',[450 400 530 125],...
              'menubar','none',...              
              'name','GUI_11',...
              'numbertitle','off',... 
              'resize','off');
S.pb = uicontrol('string','',...
                 'callback',{},...
                 'units','pixels',...
                 'fontsize',55,...
                 'fontweight','bold',...
                 'position',[15,10,500,100]);
% This is the amount of time needed to setup
setupTime = 5;
for n = setupTime:-1:1
    set(S.pb,'string',sprintf('Beginning in %d...',n));
    pause(1);
end
set(S.pb,'callback',{@pb_call});
set(S.pb,'string','Stop Demo!','fontsize',11);  
set(S.fh,'pos',[0 880 120 50]);
set(S.pb,'position',[15,10,100,30]);

drawnow;

handshake;

nData = fread(sPort,1,'uint16');
fprintf('%d Data\n',nData);

maxDist = fread(sPort,1,'uint16');
fprintf('%d is the Max Dist in cm\n',maxDist);

while true
    constantThreshold = fread(sPort,1,'uint16');
    data = fread(sPort,nData,'uint16');
    thresholds = fread(sPort,nData,'float32');
    outputFound = fread(sPort,nData,'uint8');
    plotDemo(nData,data,outputFound,thresholds,cut,constantThreshold,maxDist/2.54)
    
    handshake;
    
    if ~ishandle(S.fh)  % Check if the figure exists.
        break;
    end
    drawnow;
end

fclose(sPort);
delete(sPort); 
clear all;

    % Helper Functions
    function [] = pb_call(varargin)
        % Callback for pushbutton
        delete(S.fh)  % Delete the figure.
    end

    function handshake
        for i = 1:5
            fwrite(sPort,255,'uint8'); %handshaking byte
        end
    end

end
