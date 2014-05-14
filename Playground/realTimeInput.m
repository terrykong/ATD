function [result, sigAmp] = realTimeInput(I,Q,SNR,PW,IPP)
% This function will take two anonymous functions w/ no input
%   representing an inphase and quadrature component of the noise. 
%   It will space the target pulses IPP apart where each pulse has
%   width PW.
%
% Input:
%   - I: Inphase anonymous function 
%       [I(n) yields n inphase noise points]
%   - Q: Quadrature anonymous function
%       [Q(n) yields n quadrature noise points]
%   - SNR: in dB
%   - PW: pulse width
%   - IPP: interpulse period

persistent current_time;
persistent A; %signal amplitude

if isempty(current_time)
    current_time = PW;
end

if isempty(A)
    n = 1000000;
    noise_amp = sqrt(I(n).^2 + Q(n).^2);
    noise_power = mean(noise_amp.^2);
    sig_power = noise_power*10^(SNR/10);
    %A = sqrt(sig_power*IPP/PW);
    A = sqrt(sig_power);
end

if current_time > IPP/11
    result = sqrt((I(1)+A)^2+Q(1)^2);
    current_time = 0;
else
    result = sqrt(I(1)^2+Q(1)^2);
    current_time = current_time + PW;
end

sigAmp = A;
