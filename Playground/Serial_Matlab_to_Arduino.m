%{
function [outSig,outThresh] = Serial_Matlab_to_Arduino(SNR,maxdata)
outSig = zeros(1,maxdata);
outThresh = zeros(1,maxdata);
%}

%% Things to consider
% 1. Do I need to provide a better seed for the random number generator?

%%SerialCheckFile; % First check if the serial communication is good
clear realTimeInput; %clear all persistent variables
clear plotATDSingle;
clear plotATDN;

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
Real_nPoint_File = menu('Input System','RealTime','NPoint','Testing','Changing Input'); % 0 = realTime input, 1 = nPoint generation, 2 = input from File,
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

%ports = scanComPorts; %for windows
%sPort = serial(ports{1}, 'baudrate', 115200, 'terminator', []); % for windows
ports = '/dev/cu.usbmodem1451';
sPort = serial(ports, 'baudrate', 115200, 'terminator', [],'Timeout',5);
set(sPort,'OutputBufferSize',620*4)
set(sPort,'InputBufferSize',620*16) %4 b/c float32 %8 b/c output
%pause(2);
fopen(sPort);
pause(2); %wait time for serial port to open


%% Send the parameters serially in the following order

% fwrite(sPort,ADres,'uint8');
% fwrite(sPort,nCells,'uint32');
% fwrite(sPort,cut,'uint32');
% fwrite(sPort,gaurd,'uint32');
% fwrite(sPort,Pfa,'float32');


%% Set up Stop Button
stopbutton = uicontrol('Style', 'Text',...
              'String', 'CLOSE THIS WINDOW TO STOP', ...
              'Position',[20,20,80,40],...
              'Callback', 'delete(gcbo)');
set(gcf,'position',[200,800,116,80])
counter = 0; %for debugging

%% Send Data
if Real_nPoint_File == 0
    disp('exited');
elseif Real_nPoint_File == 1;
    disp('Real Time Input');
elseif Real_nPoint_File == 2
    disp('N Points at a time');
elseif Real_nPoint_File == 3
    disp('Testing Input');
end

%This is for CHanging Input
transitionPeriod = 100;
transitionCounter = 0;

while ishandle(stopbutton)
    if Real_nPoint_File == 0
        % Do nothing
        break;
    elseif Real_nPoint_File == 1 % From Real-time
        % From Real-Time
        noise_in = @(n) normrnd(mu*ones(1,n),sig*ones(1,n));
        noise_quad = @(n) normrnd(mu*ones(1,n),sig*ones(1,n));
        [data, sigAmp] = realTimeInput(noise_in,noise_quad,SNR,PW,T);
        [data, resString] = ADconversion(data,ADres,sigAmp);
        fwrite(sPort,data,resString);
        threshold = fread(sPort,1,'float32');
        found = fread(sPort,1,'uint8');
        plotATDSingle(100,1,data,found,threshold,cut,false,50)
        
        %for debugging
        counter = counter + 1;
        xlabel(counter);
    elseif Real_nPoint_File == 2 % N-points at a time
        nPoint = nRangeCells + cut+380;
        % Change output buffer size
        
        fwrite(sPort,nPoint,'uint16');
        disp(fread(sPort,1,'uint16'));
        thresholds = zeros(1,nPoint); 
        outputFound = zeros(1,nPoint); 
        noise_in = @(n) normrnd(mu*ones(1,n),sig*ones(1,n));
        noise_quad = @(n) normrnd(mu*ones(1,n),sig*ones(1,n));
        [data, sigAmp] = nPointInput(noise_in,noise_quad,SNR,PW,T,nPoint);
        [data, resString] = ADconversion(data,ADres,sigAmp);
        
        for d = 1:nPoint
            fwrite(sPort,data(d),resString);
        end
        tic;
        disp('Finished sending data')
        %fwrite(sPort,data,resString);
        for t = 1:nPoint
            thresholds(t) = fread(sPort,1,'float32');
            if t == 1
                toc;
            end
        end
        disp('Finished reading threshold')
        %thresholds = fread(sPort,nPoint,'float32');
        for o = 1:nPoint
            outputFound(o) = fread(sPort,1,'uint8');
        end
        %outputFound = fread(sPort,nPoint,'uint8');
        for i = 1:nPoint-1
            plotATDSingle(nPoint,PW,data(i),outputFound(i),thresholds(i),cut,false,50)
        end
        plotATDSingle(nPoint,PW,data(i),outputFound(i),thresholds(i),cut,false,50)
        %plotATDN(nPoint,PW,data,outputFound,thresholds,20);
        
        disp('press a key');
        pause;
        disp('continuing');
    elseif Real_nPoint_File == 3 % Testing
        noise_in = @(n) normrnd(mu*ones(1,n),sig*ones(1,n));
        noise_quad = @(n) normrnd(mu*ones(1,n),sig*ones(1,n));
        [data, sigAmp] = realTimeInput(noise_in,noise_quad,SNR,PW,T);
        [data, resString] = ADconversion(data,ADres,sigAmp);
        fwrite(sPort,data,resString);
        threshold = fread(sPort,1,'float32');
        found = fread(sPort,1,'uint8');
