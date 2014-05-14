function pause2(seconds)
% can pause with a resolution around 1ms ~10x faster than pause
tic;
while toc < seconds
end
end