%         if mod(counter,90) == 0
%             plotATDSingle(100,PW,data,found,threshold,cut);
%         else
%             plotATDSingle(100,PW,data,found,threshold,cut,true);
%         end
        %plotATDSingle(100,PW,data,found,threshold,cut,true);
%         [currentsig,currentthr] = plotATDSingle(100,PW,data,found,threshold,cut,true);
        plotATDSingle(100,PW,data,found,threshold,cut);
        %for debugging
        counter = counter + 1;
        xlabel(counter);
        
        if counter == trialindex
            if found
                success = success + 1;
            end
            trialindex = trialindex + indexdelta;
            thistrial = thistrial + 1;
            if thistrial > trials
                break;
            end
        end
        
%         outsig(counter) = currentsig;
%         outThresh(counter) = currentthr;
    elseif Real_nPoint_File == 4 % Changing Input
        % From Real-Time
        SNR = 16;
        if transitionCounter > transitionPeriod
            switch sig
                case 1
                    sig = 5;
                case 5
                    sig = 9;
                case 9
                    sig = 6;
                case 6
                    sig = 3;
                case 3
                    sig = 1;
                otherwise
                    sig = 1;
            end
            transitionCounter = 0;
        end
        transitionCounter = transitionCounter + 1;
                    
        noise_in = @(n) normrnd(mu*ones(1,n),sig*ones(1,n));
        noise_quad = @(n) normrnd(mu*ones(1,n),sig*ones(1,n));
        
        A = sqrt(sig^2*10^(SNR/10));
        
        if floor(transitionPeriod/2) == transitionCounter
            data = sqrt((noise_in(1)+A)^2+noise_quad(1)^2);
        else
            data = sqrt(noise_in(1)^2+noise_quad(1)^2);
        end
        
        if sqrt(sig^2*10^(SNR/10)) > 256
            error('SNR is too high or sig is too high');
        else
            data = data*255/sqrt(9^2*10^(SNR/10));
        end
        
        %[data, resString] = ADconversion(data,ADres,sigAmp);
        fwrite(sPort,data,sprintf('uint%d',ADres));
        threshold = fread(sPort,1,'float32');
        found = fread(sPort,1,'uint8');
        
        if mod(counter,3) == 0
            plotATDSingle(500,1,data,found,threshold,cut,false,50)
        else
            plotATDSingle(500,1,data,found,threshold,cut,true,50)
        end
        
        %for debugging
        counter = counter + 1;
        xlabel(counter);
    end
    pause2(tStep);
end

%fprintf('SNR = %f, Pfa = %f\n',SNR,Pfa);
%fprintf('Trials: %d\nSuccesses: %d\nProbability of detection: %f\n',trials,success,success/trials);


fclose(sPort);
delete(sPort); 
clear all;

